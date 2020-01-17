#include "TCPServer.h"
#include <vector>
#include "Server.h"
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

TCPServer::TCPServer()
{
    if ((this->socketFD = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        throw std::runtime_error("Socket creation failed\n");
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
    // create the sockaddr struct
    this->address.sin_family = AF_INET;   
    this->address.sin_addr.s_addr = INADDR_ANY;   
    this->address.sin_port = htons( port );

    // bind server to passed in ip and port
    if ((bind(this->socketFD, (struct sockaddr *)&this->address, sizeof(this->address))) < 0)
        throw std::runtime_error("Bind failed\n");
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
    int newSocket, socketActivity, maxSocket=this->socketFD;

    // create timeval struct for timeout
    struct timeval timeVal;
    timeVal.tv_sec = 0;
    timeVal.tv_usec = 10; // timeout after 10 microseconds

    //create the fd_set, can monitor multiple file descriptors
    fd_set readfds;
    
    char readMsg[1024];
    int readMsgSize;

    //ensure listen return successfully
    if (listen(this->socketFD, 3) < 0)   
        throw std::runtime_error("Listen failed\n");
        
    
    while(true)
    {   
        //clear the socket set  
        FD_ZERO(&readfds);   
        //add server socket to set  
        FD_SET(this->socketFD, &readfds);    
        
        //add each of the clients to the readfds to be monitored
        for(int clientSocket : clientSocketList)
        {
            if(clientSocket > 0)   
                FD_SET(clientSocket , &readfds);
            if(clientSocket > maxSocket)
                maxSocket = clientSocket; 
        }

        // poll the sockets of the readfds for activity
        socketActivity = select(maxSocket + 1 , &readfds, NULL, NULL, &timeVal);   
    
        // because non-blocking, ensure on fail it is not due to select still spinning ip
        if ((socketActivity < 0) && (errno!=EINTR))   
            printf("socketActivty select error");      
            
        //If something happened on the server socket, then its an incoming connection  
        if (FD_ISSET(this->socketFD, &readfds))   
        {   
            int addrLen = sizeof(this->address);
            if ((newSocket = accept(this->socketFD, (struct sockaddr *)&this->address, (socklen_t*)&addrLen)) < 0)   
                throw std::runtime_error("Accept failed\n");
            
            //inform user of connection  
            printf("Connected! Socket FD: %d, ip: %s, port: %d\n", newSocket, inet_ntoa(address.sin_addr), ntohs (address.sin_port));
        
            //send new connection menu message
            this->serverSend(newSocket, this->menuMsg);
                
            //add new socket to array of sockets
            clientSocketList.push_back(newSocket);
            
        }   
            
        //else its some operation on some other socket, loop through the clients
        for(auto it=clientSocketList.begin(); it!=clientSocketList.end(); it++)
        {
            int clientSocket = *it;
            if(clientSocket == 0) 
                continue;  
            
            // if we found the client that had activity
            if (FD_ISSET(clientSocket, &readfds))   
            {   
                //Check if it was for closing , and also read the incoming message  
                memset(readMsg, 0, 1024);
                if ((readMsgSize = read(clientSocket, readMsg, 1024)) == 0)   
                {   
                    //it was for closing, somebody disconnected, get details
                    int addrLen = sizeof(this->address);
                    getpeername(clientSocket , (struct sockaddr*)&this->address, (socklen_t*)&addrLen);   
                    printf("Disconnected! ip %s, port %d\n", inet_ntoa(this->address.sin_addr), ntohs(this->address.sin_port));   
                    
                    //Close the socket and delete the client from the list
                    close( clientSocket );
                    clientSocketList.erase(it--);
                    
                }   
                else 
                    // it wasn't for closing, handle the msg the client sent the server
                    handleMsg(readMsg, readMsgSize, clientSocket);
                      
            }   
        }   
    }      
}

/**********************************************************************************************
 * shutdown - Cleanly closes the socket FD.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::shutdown()
{
    exit(0);
}


/**********************************************************************************************
 * serverSend - sends a std::string to the given socket
 *
 *    Throws: runtime_error for unrecoverable types
 **********************************************************************************************/
void TCPServer::serverSend(int socketFD, std::string &msg)
{
    const char* cstring = msg.c_str();
    if( (send(socketFD, cstring, strlen(cstring), 0)) != strlen(cstring) )   //send messages function.
        throw std::runtime_error("Server to client send failed\n");
}

/**********************************************************************************************
 * handleMsg - handles the message recieved from the client
 *
 *   
 **********************************************************************************************/
void TCPServer::handleMsg(char readMsg[], int readMsgSize, int clientSocket)
{
    //set the string terminating NULL byte on the end of the data read  
    readMsg[readMsgSize] = '\0';

    // create a std::string from the char buffer
    std::string rMsg(readMsg, readMsgSize);

    // look for a \n in the std::string rMsg
    int curpos = rMsg.find("\n"); //returns -1 if \n not found

    // while we still found a \n in rMsg
    while (curpos != -1)
    {
        // make a substring of the command up into the \n
        std::string cmd;
        cmd = rMsg.substr(0, curpos+1);

        // erase the substring from the whole rMsg
        rMsg.erase(0, curpos+1);

        // server outputs the command it recieved to the screen
        std::cout << "Responding to: " + cmd;

        // default reply from server
        std::string sendMsg = "Unknown command: " + cmd;

        // if valid reply, update sendMsg
        if (cmd == "hello\n")
            sendMsg = "Well hello to you too!\n";
        else if (cmd == "1\n")
            sendMsg = "One is the loneliest number...\n";
        else if (cmd == "2\n")
            sendMsg = "Two can be as bad as one...\n";
        else if (cmd == "3\n")
            sendMsg = "Three is the sadest experience...\n";
        else if (cmd == "4\n")
            sendMsg = "Four is just no good anymore...\n";
        else if (cmd == "5\n")
            sendMsg = "Thank you Dayton! We are Three Dog Night!\n";
        else if (cmd == "passwd\n")
            sendMsg = "Better keep that safe...\n";
        else if (cmd == "menu\n")
            sendMsg = this->menuMsg;
        else if (cmd == "exit\n")
        {
            // if a client called exit, loop through the clients to find the right fd
            for (int i=0;i < clientSocketList.size(); i++)
            {
                // found the client
                if (clientSocketList.at(i) == clientSocket)

                    // print info on the client to the server screen
                    int addrLen = sizeof(this->address);
                    getpeername(clientSocket , (struct sockaddr*)&this->address, (socklen_t*)&addrLen);   
                    printf("Disconnected! ip %s, port %d\n", inet_ntoa(this->address.sin_addr), ntohs(this->address.sin_port));

                    // close the client, set the FD to 0 in the client list (avoids erasure of active iterator object)
                    close(clientSocket);
                    clientSocketList.at(i)=0;
                    break; // no need to keep looking for the client
                }
            }
        }

        // only send sendMsg when the client isn't exiting
        if (cmd != "exit\n")
            serverSend(clientSocket, sendMsg);

        // find the next \n so we can continue parsing the whole rMsg that was sent
        curpos = rMsg.find("\n");
    }
}