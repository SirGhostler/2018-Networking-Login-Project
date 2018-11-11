#pragma once

//== Includes =======================

#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

#include "Login.h"
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>
#include <RakString.h>

//== Classes =======================

class GameServer
{
public:

	GameServer();			// Constructor
	~GameServer();			// Deconstructor

	// ...
	void gs_handleNetworkMessages(RakNet::RakPeerInterface* pPeerInterface);

protected:

	// Instance of Login
	Login login;

	// Instance of PeerInterface
	RakNet::RakPeerInterface* m_pPeerInterface;

	std::unordered_map<int, std::string> m_tokenToUsername;
};