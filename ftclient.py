# ==================================================================
# Program Filename: ftclient.py
# Author: Peter Nguyen
# Date: 11/26/17
# CS 372-400
# Description: Project 2 -
#
# ==================================================================

import sys
from socket import *

# ==================================================================
# Function: serverConnect
# Description:
#
# Parameters:
# Returns:
#  ==================================================================

def serverConnect(userHost, userPort):
    serverName = userHost
    serverPort = int(userPort)
    newSocket = socket(AF_INET, SOCK_STREAM)
    # Connect to server
    print("Connecting ...")
    newSocket.connect((serverName, serverPort))
    return newSocket

# ==================================================================
# Function: sendCommand
# Description:
#
# Parameters:
# Returns:
#  ==================================================================

def sendCommand(command, fileName, dataPort, clientSock):
    if fileName == "":
        string = command + " " + dataPort
    else:
        string = command + " " + fileName + " " + dataPort
    clientSock.send(string.encode())

# ==================================================================
# Function: receiveReady
# Description:
#
# Parameters:
# Returns:
#  ==================================================================

def confirmCommand(clientSock):
    # Receive either INVALID COMMAND or READY message
    # READY means the data port is ready for connection
    readyMsg = ""
    readyMsg = clientSock.recv(1024).decode()
    if readyMsg == "INVALID COMMAND":
        print(readyMsg)
        sys.exit("Exiting Program")
    while readyMsg != "READY":
        readyMsg = clientSock.recv(1024)
    print(readyMsg.decode())

# ==================================================================
# Function: readSocket
# Description:
#
# Parameters:
# Returns:
#  ==================================================================
def readSocket(dataSock, buffer):
    while True:
        print("")

# ==================================================================
# Main Program
# ==================================================================

# Get server name, port # and command from command line
hostArg = sys.argv[1]
serverPortArg = sys.argv[2]
commandArg = sys.argv[3]
# Connect to server
clientSocket = serverConnect(hostArg, serverPortArg)

# Send command, file name and data port # to server
# depending on the number of arguments from command line
if len(sys.argv) < 5:
    print("Not enough arguments")
elif len(sys.argv) == 5:
    dataPortArg = sys.argv[4]
    sendCommand(commandArg, "", dataPortArg, clientSocket)
elif len(sys.argv) == 6:
    fileArg = sys.argv[4]
    dataPortArg = sys.argv[5]
    sendCommand(commandArg, fileArg, dataPortArg, clientSocket)
else:
    print("Incorrect number of arguments")

confirmCommand(clientSocket)

# Connect to Data Socket
dataSocket = serverConnect(hostArg, dataPortArg)

if commandArg == "-l":
    dirContents = dataSocket.recv(5000)
    print(dirContents.decode())
elif commandArg == "-g":
    fileSize = dataSocket.recv(5000)
    print(fileSize.decode())

    # Run readSock

    # write buffer to file in current dir

dataSocket.close()
clientSocket.close()
