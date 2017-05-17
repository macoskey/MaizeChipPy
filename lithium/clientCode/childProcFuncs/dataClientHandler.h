

void dataClientHandler(struct ENETmsg *enetMSG, struct INITvars *initVars, struct ACQsetup *acqVars, int *chld2prnt_pipe, int *prnt2chld_pipe){
	
	close(chld2prnt_pipe[1]); close(prnt2chld_pipe[0]); // close the ends of the pipe that aren't used
	
	struct FPGAvars FPGA; 
	if(FPGA_init(&FPGA) == 0){ // initialize fpga comms. kill everything if fpga can't be intialized
		printf("FPGA not initialized. Ending Program.\n");
		close(chld2prnt_pipe[0]); close(prnt2chld_pipe[1]);
		munmap( initVars, sizeof(*initVars)); munmap( enetMSG, sizeof(*enetMSG)); munmap( acqVars, sizeof(*acqVars));
		kill(0,SIGTERM);
		kill(0,SIGKILL);
	} 
	
	
	int modeSelect = 1;
	while(enetMSG->runner == 1 && modeSelect == 1){
		
		switch(acqVars->forkSelect){
			case(0):{
				printf("Operating in Fork Server Mode\n");
				acqVars->numOnBoardAcqs = 1;
				modeSelect = forkModeClient(&FPGA, enetMSG, initVars, acqVars, chld2prnt_pipe, prnt2chld_pipe);	
				break;	
			}
			case(1):{
				printf("Operating in Select Server Mode\n");
				acqVars->numOnBoardAcqs = 1;
				modeSelect = selectModeClient(&FPGA, enetMSG, initVars, acqVars, chld2prnt_pipe, prnt2chld_pipe);
				break;	
			}
			case(2):{
				printf("Operating in Fork Server Mode with On Board Data Storage\n");	
				modeSelect = forkModeClientOnBoardMem(&FPGA, enetMSG, initVars, acqVars, chld2prnt_pipe, prnt2chld_pipe);	
				break;	
			}
			case(3):{
				printf("Operating in Select Server Mode with On Board Data Storage\n");
				modeSelect = selectModeClientOnBoardMem(&FPGA, enetMSG, initVars, acqVars, chld2prnt_pipe, prnt2chld_pipe);
				break;	
			}
			default:{
				printf("Defaulting to Select Server Mode, forkSelect = %d [only 0-3 are valid]\n",acqVars->forkSelect);
				modeSelect = selectModeClient(&FPGA, enetMSG, initVars, acqVars, chld2prnt_pipe, prnt2chld_pipe);
				break;	
			}
		}
	}
	
	// shut it all down
	printf("\n\t-dataClientHandler Layer Cleanup\n\n");
	close(chld2prnt_pipe[1]); close(prnt2chld_pipe[0]);
	munmap( initVars, sizeof(*initVars)); munmap( enetMSG, sizeof(*enetMSG)); munmap( acqVars, sizeof(*acqVars));
	FPGA_close(&FPGA);
} 

