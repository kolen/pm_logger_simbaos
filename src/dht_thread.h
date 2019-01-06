// Reading from DHT sensor requires disabling interrupts, if you need
// interrupts for other functionality such as UART, lock this mutex
// while you require interrupts
//
// There's no mutexes in Simba 15.0.3, so using rwlock
struct rwlock_t dht_lock;

void* dht_thread_main(void* _unused);
