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
 

#define ENET_STOP_BIT 0
#define FPGA_TRAN_BIT 1
#define FPGA_ACQN_BIT 2
  
//~ #define ARRAY_LEN 1024 

#define HW_REGS_BASE ( ALT_STM_OFST )  
#define HW_REGS_SPAN ( 0x04000000 )   
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )  
//~ #define PORTNUM 3333  
 
 
uint32_t State = 0;
uint32_t usleepTime = 1;


#include "structDeclarations.h" 
#include "initializationFuncs.h"  
#include "FPGAfuncs.h"  
#include "ENETfuncs.h"
#include "stateMachineFuncs.h"   




void stateChecker(struct FPGAvars *FPGA, struct ENETcommVar *ENET, struct INITvars *initVars){ 
	State ^= (-(ENET->enetWait) ^ State) & (1 << ENET_STOP_BIT); 
	State ^= (-(*(uint32_t volatile *)FPGA->transReady) ^ State) & (1 << FPGA_TRAN_BIT);
	//~ State ^= (-(1) ^ State) & (1 << FPGA_TRAN_BIT);
	State ^= (-((initVars->acqsPerTrig-initVars->acqNum)>1) ^ State) & (1 << FPGA_ACQN_BIT);
	//~ printf("\nState = %zu \n",State);
}


int main(int argc, char *argv[]) {
	printf("into main!\n");
	 
	struct ENETcommVar enetComms; 
	printf("enetComms!\n");
	
	struct INITvars initVars;
	printf("initVars!\n");  
	
	loadVars(argc,argv,&initVars,&enetComms);
	printf("initVars LOADED!\n"); 
	
	struct FPGAvars FPGA;
	if(FPGA_init(&FPGA) == 0) return(1); 
	FPGA_stateReset(&FPGA,1);
	usleep(1);
	FPGA_stateReset(&FPGA,0);
	*(uint32_t *)FPGA.read_addr = 0;
	printf("FPGAVars LOADED!\n"); 
	   
	struct ENETvars ENET_1; 
	//~ usleep(initVars.tDelay);
	ENET_init(&ENET_1,&initVars); // server IP address
	usleep(usleepTime);   
	ENET_recv(&ENET_1,&enetComms);
	printf("ENETVars LOADED!\n"); 
		    
	struct chData hpsData; // structure to organize and store the  8channels worth of 8bit data
	hpsData.c14 = (uint32_t *)malloc( initVars.acqsPerTrig*initVars.dataLen*sizeof(uint32_t));
	hpsData.c58 = (uint32_t *)malloc( initVars.acqsPerTrig*initVars.dataLen*sizeof(uint32_t));
	
	
	STATE_ENETfpga(&ENET_1, &enetComms, &FPGA, &hpsData, &initVars);

	int cnt = 0;
	
	while(enetComms.runner == 1){ 
		usleep(usleepTime); 
		stateChecker(&FPGA, &enetComms, &initVars); 
		cnt++;
	
		switch(State){
			
			case(0): { // no more acqs for this transmit cycle, fpga not ready to transmit, no input from server
				//~ if(cnt%500==0){printf("case0: (cnt %d, stateVal %d) %d %d %d %d, %d\n", cnt, *(uint32_t *)FPGA.stateVal, enetComms.runner, enetComms.enetMSG, enetComms.val, enetComms.enetWait,*(uint32_t volatile *)FPGA.transReady);}
				break; // Do Nothing, wait until data is ready to be acquired
			} 
						 
			case(1): { // no more acqs for this transmit cycle, fpga not ready to transmit, input from server
				//~ printf("case1 %d %d %d %d\n", enetComms.runner, enetComms.enetMSG, enetComms.val, enetComms.enetWait);
				STATE_ENETfpga(&ENET_1, &enetComms, &FPGA, &hpsData, &initVars);
				break; 
			}       
			 
			case(2): { // no more acqs for this transmit cycle, fpga ready to transmit, no input from server
				//~ printf("transReady = %d\n",*(uint32_t volatile *)FPGA.transReady);
				//~ if(cnt%50==0){printf("case2 %d (trigNum %d) %d %d %d %d\n", cnt, *(uint32_t *)FPGA.trigCntr, enetComms.runner, enetComms.enetMSG, enetComms.val, enetComms.enetWait);}
				//~ usleep(initVars.tDelay);
				FPGA_getData(&FPGA, &hpsData, &initVars);
				usleep(usleepTime);
				ENET_send(&ENET_1, &hpsData, &initVars);  
				usleep(usleepTime);
				ENET_recv(&ENET_1,&enetComms);
				usleep(usleepTime);
				STATE_ENETfpga(&ENET_1, &enetComms, &FPGA, &hpsData, &initVars);

				initVars.acqNum = 0;	 
				break;			
			}	
			
			case(3): { // no more acqs for this transmit cycle, fpga ready to transmit, input from server
				//~ if(cnt%1==0){ printf("case3\n"); }
				STATE_ENETfpga(&ENET_1,&enetComms,&FPGA, &hpsData, &initVars);
				break;	
			}
			
			case(4): { // more acqs for this transmit cycle, fpga not ready to transmit, no input from server
				//~ printf("case4\n");
				break; // Do Nothing, wait until data is ready to be acquired
			}
			
			case(5): { // more acqs for this transmit cycle, fpga not ready to transmit, input from server
				//~ printf("case5\n");
				STATE_ENETfpga(&ENET_1, &enetComms, &FPGA, &hpsData, &initVars);
				break;
			}
			
			case(6): { // more acqs for this transmit cycle, fpga ready to transmit, no input from server
				//~ printf("case6\n");
				FPGA_getData(&FPGA, &hpsData, &initVars);
				
				initVars.acqNum++;		
				break;			
			}	
			
			case(7): { // more acqs for this transmit cycle, fpga ready to transmit, input from server
				//~ printf("case7\n");
				STATE_ENETfpga(&ENET_1,&enetComms,&FPGA, &hpsData, &initVars);
				break;
			}
				
			default: {
				printf("undefined state, ending run\n");
				enetComms.runner = 0;
				break;
			}						
		}
		//~ printf("\nWHILE END\n");
		usleep(usleepTime); 
	}
	printf("\nPROGRAM END\n");
	free(hpsData.c14); free(hpsData.c58);
	ENET_read(&ENET_1,&enetComms);
	FPGA_close(&FPGA);
	ENET_close(&ENET_1); 
		

	return( 0 );
}
