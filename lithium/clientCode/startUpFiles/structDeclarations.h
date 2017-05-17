
struct INITvars{
	const char *serverIP;
	const char *localIP;
	int boardNumber;
	int dataPort;
	int msgPort;
	int pid;
};

struct STATEvars{
	int volatile setupState;
};

struct ACQsetup{	
	int volatile acqsPerTrig;
	int volatile acqNum;
	int volatile recordLength;
	int volatile trigDelay;
	int volatile enetSendDelay;
	int volatile forkSelect;
	int volatile numOnBoardAcqs;
	int volatile onBoardCounter;
};

struct MAIZEsetup{	
	uint32_t aCommLen;
	uint32_t *aCommands;
	
	uint32_t bMemLocs;
	uint32_t *bArray;
};


struct chData{
	uint32_t *c18;
};


struct ENETmsg{
	int enetWait;
	int enetState;
	int enetMSG;
	int val;
	int runner;
};


struct ENETsock{
	int server_sockid;
	int server_sockmsg;
	const char *serverIP;
	struct sockaddr_in server_sockaddr;
};


struct FPGAvars{
	
	void *virtual_base; // base address all others addresses start at
	int fd; // memory file where registers values are mapped to
	
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
