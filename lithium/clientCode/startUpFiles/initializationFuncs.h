
void initializeVars(int argc, char *argv[], struct INITvars *initVars, struct ENETmsg *enetMSG, struct ACQsetup *acqSetup, struct MAIZEsetup *maizeVars, struct STATEvars *stateVars){
		
	initVars->serverIP = "192.168.1.101";
	initVars->localIP = NULL;
	
	FILE * fp;
    char * line = NULL;
    size_t len = 0;
    fp = fopen("/home/root/boardSetup", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    getline(&line, &len, fp); initVars->boardNumber = atoi(line);
    getline(&line, &len, fp); initVars->msgPort = atoi(line);
    getline(&line, &len, fp); initVars->dataPort = atoi(line);
    fclose(fp);
    
	enetMSG->enetWait = 1;
	enetMSG->runner = 1;
	
	acqSetup->setupState = 0;
	acqSetup->acqsPerTrig = 1;
	acqSetup->acqNum = 0;
	acqSetup->recordLength = 1024;
	acqSetup->trigDelay = 0;
	acqSetup->forkSelect = 1; // defaults to select server
	acqSetup->numOnBoardAcqs = 1; // defaults to select server
	acqSetup->onBoardCounter = 0;
	acqSetup->enetSendDelay = 0;
	
	maizeVars->aCommLen = 1;
	maizeVars.aCommands = (uint32_t *)malloc( maizeVars->aCommLen*sizeof(uint32_t));
	
	maizeVars->bMemLocs = 1;
	maizeVars.bArray = (uint32_t *)malloc( maizeVars->bMemLocs*sizeof(uint32_t));
	
	stateVars->setupState = 0;
	
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
				acqSetup->acqsPerTrig = atoi(optarg);
				printf("\nAcquisitions Per Trigger = %d\n", acqSetup->acqsPerTrig);
				break;
				
			case 'r': //acquisitions per trigger
				acqSetup->recordLength = atoi(optarg);
				printf("\nData Length = %d\n", acqSetup->recordLength);
				break;
				
			case 't': //initial trigger delay
				acqSetup->trigDelay = atoi(optarg);
				printf("\nTrigger Delay = %d\n", acqSetup->trigDelay);
				break;
				
			case 'f': //server mode, fork/select [0/1]
				acqSetup->forkSelect = atoi(optarg);
				if( acqSetup->forkSelect == 0 ){
					printf("Operating in fully forked server mode\n");
				} else {
					acqSetup->forkSelect = 1;
					printf("Operating in select server mode\n");
				}
				break;
				
			case 'e': //initial trigger delay
				acqSetup->enetSendDelay = atoi(optarg);
				printf("\nENET Transmit Delay = %d\n", acqSetup->enetSendDelay);
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







