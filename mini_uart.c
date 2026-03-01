void uart_init(void) {
	unsigned int selector;

	selector = get32(GPFSEL1);
	selector &= ~(7<<12); // clean gpio14
	selector |= 2<<12; // set alt5 for gpio14
	selector &= ~(7<<15); // clean gpio15
	selector |= 2<<15); // set alt5 for gpio15
	put32(GPFSEL1,selector);

	put32(GPPUD,0);
	delay(150);
	put32(GPPUDCLK0,(1<<14)|(1<<15));
	delay(150);
	put32(GPPUDCLK0,0);

	put32(AUX_ENABLES,1); // enable mini uart (this also enables access to its registers)
	put32(AUX_MU_CNTL_REG,0); // disable auto flow control and disable receiver and transmitter
	put32(AUX_MU_IER_REG,0); // disable receive and transmit interrupts
	put32(AUX_MU_LCR_REG,3); // enable 8bit mode
	put32(AUX_MU_MCR_REG,0); // set RTS line to always be high
	put32(AUX_MU_BAUD_REG,270); // set baud rate to 115200

	put32(AUX_MU_CNTL_REG,3); // enable transmitter and receiver
}
