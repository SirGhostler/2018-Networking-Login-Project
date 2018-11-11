//== Includes =======================

#include "Login.h"
#include "GameMessages.h"

//== Functions =======================

Login::Login()
{
}
Login::~Login()
{
}

// Client
void Login::loginClient(RakNet::RakPeerInterface* pPeerInterface, RakNet::SystemAddress & server)
{
	// Create std::string variable for cin input
	std::string input;
	// Create BitSTream variable for user information
	RakNet::BitStream bs;

	// Create RakString variable for the username
	RakNet::RakString usernameRS;
	// Input username
	std::cout << "Please input your username: " << std::endl;
	std::cin >> input;
	// Assign std::string to RakString variable
	usernameRS = input.c_str();

	// Create RakString variable for the password
	RakNet::RakString passwordRS;
	// Input password
	std::cout << "Please input your password: " << std::endl;
	std::cin >> input;
	// Assign std::string to RakString variable
	passwordRS = input.c_str();

	// Write information to the BitStream
	bs.Write((RakNet::MessageID)GameMessages::ID_CLIENT_USER_DATA);
	bs.Write(usernameRS);
	bs.Write(passwordRS);

	// Send information
	pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, server, false);
}

// Game Server
bool Login::checkClientDetails(RakNet::RakString username, RakNet::RakString password, std::unordered_map<std::string, LoginData>& loginDetails, std::string* errorMessage)
{
	// Convert the username from RakString to std::string
	std::string usernameSTR = username.C_String();
	// Check to see if it matches with anything
	auto it = loginDetails.find(usernameSTR);

	if (loginDetails.find(usernameSTR) == loginDetails.end()) // If usernameSTR does not match with anything
	{
		std::cout << "Could not find " << usernameSTR << " in the database. Please try again." << std::endl;
		return false;
	}
	else // If usernameSTR does match with something
	{
		LoginData& data = loginDetails[usernameSTR];
		std::cout << "The given username " << "'" << usernameSTR << "'" << " has been accepted." << std::endl;

		// Convert the password from RakString to std::string
		std::string passwordSTR = password.C_String();
		// Check to see if it matches with the username
		if (passwordSTR != data.password) // If passwordSTR does not match with the username
		{
			std::cout << "Password " << passwordSTR << " does not match the username. Please try again." << std::endl;
			return false;
		}
		else // If passwordSTR does match with the username
		{
			std::cout << "The given password " << "'" << passwordSTR << "'" << " has been accepted.\nGenerating token now.\n" << std::endl;
			return true;
		}
	}

	//if (errorMessage) *errorMessage = "It failed because blah";

	return false;
}
void Login::directClientToAuthServer()
{
}
void Login::grantClientAccess()
{
}

// Authentication Server
int Login::generateClientToken()
{
	// Create a random number for the token
	int randomNum = rand() % 1000;
	// Return said random number
	return randomNum;
}
void Login::directClientToGameServer()
{
}

// ...
void Login::handleNetworkMessages()
{
}
void Login::initializeNetworkConnection()
{
}

//== Main =======================