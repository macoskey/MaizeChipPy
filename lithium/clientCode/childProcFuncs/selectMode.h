
int selectModeDataClient(struct FPGAvars *FPGA, struct ENETcommVar *enetComms, struct INITvars *initVars, struct SETUPvars *setupVars, int *chld2prnt_pipe, int *prnt2chld_pipe){
	int thisRunMode = 1;
		
	struct ENETvars ENETdata; 	
	ENET_DataInitSelect(&ENETdata,initVars); 
	
	struct chData hpsData; 	
	hpsData.c18 = (uint32_t *)malloc( 2*setupVars->acqsPerTrig*setupVars->recordLength*sizeof(uint32_t));
	
	uint32_t *onBoardData;
	onBoardData = (uint32_t *)malloc( 2*setupVars->acqsPerTrig*setupVars->recordLength*setupVars->numOnBoardAcqs*sizeof(uint32_t));
	
	fd_set set;
	struct timeval timeout;
	int rv; 
	
	FD_ZERO(&set); FD_SET(chld2prnt_pipe[0], &set);
	timeout.tv_sec = 0; timeout.tv_usec = 1000;
	
	int stateCheckWait = 1; // sleep timer (us) to allow variables to propagate before and after state checker
	while(enetComms->runner == 1){
		
		if(setupVars->forkSelect != thisRunMode) break;
		
		stateChecker(FPGA, enetComms, setupVars, &stateCheckWait);
		
		switch(State){ // ethernet ALWAYS goes first, program blocked until enet says 'go'. the rest is conditional
			
			case(0):{ // [0,0,0], nothing. getting stuck here will burn cpu though. always initializing enetWait to 1 will help
				//~ printf("case0\n");
				break; 
			}
			
			case(1):{ // [0,0,1], wait for messages from ethernet
				//~ printf("case1\n");
				while(enetComms->enetWait == 1 && setupVars->forkSelect == thisRunMode){ // read is blocking, won't exit this case until ethernet says 'go'
					rv = select(chld2prnt_pipe[0]+1, &set, NULL, NULL, &timeout);
					if(rv>0){
						read(chld2prnt_pipe[0],enetComms,sizeof(enetComms));
					}
				}
				 
				if (enetComms->runner == 1 && setupVars->forkSelect == thisRunMode){	
					STATE_FPGAhpsChanger(FPGA, &hpsData, setupVars, onBoardData);
				}
				break;
			}
			
			case(2):{ // [0,1,0], Done with acqs, FPGA ready to send data to HPS/server
				//~ printf("case2\n");
				FPGA_getData(FPGA, &hpsData, setupVars); // collect data from fpga gpio regs
				usleep(setupVars->enetSendDelay);
				ENET_selectSend(&ENETdata, &hpsData, setupVars); // send data to server
				setupVars->acqNum = 0; // if data was from multiple triggers, reset acqnum to 0
				break;
			} 
			
			case(3):{ // [0,1,1], FPGA ready to send data to HPS, message from ethernet
				// generally shouldn't get into this state, give precedence to ethernet in case of kill signal, wipe data
				//~ printf("case3\n");
				while(enetComms->enetWait == 1 && setupVars->forkSelect == thisRunMode){
					rv = select(chld2prnt_pipe[0]+1, &set, NULL, NULL, &timeout);
					if(rv>0){
						read(chld2prnt_pipe[0],enetComms,sizeof(enetComms));
					}
				}
					
				if (enetComms->runner == 1 && setupVars->forkSelect == thisRunMode){
					setupVars->acqNum = 0;								
					STATE_FPGAhpsChanger(FPGA, &hpsData, setupVars, onBoardData);
				}
				break;
			}
			 
			case(4):{ // [1,0,0], still acquiring data. let it ride
				//~ printf("case4\n");
				break;
			}  
			
			case(5):{ // [1,0,1], still acquiring data, but message from ethernet
				// generally shouldn't get into this state, give precedence to ethernet in case of kill signal, wipe data
				//~ printf("case5\n");
				while(enetComms->enetWait == 1 && setupVars->forkSelect == thisRunMode){
					//~ read(chld2prnt_pipe[0],enetComms,sizeof(enetComms));
					rv = select(chld2prnt_pipe[0]+1, &set, NULL, NULL, &timeout);
					if(rv>0){
						read(chld2prnt_pipe[0],enetComms,sizeof(enetComms));
					}
				}
				
				if (enetComms->runner == 1 && setupVars->forkSelect == thisRunMode){
					setupVars->acqNum = 0;										
					STATE_FPGAhpsChanger(FPGA, &hpsData, setupVars, onBoardData);
				}
				break;
			}
			
			case(6):{ // [1,1,0], still acquiring data, FPGA ready to transmit to HPS. let it ride
				//~ printf("case6\n");
				FPGA_getData(FPGA, &hpsData, setupVars);
				setupVars->acqNum++;
				break; 
			}
			
			case(7):{ // [1,1,1], all things happening. ethernet gets precedence
				// generally shouldn't get into this state, give precedence to ethernet in case of kill signal, wipe data
				//~ printf("case7\n");
				while(enetComms->enetWait == 1 && setupVars->forkSelect == thisRunMode){
					//~ read(chld2prnt_pipe[0],enetComms,sizeof(enetComms));
					rv = select(chld2prnt_pipe[0]+1, &set, NULL, NULL, &timeout);
					if(rv>0){
						read(chld2prnt_pipe[0],enetComms,sizeof(enetComms));
					}
				}
				
				if (enetComms->runner == 1 && setupVars->forkSelect == thisRunMode){
					setupVars->acqNum = 0;										
					STATE_FPGAhpsChanger(FPGA, &hpsData, setupVars, onBoardData);
				}
				break; 
			}
			
			default:{ // Something terrible probably happened. Kill it dead
				printf("undefined state, ending program\n");
				enetComms->runner = 0;
				usleep(100);
				break;
			}
				
		}
		
	}
	
	if(enetComms->runner == 1 && setupVars->forkSelect != thisRunMode){
		// mode change
		enetComms->enetWait = 1; enetComms->enetMSG = 0; enetComms->val = 0;
		printf("\n------------ CHANGING SERVER MODE ------------\n\n\t-selectModeClient Layer Cleanup\n");
		free(hpsData.c18);
		free(onBoardData);
		ENET_close(&ENETdata); 
		return(1);		
	}else{
		// shut it all down
		printf("\n------------ PROGRAM END ------------\n\n\t-selectModeClient Layer Cleanup\n");
		free(hpsData.c18);
		free(onBoardData);
		ENET_close(&ENETdata); 
		return(0);
	}
	
	return(0);
}
