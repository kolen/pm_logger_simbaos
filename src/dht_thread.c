#include "simba.h"
#include "dht.h"
#include "dht_thread.h"

#define DHT_PIN pin_d3_dev

struct rwlock_t dht_lock;

void* dht_thread_main(void* _unused) {
  struct log_object_t dht_log;
  struct dht_driver_t dht;

  log_object_init(&dht_log, "dht", LOG_UPTO(INFO));

  dht_module_init();
  rwlock_module_init();
  dht_init(&dht, &DHT_PIN);

  rwlock_init(&dht_lock);
  log_object_print(&dht_log, LOG_INFO, OSTR("Initialized dht\n"));
  while (1) {
    float temperature, humidity;

    rwlock_writer_take(&dht_lock);
    int result = dht_read(&dht, &temperature, &humidity);
    rwlock_writer_give(&dht_lock);
    log_object_print(&dht_log, LOG_INFO,
		     OSTR("(%d) Temperature: %f, humidity: %f\n"),
			  result, temperature, humidity);
  }
}
