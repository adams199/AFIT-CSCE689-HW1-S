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
#include <fcntl.h>

class FileDesc
{
private:
    /* data */
public:

    

    FileDesc(/* args */) {};
    ~FileDesc() {};
    
    void socketSend(int socketFD, std::string &msg)
    {
        const char* cstring = msg.c_str();
        if( (send(socketFD, cstring, strlen(cstring), 0)) != strlen(cstring) )   //send messages function.
            throw std::runtime_error("Server to client send failed\n");
    }

    void readMessage(int socketFD, std::string &msg )
    {

    }


};


class serverSocketFD : FileDesc  // Code influenced from https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
{
public:
    int socketFD = 0;
    struct sockaddr_in address;
    socklen_t addrLen;
    std::string menuMsg = "Your menu options are:\r\n\thello\r\n\t1-5\r\n\tpasswd\r\n\texit\r\n\tmenu\r\n";

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
        struct timeval timeVal;
        timeVal.tv_sec = 0;
        timeVal.tv_usec = 10; // timeout after 10 microseconds
        fd_set readfds;
        

        char dataBuffer[1024];
        int readMsg;

        for (int i = 0; i < maxClients; i++)   
            clientSocketList[i] = 0;

        if (listen(this->socketFD, 3) < 0)   
            throw std::runtime_error("Listen failed\n");
            
        
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
            socketActivity = select(maxSocket + 1 , &readfds, NULL, NULL, &timeVal);   
        
            if ((socketActivity < 0) && (errno!=EINTR))   
                printf("socketActivty select error");      
                
            //If something happened on the master socket, then its an incoming connection  
            if (FD_ISSET(this->socketFD, &readfds))   
            {   
                if ((newSocket = accept(this->socketFD, (struct sockaddr *)&address, (socklen_t*)&this->addrLen))<0)   
                    throw std::runtime_error("Accept failed\n");
                
                //inform user of socket number - used in send and receive commands  
                printf("Connected! Socket FD: %d, ip: %s, port: %d\n", newSocket, inet_ntoa(address.sin_addr), ntohs (address.sin_port));
            
                //send new connection menu message
                this->socketSend(newSocket, this->menuMsg);
                    
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
                if (clientSocketList[i] == 0)
                  continue;

                clientSocket = clientSocketList[i];   
                    
                if (FD_ISSET(clientSocket, &readfds))   
                {   
                    //Check if it was for closing , and also read the  
                    //incoming message  
                    memset(dataBuffer, 0, 1024);
                    if ((readMsg = read(clientSocket, dataBuffer, 1024)) == 0)   
                    {   
                        //Somebody disconnected , get his details and print  
                        getpeername(clientSocket , (struct sockaddr*)&this->address, (socklen_t*)&this->addrLen);   
                        printf("Disconnected! ip %s, port %d\n", inet_ntoa(this->address.sin_addr), ntohs(this->address.sin_port));   
                        
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
                        //send(clientSocket, dataBuffer, strlen(dataBuffer), 0);
                        printf("Responding to: %s", dataBuffer);
                    }   
                }   
            }   
        }      
    }
};

class clientSocketFD : FileDesc
{
    public:
    int socketFD;

    clientSocketFD()
    {
        if ((this->socketFD = socket(AF_INET, SOCK_STREAM, 0)) == 0)
            throw std::runtime_error("Socket creation failed\n");

        fcntl(socketFD, F_SETFL, fcntl(socketFD, F_GETFL) | O_NONBLOCK);
    };
    ~clientSocketFD() {};

    void clientConnect(const char *ip_addr, unsigned short port)
    {
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET; 
        serv_addr.sin_port = htons(port); 
        if(inet_pton(AF_INET, ip_addr, &serv_addr.sin_addr)<=0)  //0, -1 is returned for lsightly different errors
            printf("Invalid address\n");
        if (connect(this->socketFD, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
            printf("Connection Failed\n"); 
    }

    void clientHandle()
    {
        struct timeval timeVal;
        timeVal.tv_sec = 0;
        timeVal.tv_usec = 10; // timeout after 10 microseconds
        fd_set readfds;
        int socketActivity;
        std::string dataBuffer = "";

        while (true)
        {
            //clear the socket set  
            FD_ZERO(&readfds);   
            FD_SET(this->socketFD, &readfds);
            FD_SET(STDIN_FILENO, &readfds);

            socketActivity = select(this->socketFD + 1, &readfds, NULL, NULL, &timeVal);
            if (FD_ISSET(STDIN_FILENO, &readfds)) // stdin fd, then there is stuff to place in buffer
            {
                char readBuf[1024];
                int amountRead = 0;
                if ((amountRead = (read(STDIN_FILENO, readBuf, 1024))) <= 0)
                    throw std::runtime_error("read error from stdin");
                std::cout << amountRead;
                std::cout << dataBuffer;
                dataBuffer += readBuf;
                int curpos;
                if ((curpos = dataBuffer.find("\n")) == std::string::npos)
                {
                    printf("found endline at %d\n", curpos);
                }
                printf("found endline at %d\n", curpos);

               std::string cmd;
               cmd = dataBuffer.substr(0, curpos+1);
               dataBuffer.erase(0, curpos+1);


               send(socketFD, cmd.c_str(), cmd.size(), 0);
            }
            if (FD_ISSET(this->socketFD, &readfds))
            {
               char checkBuf[1024];
                if (read(this->socketFD, checkBuf, 1024) <= 0) //if select sees data but read none then connection closed
                {
                    std::cout << "The server closed the connection\n";
                    exit(0);
                }
                std::cout << checkBuf;
            }

            //this->socketSend(this->socketFD, sendMsg);
    
           // cddhar readMsg[1024] = {0};
           // if (read(this->socketFD, readMsg, 1024) < 0)
            //    printf("Read failed\n"); 
            //printf("%s\n", readMsg);
        }
    }


}; //if select detects data and read returns 0 its been closed
// server needs to read exit command and call close(client)
//select the stdin (fd 0) to see if there is data and then put in buffer and
//if buffer has a \n then send that command

