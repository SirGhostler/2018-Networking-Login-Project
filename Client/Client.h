#pragma once

//== Includes =======================

#include <iostream>
#include <string>
#include <thread>

#include "Login.h"
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>

//== Classes =======================

class Client
{
public:

	Client();			// Constructor
	~Client();			// Deconstructor

	void update(float deltaTime);

	// Initialize a connection
	void cl_handleNetworkConnection();
	void cl_initializeNetworkConnection();

	// Handle incoming packets
	void cl_handleNetworkMessages();

	// IP for the Client
	const char* GAME_SERVER_IP = "127.0.0.1";
	const char* GAME_AUTHENTICATION_IP = "127.0.0.1";
	// Port for the Client
	const unsigned short GAME_SERVER_PORT = 5456;
	const unsigned short GAME_AUTHENTICATION_PORT = 5457;
	// System Address for the Client
	RakNet::SystemAddress GAME_SERVER_ADDRESS;
	RakNet::SystemAddress GAME_AUTHENTICATION_ADDRESS;

protected:

	// Instance of Login
	Login login;

	int m_token = -1;

	// Instance of PeerInterface
	RakNet::RakPeerInterface* m_pPeerInterface;

	enum Server
	{
		Authentication,
		Game,
	};

	Server currentlyConnectedServer = Game;
};