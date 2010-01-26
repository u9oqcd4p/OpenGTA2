#include "opengta_sv.h"

#ifdef _WIN32
	#undef WINGDIAPI
	#undef APIENTRY
	#include <winsock2.h>
#else
	#include <errno.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>

	#include <unistd.h>
#endif

Network_Manager Network;

#define NETWORK_PROTOCOL_VERSION	1

void netCallback_SERVERINFO(Network_Connection* Connection, Network_Message* Msg) {
	//Format
	//[0][1] Message type:
	//           0: Protocol version check
	//           1: Short server information query
	//           2: Extended server information query
	//         100: Protocol version reply
	//         101: Short server information reply
	//         102: Extended server information reply
	
	int msgType = Msg->ReadByte();
	switch (msgType) {
		case 0:
			Network_Message* replyMsg;
			replyMsg = Connection->NewMessage();
			replyMsg->Start(NETMESSAGE_SERVERINFO);
				replyMsg->SendByte(100);
				replyMsg->SendInt(NETWORK_PROTOCOL_VERSION); //protocol version
				replyMsg->SendInt(2); //game version
			replyMsg->End();
		break;
		case 100:
			if (!Network.IsServer) {
				int protocolVersion = Msg->ReadInt();
				if (protocolVersion != NETWORK_PROTOCOL_VERSION) {
					logWrite("Network error: protocol version mismatch (expected %d, got %d)",
						NETWORK_PROTOCOL_VERSION, protocolVersion);
					Connection->Close();
				}
			}
		break;
	}
}

void netCallback_TIMESYNC(Network_Connection* Connection, Network_Message* Msg) {
	//Format:
	//[0][4] Time (32-bit float)
	if ((!Network.IsServer) && (Network.prevTimeSyncMessageTime > 0) && (Network.IsConnected)) {
		if (Network.prevTimeSyncMessageTime > 0) { //Expect server reply
			float packetLatency = (ServerTimer.Time() - Network.prevTimeSyncMessageTime) / 2;
			float serverTime = Msg->ReadFloat() + packetLatency;

			//Adjust internal clock
			float timeDelta = serverTime - ServerTimer.Time();
			if (Network.remainingTimeSyncMsgs > 0) {
				Network.remainingTimeSyncMsgs--;
				ServerTimer.starttime -= timeDelta;
			}

			Network.remoteServerFPS = Msg->ReadFloat();
			Network.packetLatency = packetLatency; //for statistics
			Network.prevTimeSyncMessageTime = 0;
		}
	} else {
		if (Network.IsServer) {
			Network_Message* timeMsg = Connection->NewMessage();

			timeMsg->Start(NETMESSAGE_TIMESYNC);
				timeMsg->SendFloat(ServerTimer.Time());
				timeMsg->SendFloat(ServerTimer.FPS());
			timeMsg->End();
	
			//logWritem("Time synchronization packet: %.5f sec",Time);
		}
	}
}

void cvMaxMessageSize_Write(int numParams, int* paramType, int* params) { Network.maxMessageSize = CVAR_GET_INT(0); }
int cvMaxMessageSize_Read(int numParams, int* paramType, int* params) { CVAR_SET_INT(0,Network.maxMessageSize); return 1; }

void cvMaxConnections_Write(int numParams, int* paramType, int* params) { 
	uint numConnections = (uint)CVAR_GET_INT(0);
	if (numConnections > Network.Connections.AllocCount) {
		Network.Connections.Reallocate(numConnections);
	} else {
		logError("Can't change number of connections: too much connections allocated");
	}
}
int cvMaxConnections_Read(int numParams, int* paramType, int* params) { 
	CVAR_SET_INT(0,Network.Connections.AllocCount); 
	return 1; 
}

void Network_Manager::Initialize() {
	logWrite("Initializing networking...");
	Available = true;

	#ifdef _WIN32
	    WSADATA wsaData;
	    WORD wVersionRequested = MAKEWORD(2, 0);

		int result = WSAStartup(wVersionRequested, &wsaData);
	    if (result != 0) {
			logWrite("Win32 networking failed (error %d)",result);
			Available = false;
		} else {
			logWritem("network: %s",wsaData.szDescription);
		}
	#else
		logWritem("network: unix sockets");
	#endif

	Connections.Preallocate(256);
	maxMessageSize = 1024;

	//Convars
	cvMaxMessageSize = Convar.Create("net_maxsize",cvMaxMessageSize_Read,cvMaxMessageSize_Write);
	cvMaxConnections = Convar.Create("net_maxconnections",cvMaxConnections_Read,cvMaxConnections_Write);
	cvSendQueueSize = Convar.Create("net_send_queue",128);
	cvTimeRate = Convar.Create("net_timerate",0.1f);
	cvFakeLag = Convar.Create("net_fakelag",0.0f);

	remainingTimeSyncMsgs = 0;
	prevTimeSyncMessageTime = 0;

	IsServer = false;
	IsConnected = false;

	networkCallback[NETMESSAGE_SERVERINFO] = netCallback_SERVERINFO;
	networkCallback[NETMESSAGE_TIMESYNC] = netCallback_TIMESYNC;

	//Initialize stats
	numBytesSent = 0;
	numBytesReceived = 0;
	numPacketsSent = 0;
	numPacketsReceived = 0;

	prevTimeSyncTick = 0;
	msgSendLock = 0;
};

void Network_Manager::Deinitialize() {
	for (uint i = 0; i < Connections.Count; i++) {
		if (Connections[i]->socketHandle >= 0) Connections[i]->Close();
	}
	Connections.Release();
}

float testTime;
void Network_Manager::Frame() {
	//Send delayed packets
		LockID lockID = Thread.EnterLock(MUTEX_NETWORK_SENDBUF);
		for (uint i = 0; i < Connections.Count; i++) {
			if (Connections[i]->connectionOpen) {
				for (uint j = 0; j < Connections[i]->sendQueue.AllocCount; j++) {
					if ((Connections[i]->sendQueue[j]->networkConnection) &&
						(Connections[i]->sendQueue[j]->sendTime > 0.0f) &&
						(curtime() > Connections[i]->sendQueue[j]->sendTime)) {
						Connections[i]->sendQueue[j]->doSendData();
					}
				}
			}
		}
	Thread.LeaveLock(lockID);

	//Update all connections
	for (uint i = 0; i < Connections.Count; i++) {
		if ((Connections[i]->socketHandle < 0) && (Connections[i]->connectionOpen)) {
			//Free up resources
			Connections[i]->Close();
			if (i == 0) {
					IsConnected = false;
					IsServer = false;
			}

			//Destroy client connections
			for (uint j = 0; j < Clients.List.AllocCount; j++) {
				if (Clients[j]->Connection == Connections[i]) {
					Clients[j]->Connection = 0;
				}
			}
		} else Connections[i]->Update();
	}

	//Check if we disconnected
	if (!IsServer) {
		if (Connections[0]->socketHandle < 0) IsConnected = false;
	}

	//Synchronize server time/get ping
	if ((!IsServer) && (IsConnected) && (curtime() - prevTimeSyncTick > (Convar.GetFloat(cvTimeRate)))) {
		prevTimeSyncTick = curtime();
		if (prevTimeSyncMessageTime <= 0) { //Make a new network request to server
			Network_Message* msg = Connections[0]->NewMessage();

			msg->Start(NETMESSAGE_TIMESYNC);
				msg->SendFloat(ServerTimer.Time());
			msg->End();

			prevTimeSyncMessageTime = ServerTimer.Time();
		}
	}

	/*if (Connections[1]->socketHandle >= 0) {
		if (Timer.Time() - testTime > 0.25f) {
			testTime = Timer.Time();
			char buf[256];
			sprintf(buf,"Time: %.3f\r",Timer.Time());

			Network_Message* msg = Connections[1]->NewMessage();
			msg->Start(100);
				msg->SendString(buf);
			msg->End();
		}
	}*/
}

Network_Connection* Network_Manager::NewConnection() {
	Network_Connection* Connection = 0;
	for (uint i = 0; i < Connections.Count; i++) {
		if ((Connections[i]->socketHandle < 0) && (!Connections[i]->connectionOpen)) {
			Connection = Connections[i];
			break;
		}
	}

	if (!Connection) Connection = Connections.Add();
	if (!Connection) logError("Out of free connections");
	return Connection;
}

void Network_Manager::StartServer(int port) {
	if (Connections.Count > 0) {
		//Close connections?
		logWrite("FIXME: A739C");
	}

	logWrite("Starting game server on port %d",port);
	Network_Connection* ServerConnection = NewConnection();
	ServerConnection->socketHandle = -1;
	if (!ServerConnection->Open(0, port)) {
		#ifdef WIN32
			logWrite("Error while starting game server! (Code: %d)",GetLastError());
		#else
			logWrite("Error while starting game server! (Code: %d)",errno);
		#endif
		IsServer = false;
		IsConnected = false;
		return;
	}

	Game.IsHostingGame = true;
	IsServer = true;
	IsConnected = false;
}

void Network_Manager::ConnectToServer(char* host, int port) {
	logWrite("Connecting to server %s:%d",host,port);
	Network_Connection* Connection = NewConnection();
	Connection->socketHandle = -1;
	if (!Connection->Open(host, port)) {
		#ifdef WIN32
			logWrite("Error while connecting to game server! (Code: %d)",GetLastError());
		#else
			logWrite("Error while connecting to game server! (Code: %d)",errno);
		#endif
		return;
	}
	
	Game.IsHostingGame = false;
	IsServer = false;
	IsConnected = true;

	QueryServer(0); //Check protocol version

	//while (true) {
		//Frame();
	//}
}

void Network_Manager::Disconnect() {
	if (IsConnected || IsServer) {
		Connections[0]->Close();
		if (IsServer) {
			for (uint i = 0; i < Connections.Count; i++) {
				if (Connections[i]->socketHandle >= 0) Connections[i]->Close();
			}
		}
	}
	IsConnected = false;
	IsServer = false;
}

void Network_Manager::QueryServer(int queryType) {
	if ((IsConnected || IsServer) && (queryType < 100)) {
		Network_Message* Msg = Connections[0]->NewMessage();
		Msg->Start(NETMESSAGE_SERVERINFO);
			Msg->SendByte(queryType); 
		Msg->End();
	}
}






bool Network_Connection::Open(char* host, int port) {
	if (socketHandle >= 0) Close();
	LockID lockID1 = Thread.EnterLock(MUTEX_NETWORK_SENDBUF);
	LockID lockID2 = Thread.EnterLock(MUTEX_NETWORK_RECVBUF);

	sendQueue.Preallocate(Convar[Network.cvSendQueueSize]->GetInt());

	//FIXME: message size in convar
	char* msgBuffer = (char*)mem.alloc(Network.maxMessageSize*(sendQueue.AllocCount+1));
	for (uint i = 0; i < sendQueue.AllocCount; i++) {
		sendQueue[i]->rawBuffer = (msgBuffer + Network.maxMessageSize*i);
		sendQueue[i]->rawSize = 0;
		sendQueue[i]->networkConnection = 0;
	}
	recvMessage.rawBuffer = (msgBuffer + Network.maxMessageSize*sendQueue.AllocCount);
	recvMessage.rawSize = 0;
	recvMessage.rawPos = 0;
	recvMessage.networkConnection = 0;
	connectionOpen = true;

	//Only initialize internals
	if ((!host) && (!port)) {
		Thread.LeaveLock(lockID1);
		Thread.LeaveLock(lockID2);
		return true;
	}

	peerAddress.sin_family = AF_INET;
	peerAddress.sin_port = htons(port);
	if (host) {
		peerAddress.sin_addr.s_addr = inet_addr(host);
		if (peerAddress.sin_addr.s_addr == (unsigned int)-1) {
			//struct hostent *he = gethostbyname(host);
			///if (he) {
			//	peerAddress.sin_addr.s_addr = ((struct in_addr*)*he->h_addr_list)->s_addr;
			//} else {
				Thread.LeaveLock(lockID1);
				Thread.LeaveLock(lockID2);
				Close();
				return false;
			//}
		}
	} else {
		peerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	}

	socketHandle = socket(AF_INET, SOCK_STREAM, 0);
	if (socketHandle < 0) {
		Thread.LeaveLock(lockID1);
		Thread.LeaveLock(lockID2);
		Close();
		return false;
	}

	if (!host) {
		int one = 1;
		setsockopt(socketHandle, SOL_SOCKET, SO_REUSEADDR, (char*)&one, sizeof(one));

		if (bind(socketHandle, (struct sockaddr*)&peerAddress, sizeof(peerAddress)) < 0) { 
			Thread.LeaveLock(lockID1);
			Thread.LeaveLock(lockID2);
			Close();
			return false;
		}

		if (listen(socketHandle, 20) < 0) {
			Thread.LeaveLock(lockID1);
			Thread.LeaveLock(lockID2);
			Close();
			return false;
		}

		logWritem("network: starting server on %.8x:%d (backlog %d)",
			peerAddress.sin_addr.S_un.S_addr,htons(peerAddress.sin_port),20);

		listenConnection = true;
	} else {
		if (connect(socketHandle, (struct sockaddr*)&peerAddress, sizeof(peerAddress)) < 0) { 
			Thread.LeaveLock(lockID1);
			Thread.LeaveLock(lockID2);
			Close();
			return false; 
		}
		listenConnection = false;
	}

	#ifdef WIN32
	    unsigned long int nonblock = 1;
	    ioctlsocket(socketHandle, FIONBIO, &nonblock);
	#else
	    int flags = fcntl(socketHandle, F_GETFL, 0);
	    if (0 > flags) return;
		if (0 > fcntl(socketHandle, F_SETFL, flags | O_NONBLOCK)) return;
	#endif

	Thread.LeaveLock(lockID1);
	Thread.LeaveLock(lockID2);
	return true;
}

void Network_Connection::Update() {
	if (listenConnection) {
		struct sockaddr_in fromAddress;
		memset(&fromAddress, 0, sizeof(fromAddress));
		#ifdef _WIN32
			int fromLen = sizeof(fromAddress);
		#else
			socklen_t fromLen = sizeof(fromAddress);
		#endif
	
	    if (canReceive()) {
			int clientHandle = accept(socketHandle, (struct sockaddr*)&fromAddress, &fromLen);
			if (clientHandle < 0) return;
	
			Network_Connection* clientConnection = Network.NewConnection();
			if (!clientConnection) return;
			clientConnection->Open(0,0); //Initialize resources
			clientConnection->peerAddress = fromAddress;
			clientConnection->socketHandle = clientHandle;

			logWritem("network: incoming connection %.8X:%d (socket: %d, cid %d)",
				fromAddress.sin_addr.S_un.S_addr,htons(fromAddress.sin_port),clientHandle,Network.Connections.Count-1);
		}
	} else {
		LockID lockID = Thread.EnterLock(MUTEX_NETWORK_RECVBUF);

		//A slightly hacky message receiver
		//msgSize is set to how big message currently should be
		//addSize is set after first byte was fetched, and says how much extra data needs to be fetched
		//        before msgSize will be reset from 1 (default) to actual message size (excluding additional data)
		int readSize = 3;
		while (canReceive() || (recvMessage.rawSize >= readSize))  {
			if (recvMessage.rawSize > 0) { //Message already carries message ID
				unsigned short msgSize = *(unsigned short*)((char*)recvMessage.rawBuffer+0);
				unsigned char msgID = *((char*)recvMessage.rawBuffer+2);
				if (msgSize > Network.maxMessageSize) msgSize = Network.maxMessageSize;
				readSize = msgSize;
				if (readSize == 0) return; //FIXME

				if (recvMessage.rawSize >= readSize) {
					if ((msgID < NETMESSAGE_LAST) && (Network.networkCallback[msgID])) {
						Network.numPacketsReceived++;
						recvMessage.rawPos = 3; //Skip message ID and size
						Network.networkCallback[msgID](this,&recvMessage);
						recvMessage.rawSize = 0;
						readSize = 3; //get ready to recv more messages
					} else { //Unhandled message, ignore
						recvMessage.rawSize = 0;
					}
				}
			}
			//read more bytes to the message
			if (recvMessage.rawSize < readSize) {
				recvMessage.rawSize += Recv((recvMessage.rawBuffer+recvMessage.rawSize),readSize-recvMessage.rawSize);
			}
		}

		Thread.LeaveLock(lockID);
	}
}







bool Network_Connection::canReceive() {
    fd_set set;
    struct timeval tv;

    FD_ZERO(&set);
#ifdef WIN32
    FD_SET((unsigned)socketHandle, &set);
#else
    FD_SET(socketHandle, &set);
#endif

    memset(&tv, 0, sizeof(struct timeval));

    int res = select(socketHandle + 1, &set, NULL, NULL, &tv);
    return (1 == res);
}

int Network_Connection::Send(void* buf, int size) {
	if (socketHandle >= 0) {
		int bytes = send(socketHandle, (char*)buf, size, 0);
		if (bytes < 0) {
			#ifdef WIN32
			logWritem("network: socket write error %d, dropping (socket: %d)", GetLastError(), socketHandle);
			#else
			logWritem("network: socket write error %d, dropping (socket: %d)", errno, socketHandle);
			#endif
			Close();
			return 0;
		}
		Network.numBytesSent += bytes;
		return bytes;
	}
	return 0;
}

int Network_Connection::Recv(void* buf, int size) {
	if (socketHandle >= 0) {
		if (!canReceive()) return 0;

		#ifdef WIN32
			int bytes = recv(socketHandle, (char*)buf, size, 0);
		#else
			int bytes = recv(socketHandle, buf, size, MSG_NOSIGNAL | MSG_DONTWAIT);
		#endif

		if (bytes <= 0) { //FIXME: proper error handling
			#ifdef WIN32
			logWritem("network: socket read error %d, dropping (socket: %d)", GetLastError(), socketHandle);
			#else
			logWritem("network: socket read error %d, dropping (socket: %d)", errno, socketHandle);
			#endif
			bytes = 0;
			Close();
		}
		Network.numBytesReceived += bytes;
		return bytes;
	} else return -1;
}

void Network_Connection::Close() {
	#ifdef _WIN32
		if (socketHandle >= 0) {
			shutdown(socketHandle, SD_BOTH);
			closesocket(socketHandle);
		}
	#else
		if (socketHandle >= 0) {
			shutdown(socketHandle, SHUT_RDWR);
			close(socketHandle);
		}
	#endif

	socketHandle = -1;

	LockID lockID1 = Thread.EnterLock(MUTEX_NETWORK_SENDBUF);
	LockID lockID2 = Thread.EnterLock(MUTEX_NETWORK_RECVBUF);

	if (sendQueue.AllocCount > 0) {
		mem.free(sendQueue[0]->rawBuffer);
		//FIXME: right now thread sync problems cause this to crash
		//This must wait until all messages from send queue will be handled (and all callbacks)
		sendQueue.Release();
	}

	Thread.LeaveLock(lockID1);
	Thread.LeaveLock(lockID2);

	connectionOpen = false;
}

Network_Message* Network_Connection::NewMessage() {
	Network_Message* msg = 0;
	//FIXME: proper locking here
	//LockID lockID = Thread.EnterLock(MUTEX_NETWORK_CLIST);

	//Try to find message in SendQueue which isn't used anywhere
	for (uint i = 0; i < sendQueue.AllocCount; i++) {
		if (!sendQueue[i]->networkConnection) { //Invalid packet
			msg = sendQueue[i];
			break;
		}
	}

	if (!msg) {
		logError("Error: network send queue overflow");
		//Thread.LeaveLock(lockID);
		return 0;
	}

	msg->rawSize = 0;
	msg->networkConnection = this;

	//Thread.LeaveLock(lockID);
	return msg;
}

/*Network_Message* Network_Connection::ReceiveMessage(int messageID, int size) {

}*/




void Network_Message::Start(int messageID) {
	//if (Network.msgSendLock) {
		//logError("Starting new networking message without finishing old one!");
	//}
	Network.msgSendLock = Thread.EnterLock(MUTEX_NETWORK_SENDBUF);

	rawSize = 0;
	if (messageID != BAD_ID) {
		SendWord(0);
		SendByte(messageID & 0xFF);
	}
}

void Network_Message::End() {
	if (!Network.msgSendLock) {
		logError("Trying to end message without starting it!");
	}
	*(unsigned short*)(rawBuffer) = rawSize; //Message size
	Network.numPacketsSent++;
	
	float fakeLag = Convar.GetFloat(Network.cvFakeLag)/1000.0f;
	if (fakeLag > 0.0f) {
		sendTime = curtime() + fakeLag;
	} else {
		doSendData();
		sendTime = 0.0f;
	}
	Thread.LeaveLock(Network.msgSendLock);
	Network.msgSendLock = 0;
}

void Network_Message::doSendData() {
	if (networkConnection) networkConnection->Send(rawBuffer,rawSize);
	networkConnection = 0; //Invalidate this message
}


#define GEN_SEND_BYTE(name,type,size) \
	void Network_Message::name(type value) { \
		if (rawSize+size >= Network.maxMessageSize) { \
			logWritem("network: message overflow (%d bytes)",rawSize); \
			return; \
		} \
	 \
		*(type*)((char*)rawBuffer+rawSize) = value; \
		rawSize += size; \
	}

#define GEN_RECV_BYTE(name,type,size) \
	type Network_Message::name() { \
		type result = *(type*)((char*)rawBuffer+rawPos); \
		rawPos += size; \
		return result; \
	}

GEN_SEND_BYTE(SendByte,char,1)
GEN_SEND_BYTE(SendWord,short,2)
GEN_SEND_BYTE(SendInt,int,4)
GEN_SEND_BYTE(SendFloat,float,4)

GEN_RECV_BYTE(ReadByte,char,1)
GEN_RECV_BYTE(ReadWord,short,2)
GEN_RECV_BYTE(ReadInt,int,4)
GEN_RECV_BYTE(ReadFloat,float,4)

#undef GEN_SEND_BYTE
#undef GEN_RECV_BYTE

void Network_Message::SendString(char* str) {
	//FIXME: better copying
	while (*str) SendByte(*str++);
}
