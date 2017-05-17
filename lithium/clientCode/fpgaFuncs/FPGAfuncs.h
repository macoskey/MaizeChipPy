

void FPGA_close(struct FPGAvars *FPGA){
	
	if( munmap( FPGA->virtual_base, HW_REGS_SPAN ) != 0 ) {
		printf( "ERROR: munmap() failed...\n" );
		close( FPGA->fd );
	}
	close( FPGA->fd );
}


void FPGA_stateReset(struct FPGAvars *FPGA, uint32_t st){
	*(uint32_t *)FPGA->stateReset = st;
	printf("\nFPGA state reset %d\n", *(uint32_t *)FPGA->stateReset);
}

void FPGA_setTrigDelay(struct FPGAvars *FPGA, uint32_t dt){
	*(uint32_t *)FPGA->trigDelay = dt;
	printf("\nFPGA trig delay %d\n", *(uint32_t *)FPGA->trigDelay);
}


void FPGA_setRecLength(struct FPGAvars *FPGA, uint32_t rL){
	*(uint32_t *)FPGA->recordLength = rL;
	printf("\nFPGA record length %d %d\n", *(uint32_t *)FPGA->recordLength,rL);
}


void FPGA_getData(struct FPGAvars *FPGA, struct chData *hpsData, struct SETUPvars *setupVars){
	
	uint32_t dlen;
	for(dlen=0 ; dlen < setupVars->recordLength ; dlen++){			
		*(uint32_t *)FPGA->read_addr = dlen;
		hpsData->c18[setupVars->acqNum*setupVars->recordLength + dlen] = *(uint32_t *)FPGA->gpio0_addr;
		hpsData->c18[setupVars->acqsPerTrig*setupVars->recordLength + setupVars->acqNum*setupVars->recordLength + dlen] = *(uint32_t *)FPGA->gpio1_addr;
	}

	*(uint32_t *)FPGA->read_addr = 0;
}





int FPGA_init(struct FPGAvars *FPGA){
	
	if( ( FPGA->fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 0 );
	}
	
	FPGA->virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, FPGA->fd, HW_REGS_BASE );
	
	//FPGA->write_addr = FPGA->virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + PIO_H2F_READ_ADDR_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	
	//FPGA->a_instr = FPGA->virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + PIO_F2H_GPIO0_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	
	//FPGA->b_instr = FPGA->virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + PIO_F2H_GPIO1_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	
	FPGA->read_addr = FPGA->virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + PIO_H2F_READ_ADDR_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	
	FPGA->gpio0_addr = FPGA->virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + PIO_F2H_GPIO0_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	
	FPGA->gpio1_addr = FPGA->virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + PIO_F2H_GPIO1_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	
	FPGA->transReady = FPGA->virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + PIO_F2H_TRANSMIT_READY_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	
	FPGA->trigDelay = FPGA->virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + PIO_H2F_TRIG_DELAY_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	
	FPGA->trigCntr = FPGA->virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + PIO_F2H_TRIG_COUNT_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	
	FPGA->recordLength = FPGA->virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + PIO_H2F_RECORD_LENGTH_BASE ) & ( unsigned long)( HW_REGS_MASK ) );

	FPGA->stateReset = FPGA->virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + PIO_H2F_FPGA_STATE_RESET_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	
	FPGA->stateVal = FPGA->virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + PIO_F2H_FPGA_STATE_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	
	if( FPGA->virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( FPGA->fd );
		return( 0 );
	}
	
	FPGA_stateReset(FPGA,1);
	usleep(1);
	FPGA_stateReset(FPGA,0);
	*(uint32_t *)FPGA->read_addr = 0;
	
	return(1);
}
