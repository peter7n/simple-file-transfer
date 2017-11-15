/*********************************************************************
 ** Program Filename: ftserver.c
 ** Author: Peter Nguyen
 ** Date: 11/26/17
 ** CS 372-400
 ** Description: Project 2 -
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int serverSetup(char *userPort);

int main(int argc, char *argv[])
{
	int serverSocket = 0;
	int connectionSocket = 0;
	char clientCommand[100];
	char dataPort[100];
	socklen_t clilen;    	// size of client address
	struct sockaddr_in cli_addr;

	// Check if user provided a valid port
	if (argc < 2)
   {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
   }

	// Initialize "welcome" socket
	serverSocket = serverSetup(argv[1]);

	while (1)
	{
		listen(serverSocket, 5);
		printf("Server is ready to receive\n");
		clilen = sizeof(cli_addr);
		connectionSocket = accept(serverSocket,
				(struct sockaddr *) &cli_addr, &clilen);

		memset(clientCommand, 0, 100);
		recv(connectionSocket, clientCommand, 100, 0);
		printf("client command: %s\n", clientCommand);
		close(connectionSocket);
	}

	return 0;
}

/*********************************************************************
 ** Function: serverSetup
 ** Description:
 **
 ** Parameters: char* userPort
 ** Returns: int, the new server socket
 *********************************************************************/

int serverSetup(char *userPort)
{
	int newSocket = 0;
	int portNum = 0;
	struct sockaddr_in serverAddress;

	// Open the socket
	newSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (newSocket < 0)
		printf("ERROR opening socket\n");

	// Set server address and port number
	bzero((char *) &serverAddress, sizeof(serverAddress)); // reset to zero's
	portNum = atoi(userPort);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNum);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	// Bind socket to address
   if (bind(newSocket, (struct sockaddr *) &serverAddress,
			sizeof(serverAddress)) < 0)
 		printf("ERROR on binding\n");

	return newSocket;
}
