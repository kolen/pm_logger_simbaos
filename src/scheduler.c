#include "scheduler.h"

#define SECONDS_IN_HOUR (60*60)

void scheduler_init(struct scheduler_t *self) {
  memset(self, sizeof(self), 0);
}

void scheduler_set_hourly(struct scheduler_t *self, uint32_t hours_mask, struct sem_t *hourly_sem) {
  self->hourly_hours_mask = hours_mask;
  self->hourly_sem = hourly_sem;
}

static int32_t scheduler_beginning_of_hour(int32_t time) {
  return time - (time % SECONDS_IN_HOUR);
}

static int scheduler_hour_number(int32_t time) {
  return (time % (SECONDS_IN_HOUR * 24)) / SECONDS_IN_HOUR;
}

static int scheduler_should_run_in_hour(int hour_number, uint32_t hours_mask) {
  return hours_mask & (1 << (23 - hour_number));
}

static int32_t scheduler_beginning_of_minute(int32_t time) {
  return time - (time % 60);
}

static int scheduler_minute_number(int32_t time) {
  return (time % SECONDS_IN_HOUR) / 60;
}

static int scheduler_should_run_in_minute(int minute_number, int minutely_period) {
  return minutely_period && ((minute_number % minutely_period) == 0);
}

void scheduler_tick(struct scheduler_t *self, int32_t current_time) {
  if (!(self->last_known_time < current_time)) {
    return; // Detected "backwards clock", might happen when correcting it
  }

  int32_t current_hour = scheduler_beginning_of_hour(current_time);
  int current_hour_number = scheduler_hour_number(current_hour);

  if (current_hour > self->hourly_last_run &&
      scheduler_should_run_in_hour(current_hour_number, self->hourly_hours_mask) &&
      self->hourly_sem) {
    self->hourly_last_run = current_hour;
    sem_give(self->hourly_sem, 1);
  }

  int32_t current_minute = scheduler_beginning_of_minute(current_time);
  int current_minute_number = scheduler_minute_number(current_minute);

  if (current_minute > self->minutely_last_run &&
      scheduler_should_run_in_minute(current_minute_number, self->minutely_period) &&
      self->minutely_sem) {
    self->minutely_last_run = current_minute;
    sem_give(self->minutely_sem, 1);
  }
}
