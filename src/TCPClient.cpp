
#include "TCPClient.h"

#include <stdio.h>
#include <iostream> 
#include <sys/types.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>
#include "FileDesc.h"


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
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(port); 
    if(inet_pton(AF_INET, ip_addr, &serv_addr.sin_addr)<=0)  //0, -1 is returned for lsightly different errors
        throw std::runtime_error("Invalid address\n");
    if (connect(this->socketFD, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 && errno != EINPROGRESS) //since non-blocking, need to check to see if it is just spinning up
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
    struct timeval timeVal;
    timeVal.tv_sec = 0;
    timeVal.tv_usec = 10; // timeout after 10 microseconds
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
    FD_SET(STDIN_FILENO, &readfds);
    socketActivity = select(1, &readfds, NULL, NULL, &timeVal);
    if (FD_ISSET(STDIN_FILENO, &readfds)) // stdin fd, then there is stuff to place in buffer
    {
        char readBuf[1024];
        int amountRead = 0;
        if ((amountRead = (read(STDIN_FILENO, readBuf, 1024))) <= 0)
            throw std::runtime_error("read error from stdin");

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
    FD_ZERO(&readfds);   
    FD_SET(this->socketFD, &readfds);
    socketActivity = select(this->socketFD + 1, &readfds, NULL, NULL, &timeVal);
    if (FD_ISSET(this->socketFD, &readfds))
    {
        char checkBuf[1024];
        if ((amountRead = read(this->socketFD, checkBuf, 1024)) <= 0) //if select sees data but read none then connection closed
        {
            std::cout << "The server closed the connection\n";
            exit(0);
        }
        std::string output(checkBuf, amountRead);
        std::cout << output; 
    }
}

