#ifndef CLIENTLIST_H
#define CLIENTLIST_H

#include "input_keys.h"
#include "network.h"

struct clientinfo_entry {
	//Client nickname, or something
	char ClientName[256];
	//Current player ped
	PedID PlayerPed;
	//Players pressed keys
	int PlayerKeys[NUM_KEYS];
	//Player keys pressed in previous frame
	int prevPlayerKeys[NUM_KEYS];

	//Is server owner
	bool serverOwner;
	
	//Is client remote or local (do we need to recv and send his info)
	//Client network connection (if he is remote)
	Network_Connection* Connection;
};

struct Client_List {
	void Initialize();
	void Deinitialize();
	void Frame();

	//Reinitialize client list (call before starting new game)
	void Reinitialize();
	void SetMaxPlayers(int numPlayers);

	//Update client list for given client
	void networkUpdateClientList(ClientID client);

	//Get client by connection
	ClientID getByConnection(Network_Connection* Connection);

	//Local client
	ClientID LocalClient;

	//Convars
	ConvarID cvMaxPlayers;

	DataArray<clientinfo_entry> List;
	clientinfo_entry* operator[](int i) { return List[i]; }
};

extern Client_List Clients;

#endif
