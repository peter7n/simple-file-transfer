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
# Main Program
# ==================================================================
# Get server name, port # and command from command line
hostArg = sys.argv[1]
serverPortArg = sys.argv[2]
commandArg = sys.argv[3]
clientSocket = serverConnect(hostArg, serverPortArg)

# Send command, file name and data port # to serverSocket
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

# Receive either INVALID COMMAND or READY message
# READY means the data port is ready for connections
readyMsg = ""
readyMsg = clientSocket.recv(1024).decode()
if readyMsg == "INVALID COMMAND":
    print(readyMsg)
    sys.exit("Error: Exiting Program")
while readyMsg != "READY":
    readyMsg = clientSocket.recv(1024)
print(readyMsg.decode())

dataSocket = serverConnect(hostArg, dataPortArg)
dataMsg = dataSocket.recv(1024)
print(dataMsg.decode())

dataSocket.close()
clientSocket.close()
