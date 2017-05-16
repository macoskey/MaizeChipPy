// Is black box, comments are for losers. 
// Abandon all hope ye who enter.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h> 
#include <netinet/in.h>  
#include <netdb.h> 
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"       
#include "hps_0h.h"     
#include <time.h>
#include <math.h>
#include <signal.h>

#define ENET_STOP_BIT 0
#define FPGA_TRAN_BIT 1
#define FPGA_ACQN_BIT 2
 
#define HW_REGS_BASE ( ALT_STM_OFST )  
#define HW_REGS_SPAN ( 0x04000000 )   
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )   
  
uint32_t State = 0;
uint32_t usleepTime = 1;
int connectionTimeOut = 5; // minutes


#include "structDeclarations.h" 
#include "initializationFuncs.h"  
#include "FPGAfuncs.h"  
#include "ENETfuncs.h"
#include "stateMachineFuncs.h"   


void stateChecker(struct FPGAvars *FPGA, struct ENETcommVar *ENET, struct SETUPvars *setupVars, int *stw);
void msgClient(struct ENETcommVar *enetComms, struct INITvars *initVars, struct SETUPvars *setupVars, int *chld2prnt_pipe, int *prnt2chld_pipe);
void dataClientHandler(struct ENETcommVar *enetComms, struct INITvars *initVars, struct SETUPvars *setupVars, int *chld2prnt_pipe, int *prnt2chld_pipe);
int selectModeClient(struct FPGAvars *FPGA, struct ENETcommVar *enetComms, struct INITvars *initVars, struct SETUPvars *setupVars, int *chld2prnt_pipe, int *prnt2chld_pipe);
int forkModeClient(struct FPGAvars *FPGA, struct ENETcommVar *enetComms, struct INITvars *initVars, struct SETUPvars *setupVars, int *chld2prnt_pipe, int *prnt2chld_pipe);
int selectModeClientOnBoardMem(struct FPGAvars *FPGA, struct ENETcommVar *enetComms, struct INITvars *initVars, struct SETUPvars *setupVars, int *chld2prnt_pipe, int *prnt2chld_pipe);
int forkModeClientOnBoardMem(struct FPGAvars *FPGA, struct ENETcommVar *enetComms, struct INITvars *initVars, struct SETUPvars *setupVars, int *chld2prnt_pipe, int *prnt2chld_pipe);


int main(int argc, char *argv[]) { // printf("into main!\n"); 
		
	struct ENETcommVar *enetComms = mmap( NULL, sizeof(*enetComms), ( PROT_READ | PROT_WRITE ), MAP_SHARED | MAP_ANONYMOUS, -1, 0 );	
	struct INITvars *initVars = mmap( NULL, sizeof(*initVars), ( PROT_READ | PROT_WRITE ), MAP_SHARED | MAP_ANONYMOUS, -1, 0 );
	struct SETUPvars *setupVars = mmap( NULL, sizeof(*setupVars), ( PROT_READ | PROT_WRITE ), MAP_SHARED | MAP_ANONYMOUS, -1, 0 );		
	
	loadVars(argc,argv,initVars,enetComms,setupVars);
	
	int chld2prnt_pipe[2]; int a;
	int prnt2chld_pipe[2]; int b;
	a = pipe(chld2prnt_pipe); if (a == -1){ perror("pipe"); exit(EXIT_FAILURE); }
	b = pipe(prnt2chld_pipe); if (b == -1){ perror("pipe"); exit(EXIT_FAILURE); }
	
	pid_t pid; 
	pid = fork(); 
	
	if( pid == 0 ){ // child: communicates with the server		
		msgClient(enetComms, initVars, setupVars, chld2prnt_pipe, prnt2chld_pipe);		
	} 
	
	else { // parent: communicates with the fpga 			
	    dataClientHandler(enetComms, initVars, setupVars, chld2prnt_pipe, prnt2chld_pipe);					
	}
	
	return( 0 );
}


void stateChecker(struct FPGAvars *FPGA, struct ENETcommVar *ENET, struct SETUPvars *setupVars, int *stw){ 
	usleep(*stw);
	State ^= (-(*(int volatile *)&ENET->enetWait) ^ State) & (1 << ENET_STOP_BIT); 
	State ^= (-(*(uint32_t volatile *)FPGA->transReady) ^ State) & (1 << FPGA_TRAN_BIT);
	State ^= (-((setupVars->acqsPerTrig-setupVars->acqNum)>1) ^ State) & (1 << FPGA_ACQN_BIT);
	usleep(*stw);
}
 

void msgClient(struct ENETcommVar *enetComms, struct INITvars *initVars, struct SETUPvars *setupVars, int *chld2prnt_pipe, int *prnt2chld_pipe){
	
	close(chld2prnt_pipe[0]); close(prnt2chld_pipe[1]);
		
	struct ENETvars ENETmsg;
	ENET_MsgInit(&ENETmsg,initVars);  // initializes ethernet comms on the child
		
	while(enetComms->runner == 1){ // listen on ethernet port while the program is running
		STATE_ENETmsg(&ENETmsg, enetComms, setupVars, chld2prnt_pipe[1]);
	}
	
	close(chld2prnt_pipe[1]); close(prnt2chld_pipe[0]);
	ENET_close(&ENETmsg);
	_exit(0); 
}


// NEED TO CLEAN UP SWITCH BETWEEN FORK/SELECT MID RUN
void dataClientHandler(struct ENETcommVar *enetComms, struct INITvars *initVars, struct SETUPvars *setupVars, int *chld2prnt_pipe, int *prnt2chld_pipe){
	
	close(chld2prnt_pipe[1]); close(prnt2chld_pipe[0]); // close the ends of the pipe that aren't used

	struct FPGAvars FPGA; 
	if(FPGA_init(&FPGA) == 0){ // initialize fpga comms. kill everything if fpga can't be intialized
		printf("FPGA not initialized. Ending Program.\n");
		close(chld2prnt_pipe[0]); close(prnt2chld_pipe[1]);
		munmap( initVars, sizeof(*initVars)); munmap( enetComms, sizeof(*enetComms)); munmap( setupVars, sizeof(*setupVars));
		kill(0,SIGTERM);
		kill(0,SIGKILL);
	} 
	
	int modeSelect = 1;
	while(enetComms->runner == 1 && modeSelect == 1){
		switch(setupVars->forkSelect){
			case(0):{
				printf("Operating in Fork Server Mode\n");
				setupVars->numOnBoardAcqs = 1;
				modeSelect = forkModeClient(&FPGA, enetComms, initVars, setupVars, chld2prnt_pipe, prnt2chld_pipe);	
				break;	
			}
			case(1):{
				printf("Operating in Select Server Mode\n");
				setupVars->numOnBoardAcqs = 1;
				modeSelect = selectModeClient(&FPGA, enetComms, initVars, setupVars, chld2prnt_pipe, prnt2chld_pipe);
				break;	
			}
			case(2):{
				printf("Operating in Fork Server Mode with On Board Data Storage\n");	
				modeSelect = forkModeClientOnBoardMem(&FPGA, enetComms, initVars, setupVars, chld2prnt_pipe, prnt2chld_pipe);	
				break;	
			}
			case(3):{
				printf("Operating in Select Server Mode with On Board Data Storage\n");
				modeSelect = selectModeClientOnBoardMem(&FPGA, enetComms, initVars, setupVars, chld2prnt_pipe, prnt2chld_pipe);
				break;	
			}
			default:{
				printf("Defaulting to Select Server Mode, forkSelect = %d [only 0 and 1 are valid]\n",setupVars->forkSelect);
				modeSelect = selectModeClient(&FPGA, enetComms, initVars, setupVars, chld2prnt_pipe, prnt2chld_pipe);
				break;	
			}
		}
	}
	
	// shut it all down
	printf("\n\t-dataClientHandler Layer Cleanup\n\n");
	close(chld2prnt_pipe[1]); close(prnt2chld_pipe[0]);
	munmap( initVars, sizeof(*initVars)); munmap( enetComms, sizeof(*enetComms)); munmap( setupVars, sizeof(*setupVars));
	FPGA_close(&FPGA);
} 


int forkModeClient(struct FPGAvars *FPGA, struct ENETcommVar *enetComms, struct INITvars *initVars, struct SETUPvars *setupVars, int *chld2prnt_pipe, int *prnt2chld_pipe){
	int thisRunMode = 0;
	
	struct ENETvars ENETdata;
	ENET_DataInit(&ENETdata,initVars); 	
	
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
		
		// bit order [2,1,0] -> [still aquiring data, fpga->hps transmit ready, enetWait]
		stateChecker(FPGA, enetComms, setupVars, &stateCheckWait);
		
		switch(State){ // ethernet ALWAYS goes first, program blocked until enet says 'go'. the rest is conditional
			
			case(0):{ // [0,0,0], nothing. getting stuck here will burn cpu though.
				break; 
			}
			
			case(1):{ // [0,0,1], wait for messages from ethernet
				while(enetComms->enetWait == 1 && setupVars->forkSelect == thisRunMode){ // read is blocking, won't exit this case until ethernet says 'go'
					
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
			
			case(2):{ // [0,1,0], Done with acqs, FPGA ready to send data to HPS/server
				FPGA_getData(FPGA, &hpsData, setupVars); // collect data from fpga gpio regs		
				ENET_send(&ENETdata, &hpsData, setupVars); // send data to server
				setupVars->acqNum = 0; // if data was from multiple triggers, reset acqnum to 0
				break;
			} 
			
			case(3):{ // [0,1,1], FPGA ready to send data to HPS, message from ethernet
				// generally shouldn't get into this state, give precedence to ethernet in case of kill signal, wipe data
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
				break;
			}  
			
			case(5):{ // [1,0,1], still acquiring data, but message from ethernet
				// generally shouldn't get into this state, give precedence to ethernet in case of kill signal, wipe data
				while(enetComms->enetWait == 1 && setupVars->forkSelect == thisRunMode){ // read is blocking, won't exit this case until ethernet says 'go'
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
				FPGA_getData(FPGA, &hpsData, setupVars);
				setupVars->acqNum++;
				break; 
			}
			
			case(7):{ // [1,1,1], all things happening. ethernet gets precedence
				// generally shouldn't get into this state, give precedence to ethernet in case of kill signal, wipe data
				while(enetComms->enetWait == 1 && setupVars->forkSelect == thisRunMode){ // read is blocking, won't exit this case until ethernet says 'go'
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
		printf("\n------------ CHANGING SERVER MODE ------------\n\n\t-forkModeClient Layer Cleanup\n");
		free(hpsData.c18);
		free(onBoardData);
		ENET_close(&ENETdata); 
		return(1);
		
	}else{
		// shut it all down
		printf("\n------------ PROGRAM END ------------\n\n\t-forkModeClient Layer Cleanup\n");
		free(hpsData.c18);
		free(onBoardData);
		ENET_close(&ENETdata); 
		return(0);
	}
	
	return(0);
}



int selectModeClient(struct FPGAvars *FPGA, struct ENETcommVar *enetComms, struct INITvars *initVars, struct SETUPvars *setupVars, int *chld2prnt_pipe, int *prnt2chld_pipe){
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
		//~ usleep(2);
		stateChecker(FPGA, enetComms, setupVars, &stateCheckWait);
		//~ usleep(2);
		//~ printf("State = %u\n",State);
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



int forkModeClientOnBoardMem(struct FPGAvars *FPGA, struct ENETcommVar *enetComms, struct INITvars *initVars, struct SETUPvars *setupVars, int *chld2prnt_pipe, int *prnt2chld_pipe){
	int thisRunMode = 2;
	
	struct ENETvars ENETdata;
	ENET_DataInit(&ENETdata,initVars); 	
	
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
		
		// bit order [2,1,0] -> [still aquiring data, fpga->hps transmit ready, enetWait]
		stateChecker(FPGA, enetComms, setupVars, &stateCheckWait);
		
		switch(State){ // ethernet ALWAYS goes first, program blocked until enet says 'go'. the rest is conditional
			
			case(0):{ // [0,0,0], nothing. getting stuck here will burn cpu though.
				break; 
			}
			
			case(1):{ // [0,0,1], wait for messages from ethernet
				while(enetComms->enetWait == 1 && setupVars->forkSelect == thisRunMode){ // read is blocking, won't exit this case until ethernet says 'go'
					
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
			
			case(2):{ // [0,1,0], Done with acqs, FPGA ready to send data to HPS/server
				FPGA_getData(FPGA, &hpsData, setupVars); // collect data from fpga gpio regs		
				ENET_send(&ENETdata, &hpsData, setupVars); // send data to server
				if(setupVars->onBoardCounter < setupVars->numOnBoardAcqs){
					memcpy(&onBoardData[setupVars->onBoardCounter*2*setupVars->acqsPerTrig*setupVars->recordLength],hpsData.c18, 2*setupVars->acqsPerTrig*setupVars->recordLength*sizeof(uint32_t));
					setupVars->onBoardCounter+=setupVars->acqsPerTrig;
				} else {
					; // need to write terminator clause -- kill child proc, end program
				}
				setupVars->acqNum = 0; // if data was from multiple triggers, reset acqnum to 0
				break;
			} 
			
			case(3):{ // [0,1,1], FPGA ready to send data to HPS, message from ethernet
				// generally shouldn't get into this state, give precedence to ethernet in case of kill signal, wipe data
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
				break;
			}  
			
			case(5):{ // [1,0,1], still acquiring data, but message from ethernet
				// generally shouldn't get into this state, give precedence to ethernet in case of kill signal, wipe data
				while(enetComms->enetWait == 1 && setupVars->forkSelect == thisRunMode){ // read is blocking, won't exit this case until ethernet says 'go'
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
				FPGA_getData(FPGA, &hpsData, setupVars);
				setupVars->acqNum++;
				break; 
			}
			
			case(7):{ // [1,1,1], all things happening. ethernet gets precedence
				// generally shouldn't get into this state, give precedence to ethernet in case of kill signal, wipe data
				while(enetComms->enetWait == 1 && setupVars->forkSelect == thisRunMode){ // read is blocking, won't exit this case until ethernet says 'go'
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
		printf("\n------------ CHANGING SERVER MODE ------------\n\n\t-forkModeClient Layer Cleanup\n");
		free(hpsData.c18);
		free(onBoardData);
		ENET_close(&ENETdata); 
		return(1);
		
	}else{
		// shut it all down
		printf("\n------------ PROGRAM END ------------\n\n\t-forkModeClient Layer Cleanup\n");
		free(hpsData.c18);
		free(onBoardData);
		ENET_close(&ENETdata); 
		return(0);
	}
	
	return(0);
}



int selectModeClientOnBoardMem(struct FPGAvars *FPGA, struct ENETcommVar *enetComms, struct INITvars *initVars, struct SETUPvars *setupVars, int *chld2prnt_pipe, int *prnt2chld_pipe){	
	int thisRunMode = 3;
		
	struct ENETvars ENETdata; 	
	ENET_DataInitSelect(&ENETdata,initVars); 
	
	struct chData hpsData; 	
	hpsData.c18 = (uint32_t *)malloc( 2*setupVars->acqsPerTrig*setupVars->recordLength*sizeof(uint32_t));
	
	uint32_t *onBoardData;
	onBoardData = (uint32_t *)malloc( 2*setupVars->recordLength*setupVars->numOnBoardAcqs*sizeof(uint32_t));
	
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
				while(enetComms->enetWait == 1 && setupVars->forkSelect == thisRunMode){ // read is blocking, won't exit this case until ethernet says 'go'
					//~ printf("case1\n");
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
				FPGA_getData(FPGA, &hpsData, setupVars); // collect data from fpga gpio regs
				ENET_selectSend(&ENETdata, &hpsData, setupVars); // send data to server
				if(setupVars->onBoardCounter < setupVars->numOnBoardAcqs){
					memcpy(&onBoardData[setupVars->onBoardCounter*2*setupVars->acqsPerTrig*setupVars->recordLength],hpsData.c18, 2*setupVars->acqsPerTrig*setupVars->recordLength*sizeof(uint32_t));
					setupVars->onBoardCounter+=setupVars->acqsPerTrig;
				} else {
					; // need to write terminator clause -- kill child proc, end program
				}
				setupVars->acqNum = 0; // if data was from multiple triggers, reset acqnum to 0
				break;
			} 
			
			case(3):{ // [0,1,1], FPGA ready to send data to HPS, message from ethernet
				// generally shouldn't get into this state, give precedence to ethernet in case of kill signal, wipe data
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
				break;
			}  
			
			case(5):{ // [1,0,1], still acquiring data, but message from ethernet
				// generally shouldn't get into this state, give precedence to ethernet in case of kill signal, wipe data
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
				FPGA_getData(FPGA, &hpsData, setupVars);
				setupVars->acqNum++;
				break; 
			}
			
			case(7):{ // [1,1,1], all things happening. ethernet gets precedence
				// generally shouldn't get into this state, give precedence to ethernet in case of kill signal, wipe data
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
		printf("\n------------ PROGRAM END ------------\n\n\t-selectModeOnBoardClient Layer Cleanup\n");
		free(hpsData.c18);
		free(onBoardData);
		ENET_close(&ENETdata); 
		return(0);
	}
	
	return(0);
}















