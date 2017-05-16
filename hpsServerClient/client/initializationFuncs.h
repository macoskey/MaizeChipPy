
void loadVars(int argc, char *argv[], struct INITvars *initVars, struct ENETcommVar *enetComms, struct SETUPvars *setupVars){
		
	initVars->serverIP = NULL;
	initVars->localIP = NULL;
	initVars->dataPort = 3300;
	initVars->msgPort = 3301;
	
	setupVars->acqsPerTrig = 1;
	setupVars->acqNum = 0;
	setupVars->recordLength = 1024;
	setupVars->trigDelay = 0;
	setupVars->forkSelect = 1; // defaults to select server
	setupVars->numOnBoardAcqs = 1; // defaults to select server
	setupVars->onBoardCounter = 0;
	setupVars->enetSendDelay = 0;
	
	enetComms->enetWait = 1;
	enetComms->runner = 1;
	
	int opt = 0;
		
	while ((opt = getopt(argc, argv, "s:l:p:m:a:d:t:f:e:")) != -1) {
		switch(opt) {
			case 's': //server IP address
				initVars->serverIP = optarg;
				printf("\nServerIP = %s\n", initVars->serverIP);
				break;
				
			case 'l': //local IP address
				initVars->localIP = optarg;
				printf("\nLocalIP = %s\n", initVars->localIP);
				break;
				
			case 'd': //data Port
				initVars->dataPort = atoi(optarg);
				printf("\ndata Port = %d\n", initVars->dataPort);
				break;
				
			case 'm': //message Port
				initVars->msgPort = atoi(optarg);
				printf("\nmsg Port = %d\n", initVars->msgPort);
				break;
				
			case 'n': //acquisitions per trigger
				setupVars->acqsPerTrig = atoi(optarg);
				printf("\nAcquisitions Per Trigger = %d\n", setupVars->acqsPerTrig);
				break;
				
			case 'r': //acquisitions per trigger
				setupVars->recordLength = atoi(optarg);
				printf("\nData Length = %d\n", setupVars->recordLength);
				break;
				
			case 't': //initial trigger delay
				setupVars->trigDelay = atoi(optarg);
				printf("\nTrigger Delay = %d\n", setupVars->trigDelay);
				break;
				
			case 'f': //server mode, fork/select [0/1]
				setupVars->forkSelect = atoi(optarg);
				if( setupVars->forkSelect == 0 ){
					printf("Operating in fully forked server mode\n");
				} else {
					setupVars->forkSelect = 1;
					printf("Operating in select server mode\n");
				}
				break;
				
			case 'e': //initial trigger delay
				setupVars->enetSendDelay = atoi(optarg);
				printf("\nENET Transmit Delay = %d\n", setupVars->enetSendDelay);
				break;
					
			case '?':
				if (optopt == 's') {
					printf("\nMissing server IP");
				} else if (optopt == 'l') {
					 printf("\nMissing local IP");
				} else if (optopt == 'd') {
					 printf("\nMissing Data Port. Defaults to 3333");
				} else if (optopt == 'm') {
					 printf("\nMissing Message Port. Defaults to 3334");
				} else if (optopt == 'n') {
					 printf("\nAcquisitions Per Trigger not set. Defaults to 1");
				} else if (optopt == 'r') {
					 printf("\nRecord Length not set. Defaults to 1024");
				} else if (optopt == 't') {
					 printf("\nTrigger Delay not set default to 0");
				} else {
					 printf("\nInvalid option received");
				}
			break;
		}
	}
}







