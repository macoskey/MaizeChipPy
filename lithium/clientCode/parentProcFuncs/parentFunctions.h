

void hpsDataAcqParamUpdater(struct ENETsock *ENET, struct ENETmsg *enetMSG, struct ACQsetup *acqVars, struct MAIZEsetup *maizeVars, struct STATEvars *stateVars, int chld2prnt_pipe){	
	
	while( enetMSG->enetState == 1 ){
		
		switch(enetMSG->enetMSG){
			case(0): { // do nothing
				break;				
			}
			
			case(1): { // pause hps data acq to change params
				acqVars->trigDelay = enetMSG->val;
				printf("TRIG DELAY SET TO %d\n",enetMSG->val);
				break;
			}
			
			case(2): { // b_stop to change fpga programming
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
			
						
			default: { // end program
				printf("State Machine Default Case, End Program\n");
				enetMSG->enetWait = 0;
				enetMSG->runner = 0;
				printf("OH NO, YOU'VE KILLED ME! ... WHY!!!!!\n");
				usleep(100);
				break;
			}
		}
		
		enetRecv(ENET,enetMSG,chld2prnt_pipe); 
		write(chld2prnt_pipe,enetMSG,sizeof(enetMSG));
	}
}


void maizeChipUpdater(struct ENETsock *ENET, struct ENETmsg *enetMSG, struct ACQsetup *acqVars, struct MAIZEsetup *maizeVars, struct STATEvars *stateVars, int chld2prnt_pipe){	
	
	while( enetMSG->enetState == 2 ){	
		
		switch(enetMSG->enetMSG){
			case(0): { // do nothing
				break;				
			}
			
			case(1): { // update a commands
				acqVars->trigDelay = enetMSG->val;
				printf("TRIG DELAY SET TO %d\n",enetMSG->val);
				break;
			}
			
			case(2): { // update b commands
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
			
						
			default: { // end program
				printf("State Machine Default Case, End Program\n");
				enetMSG->enetWait = 0;
				enetMSG->runner = 0;
				printf("OH NO, YOU'VE KILLED ME! ... WHY!!!!!\n");
				usleep(100);
				break;
			}
		}
		
		enetRecv(ENET,enetMSG,chld2prnt_pipe); 
		write(chld2prnt_pipe,enetMSG,sizeof(enetMSG));
	}
}



void enetParentStateMachine(struct ENETsock *ENET, struct ENETmsg *enetMSG, struct ACQsetup *acqVars, struct MAIZEsetup *maizeVars, struct STATEvars *stateVars, int chld2prnt_pipe){	
	
	enetRecv(ENET,enetMSG,chld2prnt_pipe);
	write(chld2prnt_pipe,enetMSG,sizeof(enetMSG));
	
	switch(enetMSG->enetState){
		case(0): { // do nothing
			break;
		}
		
		case(1): { // pause hps data acq to change params
			hpsDataAcqParamUpdater(enetSock, enetMSG, acqVars, maizeVars, stateVars, chld2prnt_pipe[1]);
			break;
		}
		
		case(2): { // b_stop to change fpga programming
			//~ b_stop();
			maizeChipUpdater(enetSock, enetMSG, acqVars, maizeVars, stateVars, chld2prnt_pipe[1]);
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
}


void enetParentMessenger(struct ENETmsg *enetMSG, struct INITvars *initVars, struct ACQsetup *acqVars, struct MAIZEsetup *maizeVars, struct STATEvars *stateVars, int *chld2prnt_pipe, int *prnt2chld_pipe){
	
	close(chld2prnt_pipe[0]); close(prnt2chld_pipe[1]);
		
	struct ENETsock enetSock;
	enetMakeParentSocket(&enetSock,initVars);  // initializes ethernet comms on the child
	enetConnect(&enetSock);
		
	while(enetMSG->runner == 1){ // listen on ethernet port while the program is running
		enetParentStateMachine(&enetSock, enetMSG, acqVars, maizeVars, stateVars, chld2prnt_pipe[1]);
	}
	
	close(chld2prnt_pipe[1]); close(prnt2chld_pipe[0]);
	enetClose(&enetSock);
	//~ _exit(0); 
}
