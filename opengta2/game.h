#ifndef GAME_H
#define GAME_H

struct Game_Manager {
	void Initialize();

	ThreadID ServerThread;
	ThreadID ClientThread;

	//This specifies dedicated server (else it works in listen mode)
	bool IsDedicatedServer;
	//Are you hosting game? If yes, it will make this server publicly connectable
	bool IsHostingGame;

	//Is game still running? Else it exists
	bool Running;

	//Is current thread server thread
	bool IsServer();
	//Is current thread client thread
	bool IsClient();

	//Connect to server
	void ConnectToServer(char* host, int port);
	//Start server
	void StartServer(int port);
	//Disconnect from server/stop
	void Disconnect();
};

extern Game_Manager Game;

#endif
