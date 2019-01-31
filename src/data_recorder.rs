use std::time::{Duration, SystemTime};

struct DataRecorder<T> {
    data: Vec<T>,
    period: Duration,
    num_samples: u32,
    last_time: Option<SystemTime>,
}

impl<T> DataRecorder<T> {
    pub fn new(period: Duration, num_samples: u32) -> Self {
        DataRecorder {
            data: Vec::new(),
            period: period,
            num_samples: num_samples,
            last_time: None,
        }
    }

    pub fn add_measurement(&mut self, time: SystemTime, measurement: T) {
        let distance_from_previous: Duration =
            self.last_time.map_or(Duration::from_secs(0), |last_time| {
                time.duration_since(last_time).unwrap()
            });

        assert_eq!(0, distance_from_previous.as_secs() % self.period.as_secs());

        self.data.push(measurement);
        self.last_time = Some(time);
    }
}
