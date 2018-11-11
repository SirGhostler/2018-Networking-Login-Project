#pragma once

//== Includes =======================

#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>
#include <RakString.h>

//== Structs =======================

struct LoginData
{
	std::string username;
	std::string password;
	std::string token;
};

//== Classes =======================

class Login
{
public:

	Login();			// Constructor
	~Login();			// Deconstructor

	// Client
	void loginClient(RakNet::RakPeerInterface* pPeerInterface, RakNet::SystemAddress & server);

	// Game Server
	bool checkClientDetails(RakNet::RakString username, RakNet::RakString password, std::unordered_map<std::string, LoginData>& loginDetails, std::string* errorMessage = nullptr);
	void directClientToAuthServer();
	void grantClientAccess();

	// Authentication Server
	int generateClientToken();
	void directClientToGameServer();

	// ...
	void handleNetworkMessages();
	void initializeNetworkConnection();

protected:

	// Instance of PeerInterface
	RakNet::RakPeerInterface* m_pPeerInterface;
};