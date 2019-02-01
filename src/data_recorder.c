#include "data_recorder.h"

void data_recorder_init(struct data_recorder_t *self, void* data_buffer, size_t sample_size, size_t num_samples, int32_t sampling_period) {
  self->data_buffer = data_buffer;
  self->sample_size = sample_size;
  self->num_samples_capacity = num_samples;
  self->last_sample_time = 0;
  self->sampling_period = sampling_period;

  memset(self->data_buffer, 0, num_samples * sample_size);
}

static void data_recorder_shift_data(struct data_recorder_t *self, int num_samples) {
  if (!num_samples) return;
  // Not efficient, but less error-prone

  /*
     ,-source

     |---------------o---------------------------|           o

      \_num_samples_/
                     '-destination
   */

  int num_samples_moved = self->num_samples_capacity - num_samples;

  memcpy(self->data_buffer + (num_samples * self->sample_size),
	 self->data_buffer,
	 num_samples_moved * self->sample_size);
  memset(self->data_buffer, 0, num_samples * self->sample_size);
}

int data_recorder_add_sample(struct data_recorder_t *self, int32_t sample_time, const void* sample) {
  int skip_samples;
  if (self->last_sample_time) {
    int32_t time_from_last = sample_time - self->last_sample_time;

    #ifdef TEST_ENVIRONMENT
    if (time_from_last % self->sampling_period != 0) return -EINVAL;
    #else
    ASSERTN(time_from_last % self->sampling_period == 0, EINVAL);
    #endif

    int32_t distance = time_from_last / self->sampling_period;
    data_recorder_shift_data(self, distance);
  } else {
    skip_samples = 0;
  }

  memcpy(self->data_buffer, sample, self->sample_size);
  self->last_sample_time = sample_time;
  return 0;
}
