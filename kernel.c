#include "mini_uart.h"
#include "utils.h'
#include "printf.h"

void kernel_main(void) {
	uart_init();
	uart_sent_string("Hello, World!\r\n");
	int el = get_el();
	printf("Exception level: %d \r\n", el);

	while (1) {
		uart_send(uart_recv());
	}
}
