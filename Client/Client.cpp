//== Includes =======================

#include "Client.h"
#include "GameMessages.h"
#include <cstdio>
#include "Login.h"

//== Functions =======================

Client::Client()
{
}
Client::~Client()
{
}

void Client::update(float deltaTime)
{
	cl_handleNetworkMessages();
}

void Client::cl_handleNetworkConnection()
{
	// Get instance off the Peer Interface
	m_pPeerInterface = RakNet::RakPeerInterface::GetInstance();
	// Initialize the connection
	cl_initializeNetworkConnection();
}
void Client::cl_initializeNetworkConnection()
{
	// Create a socket descriptor to describe this connection
	// No data needed, as we will be connecting to a server
	RakNet::SocketDescriptor sd;

	//Now call startup - max of 1 connections (to the server)
	m_pPeerInterface->Startup(2, &sd, 1);
	// Read out which server we are connecting to and its IP
	std::cout << "Attempting to connect to server at: " << GAME_SERVER_IP << ":" << GAME_SERVER_PORT << std::endl;

	//Now call connect to attempt to connect to the given server
	RakNet::ConnectionAttemptResult res = m_pPeerInterface->Connect(GAME_SERVER_IP, GAME_SERVER_PORT, nullptr, 0);

	//Finally, check to see if we connected, and if not, throw a error
	if (res != RakNet::CONNECTION_ATTEMPT_STARTED)
	{ std::cout << "Unable to start connection, Error number: " << res << std::endl; }
}

void Client::cl_handleNetworkMessages()
{
	RakNet::Packet* packet;

	for (packet = m_pPeerInterface->Receive(); packet; m_pPeerInterface->DeallocatePacket(packet), packet = m_pPeerInterface->Receive())
	{
		switch (packet->data[0])
		{
			case ID_REMOTE_NEW_INCOMING_CONNECTION:
			{
				std::cout << "Another client has connected.\n";
				break;
			}

			case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			{
				std::cout << "Another client has disconnected.\n";
				break;
			}

			case ID_REMOTE_CONNECTION_LOST:
			{
				std::cout << "Another client has lost the connection.\n";
				break;
			}

			case ID_CONNECTION_REQUEST_ACCEPTED:
			{
				std::cout << "Our connection request has been accepted.\n";
				break;
			}

			case ID_NO_FREE_INCOMING_CONNECTIONS:
			{
				std::cout << "The server is full.\n";
				break;
			}

			case ID_DISCONNECTION_NOTIFICATION:
			{
				std::cout << "We have been disconnected.\n";
				break;
			}

			case ID_CONNECTION_LOST:
			{
				std::cout << "Connection lost.\n";
				break;
			}

			// When the authentication server requests the user to log in
			//========================================================================================
			case ID_AUTHENTICATION_SERVER_TRIGGER_CLIENT_LOGIN:
			{
				// Save system address for the packet sender
				GAME_AUTHENTICATION_ADDRESS = packet->systemAddress;

				std::cout << std::endl << "We have been requested to log in by the authentication server.\nPlease log in with your details." << std::endl;
				login.loginClient(m_pPeerInterface, GAME_AUTHENTICATION_ADDRESS);
				break;
			}
			//========================================================================================

			// When we log in through the authentication server, upon it being successful
			// we get a notification and a token
			//========================================================================================
			case ID_AUTHENTICATION_SERVER_CLIENT_LOGIN_SUCCESS:
			{
				std::cout << std::endl << "The log in request from the authentication server was successful!" << std::endl;
				// Create BitStream variable for the token information
				RakNet::BitStream tokenInfoBS(packet->data, packet->length, false);
				tokenInfoBS.IgnoreBytes(sizeof(RakNet::MessageID));
				// Create RakString variable for the token
				int token;
				// Read it out to the console
				tokenInfoBS.Read(token);
				m_token = token;
				std::cout << "Your generated token is: " << token << ".\nIt, along with your username, will be sent to the game server." << std::endl;
		
				m_pPeerInterface->CloseConnection(GAME_AUTHENTICATION_ADDRESS, true, 0);
				
				// Create BitStream variable for the token
				RakNet::BitStream tokenBS;

				std::cout << "\nThe game server has requested our token, it will now be sent and checked.\n" << std::endl;

				// Write information to the BitStream
				tokenBS.Write((RakNet::MessageID)GameMessages::ID_CLIENT_TOKEN_DATA);
				tokenBS.Write(m_token);

				// Send information
				m_pPeerInterface->Send(&tokenBS, HIGH_PRIORITY, RELIABLE_ORDERED, 0, GAME_SERVER_ADDRESS, false);
				break;
			}
			//========================================================================================

			// The game server sends us a authentication request after we connect, we pass it our token,
			// then we are granted access to log in
			//========================================================================================
			case ID_GAME_SERVER_START_CLIENT_TOKEN_CHECK:
			{
				//if (currentlyConnectedServer != Game) continue;

				// Save system address for the packet sender
				GAME_SERVER_ADDRESS = packet->systemAddress;

				// Create BitStream variable for the token
				RakNet::BitStream tokenBS;

				std::cout << "\nThe game server has requested our token, it will now be sent and checked.\n" << std::endl;

				// Write information to the BitStream
				tokenBS.Write((RakNet::MessageID)GameMessages::ID_CLIENT_TOKEN_DATA);
				tokenBS.Write(m_token);

				// Send information
				m_pPeerInterface->Send(&tokenBS, HIGH_PRIORITY, RELIABLE_ORDERED, 0, GAME_SERVER_ADDRESS, false);
				break;
			}
			//========================================================================================

			// Authentication request complete, success
			//========================================================================================
			case ID_GAME_SERVER_LOGIN_SUCCESS:
			{
				if (currentlyConnectedServer == Game) continue;

				std::cout << "Token check success, you are now logged in.\n" << std::endl;
				break;
			}
			//========================================================================================

			// Authentication request complete, failure
			//========================================================================================
			case ID_GAME_SERVER_LOGIN_FAIL:
			{
				if (currentlyConnectedServer != Game) continue;

				std::cout << "Token check failure, your token is invalid." << std::endl;
				std::cout << "You will be disconnected and redirected to the authentication server.\n" << std::endl;

				// Disconnect from the game server...
				//m_pPeerInterface->CloseConnection(packet->systemAddress, false, 0, HIGH_PRIORITY);

				// And connect to the authentication server,
				// read out which server we are connecting to and its IP
				std::cout << "Attempting to connect to server at: " << GAME_AUTHENTICATION_IP << ":" << GAME_AUTHENTICATION_PORT << std::endl;
				// Now call connect to attempt to connect to the given server
				RakNet::ConnectionAttemptResult res = m_pPeerInterface->Connect(GAME_AUTHENTICATION_IP, GAME_AUTHENTICATION_PORT, nullptr, 0);
				currentlyConnectedServer = Authentication;
				break;

			}
			//========================================================================================

			case ID_GAME_SERVER_TRIGGER_RELOGIN:
			{
				std::cout << "Your details have been confirmed, your token will need to be checked again." << std::endl;
				break;
			}

			default:
			{
				std::cout << "Received a message with a unknown id: " << packet->data[0] << std::endl << std::endl;
				break;
			}
		}
	}
}

//== Main =======================

int main()
{ 
	// Get instance of the client
	Client client;

	client.cl_handleNetworkConnection();

	while (true)
	{
		client.cl_handleNetworkMessages();
	}

	std::getchar();
}