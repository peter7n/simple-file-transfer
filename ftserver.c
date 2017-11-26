/*********************************************************************
 ** Program Filename: ftserver.c
 ** Author: Peter Nguyen
 ** Date: 11/26/17
 ** CS 372-400
 ** Description: Project 2 - Server program for a simple file transfer
 ** application using two socket connections - one for control,
 ** the other for data transfer
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

#define FILENAME_SIZE 256
#define BUFF_SIZE 70000

int serverSetup(int userPort);
void receiveCommand(int controlSock);
int dataSocketSetup(int userPort, int controlSock);
void executeCommand(char* command, char* fileName, int dataSock, int dPort);
void writeSocket(int dataSock, char* buffer);
int isNumber(char* str);
void error(const char *msg);

int main(int argc, char *argv[])
{
	int serverSocket = 0;
	int connectionSocket = 0;
	socklen_t clilen;    	// size of client address
	struct sockaddr_in cli_addr;

	// Check if user provided a valid port #
	if (argc < 2 || argc > 2)
   {
		fprintf(stderr, "ERROR: wrong number of arguments - please provide port number\n");
		exit(1);
   }
	else
	{
		if (!isNumber(argv[1]))
		{
			fprintf(stderr, "ERROR: not a number\n");
			exit(1);
		}
		if (atoi(argv[1]) > 65535)
		{
			fprintf(stderr, "ERROR: port must be 65535 or under\n");
			exit(1);
		}
	}

	// Initialize Control "welcome" socket
	serverSocket = serverSetup(atoi(argv[1]));

	while (1)
	{
		listen(serverSocket, 5);
		printf("Server is ready to receive on %s\n", argv[1]);
		clilen = sizeof(cli_addr);
		connectionSocket = accept(serverSocket,
				(struct sockaddr *) &cli_addr, &clilen);
		printf("Connecting to client\n");

		receiveCommand(connectionSocket);

		close(connectionSocket);
	}

	return 0;
}

/*********************************************************************
 ** Function: serverSetup
 ** Description: Binds socket to a user-provided port number
 ** and returns the the newly created server socket
 ** Parameters: int userPort
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
 ** Description: Control socket receives command, file name and
 ** port # from client. Then either execute the command or send an
 ** error message to client if invalid command or file not found
 ** Parameters: int controlSock
 *********************************************************************/
void receiveCommand(int controlSock)
{
	int userPort = 0;
	int dataSocket = 0;
	char* strFrag;
	char strArray[3][FILENAME_SIZE];
	char clientCommand[100];
	char errorMsg[30] = "INVALID COMMAND";
	char errorMsg2[30] = "FILE NOT FOUND";
	char fileName[FILENAME_SIZE];

	// Receive command string from client
	memset(clientCommand, 0, sizeof(clientCommand));
	recv(controlSock, clientCommand, sizeof(clientCommand), 0);

	// Parse the string for commmand, (filename), port#
	strFrag = strtok(clientCommand, " ");
	int i = 0;
	while (strFrag != NULL)
	{
		strcpy(strArray[i], strFrag);
		strFrag = strtok(NULL, " ");
		i++;
	}

	// Get the data port # and file name (if exists)
	if (i == 2)
		userPort = atoi(strArray[1]);
	else
	{
		strcpy(fileName, strArray[1]);
		userPort = atoi(strArray[2]);
	}

	// Open the data socket and execute the command
	// Otherwise send Invalid Command message
	if (strcmp(strArray[0], "-l") == 0)
	{
		dataSocket = dataSocketSetup(userPort, controlSock);
		executeCommand(strArray[0], NULL, dataSocket, userPort);
	}
	else if (strcmp(strArray[0], "-g") == 0)
	{
		if (access(fileName, F_OK) != -1)
		{
			dataSocket = dataSocketSetup(userPort, controlSock);
			executeCommand(strArray[0], fileName, dataSocket, userPort);
		}
		else
		{
			printf("FILE %s NOT FOUND\n", fileName);
			send(controlSock, errorMsg2, strlen(errorMsg2), 0);
		}
	}
	else
	{
		printf("INVALID COMMAND\n");
		send(controlSock, errorMsg, strlen(errorMsg), 0);
	}
}

/*********************************************************************
 ** Function: dataSocketSetup
 ** Description: Creates the data socket and accepts the new
 ** connection from the client
 ** Parameters: int userPort, int ControlSock
 *********************************************************************/
int dataSocketSetup(int userPort, int controlSock)
{
	int dataSocket = 0;
	int conSocket = 0;
	char readyMsg[30] = "READY";
	socklen_t clilen;
	struct sockaddr_in cli_addr;

	// Initialize the Data "welcome" socket
	dataSocket = serverSetup(userPort);
	listen(dataSocket, 1);

	// Send READY message for client to connect to Data socket
	send(controlSock, readyMsg, strlen(readyMsg), 0);
	clilen = sizeof(cli_addr);

	// Accept connection on Data socket
	conSocket = accept(dataSocket,
			(struct sockaddr *) &cli_addr, &clilen);
	printf("Connecting to data port on %d\n", userPort);

	return conSocket;
}

/*********************************************************************
 ** Function: executeCommand
 ** Description: Gets either the contents of the directory or the
 ** requested file and sends them to the client
 ** Parameters: char* command, char* fileName, int dataSock, int dPort
 *********************************************************************/
void executeCommand(char* command, char* fileName, int dataSock, int dPort)
{
	FILE* dirFilePtr;
	FILE* filePtr;
	char tempDirBuff[FILENAME_SIZE];
	char directoryBuff[500];
	char confirm[15];
	char intToStr[10];
	char tempBuffer[BUFF_SIZE];
	char textBuffer[BUFF_SIZE];
	int dataSizeNum = 0;
	int returnStatus = 0;

	// Command received: get directory contents
	if (strcmp(command, "-l") == 0)
	{
		textBuffer[0] = '\0';
		dirFilePtr = popen("ls","r");

		// Read directory contents and store in textBuffer
		while (fgets(tempDirBuff, FILENAME_SIZE, dirFilePtr) != NULL)
			strcat(textBuffer, tempDirBuff);
		pclose(dirFilePtr);
		printf("Sending directory contents to client on data port %d\n", dPort);
	}
	// Command received: get file
	else if (strcmp(command, "-g") == 0)
	{
		// Read the file into buffer
		filePtr = fopen(fileName, "r");
		if (filePtr == NULL)
		{
			fprintf(stderr, "Could not open file\n");
			exit(1);
		}
		bzero(textBuffer, BUFF_SIZE);
		while (fgets(tempBuffer, BUFF_SIZE, filePtr))
			strcat(textBuffer, tempBuffer);
		fclose(filePtr);
		printf("Sending file %s to client on data port %d\n", fileName, dPort);
	}
	else
	{
		printf("INVALID COMMAND\n");
		return;
	}

	// Send the data size of the file to the client
	dataSizeNum = strlen(textBuffer);
	sprintf(intToStr, "%d", dataSizeNum);	// convert to string
   returnStatus = send(dataSock, intToStr, sizeof(intToStr), 0);
   if (returnStatus < 0)
   	error("ERROR writing data size");

	// Receive confirmation from client before writing
	memset(confirm, 0, 5);
	recv(dataSock, confirm, sizeof(confirm), 0);

	if (strcmp(confirm, "SIZE OK") == 0)
	{
	   // Send the file contents to the client
	   writeSocket(dataSock, textBuffer);
	}
	else
		printf("Size not received by client\n");

	close(dataSock);
}

/*********************************************************************
 ** writeSocket
 ** Description: Writes data to the specified socket from the
 ** specified buffer
 ** Parameters: int dataSock, char* buffer
 *********************************************************************/
void writeSocket(int dataSock, char* buffer)
{
  int bytesWrit,
      totalBytesWrit = 0,
      index,
      tempIndex;
  char tempBuffer[BUFF_SIZE];

  bytesWrit = write(dataSock, buffer, strlen(buffer));

  if (bytesWrit < 0)
    error("ERROR writing to socket");
  else if (bytesWrit < strlen(buffer))
  {
    // Continue to write until all data has been sent
    while (totalBytesWrit != strlen(buffer))
    {
      totalBytesWrit += bytesWrit;
      index = totalBytesWrit + 1;
      tempIndex = 0;
      do
      // Copy remaining data to be written into temp buffer
      {
        tempBuffer[tempIndex] = buffer[index];
        index++;
        tempIndex++;
      } while (buffer[index] != '\0');

      bytesWrit = write(dataSock, tempBuffer, strlen(tempBuffer));
    }
  }
}

/*********************************************************************
 ** isNumber
 ** Description: Returns 1 if the provided string is a number, 0 otherwise
 ** Parameters: char* str
 *********************************************************************/
int isNumber(char* str)
{
	int i = 0;

	while(str[i] != '\0')
	{
		if (!isdigit(str[i]))
			return 0;
		i++;
	}
	return 1;
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
