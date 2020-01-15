
#include "TCPClient.h"

#include <stdio.h>
#include <iostream> 
#include <sys/types.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 


/**********************************************************************************************
 * TCPClient (constructor) - Creates a Stdin file descriptor to simplify handling of user input. 
 *
 **********************************************************************************************/

TCPClient::TCPClient() {
}

/**********************************************************************************************
 * TCPClient (destructor) - No cleanup right now
 *
 **********************************************************************************************/

TCPClient::~TCPClient() {

}

/**********************************************************************************************
 * connectTo - Opens a File Descriptor socket to the IP address and port given in the
 *             parameters using a TCP connection.
 *
 *    Throws: socket_error exception if failed. socket_error is a child class of runtime_error
 **********************************************************************************************/

void TCPClient::connectTo(const char *ip_addr, unsigned short port)
{
    int sock = 0; 
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        printf("Socket creation failed\n"); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(port); 

    if(inet_pton(AF_INET, ip_addr, &serv_addr.sin_addr)<=0)  //0, -1 is returned for lsightly different errors
        printf("Invalid address\n");

   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        printf("Connection Failed\n"); 

    this->socketFD = sock;
}

/**********************************************************************************************
 * handleConnection - Performs a loop that checks if the connection is still open, then 
 *                    looks for user input and sends it if available. Finally, looks for data
 *                    on the socket and sends it.
 * 
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPClient::handleConnection()
{
    //std::string readmsg, sendmsg;
    std::string sendmsg; 
    std::getline(std::cin, sendmsg);
    char sendmsgchar[sendmsg.size() + 1];
	sendmsg.copy(sendmsgchar, sendmsg.size() + 1);
	sendmsgchar[sendmsg.size()] = '\0';
    if (send(this->socketFD, sendmsgchar, strlen(sendmsgchar), 0) < 0)
        printf("Connection has been terminated\n"); 
    
    char readmsg[1024] = {0};
    if (read(this->socketFD, readmsg, 1024) < 0)
        printf("Read failed\n"); 

    printf("%s\n", readmsg); 
}

/**********************************************************************************************
 * closeConnection - Your comments here
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPClient::closeConn() {
}


