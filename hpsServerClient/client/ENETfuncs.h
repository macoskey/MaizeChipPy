
void ENET_connect(struct ENETvars *ENET){
	ENET->server_sockid = socket(AF_INET, SOCK_STREAM, 0);
	connect(ENET->server_sockid, (struct sockaddr *)&ENET->server_sockaddr, sizeof(ENET->server_sockaddr));
}

void ENET_close(struct ENETvars *ENET){
	close(ENET->server_sockid);
}

void ENET_DataInitSelect(struct ENETvars *ENET, struct INITvars *initVars){
	
	ENET->server_addr = initVars->serverIP;		
	ENET->server_sockaddr.sin_port = htons(initVars->dataPort);
	ENET->server_sockaddr.sin_family = AF_INET;
	ENET->server_sockaddr.sin_addr.s_addr = inet_addr(ENET->server_addr);	
}

void ENET_send(struct ENETvars *ENET, struct chData *hpsData, struct SETUPvars *setupVars){
	
	if(send(ENET->server_sockid, hpsData->c18, 2*setupVars->acqsPerTrig*setupVars->recordLength*sizeof(uint32_t) , MSG_CONFIRM) < 0)
		printf("%d NO SEND2!!!!\n",ENET->server_sockid);
}

void ENET_selectSend(struct ENETvars *ENET, struct chData *hpsData, struct SETUPvars *setupVars){

	ENET_connect(ENET);	
	if(send(ENET->server_sockid, hpsData->c18, 2*setupVars->acqsPerTrig*setupVars->recordLength*sizeof(uint32_t) , MSG_CONFIRM) < 0)
		printf("%d NO SEND2!!!!\n",ENET->server_sockid);
	ENET_close(ENET);	
}

void ENET_read(struct ENETvars *ENET, struct ENETcommVar *enetComms){
	recv(ENET->server_sockid, enetComms , sizeof(struct ENETcommVar),MSG_DONTWAIT);
}


void ENET_recv(struct ENETvars *ENET, struct ENETcommVar *enetComms, int chld2prnt_pipe){
	recv(ENET->server_sockid, enetComms, sizeof(struct ENETcommVar),MSG_WAITALL);
	//~ printf("enetRecv - Wait Val: %d \n", enetComms->enetWait);
	
	write(chld2prnt_pipe,enetComms,sizeof(enetComms));
	//~ usleep(100);
}


void ENET_DataInit(struct ENETvars *ENET, struct INITvars *initVars){
	printf("\ndataInit\n");
	struct timeval t0,t1;
	int diff; 
	ENET->server_addr = initVars->serverIP;
		

	ENET->server_sockid = socket(AF_INET, SOCK_STREAM, 0);
		
	ENET->server_sockaddr.sin_port = htons(initVars->dataPort);
	ENET->server_sockaddr.sin_family = AF_INET;
	ENET->server_sockaddr.sin_addr.s_addr = inet_addr(ENET->server_addr);
	
	
	gettimeofday(&t0,NULL);
	
	if(connect(ENET->server_sockid, (struct sockaddr *)&ENET->server_sockaddr, sizeof(ENET->server_sockaddr))  == -1)		
		while(connect(ENET->server_sockid, (struct sockaddr *)&ENET->server_sockaddr, sizeof(ENET->server_sockaddr))  == -1){
			gettimeofday(&t1,NULL);
			diff = (t1.tv_sec-t0.tv_sec);
			if(diff>(connectionTimeOut*60)){
				printf("NO CONNECT!!!!\n");
				break;
			}	
		}
	
}



void ENET_MsgInit(struct ENETvars *ENET, struct INITvars *initVars){
	printf("\nmsgInit\n");
	struct timeval t0,t1;
	int diff; 
	ENET->server_addr = initVars->serverIP;

	ENET->server_sockid = socket(AF_INET, SOCK_STREAM, 0);
		
	ENET->server_sockaddr.sin_port = htons(initVars->msgPort);
	ENET->server_sockaddr.sin_family = AF_INET;
	ENET->server_sockaddr.sin_addr.s_addr = inet_addr(ENET->server_addr);
	
	
	gettimeofday(&t0,NULL);
	
	if(connect(ENET->server_sockid, (struct sockaddr *)&ENET->server_sockaddr, sizeof(ENET->server_sockaddr))  == -1)		
		while(connect(ENET->server_sockid, (struct sockaddr *)&ENET->server_sockaddr, sizeof(ENET->server_sockaddr))  == -1){
			gettimeofday(&t1,NULL);
			diff = (t1.tv_sec-t0.tv_sec);
			if(diff>(connectionTimeOut*60)){
				printf("NO CONNECT!!!!\n");
				break;
			}	
		}
	
}


