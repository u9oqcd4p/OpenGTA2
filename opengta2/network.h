#ifndef NETWORK_H
#define NETWORK_H

#ifdef _WIN32
	#undef WINGDIAPI
	#undef APIENTRY
	#include <winsock2.h>
#else
	#include <sys/socket.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
#endif

//SV    Server info query
#define NETMESSAGE_SERVERINFO		0
//SV,CL Time synchronization message
#define NETMESSAGE_TIMESYNC			1
//SV    Register new client/connect to server
#define NETMESSAGE_NEWCLIENT		2
//CL    List of clients on server/list update
#define NETMESSAGE_CLIENTLIST		3
//CL,SV Chat message
#define NETMESSAGE_CHAT				4
//CL    Pedestrian state synchronization
#define NETMESSAGE_PEDSYNC			5
//SV    Request ped resync
#define NETMESSAGE_PEDRESYNC		6
//CL,SV Input from client
#define NETMESSAGE_CLIENTINPUT		7
#define NETMESSAGE_LAST				8

typedef unsigned int ConnectionID;
struct Network_Connection;//MESSAGE_LAST

struct Network_Message {
	char* rawBuffer;
	int rawSize; //total size
	int rawPos; //read position
	Network_Connection* networkConnection;

	//Start new message
	void Start(int messageID);
	//End message and send it
	void End();

	//Internal: send actual data
	void doSendData();
	float sendTime; //target send time, if packet must be delayed

	void SendByte(char value);
	void SendWord(short value);
	void SendInt(int value);
	void SendFloat(float value);

	char ReadByte();
	short ReadWord();
	int ReadInt();
	float ReadFloat();

	//void SendString(char* str, bool nullTerminated);
	void SendString(char* str);
};

struct Network_Connection {
	//Open new connection to target host, port
	bool Open(char const* host, int port);
	//Close connection
	void Close();

	//Send raw data (returns actual number of bytes sent)
	int Send(void* buf, int size);
	//Receive raw data into buffer (returns actual number of bytes read)
	int Recv(void* buf, int size);

	//Update connection state
	void Update();

	//Create new message to send
	Network_Message* NewMessage();
	//Receive message from network (if there's any messages in queue)
	//Network_Message* ReceiveMessage(int messageID, int size);

	bool canReceive();
	bool canSend();

	//Is this listen connection or active connection
	bool listenConnection;
	int socketHandle;
	struct sockaddr_in peerAddress;
	bool connectionOpen;

	//Network message send queue (in case more than one thread sends messages over this connection)
	DataArray<Network_Message> sendQueue;
	//Message that is currently being received
	Network_Message recvMessage;
};


typedef void (NetworkCallbackFunction)(Network_Connection* Connection, Network_Message* Msg);

struct Network_Manager {
	void Initialize();
	void Deinitialize();
	void Frame();
	bool Available;

	//Is hosting server?
	bool IsServer;
	//Is connected to server?
	bool IsConnected;

	//Start a new server
	void StartServer(int port);
	//Connect to server
	void ConnectToServer(char const* host, int port);
	//Disconnect from server
	void Disconnect();

	//Various queries
	void QueryServer(int queryType);

	//void StartMessage(
	//ConnectionID Open(char* host, int port);

	//Callbacks for network messages
	NetworkCallbackFunction* networkCallback[NETMESSAGE_LAST];

	//Maximum message size, in bytes. Do not write
	int maxMessageSize; 
	//float extraPacketDelay; //This will increase delay between sent packets

	//Convars for networking
	ConvarID cvMaxMessageSize;
	ConvarID cvMaxConnections;
	ConvarID cvSendQueueSize;
	ConvarID cvTimeRate;
	ConvarID cvFakeLag;

	//Statistics
	int numBytesSent;
	int numBytesReceived;
	int numPacketsSent;
	int numPacketsReceived;
	float packetLatency;
	float remoteServerFPS;

	//Number of remaining time synchronization exchanges
	int remainingTimeSyncMsgs;
	float prevTimeSyncMessageTime;
	float prevTimeSyncTick;

	//Mutex locks for Network_Message::Send
	LockID msgSendLock;

	DataArray<Network_Connection> Connections;
	Network_Connection* operator[](int i) { return Connections[i]; }

	//Create new connection
	Network_Connection* NewConnection();
};

extern Network_Manager Network;

#endif
