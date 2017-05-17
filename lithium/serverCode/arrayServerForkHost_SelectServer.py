import socket, sys
import select
import matplotlib.pyplot as plt
import struct
import numpy as np
import mmap
import os
import time
import csv
#~ from config import *

#~ os.system('bash sshIntoFPGAs.sh')
MSG_PORTS = []
HOST = ''
BASE_PORT = 3400

NFPGAS = 4
Nchannels = 8

hpsClock = 20.0 # MHz
trigDelay = 100 # us
tsleep = 0.001
recLen = 4096 # number of data points to collect
acqsPerTrig = 1

NPulses = 1
everyN = 1
cntMax = NPulses # NUMBER OF PULSES TO ACQUIRE DATA FROM

acqLen = int(np.ceil(NPulses*1.0/everyN*1.0))


# board_ips - [elements numbers]
# 116 [40 48 56 64 72 80 88 96]
# 117 [8 16 24 32 104 112 120 128]
# 115 [168 176 184 192 200 208 216 224]
# 114 [136 144 152 160 232 240 248 256]


DATA_SET = 0

def make_csv(data,dataSetN):
	
	f = open('{}{}{}'.format('dummy',dataSetN,'.csv'), 'wb')
	c = csv.writer(f,delimiter=' ')
	for n in range(0,acqLen*(recLen+1)):
		try:
			c.writerow(data[n,:])
		except:
			pass
		
	f.close()
	
	print 'Done Writing Data'

def plotData(data):
	t = np.linspace(0,recLen,recLen)/20+trigDelay
	#~ cc = struct.Struct('= 2048L')
	#~ c18 = np.array(cc.unpack(data),dtype=np.uint32)
	c18 = data[1]
	#~ print c18.shape
	c14a,c58a = c18[0:recLen],c18[recLen:]
	dt14 = np.dtype((np.uint32,{'c1':(np.uint8,0),'c2':(np.uint8,1),'c3':(np.uint8,2),'c4':(np.uint8,3)}))
	dt58 = np.dtype((np.uint32,{'c5':(np.uint8,0),'c6':(np.uint8,1),'c7':(np.uint8,2),'c8':(np.uint8,3)}))
		
	c14,c58 = c14a.view(dtype=dt14),c58a.view(dtype=dt58)
	#~ print c14.shape, c14['c1'], c14['c2'], c14['c3'], c14['c4']
	#~ print c58.shape, c58['c5'], c58['c6'], c58['c7'], c58['c8']
			
	c1,c2,c3,c4 = c14['c1'], c14['c2'], c14['c3'], c14['c4']
	c5,c6,c7,c8 = c58['c5'], c58['c6'], c58['c7'], c58['c8']
	
	#~ plt.plot(c14a)
	#~ plt.show()
	plt.title(data[0])	
	plt.plot(t,c1,t,c2+275*1,t,c3+275*2,t,c4+275*3,t,c5+275*4,t,c6+275*5,t,c7+275*6,t,c8+275*7)
	plt.show()


dataPortRun = mmap.mmap(-1, 1+1, mmap.MAP_SHARED | mmap.MAP_ANONYMOUS)
dataPortSharedMem = mmap.mmap(-1, 32+1, mmap.MAP_SHARED | mmap.MAP_ANONYMOUS)

msgPortRun = mmap.mmap(-1, 1+1, mmap.MAP_SHARED | mmap.MAP_ANONYMOUS)
msgPortSharedMem = mmap.mmap(-1, 32+1, mmap.MAP_SHARED | mmap.MAP_ANONYMOUS)

RECV_BUFFER = Nchannels*recLen*acqsPerTrig



cc = struct.Struct('{}{}{}'.format('= ',recLen*2,'L'))


class MsgServer():
	
	def sockSendMSG(self,enetWait,msg,val,enetRun):
		# enetWait = 0: tells hps to run program normally
		# enetWait = 1: tells hps to wait for incoming messages
		
		# msg = 0: do nothing
		# msg = 1: set trig delay
		# msg = 2: set record length
		# msg = 3: set acqs per trig
		# msg = 4: change fork/select client mode
		# msg = 5: set number of acquisition data sets to store on HPS
		# msg = 6+: kill program
		
		# val = integer value to give to msg
		
		# enetRun = 0: kill program
		# enetRun = 1: program runs
		msg = struct.pack('IIII',enetWait,msg,val,enetRun)

		for m in range(0,self.Mclients):
			self.commSock[m][0].send(msg)
			time.sleep(1.0e-6)
	
	def sockKillAll(self):
		
		self.sockSendMSG(0,50,0,0)
		time.sleep(10e-3)
		for m in range(0,self.Mclients):	
			self.commSock[m][0].close()

	def sockSetTrigDelay(self,trigDelay):
		self.sockWait()
		trigClockCycles = trigDelay*hpsClock
		#~ print trigClockCycles
		self.sockSendMSG(1,1,int(trigClockCycles),1) # tell hps to wait before progressing

	def sockSetRecordLength(self,recLen):
		self.sockWait()
		self.sockSendMSG(1,2,int(recLen),1) # tell hps to wait before progressing

	def sockSetAcqsPerTrig(self,acqs):
		self.sockWait()
		self.sockSendMSG(1,3,acqs,1) # tell hps to wait before progressing
		
	def sockSetServerMode(self,serverMode):
		self.sockWait()
		if serverMode < 4:
			self.sockSendMSG(1,4,serverMode,1) # tell hps to wait before progressing
		else:
			print 'Invalid serverMode value. Defaulting to 1 (Select Server)'
			self.sockSendMSG(1,4,1,1) # tell hps to wait before progressing
	
	def sockSetNumOnBoard(self,numOnBoard):
		self.sockWait()
		self.sockSendMSG(1,5,numOnBoard,1)
				
	def sockWait(self):
		self.sockSendMSG(1,0,0,1)

	def sockRun(self):
		self.sockWait()
		self.sockSendMSG(0,0,0,1)
	
	def SetupMsgPorts(self,msgPort):
		msgSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		msgSock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
		msgSock.setsockopt(socket.IPPROTO_TCP,socket.TCP_NODELAY,1)
		msgSock.bind((HOST,msgPort))
		msgSock.listen(1)
		sockfd,addr = msgSock.accept()		
		return [sockfd,addr]
	
	def __init__(self,Mclients):
		
		self.Mclients = Mclients
		self.commSock = []
		portStart = 0+BASE_PORT 
					
		for n in range(0,2*Mclients,2):
			print "message Port listening on", portStart+n+1
			self.commSock.append(self.SetupMsgPorts(portStart+n+1))



el_array = np.zeros((32,2))
class DataServer():
	
	def setupSocket(self,dataPort):
		dataSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		dataSock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
		dataSock.setsockopt(socket.IPPROTO_TCP,socket.TCP_NODELAY,1)
		dataSock.bind((HOST,dataPort))
		dataSock.listen(10)
	
		return dataSock
	
	def forkDataServer(self,serverN,portStart,portRange):
		prnt2chld_pipe = os.pipe()
		chld2prnt_pipe = os.pipe()
		
		pid = os.fork()
		if pid == 0:
			dt14 = np.dtype((np.uint32,{'c1':(np.uint8,0),'c2':(np.uint8,1),'c3':(np.uint8,2),'c4':(np.uint8,3)}))
			cpid = os.getpid()
			os.close(chld2prnt_pipe[0])
			os.close(prnt2chld_pipe[1])
			
			self.dataSocks = []
			self.activeDataSocks = []
			self.dataStore = []
			
			dataRunner = '1'
			
			for portNum in range(portStart,portStart+portRange,2):
				self.dataSocks.append(self.setupSocket(portNum))
				
			clientCnt = portRange/2
			cnt,cnt1,cnt2 = 0,0,0
			
			c184 = np.zeros((acqLen*(recLen+1),32))
			dataTmp = np.zeros((recLen+1,32))
			while cnt<cntMax:
				
				readableSocks,wrSock,erSock = select.select(self.dataSocks, [], [])
				
				for rdSock in readableSocks:
					for sock in self.dataSocks:
						if rdSock == sock:
							cliSock, addr = sock.accept()
							ad1 = int(addr[0].split('.')[3])
							
							data = cliSock.recv(RECV_BUFFER,socket.MSG_WAITALL)
							
							
							drrr = np.array(cc.unpack(data),dtype=np.uint32)									
							c18 = drrr.view(dtype=dt14)				
							c1,c2,c3,c4 = c18['c1'], c18['c2'], c18['c3'], c18['c4']
							
							dataTmp[0,cnt1*8:(cnt1*8+7)] = ad1 
									
							dataTmp[1:,cnt1*8] = c1[0:len(drrr)/2]
							dataTmp[1:,cnt1*8+1] = c2[0:len(drrr)/2]
							dataTmp[1:,cnt1*8+2] = c3[0:len(drrr)/2]
							dataTmp[1:,cnt1*8+3] = c4[0:len(drrr)/2]
							dataTmp[1:,cnt1*8+4] = c1[len(drrr)/2:]
							dataTmp[1:,cnt1*8+5] = c2[len(drrr)/2:]
							dataTmp[1:,cnt1*8+6] = c3[len(drrr)/2:]
							dataTmp[1:,cnt1*8+7] = c4[len(drrr)/2:]
							print cnt2, ad1
							#~ for m in range(0,8):
								#~ plt.plot(dataTmp[1:,cnt1*8+m])
								#~ plt.title(m)
								#~ plt.show()
								
							cliSock.close()
							
							cnt1+=1
							if cnt1 == 4:
								#~ if np.mod(cnt,everyN*80) < 80:
								#~ try:
								c184[cnt2*(recLen+1):(cnt2+1)*(recLen+1),:] = dataTmp
								#~ except:
									#~ pass
									
								cnt2+=1
									#~ print cnt
									
								dataTmp = np.zeros((recLen+1,32))
								cnt1=0
								cnt+=1
								
				

									
				
			
			make_csv(c184,DATA_SET)				
							
			#~ if cnt == 50:	
			
			#~ print 'Pulse Number:', cnt, 'selectLoop Time -',np.round((time.time()-t0),2)
			#~ for g in range(0,4):
				#~ plotData([g,c184[g]])		
			os.close(chld2prnt_pipe[1])
			os.close(prnt2chld_pipe[0])
			os._exit(0)
								
		else:
			os.close(chld2prnt_pipe[0])
			os.close(prnt2chld_pipe[1])
			
			return chld2prnt_pipe[1], prnt2chld_pipe[0]
			
	def __init__(self,Nservers,Mclients):
		
		self.comms = []
		self.runner = 1
		portStart = 0+BASE_PORT 
		
		if np.mod(Mclients,Nservers) == 0:
			portRange = Mclients/Nservers		
			for m in range(0,Nservers):				
				chld2prnt_pipe, prnt2chld_pipe = self.forkDataServer(m,portStart,2*portRange)
				portStart+=2*portRange
						
		else:
			portRange = np.ceil(Mclients/Nservers)
			oddClients = np.mod(Mclients,portRange)
			
			for m in range(0,Nservers):
				if m<oddClients:
					chld2prnt_pipe, prnt2chld_pipe = self.forkDataServer(m,portStart,portRange)
					portStart+=2*portRange
				else:
					chld2prnt_pipe, prnt2chld_pipe = self.forkDataServer(m,portStart,portRange-1)
					portStart+=2*(portRange-1)



Nforks = 1
Nclients = 4					

DS = DataServer(Nforks,Nclients)
MS = MsgServer(Nclients)

		
MS.sockWait()	
		
raw_input("System Waiting. Press enter to continue...")
MS.sockWait()
#~ MS.sockSetServerMode(1)
#~ MS.sockSetNumOnBoard(80*1e5)
MS.sockSetTrigDelay(trigDelay)
MS.sockSetRecordLength(recLen)
MS.sockRun()

raw_input("press enter to kill it dead...\n")
MS.sockWait()
MS.sockKillAll()			

raw_input("just hanging out...\n")
pid = os.getpid()
pgid = os.getpgid(pid)
os.killpg(pgid,9)



