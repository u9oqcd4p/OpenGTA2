#include "opengta_sv.h"	

Ped_Manager Peds;

void netCallback_PEDSYNC(Network_Connection* Connection, Network_Message* Msg) {
	//LockID lockID = Thread.EnterLock(MUTEX_PED);

	if (Network.IsServer) {
		//Do nothing really
	} else {
		//Format:
		//[0][2] num peds (peds table)
		//        [?+0][4] Ped ID
		//        [?+4][4*6] Ped X,Y,Z VX,VY,VZ
		uint numPeds = Msg->ReadWord();
		while (Peds.pedPool.Count < numPeds) Peds.Create(Vector3f(0.0f,0.0f,0.0f));
		for (uint i = 0; i < numPeds; i++) {
			uint pedID = Msg->ReadInt();

			float correctCoef = 0.0f;
			Vector3f newPosition;
			newPosition.x = Msg->ReadFloat();
			newPosition.y = Msg->ReadFloat();
			newPosition.z = Msg->ReadFloat();

			Vector3f newVelocity;
			newVelocity.x = Msg->ReadFloat();
			newVelocity.y = Msg->ReadFloat();
			newVelocity.z = Msg->ReadFloat();

			float dist = ((*Peds.pedPool[i])->Position - newPosition).Length();
			correctCoef = max(0.0f,min(0.95f,1.0f-dist));
			correctCoef *= correctCoef;

			if ((dist < 8.0f) && 
				(((*Peds.pedPool[i])->Velocity.Length() > 0.1f) ||
				(newVelocity.Length() > 0.1f))) correctCoef = 1.0f;

			(*Peds.pedPool[i])->netPosition = newPosition;
			(*Peds.pedPool[i])->netVelocity = newVelocity;

			(*Peds.pedPool[i])->ID = pedID;
			(*Peds.pedPool[i])->Position = (*Peds.pedPool[i])->Position * correctCoef + newPosition * (1-correctCoef);
			(*Peds.pedPool[i])->Velocity = (*Peds.pedPool[i])->Velocity * correctCoef + newVelocity * (1-correctCoef);
			float newHdg = (*Peds.pedPool[i])->Heading * correctCoef + (1-correctCoef) * Msg->ReadFloat();
			(*Peds.pedPool[i])->controlClient = Msg->ReadInt();
			if ((*Peds.pedPool[i])->controlClient != Clients.LocalClient) {
				(*Peds.pedPool[i])->Heading = newHdg;
			}


			//(*Peds.pedPool[i])->ID = pedID;
		}
	}

	//Thread.LeaveLock(lockID);
}

void netCallback_PEDRESYNC(Network_Connection* Connection, Network_Message* Msg) {
	if (Network.IsServer) {
		//networkSendResync(
	}
}

void Ped_Manager::networkResync() {
	if (Network.IsConnected) {
		Network_Message* msg = Network.Connections[0]->NewMessage();
		msg->Start(NETMESSAGE_PEDRESYNC);
		msg->End();
	}
}

PedID Ped_Manager::Create(Vector3f pedPos) {
	LockID lockID = Thread.EnterLock(MUTEX_PED);
		if (pedPool.Count == pedPool.AllocCount) {
			logWrite("ERROR: Out of peds");
			return BAD_ID;
		}
	
		(*pedPool[pedPool.Count])->ID = globalPedCounter;
		(*pedPool[pedPool.Count])->Born(pedPos);
		pedPool.Count++;
	Thread.LeaveLock(lockID);
	return globalPedCounter++;
}

void Ped_Manager::Kill(PedID ped) {
	LockID lockID = Thread.EnterLock(MUTEX_PED);
		for (uint j = 0; j < pedPool.Count; j++) {
			if ((*pedPool[j])->ID == ped) {
				(*pedPool[j])->Die();
				*pedPool[j] = *pedPool[pedPool.Count-1];
				pedPool.Count--;
				Thread.LeaveLock(lockID);
				return;
			}
		}
	Thread.LeaveLock(lockID);
}

void Ped_Manager::Initialize() {
	logWrite("Initializing ped pool");
	pedData.Preallocate(1024);
	pedPool.Preallocate(1024);
	for (uint i = 0; i < pedPool.AllocCount; i++) {
		*pedPool[i] = pedData[i];
	}
	globalPedCounter = 0;

	Network.networkCallback[NETMESSAGE_PEDSYNC] = netCallback_PEDSYNC;
	Network.networkCallback[NETMESSAGE_PEDRESYNC] = netCallback_PEDRESYNC;
	testTime = ServerTimer.Time();

	cvPedGravity = Convar.Create("phys_ped_gravity",4.0f);
	cvPedBoundary = Convar.Create("phys_ped_boundary",0.1f);
	cvPedForwardSpeed = Convar.Create("phys_ped_forwardspeed",1.5f);
	cvPedBackwardSpeed = Convar.Create("phys_ped_backwardspeed",1.5f);
	cvPedMaxSlope = Convar.Create("phys_ped_max_slope",1.0f);
}

void Ped_Manager::Reinitialize() {
	LockID lockID = Thread.EnterLock(MUTEX_PED);
		pedPool.Count = 0;
		globalPedCounter = 0;
	Thread.LeaveLock(lockID);
}

void Ped_Manager::Deinitialize() {
	logWrite("Deinitializing ped pool");
	pedPool.Release();
	pedData.Release();
}

void Ped_Manager::networkSendResync(ClientID client) {
	if (client == BAD_ID) return;
	if ((Clients[client]->Connection) && (Clients[client]->ClientName[0]) && (Clients[client]->Connection->socketHandle >= 0)) {
		Network_Message* Msg = Clients[client]->Connection->NewMessage();
		Msg->Start(NETMESSAGE_PEDSYNC);
			Msg->SendWord(pedPool.Count);
			for (uint j = 0; j < pedPool.Count; j++) {
				Msg->SendInt((*pedPool[j])->ID);
				Msg->SendFloat((*pedPool[j])->Position.x);
				Msg->SendFloat((*pedPool[j])->Position.y);
				Msg->SendFloat((*pedPool[j])->Position.z);
				Msg->SendFloat((*pedPool[j])->Velocity.x);
				Msg->SendFloat((*pedPool[j])->Velocity.y);
				Msg->SendFloat((*pedPool[j])->Velocity.z);
				Msg->SendFloat((*pedPool[j])->Heading);
				Msg->SendInt((*pedPool[j])->controlClient);
			}
		Msg->End();
	}
}

void Ped_Manager::Frame() {
	LockID lockID = Thread.EnterLock(MUTEX_PED);
		for (uint i = 0; i < pedPool.Count; i++) {
			(*pedPool[i])->Live();
		}
	Thread.LeaveLock(MUTEX_PED);

	if (Network.IsServer) {
		if (ServerTimer.Time() - testTime > Convar.GetFloat("net_test_rate",0.1f)) {
			testTime = ServerTimer.Time();
			for (uint i = 0; i < Clients.List.AllocCount; i++) networkSendResync(i);
		}
	}
}