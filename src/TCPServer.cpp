#include "TCPServer.h"
#include <stdio.h>
#include <iostream> 
#include <sys/types.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 

TCPServer::TCPServer() {

}


TCPServer::~TCPServer() {

}

/**********************************************************************************************
 * bindSvr - Creates a network socket and sets it nonblocking so we can loop through looking for
 *           data. Then binds it to the ip address and port
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::bindSvr(const char *ip_addr, short unsigned int port)
{
    int sock = 0;; 
    struct sockaddr_in addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        printf("Socket creation failed\n"); 
   
    addr.sin_family = AF_INET; 
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY; 

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr))<0) 
        printf("Server bind failed\n");

    this->socketFD = sock;
    this->addrServ = addr;
}

/**********************************************************************************************
 * listenSvr - Performs a loop to look for connections and create TCPConn objects to handle
 *             them. Also loops through the list of connections and handles data received and
 *             sending of data. 
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::listenSvr()
{
    int newSocket;
    if (listen(this->socketFD, 3) < 0) 
        printf("Listen failed");

    newSocket = accept(this->socketFD, (struct sockaddr *)&this->addrServ, (socklen_t*)sizeof(this->addrServ));
    if (newSocket < 0) 
        printf("Accept failed");
    else
        this->connectionList.push_back(newSocket);



    std::string sendmsg; 
    std::getline(std::cin, sendmsg);
    char sendmsgchar[sendmsg.size() + 1];
	sendmsg.copy(sendmsgchar, sendmsg.size() + 1);
	sendmsgchar[sendmsg.size()] = '\0';
    if (send(this->socketFD, sendmsgchar, strlen(sendmsgchar), 0) < 0)
        printf("Connection has been terminated\n"); 
    
    char readmsg[1024] = {0};
    if (read(this->connectionList[0], readmsg, 1024) < 0) //but read all of them
        printf("Read failed\n"); 

    printf("%s\n", readmsg); 
}

/**********************************************************************************************
 * shutdown - Cleanly closes the socket FD.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::shutdown() {
}
