#include <stddef.h>
#include "boards_exti.h"
#include "simba.h"
#include "sds011.h"

struct uart_soft_driver_t uart;
struct exti_driver_t uart_exti;
#define UART_RECEIVE_BUFFER_SIZE 256
#define SDS_UART_RX_PIN pin_d5_dev
#define SDS_UART_TX_PIN pin_d6_dev
#define SDS_UART_RX_EXTI exti_d5_dev
char uart_receive_buffer[UART_RECEIVE_BUFFER_SIZE];

#define SDS011_COMMAND_HEAD 0xaa
#define SDS011_COMMAND_TAIL 0xab
#define SDS011_COMMAND_QUERY_DATA 0xb4

const char sds_command_head = SDS011_COMMAND_HEAD;
const char sds_command_tail = SDS011_COMMAND_TAIL;

const char sds_command_data_get_reporting_mode[15] = {
  0x02, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xFF, 0xFF
};

const char sds_command_data_set_query_reporting_mode[15] = {
  0x02, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xFF, 0xFF
};

const char sds_command_query_data[15] = {
  0x04,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xFF, 0xFF
};

#define SDS011_COMMAND_REPORTING_MODE 0xb4

char sds011_command_checksum(const char *data, size_t data_length)
{
  size_t i;
  const char *data_p = data;
  char checksum = 0;
  for(i = 0; i < data_length; i++) {
    checksum += *data_p;
    data_p++;
  }
  return checksum;
}

void sds011_send_command(struct uart_soft_driver_t *uart, char command, const char *data, size_t data_length)
{
  uart_soft_write(uart, &sds_command_head, 1);
  uart_soft_write(uart, &command, 1);
  uart_soft_write(uart, data, data_length);

  char checksum = sds011_command_checksum(data, data_length);

  uart_soft_write(uart, &checksum, 1);
  uart_soft_write(uart, &sds_command_tail, 1);
}

// data is 6 bytes
int sds011_read_command(struct uart_soft_driver_t *uart, char *command, char *data)
{
  char command_buffer[9];
  do {
    uart_soft_read(uart, &command_buffer, 1);
  } while (command_buffer[0] != SDS011_COMMAND_HEAD);
  uart_soft_read(uart, &command_buffer, 8);

  char checksum = sds011_command_checksum(&command_buffer[1], 6);
  if (checksum == command_buffer[7] && command_buffer[8] == SDS011_COMMAND_TAIL) {
    memcpy(&command_buffer[1], &data, 6);
    *command = command_buffer[0];
    return TRUE;
  } else {
    return FALSE;
  }
}

void sds011_get_reporting_mode(struct uart_soft_driver_t *uart)
{
  sds011_send_command(uart,
		      SDS011_COMMAND_REPORTING_MODE,
		      sds_command_data_get_reporting_mode,
		      sizeof(sds_command_data_get_reporting_mode));
}

void sds011_query_data(struct uart_soft_driver_t *uart)
{
  sds011_send_command(uart,
		      SDS011_COMMAND_QUERY_DATA,
		      sds_command_query_data,
		      sizeof(sds_command_query_data));
}

struct uart_driver_t main_uart;
char main_uart_rxbuf[16];

void* sds011_main(void* _unused)
{
  uart_module_init(); // TODO: maybe unnecessary
  uart_init(&main_uart, &uart_device[0], 9600, &main_uart_rxbuf, 16);

  char test[] = "Test\n";
  uart_write(&main_uart, test, sizeof(test));

  exti_module_init();
  uart_soft_init(&uart,
		 &SDS_UART_TX_PIN,
		 &SDS_UART_RX_PIN,
		 &SDS_UART_RX_EXTI,
		 9600,
		 &uart_receive_buffer,
		 UART_RECEIVE_BUFFER_SIZE);
  sds011_get_reporting_mode(&uart);
  thrd_sleep(1);
  sds011_get_reporting_mode(&uart);
  thrd_sleep(1);

  char hex_buf[5];
  int i;

  while(1) {
    thrd_sleep(3);
    sds011_query_data(&uart);
    thrd_sleep_ms(100);
    char command;
    char data[6];
    sds011_read_command(&uart, &command, data);

    std_snprintf(hex_buf, 5, "%x: ", command);
    uart_write(&main_uart, hex_buf, std_strlen(hex_buf));
    for(i=0; i<6; i++) {
      std_snprintf(hex_buf, 5, "%x ", data[i]);
      uart_write(&main_uart, hex_buf, std_strlen(hex_buf));
    }
    std_snprintf(hex_buf, 5, "\n");
    uart_write(&main_uart, hex_buf, std_strlen(hex_buf));
  }
}
