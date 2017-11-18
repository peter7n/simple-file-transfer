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

#define FILENAME_SIZE 256
#define BUFF_SIZE 70000

int serverSetup(int userPort);
void receiveCommand(int controlSock);
int dataSocketSetup(int userPort, int controlSock);
void sendDirectory(int dataSock);
void transferFile(char* fileName, int dataSock);
void writeSocket(int dataSock, char* buffer);
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
		printf("Server is ready to receive ...\n");
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
	int dataSocket = 0;
	char* strFrag;
	char strArray[3][FILENAME_SIZE];
	char clientCommand[100];
	char errorMsg[30] = "INVALID COMMAND";
	char fileName[FILENAME_SIZE];

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

	// Get the data port # and file name (if exists)
	if (i == 2)
		userPort = atoi(strArray[1]);
	else
		strcpy(fileName, strArray[1]);
		userPort = atoi(strArray[2]);

	// Open the data socket and execute the command
	// Otherwise send Invalid Command message
	if (strcmp(strArray[0], "-l") == 0)
	{
		dataSocket = dataSocketSetup(userPort, controlSock);
		sendDirectory(dataSocket);
	}
	else if (strcmp(strArray[0], "-g") == 0)
	{
		dataSocket = dataSocketSetup(userPort, controlSock);
		transferFile(fileName, dataSocket);
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
	printf("Connecting to Data Port ...\n");

	// Send READY message for client to connect to Data socket
	send(controlSock, readyMsg, strlen(readyMsg), 0);
	clilen = sizeof(cli_addr);

	// Accept connection on Data socket
	conSocket = accept(dataSocket,
			(struct sockaddr *) &cli_addr, &clilen);

	return conSocket;
}

/*********************************************************************
 ** Function: sendDirectory
 ** Description:
 **
 ** Parameters:
 ** Returns:
 *********************************************************************/
void sendDirectory(int dataSock)
{
	char msg[50] = "Sending directory on data port!";
	char tempBuff[FILENAME_SIZE];
	char directoryBuff[500];
	FILE* filePtr = popen("ls","r");
	directoryBuff[0] = '\0';

	// Read directory contents and store in directoryBuff
	while (fgets(tempBuff, FILENAME_SIZE, filePtr) != NULL)
		strcat(directoryBuff, tempBuff);
	pclose(filePtr);
	printf("%s", directoryBuff);

	// Send directory contents
	send(dataSock, directoryBuff, strlen(directoryBuff), 0);
	close(dataSock);
}

/*********************************************************************
 ** Function: transferFile
 ** Description:
 **
 ** Parameters:
 ** Returns:
 *********************************************************************/
void transferFile(char* fileName, int dataSock)
{
	FILE* filePtr;
	char msg[50] = "Sending file on data port!";
	char intToStr[10];
	char tempBuffer[BUFF_SIZE];
	char textBuffer[BUFF_SIZE];
	int dataSizeNum = 0;
	int returnStatus = 0;

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

	printf("%s", textBuffer);

	// Send the data size of the file to the client
	dataSizeNum = strlen(textBuffer);
	sprintf(intToStr, "%d", dataSizeNum);	// convert to string
   returnStatus = send(dataSock, intToStr, sizeof(intToStr), 0);
   if (returnStatus < 0)
     error("ERROR writing data size");
   // Send the file contents to the client
   // writeSocket(dataSock, textBuffer);

	// send(dataSock, textBuffer, strlen(textBuffer), 0);
	close(dataSock);
}

/*********************************************************************
 ** writeSocket
 ** Description: Writes data to the specified socket from the
 ** specified buffer in "chunks" of 1000 bytes.
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
      // Copy remaining data to be written until temp buffer
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
 ** error
 ** Description: Displays an error message
 ** Parameters: const char *msg
 *********************************************************************/
void error(const char *msg)
{
  perror(msg);
  exit(1);
}
