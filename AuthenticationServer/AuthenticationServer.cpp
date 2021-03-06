//== Includes =======================

#include "AuthenticationServer.h"
#include "GameMessages.h"
#include "Login.h"

//== Functions =======================

AuthenticationServer::AuthenticationServer()
{
	userDetails["DoddzyCodes"] = { "DoddzyCodes", "noyoudont"      , "" };
	userDetails["a"] = { "a", "a"      , "" };
	userDetails["SonicSol"]    = { "SonicSol"   , "TurnTheBeatBack", "" };
	userDetails["RobbieSla"]   = { "RobbieSla"  , "devourlol"      , "" };

	userTokens[""] = { "", "", "" };
}
AuthenticationServer::~AuthenticationServer()
{
}

void AuthenticationServer::as_handleNetworkMessages(RakNet::RakPeerInterface * pPeerInterface)
{
	RakNet::Packet* packet;

	while (true)
	{
		for (packet = pPeerInterface->Receive(); packet; pPeerInterface->DeallocatePacket(packet), packet = pPeerInterface->Receive())
		{
			switch (packet->data[0])
			{
				//========================================================================================
				case ID_NEW_INCOMING_CONNECTION:
				{
					std::cout << "A connection with a client is incoming.\n";
					std::cout << "The connection request has been accepted and a client has connnected. A request for the client to log in has been sent.\n" << std::endl;

					// Create BitStream variable for user notification
					RakNet::BitStream bs;
					// Write information to the BitStream
					bs.Write((RakNet::MessageID)GameMessages::ID_AUTHENTICATION_SERVER_TRIGGER_CLIENT_LOGIN);
					// Send information
					pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
					break;
				}
				//========================================================================================

				// When the client triggers the authentication server
				// to generate a token for them
				//========================================================================================
				case ID_CLIENT_TRIGGER_TOKEN_GENERATION:
				{
					std::cout << "Client log in accepted. Token is now being generated.\n";
				}
				//========================================================================================

				case ID_CONNECTION_REQUEST_ACCEPTED:
				{
					std::cout << "Our connection request has been accepted. Connected to game server!\n" << std::endl;
					m_gameServerAddress = packet->systemAddress;
					break;
				}

				case ID_DISCONNECTION_NOTIFICATION:
				{
					std::cout << "A client has disconnected.\n";
					break;
				}

				case ID_CONNECTION_LOST:
				{
					std::cout << "A client lost the connection.\n";
					break;
				}

				case ID_GAME_SERVER_TEXT_MESSAGE:
				{
					RakNet::BitStream bsIn(packet->data, packet->length, false);
					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					RakNet::RakString str; bsIn.Read(str);
					std::cout << str.C_String() << std::endl;
					break;
				}

				// When the server recieves user details data
				//========================================================================================
				case ID_CLIENT_USER_DATA:
				{
					std::cout << "Recieved a message containing user data.\n" /*<< packet->data[0]*/;

					// Create BitStream variable for the username
					RakNet::BitStream usernameBS(packet->data, packet->length, false);
					usernameBS.IgnoreBytes(sizeof(RakNet::MessageID));
					// Create RakString variable for the username
					RakNet::RakString usernameRS;
					usernameBS.Read(usernameRS);
					// Read it out to the console
					std::cout << "Recieved username: " << "'" << usernameRS.C_String() << "'" << std::endl;

					// Create RakString variable for the password
					RakNet::RakString passwordRS;
					usernameBS.Read(passwordRS);
					// Read it out to the console
					std::cout << "Recieved password: " << "'" << passwordRS.C_String() << "'" << std::endl;

					// String for error messages
					std::string errorMessage;
					// Run function to compare details
					bool loggedin = login.checkClientDetails(usernameRS, passwordRS, userDetails, &errorMessage);

					// If the previous function returns true, then the login
					// was successful
					if (loggedin == true)
					{
						// Create variable for the random number which will become the token
						int tokenINT = login.generateClientToken();

						// Create BitStream variable for the password
						RakNet::BitStream usernameAndTokenBS;
						// Write information to the BitStream
						usernameAndTokenBS.Write((RakNet::MessageID)GameMessages::ID_AUTHENTICATION_SERVER_SEND_SUCCESS_INFO_TO_GAME_SERVER);
						usernameAndTokenBS.Write(usernameRS);
						usernameAndTokenBS.Write(tokenINT);
						usernameAndTokenBS.Write(packet->systemAddress);
						// Send information to the game server
						pPeerInterface->Send(&usernameAndTokenBS, HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_gameServerAddress, false);

						std::cout << "Username and token information have been sent to the game server." << std::endl;
					}
					break;
				}

				case ID_GAME_SERVER_ACKNOWLEDGEMENT:
				{
					// Create BitStream variable for the information
					RakNet::BitStream inBS(packet->data, packet->length, false);
					inBS.IgnoreBytes(sizeof(RakNet::MessageID));
					// Read out information
					RakNet::SystemAddress peerAddress;
					inBS.Read(peerAddress);
					int token;
					inBS.Read(token);

					// Create BitStream variable for the password
					RakNet::BitStream bsOut;
					// Write information to the BitStream
					bsOut.Write((RakNet::MessageID)GameMessages::ID_AUTHENTICATION_SERVER_CLIENT_LOGIN_SUCCESS);
					bsOut.Write(token);
					// Send information to the client
					pPeerInterface->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, peerAddress, false);
					break;
				}
				//========================================================================================

				default:
				{
					std::cout << "Received a message with a unknown id: " << packet->data[0] << std::endl << std::endl;
					break;
				}
			}
		}
	}
}


//== Main =======================

int main()
{
	// Define port number
	const unsigned short PORT = 5457;
	// Get instance of Peer Interface
	RakNet::RakPeerInterface* pPeerInterface = nullptr;

	// Startup the server, and start it listening to clients
	std::cout << "Starting up the authentication server..." << std::endl;
	// Initialize the Raknet peer interface first
	pPeerInterface = RakNet::RakPeerInterface::GetInstance();

	// Create a socket descriptor to describe this connection
	RakNet::SocketDescriptor sd(PORT, 0);
	// Now call startup - max of 32 connections, on the assigned port
	pPeerInterface->Startup(32, &sd, 1);

	pPeerInterface->SetMaximumIncomingConnections(32);

	// Get instance of the server
	AuthenticationServer server;

	// Now connect to them game server,
	// read out which server we are connecting to and its IP
	std::cout << "\nAttempting to connect to the game server at: " << server.GAME_SERVER_IP << ":" << server.GAME_SERVER_PORT << std::endl;
	// Now call connect to attempt to connect to the given server
	RakNet::ConnectionAttemptResult res = pPeerInterface->Connect(server.GAME_SERVER_IP, server.GAME_SERVER_PORT, nullptr, 0);

	server.as_handleNetworkMessages(pPeerInterface);

	return 0;
}