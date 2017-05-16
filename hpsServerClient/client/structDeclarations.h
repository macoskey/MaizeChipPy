
struct INITvars{
	const char *serverIP;
	const char *localIP;
	int dataPort;
	int msgPort;
};


struct SETUPvars{	
	int volatile acqsPerTrig;
	int volatile acqNum;
	int volatile recordLength;
	int volatile trigDelay;
	int volatile enetSendDelay;
	int volatile forkSelect;
	int volatile numOnBoardAcqs;
	int volatile onBoardCounter;
};


struct chData{
	uint32_t *c18;
};


struct ENETcommVar{
	int enetWait;
	int enetMSG;
	int val;
	int runner;
};


struct ENETvars{
	int server_sockid;
	int server_sockmsg;
	const char *server_addr;
	struct sockaddr_in server_sockaddr;
};


struct FPGAvars{
	
	void *virtual_base; // base address all others addresses start at
	int fd; // memory file where registers values are mapped to
	
	// pointer to the register where LED is controlled from
	void *led_addr; 
	
	unsigned long volatile *read_addr; 
	
	unsigned long volatile *gpio0_addr; 
	unsigned long volatile *gpio1_addr;
	
	unsigned long volatile *transReady;
	unsigned long volatile *trigDelay;
	unsigned long volatile *recordLength;
	unsigned long volatile *stateReset;
	unsigned long volatile *trigCntr;
	unsigned long volatile *stateVal;
	
	 
};
