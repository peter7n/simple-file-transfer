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

int serverSetup(int userPort);
void receiveCommand(int controlSock);
void dataSocketSetup(int userPort, int controlSock);
void sendDirectory();
void transferFile();
void error(const char *msg);

int main(int argc, char *argv[])
{
	int serverSocket = 0;
	int connectionSocket = 0;
	socklen_t clilen;    	// size of client address
	struct sockaddr_in cli_addr;

	// Check if user provided a valid port
	if (argc < 2)
   {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
   }

	// Initialize Control "welcome" socket
	serverSocket = serverSetup(atoi(argv[1]));

	while (1)
	{
		listen(serverSocket, 5);
		printf("Server is ready to receive\n");
		clilen = sizeof(cli_addr);
		connectionSocket = accept(serverSocket,
				(struct sockaddr *) &cli_addr, &clilen);

		receiveCommand(connectionSocket);

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
int serverSetup(int userPort)
{
	int newSocket = 0;
	int portNum = 0;
	struct sockaddr_in serverAddress;

	// Open the socket
	newSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (newSocket < 0)
		error("ERROR opening socket");

	// Set server address and port number
	bzero((char *) &serverAddress, sizeof(serverAddress)); // reset to zero's
	portNum = userPort;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNum);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	// Bind socket to address
   if (bind(newSocket, (struct sockaddr *) &serverAddress,
			sizeof(serverAddress)) < 0)
 		error("ERROR on binding");

	return newSocket;
}

/*********************************************************************
 ** Function: receiveCommand
 ** Description:
 **
 ** Parameters:
 ** Returns:
 *********************************************************************/
void receiveCommand(int controlSock)
{
	int userPort = 0;
	char* strFrag;
	char strArray[3][30];
	char clientCommand[100];
	char errorMsg[30] = "INVALID COMMAND";

	// Receive command string from client
	memset(clientCommand, 0, sizeof(clientCommand));
	recv(controlSock, clientCommand, sizeof(clientCommand), 0);
	printf("client command: %s\n", clientCommand);

	// Parse the string for commmand, (filename), port#
	strFrag = strtok(clientCommand, " ");
	int i = 0;
	while (strFrag != NULL)
	{
		strcpy(strArray[i], strFrag);
		printf("%s\n", strArray[i]);
		strFrag = strtok(NULL, " ");
		i++;
	}

	// Get the data port #
	if (i == 2)
		userPort = atoi(strArray[1]);
	else
		userPort = atoi(strArray[2]);

	// Execute the command sent or send error message
	if (strcmp(strArray[0], "-l") == 0)
	{
		dataSocketSetup(userPort, controlSock);
		sendDirectory();
	}
	else if (strcmp(strArray[0], "-g") == 0)
	{
		dataSocketSetup(userPort, controlSock);
		transferFile();
	}
	else
	{
		printf("INVALID COMMAND\n");
		send(controlSock, errorMsg, strlen(errorMsg), 0);
	}
}

/*********************************************************************
 ** Function: dataSocketSetup
 ** Description:
 **
 ** Parameters:
 ** Returns:
 *********************************************************************/
void dataSocketSetup(int userPort, int controlSock)
{
	int dataSocket = 0;
	int conSocket = 0;
	char readyMsg[30] = "READY";
	char msg[30] = "Hello, world on data port!";
	socklen_t clilen;
	struct sockaddr_in cli_addr;

	// Initialize the Data "welcome" socket
	dataSocket = serverSetup(userPort);
	listen(dataSocket, 1);
	printf("Data Port ready to receive\n");

	// Send READY message for client to connect to Data socket
	send(controlSock, readyMsg, strlen(readyMsg), 0);
	clilen = sizeof(cli_addr);

	// Accept connection on Data socket
	conSocket = accept(dataSocket,
			(struct sockaddr *) &cli_addr, &clilen);
	send(conSocket, msg, strlen(msg), 0);
	close(conSocket);
}

void sendDirectory()
{
	printf("***Perform -l command here\n");
}

void transferFile()
{
	printf("***Perform -g command here\n");
}

/*********************************************************************
 ** error
 ** Description: Displays an error message
 ** Parameters: const char *msg
 *********************************************************************/
void error(const char *msg)
{
  perror(msg);
  exit(1);
}
