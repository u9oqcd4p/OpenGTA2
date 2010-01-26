#ifndef MESSAGE_H
#define MESSAGE_H

/*enum { MESSAGE_CHAT = 0, 
	   MESSAGE_TEXT = 1,
	   MESSAGE_USER = 2,
	   MESSAGE_WORLDUPDATE = 3} MessageType;

struct Message_Manager {
	char messageBuf[256];

	void Start(MessageType msgType = MESSAGE_USER);
	void Write(void* buf, int sz);
	void End();


};*/

extern Message_Manager Message;

#endif