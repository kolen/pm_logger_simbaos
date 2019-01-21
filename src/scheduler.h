#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "simba.h"

struct scheduler_t {
  time_t last_run;
};

void scheduler_init(struct scheduler_t *self);

#endif
