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

class AuthenticationServer
{
public:

	AuthenticationServer();		// Constructor
	~AuthenticationServer();	// Deconstructor

	// ...
	void as_handleNetworkMessages(RakNet::RakPeerInterface* pPeerInterface);

	const char* GAME_SERVER_IP = "127.0.0.1";
	const unsigned short GAME_SERVER_PORT = 5456;

protected:

	// Instance of Login
	Login login;

	// Instance of PeerInterface
	RakNet::RakPeerInterface* m_pPeerInterface;

	// Unordered Map for user tokens
	std::unordered_map<std::string, LoginData> userTokens;

	// Unordered Map for user details
	std::unordered_map<std::string, LoginData> userDetails;

	RakNet::SystemAddress m_gameServerAddress;
};