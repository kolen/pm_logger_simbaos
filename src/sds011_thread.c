#include "simba.h"
#include "sds011.h"
#include "boards_exti.h"

struct uart_soft_driver_t uart;
struct exti_driver_t uart_exti;
#define UART_RECEIVE_BUFFER_SIZE 256
#define SDS_UART_RX_PIN pin_d5_dev
#define SDS_UART_TX_PIN pin_d6_dev
#define SDS_UART_RX_EXTI exti_d5_dev
char uart_receive_buffer[UART_RECEIVE_BUFFER_SIZE];
struct log_object_t sds011_log;

void* sds011_thread_main(void* _unused)
{
  log_object_init(&sds011_log, "sds011", LOG_UPTO(INFO));
  log_object_print(&sds011_log, LOG_INFO, OSTR("Starting sds011 module"));

  exti_module_init();
  uart_soft_init(&uart,
		 &SDS_UART_TX_PIN,
		 &SDS_UART_RX_PIN,
		 &SDS_UART_RX_EXTI,
		 9600,
		 &uart_receive_buffer,
		 UART_RECEIVE_BUFFER_SIZE);

  struct sds011_device_t sds011;
  sds011_init_with_uart_soft(&sds011, &uart);
  sds011_query_data_reporting_mode(&sds011);
  thrd_sleep(1);
  sds011_query_data_reporting_mode(&sds011);
  thrd_sleep(1);

  while(1) {
    thrd_sleep(3);

    sds011_query_measurement(&sds011);

    thrd_sleep_ms(100);
    struct sds011_reply_t reply;
    int result;
    result = sds011_read_reply(&sds011, &reply);

    if (!result) {
      log_object_print(&sds011_log, LOG_INFO, OSTR("Error reading packet."));
      continue;
    }

    /* char hex_buf[4 + 3 * 6 + 2]; */
    /* char *hex_buf_cur; */
    /* int i; */
    /* std_snprintf(hex_buf, 5, "%02x: ", (unsigned char)reply.command); */
    /* hex_buf_cur = hex_buf + 4; */
    /* for(i=0; i<6; i++) { */
    /*   std_snprintf(hex_buf_cur, 3, "%02x ", (unsigned char)reply.data[i]); */
    /*   hex_buf_cur += 3; */
    /* } */
    /* std_snprintf(hex_buf_cur, 5, "\n"); */
    /* log_object_print(&sds011_log, LOG_INFO, hex_buf); */

    if (reply.type == sds011_reply_measurement) {
      log_object_print(&sds011_log,
		       LOG_INFO,
		       OSTR("PM 2.5: %5.1f, PM 10: %5.1f"),
		       reply.measurement.pm2_5 / 10.0,
		       reply.measurement.pm10 / 10.0);
    }
  }
}
