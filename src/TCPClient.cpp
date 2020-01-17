
#include "TCPClient.h"
#include <vector>
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>      
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h>
#include <fcntl.h>


/**********************************************************************************************
 * TCPClient (constructor) - Creates a Stdin file descriptor to simplify handling of user input. 
 *
 **********************************************************************************************/

TCPClient::TCPClient()
{
    if((this->socketFD = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        throw std::runtime_error("Socket creation failed\n");
    fcntl(this->socketFD, F_SETFL, fcntl(this->socketFD, F_GETFL) | O_NONBLOCK);
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
    //create the sock address struct
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(port); 

    //0, -1 is returned for sightly different errors
    if(inet_pton(AF_INET, ip_addr, &serv_addr.sin_addr)<=0)  
        throw std::runtime_error("Invalid address\n");

     //since non-blocking, need to check to see if it is just spinning up
    if (connect(this->socketFD, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 && errno != EINPROGRESS)
        throw std::runtime_error("Connection Failed\n");
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
    //create the timeVal struct for timeout of sends and reads
    struct timeval timeVal;
    timeVal.tv_sec = 0;
    timeVal.tv_usec = 10; // timeout after 10 microseconds
    
    //create the fd_set, can monitor multiple file descriptors
    fd_set readfds;

    while (true)
    {
        this->sendMsg(readfds, timeVal);
        this->readMsg(readfds, timeVal);
    }
}

/**********************************************************************************************
 * closeConnection - Your comments here
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPClient::closeConn()
{
    exit(1);
}

/**********************************************************************************************
 * sendMsg - reads the data from STDIN and sends it to the server
 *
 *    Throws: runtime_error for unrecoverable types
 **********************************************************************************************/
void TCPClient::sendMsg(fd_set &readfds, struct timeval &timeVal)
{
    int socketActivity;
    //clear the socket set  
    FD_ZERO(&readfds);

    //set the stdin for readfds   
    FD_SET(STDIN_FILENO, &readfds);

    //poll the sockets (stdin) for activity
    socketActivity = select(1, &readfds, NULL, NULL, &timeVal);

    ////if stdin fd, then there something has been written, place in buffer
    if (FD_ISSET(STDIN_FILENO, &readfds)) 
    {
        char readBuf[1024];
        int amountRead = 0;
        if ((amountRead = (read(STDIN_FILENO, readBuf, 1024))) <= 0)
            throw std::runtime_error("read error from stdin");

        //send what was read from stdin
        send(socketFD, readBuf, amountRead, 0);
    }
}

/**********************************************************************************************
 * readMsg - reads the data from server and prints it
 *
 *    Throws: runtime_error for unrecoverable types
 **********************************************************************************************/
void TCPClient::readMsg(fd_set &readfds, struct timeval &timeVal)
{
    int socketActivity, amountRead=0;

    // set the client fd into the readfds
    FD_ZERO(&readfds);   
    FD_SET(this->socketFD, &readfds);

    // poll the sockets for the client fd
    socketActivity = select(this->socketFD + 1, &readfds, NULL, NULL, &timeVal);
    if (FD_ISSET(this->socketFD, &readfds))
    {
        char checkBuf[1024];
        //if client FD was set but read no data then connection closed
        if ((amountRead = read(this->socketFD, checkBuf, 1024)) <= 0) 
        {
            std::cout << "The server closed the connection\n";
            exit(0);
        }

        //else put what we read to the screen
        std::string output(checkBuf, amountRead);
        std::cout << output; 
    }
}

