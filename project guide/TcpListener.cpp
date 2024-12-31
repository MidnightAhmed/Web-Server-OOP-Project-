#include "TcpListener.h"


int TcpListener::init()
{
	// Initialze winsock
	WSADATA wsData; // structure containing data about winsock implementation
	WORD ver = MAKEWORD(2, 2); // makes a 32bit word containing the winsock ver(here its v2.2)

	int wsOk = WSAStartup(ver, &wsData); // starts the winsock
	if (wsOk != 0) // if its initialized successfully, it returns zero
	{
		return wsOk;
	}

	// Create a socket
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	// AF_INET tells the ip address is in IPV4 format
	// SOCK_STREAM tells the connection in tcp 
	if (m_socket == INVALID_SOCKET) // checks if its success
	{
		return WSAGetLastError();
	}

	// Bind the ip address and port to a socket
	sockaddr_in hint;
	//sockaddr_in is a struct representing socket address
	hint.sin_family = AF_INET;
	//AF_INET tells the address family is ipv4
	hint.sin_port = htons(m_port);
	//htons converts the port # into network byte order i.e. big-endian 
	inet_pton(AF_INET, m_ipAddress, &hint.sin_addr);
	//converts ip address into binary and stores in struct called sin_addr
	
	if (bind(m_socket, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR)
	{
		return WSAGetLastError();
	}
	// now the socket is bind to the address using the pointer to the
	// hint structure we created

	// Tell Winsock the socket is for listening 
	//SOMAXCONN is the max. # of connections in the queue of socket
	if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR)
	{
		return WSAGetLastError();
	}

	// Create the master file descriptor set and zero it
	// FD_ZERO() clears the fd set
	FD_ZERO(&m_master);

	// Add our first socket that we're interested in interacting with; the listening socket!
	// It's important that this socket is added for our server or else we won't 'hear' incoming
	// connections 
	// FD_SET() adds a socket to the set
	FD_SET(m_socket, &m_master);

	return 0;
}

int TcpListener::run()
{
	// this will be changed by the \quit command (see below, bonus not in video!)
	bool running = true;

	while (running)
	{
		// Make a copy of the master file descriptor set, this is SUPER important because
		// the call to select() is _DESTRUCTIVE_. The copy only contains the sockets that
		// are accepting inbound connection requests OR messages. 

		// E.g. You have a server and it's master file descriptor set contains 5 items;
		// the listening socket and four clients. When you pass this set into select(), 
		// only the sockets that are interacting with the server are returned. Let's say
		// only one client is sending a message at that time. The contents of 'copy' will
		// be one socket. You will have LOST all the other sockets.

		// SO MAKE A COPY OF THE MASTER LIST TO PASS INTO select() !!!

		fd_set copy = m_master;

		// See who's talking to us
		// finds how many sockets are active(either trying to connect or sharing data)
		// select() also edits the copy fd_set
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		// Loop through all the current connections / potential connect
		for (int i = 0; i < socketCount; i++)
		{
			// Makes things easy for us doing this assignment
			SOCKET sock = copy.fd_array[i];

			// Is it an inbound communication?
			// checks if sock is listening socket, if yes then it makes a client socket
			// (conncected socket)
			if (sock == m_socket)
			{
				// Accept a new connection
				SOCKET client = accept(m_socket, nullptr, nullptr);

				// Add the new connection to the list of connected clients
				FD_SET(client, &m_master);

			}
			// if its not a listening socket then it is a connected socket
			else // It's an inbound message
			{
				char buf[4096];
				ZeroMemory(buf, 4096);

				// Receive message
				// recv() stores the data in buf, and returns the # of bytes recieved
				// if no data is recieved, means the client dropped(disconnected)
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0)
				{
					// Drop the client
					FD_CLR(sock, &m_master);
					closesocket(sock);
				}
				else
				{
					// now the msg from client is used to send the data back
					onMessageReceived(sock, buf, bytesIn);
				}
			}
		}
	}

	// Remove the listening socket from the master file descriptor set and close it
	// to prevent anyone else trying to connect.
	FD_CLR(m_socket, &m_master);
	closesocket(m_socket);

	while (m_master.fd_count > 0)
	{
		// Get the socket number
		SOCKET sock = m_master.fd_array[0];

		// Remove it from the master file list and close the socket
		FD_CLR(sock, &m_master);
		closesocket(sock);
	}

	// Cleanup winsock
	WSACleanup();
	return 0;
}


void TcpListener::sendToClient(int clientSocket, const char* msg, int length)
{
	send(clientSocket, msg, length, 0);
}

void TcpListener::onMessageReceived(int clientSocket, const char* msg, int length)
{

}
