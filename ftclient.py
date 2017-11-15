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
    string = command + " " + fileName + " " + dataPort
    clientSock.send(string.encode())
    clientSock.close()

# ==================================================================
# Main Program
# ==================================================================
# Get server name and port # from command line
clientSocket = serverConnect(sys.argv[1], sys.argv[2])

# Send command, file name and data port # to serverSocket
if len(sys.argv) < 5:
    print("Not enough arguments")
elif len(sys.argv) == 5:
    sendCommand(sys.argv[3], "", sys.argv[4], clientSocket)
elif len(sys.argv) == 6:
    sendCommand(sys.argv[3], sys.argv[4], sys.argv[5], clientSocket)
else:
    print("Incorrect number of arguments")

dataSocket = serverConnect(sys.argv[1], "32342")
dataMsg = dataSocket.recv(1024).decode()
print dataMsg
dataSocket.close()
