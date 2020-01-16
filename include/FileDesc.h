#pragma once
#include <string>
#include <cstring>
#include <iostream>
#include <stdio.h>  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>   
#include <arpa/inet.h>   
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> 
#include <exception>

class FileDesc
{
private:
    /* data */
public:

    

    FileDesc(/* args */) {};
    ~FileDesc() {};
    
    void writeMessage( std::string &msg )
    {

    }

    void readMessage( std::string &msg )
    {

    }


};


class serverSocketFD : FileDesc  // Code influenced from https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
{
public:
    int socketFD = 0;
    struct sockaddr_in address;
    socklen_t addrLen;

    serverSocketFD()
    {
        if ((this->socketFD = socket(AF_INET, SOCK_STREAM, 0)) == 0)
            throw std::runtime_error("Socket creation failed\n");
    };
    ~serverSocketFD() {};

    void socketBind(const char *ip_addr, short unsigned int port)
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;   
        addr.sin_addr.s_addr = INADDR_ANY;   
        addr.sin_port = htons( port );
        //this->address = address;
        this->addrLen = sizeof(address);
        if ((bind(this->socketFD, (struct sockaddr *)&addr, sizeof(addr))) < 0)
        {
            perror("errno is");
            throw std::runtime_error("Bind failed\n");
        }
    }

    void socketListen()
    {
        int newSocket, clientSocket, clientSocketList[10], maxClients=10, socketActivity, maxSocket;
        struct timeval* timeVal;
        timeVal->tv_sec = 0;
        timeVal->tv_usec = 10; // timeout after 10 microseconds
        fd_set readfds;
        

        char dataBuffer[1024];
        int readMsg;

        for (int i = 0; i < maxClients; i++)   
            clientSocketList[i] = 0;

        if (listen(this->socketFD, 3) < 0)   
            throw std::runtime_error("Listen failed\n");
            
        //accept the incoming connection     
        //puts("Waiting for connections ...");   
            
        while(true)
        {   
            //clear the socket set  
            FD_ZERO(&readfds);   
        
            //add master socket to set  
            FD_SET(this->socketFD, &readfds);   
            maxSocket = this->socketFD;   
                
            //add child sockets to set  
            for (int i = 0 ; i < maxClients ; i++)   
            {   
                //socket descriptor  
                clientSocket = clientSocketList[i];   
                    
                //if valid socket descriptor then add to read list  
                if(clientSocket > 0)   
                    FD_SET( clientSocket , &readfds);   
                    
                //highest file descriptor number, need it for the select function  
                if(clientSocket > maxSocket)   
                    maxSocket = clientSocket;   
            }   
        
            //wait for an activity on one of the sockets , timeout is NULL ,  
            //so wait indefinitely  
            socketActivity = select(maxSocket + 1 , &readfds, NULL, NULL, timeVal);   
        
            if ((socketActivity < 0) && (errno!=EINTR))   
                printf("socketActivty select error");      
                
            //If something happened on the master socket, then its an incoming connection  
            if (FD_ISSET(this->socketFD, &readfds))   
            {   
                if ((newSocket = accept(this->socketFD, (struct sockaddr *)&address, (socklen_t*)&this->addrLen))<0)   
                    throw std::runtime_error("Accept failed\n");
                
                //inform user of socket number - used in send and receive commands  
                printf("Connected! Socket FD: %d, ip: %s, port: %d\n", newSocket, inet_ntoa(address.sin_addr), ntohs (address.sin_port));
            
                //send new connection greeting message  
                //if( send(newSocket, message, strlen(message), 0) != strlen(message) )   //send messages function.
                //    throw runtime_error("Server to client send failed\n");
                    
                //add new socket to array of sockets  
                for (int i = 0; i < maxClients; i++)   
                {   
                    //if position is empty  
                    if( clientSocketList[i] == 0 )   
                    {   
                        clientSocketList[i] = newSocket;                          
                        break;   
                    }   
                }   
            }   
                
            //else its some IO operation on some other socket 
            for (int i = 0; i < maxClients; i++)   
            {   
                clientSocket = clientSocketList[i];   
                    
                if (FD_ISSET(clientSocket, &readfds))   
                {   
                    //Check if it was for closing , and also read the  
                    //incoming message  
                    if ((readMsg = read(clientSocket, dataBuffer, 1024)) == 0)   
                    {   
                        //Somebody disconnected , get his details and print  
                        getpeername(clientSocket , (struct sockaddr*)&this->address, (socklen_t*)&this->addrLen);   
                        printf("Host disconnected, ip %s, port %d\n", inet_ntoa(this->address.sin_addr), ntohs(this->address.sin_port));   
                        
                        //Close the socket and mark as 0 in list for reuse  
                        close( clientSocket );   
                        clientSocketList[i] = 0;   
                    }   
                        
                    //Echo back the message that came in  
                    else 
                    {   
                        //set the string terminating NULL byte on the end  
                        //of the data read  
                        dataBuffer[readMsg] = '\0';   
                        send(clientSocket, dataBuffer, strlen(dataBuffer), 0);   
                    }   
                }   
            }   
        }      
    } 
};

class clientSocketFD : FileDesc
{
    public:
    clientSocketFD() {};
    ~clientSocketFD() {};


};

