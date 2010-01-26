#ifndef RECYCLE_PED_H
#define RECYCLE_PED_H

//FIXME: some kind of unique remap is required
struct Ped_Manager {
	PedID Create(Vector3f pedPos);
	void Kill(PedID ped);

	void Initialize();
	void Deinitialize();
	void Frame();

	//Reinitialize/kill all pedestrians
	void Reinitialize();

	//Request full resync
	void networkResync();
	void networkSendResync(ClientID client);

	//Global pedestrian counter
	PedID globalPedCounter;

	float testTime;
	
	//Convars
	ConvarID cvPedGravity;
	ConvarID cvPedBoundary;
	ConvarID cvPedForwardSpeed;
	ConvarID cvPedBackwardSpeed;
	ConvarID cvPedMaxSlope;

	DataArray<Ped*> pedPool;
	DataArray<Ped> pedData;
	Ped* operator[](uint i) { 
			LockID lockID = Thread.EnterLock(MUTEX_PED);
			for (uint j = 0; j < pedPool.Count; j++)
				if ((*pedPool[j])->ID == i) {
					Thread.LeaveLock(lockID);
					return *pedPool[j];
				}
			Thread.LeaveLock(lockID);
			return 0;
		};
};

extern Ped_Manager Peds;

#endif