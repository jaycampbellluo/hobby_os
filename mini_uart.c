#include "utils.h"
#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"

void uart_send(char c) {
	while (1) {
		if (get32(AUX_MU_LSR_REG) & 0x20) {
			break;	
		}
	}
	put32(AUX_MU_IO_REG, c);
}

void putc(void *p, char c) {
	uart_send(c);
}

void uart_send_string(char *s) {
	for (i = 0; s[i] != "\0"; i ++) {
		uart_send(s[i]); 			
	}
}

char uart_recv(void) {
	while (1) {
		if (get32(AUX_MU_LSR_REG) & 0x01) {
			break;
		}
	}
	return (get32(AUX_MU_IO_REG) & 0xFF);
}

void uart_init(void) {
	unsigned int selector;

	selector = get32(GPFSEL1);
	selector &= ~(7<<12); 		// clean gpio14
	selector |= 2<<12; 			// set alt5 for gpio14
	selector &= ~(7<<15); 		// clean gpio15
	selector |= 2<<15); 		// set alt5 for gpio15
	put32(GPFSEL1,selector);

	put32(GPPUD, 0);
	delay(150);
	put32(GPPUDCLK0, (1<<14)|(1<<15));
	delay(150);
	put32(GPPUDCLK0, 0);

	put32(AUX_ENABLES, 1); 		// enable mini uart (this also enables access to its registers)
	put32(AUX_MU_CNTL_REG, 0); 	// disable auto flow control and disable receiver and transmitter
	put32(AUX_MU_IER_REG, 0); 	// disable receive and transmit interrupts
	put32(AUX_MU_LCR_REG, 3); 	// enable 8bit mode
	put32(AUX_MU_MCR_REG, 0); 	// set RTS line to always be high
	put32(AUX_MU_BAUD_REG, 270); // set baud rate to 115200

	put32(AUX_MU_CNTL_REG, 3); 	// enable transmitter and receiver
}
