#![allow(dead_code)]
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use std::boxed::Box;
use std::cell::RefCell;
use std::os::raw::c_void;
use std::rc::Rc;
use std::time::{SystemTime, UNIX_EPOCH};

mod internal {
    include!(concat!(env!("OUT_DIR"), "/scheduler_bindings.rs"));
}

// This is most stupid way to do tests: to make "Safe" API for tested
// code first, creating lots of wrapper code, but still why not?

pub struct Scheduler<'a> {
    scheduler: internal::scheduler_t,
    hourly_callback: Option<Callback<'a>>,
    minutely_callback: Option<Callback<'a>>,
}

struct Callback<'a> {
    magic: String,
    callback: Box<FnMut() + 'a>,
}

impl<'a> Callback<'a> {
    pub fn new(callback: impl FnMut() + 'a) -> Self {
        Callback {
            callback: Box::new(callback),
            magic: String::from("Yieee"),
        }
    }

    pub unsafe extern "C" fn external_callback(callback_arg: *mut c_void) {
        println!("callback arg: {:?}", callback_arg);
        let cb: &mut Callback = &mut *(callback_arg as *mut Callback);
        println!("Magic: {}", cb.magic);
        (cb.callback)();
    }

    pub fn to_external_callback_param(self: &mut Self) -> *mut c_void {
        println!(
            "setting callback: {:?}",
            (self as *mut Callback as *mut c_void)
        );
        self as *mut Callback as *mut c_void
    }
}

impl<'a> Scheduler<'a> {
    pub fn new() -> Self {
        let mut scheduler = internal::scheduler_t::default();
        unsafe { internal::scheduler_init(&mut scheduler) }
        Scheduler {
            scheduler: scheduler,
            hourly_callback: None,
            minutely_callback: None,
        }
    }

    pub fn set_hourly(self: &mut Self, hourly_mask: u32, callback: impl FnMut() + 'a) {
        assert!(hourly_mask <= 0b1111_1111_1111_1111_1111_1111);
        let cb = Callback::new(callback);
        self.hourly_callback = Some(cb);
        unsafe {
            internal::scheduler_set_hourly(
                &mut self.scheduler,
                hourly_mask,
                Some(Callback::external_callback),
                self.hourly_callback
                    .as_mut()
                    .unwrap()
                    .to_external_callback_param(),
            );
        }
    }

    pub fn set_minutely(self: &mut Self, period: i32, callback: impl FnMut() + 'a) {
        assert!(period > 0);
        assert!(period < 60);
        let cb = Callback::new(callback);
        self.minutely_callback = Some(cb);
        unsafe {
            internal::scheduler_set_minutely(
                &mut self.scheduler,
                period,
                Some(Callback::external_callback),
                self.minutely_callback
                    .as_mut()
                    .unwrap()
                    .to_external_callback_param(),
            );
        }
    }

    pub fn tick(self: &mut Self, time: SystemTime) {
        unsafe {
            internal::scheduler_tick(
                &mut self.scheduler,
                time.duration_since(UNIX_EPOCH).unwrap().as_secs() as i32,
            );
        }
    }
}

struct CallbackChecker {
    count: Rc<RefCell<u32>>,
}

impl CallbackChecker {
    pub fn new() -> Self {
        CallbackChecker {
            count: Rc::new(RefCell::new(0)),
        }
    }

    pub fn num_calls(&self, block: impl FnOnce()) -> u32 {
        let before = *self.count.borrow();
        block();
        *self.count.borrow() - before
    }

    pub fn callback(&mut self) -> impl FnMut() {
        let count2 = Rc::clone(&self.count);
        move || *count2.borrow_mut() += 1
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::time::{Duration, UNIX_EPOCH};

    #[test]
    fn hourly_test() {
        let mut scheduler = Scheduler::new();
        let mut callback_checker = CallbackChecker::new();
        // 2019-01-25 01:20:08
        let time_base = UNIX_EPOCH + Duration::from_secs(1548379208);

        scheduler.set_hourly(0b0100_1111_1111_1111_0000_0000, callback_checker.callback());

        // 0th hour is excluded by mask
        assert_eq!(
            0,
            callback_checker.num_calls(|| {
                scheduler.tick(UNIX_EPOCH + Duration::from_secs(1));
                scheduler.tick(UNIX_EPOCH + Duration::from_secs(3));
            })
        );
        // 1st hour is included
        assert_eq!(
            1,
            callback_checker.num_calls(|| {
                scheduler.tick(time_base);
            })
        );
        // Should not run again in 1st hour
        assert_eq!(
            0,
            callback_checker.num_calls(|| {
                scheduler.tick(UNIX_EPOCH + Duration::from_secs(1));
                scheduler.tick(time_base + Duration::from_secs(10));
                scheduler.tick(time_base + Duration::from_secs(60 * 50 + 3));
            })
        );
        // 4th hour
        assert_eq!(
            1,
            callback_checker.num_calls(|| {
                scheduler.tick(time_base + Duration::from_secs(60 * 60 * 3));
            })
        );
    }

    #[test]
    fn minutely_test() {
        let mut scheduler = Scheduler::new();
        let mut callback_checker = CallbackChecker::new();

        scheduler.set_minutely(5, callback_checker.callback());

        // 2017-12-31 21:00:00
        let time_base = UNIX_EPOCH + Duration::from_secs(1514754000);

        assert_eq!(
            0,
            callback_checker.num_calls(|| {
                scheduler.tick(time_base + Duration::from_secs(60 * 3));
            })
        );
        assert_eq!(
            1,
            callback_checker.num_calls(|| {
                scheduler.tick(time_base + Duration::from_secs(60 * 5 + 3));
            })
        );
        assert_eq!(
            0,
            callback_checker.num_calls(|| {
                scheduler.tick(time_base + Duration::from_secs(60 * 5 + 59));
            })
        );
        assert_eq!(
            1,
            callback_checker.num_calls(|| {
                scheduler.tick(time_base + Duration::from_secs(60 * 20 + 4));
            })
        );
        assert_eq!(
            0,
            callback_checker.num_calls(|| {
                scheduler.tick(time_base + Duration::from_secs(60 * 15 + 4));
            })
        );
    }
}
