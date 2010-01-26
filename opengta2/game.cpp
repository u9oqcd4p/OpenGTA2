#include "opengta.h"
#include "opengta_sv.h"

Game_Manager Game;

void cvConnect_Write(int numParams, int* paramType, int* params) {
	char* host = CVAR_GET_CSTRING(0);
	int port = CVAR_GET_INT(1);
	if (!port) port = 99;
	if (host) {
		Game.ConnectToServer(host,port);
	}
}

void cvHost_Write(int numParams, int* paramType, int* params) {
	int port = CVAR_GET_INT(0);
	if (!port) port = 99;
	Game.StartServer(port);
}

void cvDisconnect_Write(int numParams, int* paramType, int* params) {
	Game.Disconnect();
}

void Game_Manager::Initialize() {
	Game.Running = true;
	Game.IsDedicatedServer = false;
	Game.IsHostingGame = false;
	#ifdef DEDICATED_SERVER
		Game.IsDedicatedServer = true;
		Game.IsHostingGame = true;
	#endif

	Convar.Create("connect",0,cvConnect_Write);
	Convar.Create("host",0,cvHost_Write);
	Convar.Create("disconnect",0,cvDisconnect_Write);
}

bool Game_Manager::IsServer() {
	return (ServerThread > 0) && (Thread.GetCurrentThreadID() == ServerThread);
}

//IsClient is true for all client threads!
bool Game_Manager::IsClient() {
	return !IsServer();
}

void Game_Manager::ConnectToServer(char const* host, int port) {
	Network.Disconnect();
	Network.ConnectToServer(host, port);
	if (Network.IsConnected) {
		Peds.Reinitialize();
		Clients.Reinitialize();
	}
}

void Game_Manager::StartServer(int port) {
	Network.Disconnect();
	Network.StartServer(99);
	if (Network.IsServer) {
		Peds.Reinitialize();
		Clients.Reinitialize();
	}
}

void Game_Manager::Disconnect() {
	Network.Disconnect();
	Peds.Reinitialize();
	Clients.Reinitialize();
}
