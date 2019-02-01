#![allow(dead_code)]
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

mod internal {
    include!(concat!(env!("OUT_DIR"), "/data_recorder_bindings.rs"));
}

use std::default::Default;
use std::mem;
use std::os::raw::c_void;
use std::time::{Duration, SystemTime, UNIX_EPOCH};

struct DataRecorder<T: Default> {
    data_recorder: internal::data_recorder_t,
    pub buffer: Vec<T>,
}

impl<T> DataRecorder<T>
where
    T: Default + Clone,
{
    pub fn new(num_samples: usize, sampling_period: Duration) -> Self {
        let mut data_recorder = internal::data_recorder_t::default();
        let mut buffer = vec![Default::default(); num_samples];

        unsafe {
            internal::data_recorder_init(
                &mut data_recorder,
                &mut buffer[..] as *mut _ as *mut c_void,
                mem::size_of::<T>(),
                num_samples,
                sampling_period.as_secs() as i32,
            );
        }

        DataRecorder {
            data_recorder: data_recorder,
            buffer: buffer,
        }
    }

    pub fn add_sample(&mut self, sample_time: SystemTime, sample: &T) -> Result<(), i32> {
        let result;
        unsafe {
            result = internal::data_recorder_add_sample(
                &mut self.data_recorder,
                sample_time.duration_since(UNIX_EPOCH).unwrap().as_secs() as i32,
                sample as *const T as *const c_void,
            );
        }
        if result == 0 {
            Ok(())
        } else {
            Err(result)
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_basics() {
        let mut data_recorder = DataRecorder::new(5, Duration::from_secs(15));
        let base_time = SystemTime::now();
        for i in 1..=15 {
            data_recorder
                .add_sample(base_time + Duration::from_secs(15 * i), &(i as f64 * 10.0))
                .unwrap();
        }
        assert_eq!(vec![150., 140., 130., 120., 110.], data_recorder.buffer);
    }
}
