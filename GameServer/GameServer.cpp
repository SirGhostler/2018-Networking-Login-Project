//== Includes =======================

#include "GameServer.h"
#include "GameMessages.h"
#include "Login.h"

//== Functions =======================

GameServer::GameServer()
{
}
GameServer::~GameServer()
{
}

void GameServer::gs_handleNetworkMessages(RakNet::RakPeerInterface* pPeerInterface)
{
	RakNet::Packet* packet = nullptr;

	while (true)
	{
		for (packet = pPeerInterface->Receive(); packet; pPeerInterface->DeallocatePacket(packet), packet = pPeerInterface->Receive())
		{
			switch (packet->data[0])
			{
				//========================================================================================
				case ID_NEW_INCOMING_CONNECTION:
				{
					std::cout << "A connection with a client is incoming." << std::endl;

					std::cout << "The connection request has been accepted and a client has connnected. An authentication request has been sent. If you are a client, please take notice.\n" << std::endl;
					// Create BitStream variable for user information
					RakNet::BitStream bs;
					// Write information to the BitStream
					bs.Write((RakNet::MessageID)GameMessages::ID_GAME_SERVER_START_CLIENT_TOKEN_CHECK);
					// Send information
					pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
					break;
				}
				//========================================================================================

				case ID_CONNECTION_REQUEST_ACCEPTED:
				{
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

				case ID_CLIENT_LOGIN_DATA:
				{
					RakNet::BitStream bs(packet->data, packet->length, false);
					pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);
					break;
			}

				// When the server recieves token data, determines whether the user
				// needs to log in or not
				//========================================================================================
				case ID_CLIENT_TOKEN_DATA:
				{
					// Create BitStream variable for the token
					RakNet::BitStream tokenBS(packet->data, packet->length, false);
					tokenBS.IgnoreBytes(sizeof(RakNet::MessageID));
					// Create RakString variable for the token
					int token;
					tokenBS.Read(token);

					// Check if the token exists and matches an existing on on the database
					auto it = m_tokenToUsername.find(token);

					std::cout << "Token received, token id is " << token << std::endl;
					// If it does exist
					if (it != m_tokenToUsername.end())
					{

						std::cout << "Token received, token matches result is " << it->first << ", username is " << it->second << std::endl << std::endl;

						RakNet::BitStream bsOut;
						bsOut.Write((RakNet::MessageID)GameMessages::ID_GAME_SERVER_LOGIN_SUCCESS);
						pPeerInterface->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
					}
					// If it doesn't exist
					else
					{
						std::cout << "Token does not exist, sending fail packed!" << std::endl;

						RakNet::BitStream bsOut;
						bsOut.Write((RakNet::MessageID)GameMessages::ID_GAME_SERVER_LOGIN_FAIL);
						pPeerInterface->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
					}
					break;
				}
				//========================================================================================

				// The client has logged into the authentication server, been granted
				// their token, and their token and username have been sent
				// to the game server
				//========================================================================================
				case ID_AUTHENTICATION_SERVER_SEND_SUCCESS_INFO_TO_GAME_SERVER:
				{
					std::cout << "Username and token information recieved!\n" << std::endl;

					// Get the packet
					RakNet::BitStream tokenBS(packet->data, packet->length, false);
					tokenBS.IgnoreBytes(sizeof(RakNet::MessageID));

					// Create RakString variable for the username
					RakNet::RakString username;
					// Read out the info
					tokenBS.Read(username);
					// Create int variable for the token
					int token;
					// Read out the info
					tokenBS.Read(token);
					
					// Peer address
					RakNet::SystemAddress peerAddress;
					tokenBS.Read(peerAddress);

					std::cout << username.C_String() << " now has token " << token << "." << std::endl;
					m_tokenToUsername[token] = username.C_String();
					std::cout << "The token has now been assigned. The client will need to log in again." << std::endl;

					// Send message back to authentication server with a acknowledgement that token has been assigned
					// Create BitStream variable for the notification
					RakNet::BitStream acknowledgeBS;
					// Write information to the BitStream
					acknowledgeBS.Write((RakNet::MessageID)GameMessages::ID_GAME_SERVER_ACKNOWLEDGEMENT);
					acknowledgeBS.Write(peerAddress);
					acknowledgeBS.Write(token);
					// Send information to the authentication server
					pPeerInterface->Send(&acknowledgeBS, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
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
	const unsigned short PORT = 5456;
	// Get instance of Peer Interface
	RakNet::RakPeerInterface* pPeerInterface = nullptr;

	// Startup the server, and start it listening to clients
	std::cout << "Starting up the game server...\n" << std::endl;
	// Initialize the Raknet peer interface first
	pPeerInterface = RakNet::RakPeerInterface::GetInstance();

	// Create a socket descriptor to describe this connection
	RakNet::SocketDescriptor sd(PORT, 0);
	// Now call startup - max of 32 connections, on the assigned port
	pPeerInterface->Startup(32, &sd, 1);

	pPeerInterface->SetMaximumIncomingConnections(32);
	// Get instance of the server
	GameServer server;

	server.gs_handleNetworkMessages(pPeerInterface);

	return 0;
}