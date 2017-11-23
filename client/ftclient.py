# ==================================================================
# Program Filename: ftclient.py
# Author: Peter Nguyen
# Date: 11/26/17
# CS 372-400
# Description: Project 2 -
#
# ==================================================================

import sys
import os.path
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
    # Receive either error message or READY message
    # READY means the data port is ready to connect or send
    readyMsg = ""
    readyMsg = clientSock.recv(1024).decode()
    if readyMsg == "INVALID COMMAND" or readyMsg == "FILE NOT FOUND":
        print(readyMsg)
        sys.exit("Exiting Program")
    while readyMsg != "READY":
        readyMsg = clientSock.recv(1024)

# ==================================================================
# Function: readSocket
# Description:
#
# Parameters:
# Returns:
#  ==================================================================

def readSocket(dataSock, buff, size):
    while True:
        data = dataSock.recv(4096)
        buff += data
        if len(buff) == size + 5:
            break
    return buff

# ==================================================================
# Function: receiveData
# Description:
#
# Parameters:
# Returns:
#  ==================================================================

def receiveData(dataSock):
    # Receive file size from server
    fileSize = dataSock.recv(5)
    fileSizeInt = int(filter(str.isdigit, fileSize))
    # Send confirmation to server that size was received
    sizeConfirm = "SIZE OK"
    dataSock.send(sizeConfirm.encode())
    # Receive file contents from server
    txtBuffer = ""
    txtBuffer = readSocket(dataSock, txtBuffer, fileSizeInt)
    return txtBuffer

# ==================================================================
# Function: executeCommand
# Description:
#
# Parameters:
# Returns:
#  ==================================================================

def executeCommand(command, fileName, data):
    if command == "-l":
        print(data)
    elif command == "-g":
        # Check if file exists in current directory
        if os.path.isfile(fileName):
            sys.exit("File already exists")
        # Write buffer to file in the current directory
        fileTrans = open(fileName, "w")
        fileTrans.write(data)
        fileTrans.close()
        print("Transfer complete")
    else:
        print("Invalid command")

# ==================================================================
# Main Program
# ==================================================================

# Get server name, port # and command from command line
hostArg = sys.argv[1]

if sys.argv[2].isdigit():
    serverPortArg = sys.argv[2]
else:
    sys.exit("Server port must be a number")

commandArg = sys.argv[3]
# Connect to server
clientSocket = serverConnect(hostArg, serverPortArg)

# Send command, file name and data port # to server
# depending on the number of arguments from command line
if len(sys.argv) < 5 or len(sys.argv) > 6:
    print("Invalid number of arguments")

elif len(sys.argv) == 5:
    fileArg = ""
    if sys.argv[4].isdigit():
        dataPortArg = sys.argv[4]
    else:
        sys.exit("Data port must be a number")
    sendCommand(commandArg, "", dataPortArg, clientSocket)
elif len(sys.argv) == 6:
    fileArg = sys.argv[4]
    if sys.argv[5].isdigit():
        dataPortArg = sys.argv[5]
    else:
        sys.exit("Data port must be a number")
    sendCommand(commandArg, fileArg, dataPortArg, clientSocket)

# Get confirmation from server that command is valid
confirmCommand(clientSocket)

# Connect to Data Socket
dataSocket = serverConnect(hostArg, dataPortArg)

# Get either directory listing or file data from server
returnedData = receiveData(dataSocket)

# Execute the specified command
executeCommand(commandArg, fileArg, returnedData)

dataSocket.close()
clientSocket.close()
