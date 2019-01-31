#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "simba.h"

struct scheduler_t {
  void (*hourly_callback)(void*, int32_t);
  void (*minutely_callback)(void*, int32_t);
  void* hourly_callback_arg;
  void* minutely_callback_arg;

  int32_t last_known_time;

  uint32_t hourly_hours_mask;
  int32_t hourly_last_run;

  int minutely_period;
  int32_t minutely_last_run;
};

void scheduler_init(struct scheduler_t *self);
void scheduler_set_hourly(struct scheduler_t *self, uint32_t hours_mask, void (*callback)(void*, int32_t), void *callback_arg);
void scheduler_set_minutely(struct scheduler_t *self, int minutely_period, void (*callback)(void*, int32_t), void *callback_arg);
void scheduler_tick(struct scheduler_t *self, int32_t current_time);

#endif
