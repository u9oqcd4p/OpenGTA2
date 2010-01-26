#include "opengta_sv.h"

Client_List Clients;


void netCallback_NEWCLIENT(Network_Connection* Connection, Network_Message* Msg) {
	LockID lockID = Thread.EnterLock(MUTEX_CLIENTLIST);
		if (Network.IsServer) { //This message tells server to register a new client
			clientinfo_entry* newClient = 0;
			int clientID = BAD_ID;
			bool isNewClient = true;

			//See if this client was already connected (if so - just change name)
			clientID = Clients.getByConnection(Connection);
			if (clientID != BAD_ID) {
				newClient = Clients.List[clientID];
				isNewClient = true;
			}

			//If he wasn't, try to find empty client slot
			if (clientID == BAD_ID) {
				for (uint i = 0; i < Clients.List.AllocCount; i++) {
					if (Clients.List[i]->ClientName[0] == 0) {
						newClient = Clients.List[i];
						clientID = i;
						break;
					}
				}
			}

			Network_Message* replyMsg = Connection->NewMessage();
			if (newClient) {
				//If it's a new client, spawn him a pedestrian
				if (isNewClient) {
					newClient->PlayerPed = Peds.Create(Vector3f(430.5f,447.5f,5.0f));
					Peds[newClient->PlayerPed]->controlClient = clientID;
					newClient->serverOwner = false;
					newClient->Connection = Connection;
				}
				//Change his name
				for (int i = 0; i < 32; i++) newClient->ClientName[i] = Msg->ReadByte();

				//Reply client with his new client ID, player ped ID, and his name (if it was changed)
				replyMsg->Start(NETMESSAGE_NEWCLIENT);
					replyMsg->SendInt(clientID);
					replyMsg->SendInt(newClient->PlayerPed);
					for (int i = 0; i < 32; i++) replyMsg->SendByte(newClient->ClientName[i]);
				replyMsg->End();

				logWrite("Connected client (%s)",newClient->ClientName);
				logWritem("ped id: %d, client id: %d",newClient->PlayerPed,clientID);

				//Send all valid clients
				Clients.networkUpdateClientList(clientID);			
			} else {
				logWrite("Denied client (%s): max players",newClient->ClientName);
				replyMsg->Start(NETMESSAGE_NEWCLIENT);
					replyMsg->SendInt(BAD_ID);
				replyMsg->End();
			}	
		} else { //This message tells client he was succesfully registered by server
			int clientID = Msg->ReadInt();
			if (clientID == BAD_ID) {
				logWrite("Server denied connection!");
				Network.Disconnect();
			} else {
				logWrite("Server accepted connection, spawning...");
				Clients.LocalClient = clientID;

				Clients[clientID]->PlayerPed = Msg->ReadInt();
				for (int i = 0; i < 32; i++) Clients[clientID]->ClientName[i] = Msg->ReadByte();

				logWritem("assigned local client id %d, ped id %d, name: %s",
					clientID,Clients[clientID]->PlayerPed,Clients[clientID]->ClientName);

				//Request resynchronize all pedestrians
				Peds.networkResync();
			}
		}
	Thread.LeaveLock(lockID);
}

void netCallback_CLIENTLIST(Network_Connection* Connection, Network_Message* Msg) {
	LockID lockID = Thread.EnterLock(MUTEX_CLIENTLIST);
		if (!Network.IsServer) { //Receive list of clients from server
			int numClients = Msg->ReadByte();
			for (int i = 0; i < numClients; i++) {
				int clientID = Msg->ReadInt();
				Clients[clientID]->PlayerPed = Msg->ReadInt();
				for (int j = 0; j < 32; j++) Clients[clientID]->ClientName[j] = Msg->ReadByte();
			}
		}
	Thread.LeaveLock(lockID);
}

void netCallback_CLIENTINPUT(Network_Connection* Connection, Network_Message* Msg) {
	LockID lockID = Thread.EnterLock(MUTEX_CLIENTLIST);
		if (Network.IsServer) { //Receive client input, send it to all other clients
			for (uint i = 0; i < Clients.List.AllocCount; i++) {
				if (Clients[i]->Connection == Connection) {
					int keys = Msg->ReadInt();
					for (int j = 0; j < 8; j++) {
						Clients[i]->PlayerKeys[j] = (keys & (1 << (j+1))) != 0;
					}
					float hdg = Msg->ReadFloat();
					Ped* playerPed = Peds[Clients[i]->PlayerPed];
					if (playerPed) playerPed->Heading = hdg;
					
					//Send message to other clients
					for (uint k = 0; k < Clients.List.AllocCount; k++) {
						if ((k != i) && 
							(Clients[k]->Connection) && (Clients[k]->ClientName[0]) && 
							(Clients[k]->Connection->socketHandle >= 0)) {
							Network_Message* msg = Clients[k]->Connection->NewMessage();
							msg->Start(NETMESSAGE_CLIENTINPUT);
								msg->SendByte(1); //Count
								msg->SendInt(i); //ClientID
								msg->SendInt(keys); //Keys
								msg->SendFloat(hdg);
							msg->End();
						}
					}
					Thread.LeaveLock(lockID);
					return;
				}
			}
	
		} else { //Receive client input (clientside)
			int numClients = Msg->ReadByte();
			for (int i = 0; i < numClients; i++) {
				int clientID = Msg->ReadInt();
				int keys = Msg->ReadInt();
				if (clientID != Clients.LocalClient) { //Do not apply keys to local client, to avoid visual lags
					for (int j = 0; j < 8; j++) {
						Clients[clientID]->PlayerKeys[j] = (keys & (1 << (j+1))) != 0;
					}
				}
			}
		}
	Thread.LeaveLock(lockID);
}

void Client_List::networkUpdateClientList(ClientID client) {
//	LockID lockID = Thread.EnterLock(MUTEX_CLIENTLIST);
		if (client == BAD_ID) return;
		if (!Clients[client]->ClientName[0]) return;
		if (!Clients[client]->Connection) return;

		//Count clients
		int numClients = 0;
		for (uint i = 0; i < Clients.List.AllocCount; i++)
			if (Clients[i]->ClientName[0]) numClients++;
	
		//Send all valid clients
		Network_Message* clistMsg = Clients[client]->Connection->NewMessage();
		clistMsg->Start(NETMESSAGE_CLIENTLIST);
			clistMsg->SendByte(numClients);
			for (uint i = 0; i < Clients.List.AllocCount; i++) {
				if (Clients[i]->ClientName[0]) {
					clistMsg->SendInt(i); //ClientID
					clistMsg->SendInt(Clients[i]->PlayerPed);
					for (int j = 0; j < 32; j++) clistMsg->SendByte(Clients[i]->ClientName[j]);
				}
			}
		clistMsg->End();
	//Thread.LeaveLock(lockID);
}

int cvMaxPlayers_Read(int numParams, int* paramType, int* params) {
	CVAR_SET_INT(0,Clients.List.AllocCount);
	return 1;
}

void cvMaxPlayers_Write(int numParams, int* paramType, int* params) {
	Clients.SetMaxPlayers(CVAR_GET_INT(0));
}

void Client_List::Initialize() {
	logWrite("Initializing client list");
	cvMaxPlayers = Convar.Create("max_players",cvMaxPlayers_Read,cvMaxPlayers_Write);

	//SetMaxPlayers(256);

	List.Preallocate(256);
	for (uint i = 0; i < List.AllocCount; i++) List[i]->Connection = 0;
	Reinitialize();

	Network.networkCallback[NETMESSAGE_NEWCLIENT] = netCallback_NEWCLIENT;
	Network.networkCallback[NETMESSAGE_CLIENTLIST] = netCallback_CLIENTLIST;
	Network.networkCallback[NETMESSAGE_CLIENTINPUT] = netCallback_CLIENTINPUT;
}

void Client_List::SetMaxPlayers(int numPlayers) {
	List.Preallocate(numPlayers);
	Reinitialize();
}

void Client_List::Reinitialize() {
	LockID lockID = Thread.EnterLock(MUTEX_CLIENTLIST);
		if (List.AllocCount > 0) {
			//Close all client connections that might be still open
			for (uint i = 0; i < List.AllocCount; i++) {	
				if (List[i]->Connection) List[i]->Connection->Close();
			}
		}

		for (uint i = 0; i < List.AllocCount; i++) {
			for (uint j = 0; j < 8; j++) List[i]->PlayerKeys[j] = 0;
			List[i]->serverOwner = false;
			List[i]->PlayerPed = BAD_ID;
			List[i]->ClientName[0] = 0;
			List[i]->Connection = 0;
		}

		if (Network.IsConnected) { //If we are connected, register ourselves in server list
			//Local client will be given out by server later
			LocalClient = BAD_ID;

			char buf[32];
			sprintf(buf,"Player %d",Random.Int(11111,99999));

			Network_Message* Msg = Network[0]->NewMessage();
			Msg->Start(NETMESSAGE_NEWCLIENT);
				for (int i = 0; i < 32; i++) Msg->SendByte(buf[i]);
			Msg->End();
		} else {
			if (!Game.IsDedicatedServer) {
				//Add master client
				strcpy(List[0]->ClientName,"LISTEN SERVER");
				List[0]->serverOwner = true;
					
				LocalClient = 0; //On listen server this specifies master clients ID
				List[0]->PlayerPed = Peds.Create(Vector3f(430.5f,447.5f,5.0f));
				Peds[List[0]->PlayerPed]->controlClient = 0;
			} else {
				//No master client on dedicated server
				LocalClient = BAD_ID;
			}
		}
	Thread.LeaveLock(lockID);
}

void Client_List::Deinitialize() {
	List.Release();
}

ClientID Client_List::getByConnection(Network_Connection* Connection) {
	for (uint i = 0; i < List.AllocCount; i++) {
		if (List[i]->Connection == Connection) {
			return i;
		}
	}
	return BAD_ID;
}

void Client_List::Frame() {
	if (Network.IsServer) {
		for (uint i = 0; i < Clients.List.AllocCount; i++) {
			if (Clients.LocalClient == i) continue;
			if (Clients[i]->ClientName[0]) {
				if ((Clients[i]->Connection) &&
					(Clients[i]->Connection->socketHandle < 0)) {
					Clients[i]->Connection = 0;
				}
				if ((!Clients[i]->Connection) && (!Clients[i]->serverOwner)) {
					Clients[i]->ClientName[0] = 0;
					Peds.Kill(Clients[i]->PlayerPed);
				}
			}
		}
	}


	if (Clients.LocalClient == BAD_ID) return;
	if (Network.IsConnected) {
		//Update control clients FIXME
		for (uint k = 0; k < Clients.List.AllocCount; k++) {
			if (Clients[k]->ClientName[0]) {
				Ped* ped = Peds[Clients[k]->PlayerPed];
				if (ped) {
					ped->controlClient = k;
				}
			}
		}
	}

	if (Network.IsConnected || Network.IsServer) {
		bool resendKeys = false;
		for (int i = 0; i < 8; i++) {
			if (Clients[Clients.LocalClient]->PlayerKeys[i] != 
				Clients[Clients.LocalClient]->prevPlayerKeys[i]) {
				Clients[Clients.LocalClient]->prevPlayerKeys[i] = Clients[Clients.LocalClient]->PlayerKeys[i];
				resendKeys = true;
			}
		}

		if (resendKeys) {
			if (Network.IsServer) {
				for (uint k = 0; k < Clients.List.AllocCount; k++) {
					if ((k != Clients.LocalClient) && 
						(Clients[k]->Connection) && (Clients[k]->ClientName[0]) && 
						(Clients[k]->Connection->socketHandle >= 0)) {
						Network_Message* msg = Clients[k]->Connection->NewMessage();
						msg->Start(NETMESSAGE_CLIENTINPUT);
							msg->SendByte(1);
							msg->SendInt(Clients.LocalClient);
							int keys = 0;
							for (int i = 0; i < 8; i++) keys = keys | (Clients[Clients.LocalClient]->PlayerKeys[i] * (1 << (i+1)));
							msg->SendInt(keys);

							Ped* playerPed = Peds[Clients[Clients.LocalClient]->PlayerPed];
							if (playerPed) msg->SendFloat(playerPed->Heading); else msg->SendFloat(0);
						msg->End();
					}
				}
			} else {
				Network_Message* msg = Network.Connections[0]->NewMessage();
				msg->Start(NETMESSAGE_CLIENTINPUT);
					int keys = 0;
					for (int i = 0; i < 8; i++) keys = keys | (Clients[Clients.LocalClient]->PlayerKeys[i] * (1 << (i+1)));
					msg->SendInt(keys);
	
					Ped* playerPed = Peds[Clients[Clients.LocalClient]->PlayerPed];
					if (playerPed) msg->SendFloat(playerPed->Heading); else msg->SendFloat(0);
				msg->End();
			}
		}
	}
}