
void enetConnect(struct ENETsock *ENET){
	ENET->server_sockid = socket(AF_INET, SOCK_STREAM, 0);
	connect(ENET->server_sockid, (struct sockaddr *)&ENET->server_sockaddr, sizeof(ENET->server_sockaddr));
}


void enetClose(struct ENETsock *ENET){
	close(ENET->server_sockid);
}


void enetSend(struct ENETsock *ENET, struct chData *hpsData, struct ACQsetup *acqVars){	
	if(send(ENET->server_sockid, hpsData->c18, 2*setupVars->acqsPerTrig*setupVars->recordLength*sizeof(uint32_t) , MSG_CONFIRM) < 0)
		printf("%d NO SEND2!!!!\n",ENET->server_sockid);
}


void enetSelectSend(struct ENETsock *ENET, struct chData *hpsData, struct ACQsetup *acqVars){
	enetConnect(ENET);	
	enetSend(ENET,hpsData,acqVars);
	enetClose(ENET);
}


void enetRead(struct ENETsock *ENET, struct ENETmsg *enetComms){
	recv(ENET->server_sockid, enetComms , sizeof(struct ENETcommVar),MSG_DONTWAIT);
}


void enetRecv(struct ENETsock *ENET, struct ENETmsg *enetMSG, int chld2prnt_pipe){
	recv(ENET->server_sockid, enetMSG, sizeof(struct ENETmsg),MSG_WAITALL);
}


void enetMakeChildSocket(struct ENETsock *ENET, struct INITvars *initVars){	
	ENET->server_addr = initVars->serverIP;		
	ENET->server_sockaddr.sin_port = htons(initVars->dataPort);
	ENET->server_sockaddr.sin_family = AF_INET;
	ENET->server_sockaddr.sin_addr.s_addr = inet_addr(ENET->server_addr);	
}


void enetMakeParentSocket(struct ENETsock *ENET, struct INITvars *initVars){	
	ENET->server_addr = initVars->serverIP;		
	ENET->server_sockaddr.sin_port = htons(initVars->msgPort);
	ENET->server_sockaddr.sin_family = AF_INET;
	ENET->server_sockaddr.sin_addr.s_addr = inet_addr(ENET->server_addr);	
}


void ENET_DataInit(struct ENETSocketVars *ENET, struct INITvars *initVars){
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







