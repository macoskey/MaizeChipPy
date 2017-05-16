
void STATE_ENETmsg(struct ENETvars *ENET, struct ENETcommVar *enetComms, struct SETUPvars *setupVars, int chld2prnt_pipe){
	
	ENET_recv(ENET,enetComms,chld2prnt_pipe);
	//~ usleep(100);
	while(enetComms->enetWait == 1){
		
		ENET_recv(ENET,enetComms,chld2prnt_pipe);	
		switch(enetComms->enetMSG){
			case(0): { // do nothing
				break;				
			}
			
			case(1): { // change trigger delay
				setupVars->trigDelay = enetComms->val;
				printf("TRIG DELAY SET TO %d\n",enetComms->val);
				break;				
			}
			
			case(2): { // change record length
				setupVars->recordLength = enetComms->val;
				if(setupVars->numOnBoardAcqs*setupVars->recordLength*2.0 > 10.0*1e9){
					printf("Can't store that much data on board, change numOnBoardAcqs or recordLength to bring it down. Ending program\n");
					enetComms->enetWait = 0;
					enetComms->runner = 0;
					printf("OH NO, YOU'VE KILLED ME! ... WHY!!!!!\n");
					usleep(100);
				}
				printf("RECORD LENGTH SET TO %d\n",enetComms->val);
				break;
			}
			
			case(3): { // change acquisitions per transmit
				setupVars->acqsPerTrig = enetComms->val;
				printf("ACQS PER TRIGGER SET TO %d\n",enetComms->val);
				break;
			}
			
			case(4): { // change server mode
				setupVars->forkSelect = enetComms->val;
				if(enetComms->val == 0){
					printf("Operating in forked server mode, no on board storage\n");
				} else if (enetComms->val == 1){
					printf("Operating in select server mode, no on board storage\n");
				} else if (enetComms->val == 2){
					printf("Operating in forked server mode with on board storage\n");
				} else if (enetComms->val == 3){
					printf("Operating in select server mode with on board storage\n");
				} else {
					printf("Invalid Run Mode, Ending Program\n");
					enetComms->enetWait = 0;
					enetComms->runner = 0;
					printf("OH NO, YOU'VE KILLED ME! ... WHY!!!!!\n");
					usleep(100);
				}
				break;
			}
			
			case(5): { // change size of on board storage
				setupVars->numOnBoardAcqs = enetComms->val;
				if(setupVars->numOnBoardAcqs*setupVars->recordLength*2.0 > 1.0*1e8){
					printf("Can't store that much data on board, change numOnBoardAcqs or recordLength to bring it down. Ending program\n");
					enetComms->enetWait = 0;
					enetComms->runner = 0;
					printf("OH NO, YOU'VE KILLED ME! ... WHY!!!!!\n");
					usleep(100);
				}
				break;
			}
						
			default: { // end program
				printf("State Machine Default Case, End Program\n");
				enetComms->enetWait = 0;
				enetComms->runner = 0;
				printf("OH NO, YOU'VE KILLED ME! ... WHY!!!!!\n");
				usleep(100);
				break;
			}
		}
		
		//~ usleep(100);
	}	
}

void STATE_FPGAhpsChanger(struct FPGAvars *FPGA, struct chData *hpsData, struct SETUPvars *setupVars, uint32_t *onBoardData){
	
	FPGA_setTrigDelay(FPGA,setupVars->trigDelay);
	FPGA_setRecLength(FPGA,setupVars->recordLength);
	
	//~ free(hpsData->c18);
	hpsData->c18 = (uint32_t *)realloc(hpsData->c18, 2*setupVars->acqsPerTrig*setupVars->recordLength*sizeof(uint32_t));
	onBoardData = (uint32_t *)realloc( onBoardData, 2*setupVars->recordLength*setupVars->numOnBoardAcqs*sizeof(uint32_t));
	//~ printf("TRIG DELAY SET TO %d\n",setupVars->trigDelay);	
	
}






