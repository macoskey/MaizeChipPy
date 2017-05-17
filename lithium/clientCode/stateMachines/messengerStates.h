
void msgStates(struct ENETvars *ENET, struct ENETmsg *enetMSG, struct ACQsetup *acqVars, struct MAIZEsetup *maizeVars, struct STATEvars *stateVars, int chld2prnt_pipe){
	
	ENET_recv(ENET,enetMSG,chld2prnt_pipe);
	
	while(enetMSG->enetWait == 1){
		
		ENET_recv(ENET,enetMSG,chld2prnt_pipe);	
		switch(enetMSG->enetMSG){
			case(0): { // do nothing
				break;				
			}
			
			case(1): { // change trigger delay
				acqVars->trigDelay = enetMSG->val;
				printf("TRIG DELAY SET TO %d\n",enetMSG->val);
				break;
			}
			
			case(2): { // change record length
				acqVars->recordLength = enetMSG->val;
				if(acqVars->numOnBoardAcqs*acqVars->recordLength > 5.0*1e9){
					printf("Can't store that much data on board, change numOnBoardAcqs or recordLength to bring it down. Ending program\n");
					enetMSG->enetWait = 0;
					enetMSG->runner = 0;
					printf("OH NO, YOU'VE KILLED ME! ... WHY!!!!!\n");
					usleep(100);
				}
				printf("RECORD LENGTH SET TO %d\n",enetMSG->val);
				break;
			}
			
			case(3): { // change acquisitions per transmit
				acqVars->acqsPerTrig = enetMSG->val;
				printf("ACQS PER TRIGGER SET TO %d\n",enetMSG->val);
				break;
			}
			
			case(4): { // change server mode
				acqVars->forkSelect = enetMSG->val;
				if(enetMSG->val == 0){
					printf("Operating in forked server mode, no on board storage\n");
				} else if (enetMSG->val == 1){
					printf("Operating in select server mode, no on board storage\n");
				} else if (enetMSG->val == 2){
					printf("Operating in forked server mode with on board storage\n");
				} else if (enetMSG->val == 3){
					printf("Operating in select server mode with on board storage\n");
				} else {
					printf("Invalid Run Mode, Ending Program\n");
					enetMSG->enetWait = 0;
					enetMSG->runner = 0;
					printf("OH NO, YOU'VE KILLED ME! ... WHY!!!!!\n");
					usleep(100);
				}
				break;
			}
			
			case(5): { // change size of on board storage
				acqVars->numOnBoardAcqs = enetMSG->val;
				if(acqVars->numOnBoardAcqs*acqVars->recordLength*2.0 > 1.0*1e8){
					printf("Can't store that much data on board, change numOnBoardAcqs or recordLength to bring it down. Ending program\n");
					enetMSG->enetWait = 0;
					enetMSG->runner = 0;
					printf("OH NO, YOU'VE KILLED ME! ... WHY!!!!!\n");
					usleep(100);
				}
				break;
			}
			
			case(6): { // change number of a commands
				maizeVars->aCommLen = enetMSG->val;
				maizeVars.aCommands = (uint32_t *)realloc(maizeVars.aCommands, maizeVars->aCommLen*sizeof(uint32_t));
				break;
			}
			
			case(7): { // change number of b memory locations
				maizeVars->bMemLocs = enetMSG->val;
				maizeVars.bArray = (uint32_t *)realloc(maizeVars.bArray, maizeVars->bMemLocs*sizeof(uint32_t));
				break;
			}
			
			case(8): { // change setupState, 0 = not ready, 1 = ready
				stateVars->setupState = enetMSG->val;
				break;
			}
						
			default: { // end program
				printf("State Machine Default Case, End Program\n");
				enetMSG->enetWait = 0;
				enetMSG->runner = 0;
				printf("OH NO, YOU'VE KILLED ME! ... WHY!!!!!\n");
				usleep(100);
				break;
			}
		}
		
		//~ usleep(100);
	}	
}



