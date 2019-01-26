#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

include!(concat!(env!("OUT_DIR"), "/scheduler_bindings.rs"));

// Semaphore stub
#[no_mangle]
pub extern fn sem_give(sem: &mut sem_t) {
    sem.count += 1;
}

#[test]
fn scheduler_test() {
    let mut scheduler = scheduler_t::default();
    let mut sem = sem_t::default();
    unsafe {
        scheduler_init(&mut scheduler);
        sem_init(&mut sem, 0, 1);
        scheduler_set_hourly(&mut scheduler, 0b0100_1111_1111_1111_0000_0000, &mut sem);
    }

    unsafe {
        scheduler_tick(&mut scheduler, 1);
        scheduler_tick(&mut scheduler, 3);
    }
    // 0th hour is excluded by mask
    assert_eq!(sem.count, 0);
    // 1st hour is included
    unsafe {
        scheduler_tick(&mut scheduler, 1548379208);
    }
    assert_eq!(sem.count, 1);
    // Should not run again in 1st hour
    unsafe {
        scheduler_tick(&mut scheduler, 1);
        scheduler_tick(&mut scheduler, 1548379209);
        scheduler_tick(&mut scheduler, 1548379210);
    }
    assert_eq!(sem.count, 1);
    // 4th hour
    unsafe {
        scheduler_tick(&mut scheduler, 1548379210 + (60 * 60 * 3));
    }
    assert_eq!(sem.count, 2);
}
