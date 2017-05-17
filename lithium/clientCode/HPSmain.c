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
#define MAIZE_STATE_BIT 3
 
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


void stateChecker(struct FPGAvars *FPGA, struct ENETcommVar *ENET, struct SETUPvars *setupVars, int *stw){ 
	usleep(*stw);
	State ^= (-(*(int volatile *)&ENET->enetWait) ^ State) & (1 << ENET_STOP_BIT); 
	State ^= (-(*(uint32_t volatile *)FPGA->transReady) ^ State) & (1 << FPGA_TRAN_BIT);
	State ^= (-((setupVars->acqsPerTrig-setupVars->acqNum)>1) ^ State) & (1 << FPGA_ACQN_BIT);
	usleep(*stw);
}


int main(int argc, char *argv[]) { // printf("into main!\n"); 
	
	// create and initialize all variables that need to be accessible to parent and child processes	
	struct INITvars *initVars = mmap( NULL, sizeof(*initVars), ( PROT_READ | PROT_WRITE ), MAP_SHARED | MAP_ANONYMOUS, -1, 0 );
	struct ENETmsg *enetMSG = mmap( NULL, sizeof(*enetMSG), ( PROT_READ | PROT_WRITE ), MAP_SHARED | MAP_ANONYMOUS, -1, 0 );	
	struct ACQsetup *acqSetup = mmap( NULL, sizeof(*acqSetup), ( PROT_READ | PROT_WRITE ), MAP_SHARED | MAP_ANONYMOUS, -1, 0 );		
	struct MAIZEsetup *maizeSetup = mmap( NULL, sizeof(*maizeSetup), ( PROT_READ | PROT_WRITE ), MAP_SHARED | MAP_ANONYMOUS, -1, 0 );
	struct STATEvars *stateVars = mmap( NULL, sizeof(*stateVars), ( PROT_READ | PROT_WRITE ), MAP_SHARED | MAP_ANONYMOUS, -1, 0 );	
	initializeVars(argc,argv,initVars,enetMSG,acqSetup,stateVars);
	
	// initialize pipes for communications between parent and child processes
	int chld2prnt_pipe[2]; int a;
	int prnt2chld_pipe[2]; int b;
	a = pipe(chld2prnt_pipe); if (a == -1){ perror("pipe"); exit(EXIT_FAILURE); }
	b = pipe(prnt2chld_pipe); if (b == -1){ perror("pipe"); exit(EXIT_FAILURE); }	
	
	// fork the program into separate data (child) and messaging (parent) processes
	pid_t pid; 
	pid = fork(); 
	
	if( pid == 0 ){ // data client (child) - handles data acquisition stuff from the fpga	to hps
	    dataClientHandler(enetMSG, initVars, acqSetup, maizeSetup, stateVars, chld2prnt_pipe, prnt2chld_pipe);		
	} 
	
	else { // message client (parent) - listens for commands from user, always has precedence
		enetParentMessenger(enetMSG, initVars, acqSetup, maizeSetup, stateVars, chld2prnt_pipe, prnt2chld_pipe);	
	}
	
	return( 0 );
}



 


















