#include "TCPServer.h"
#include <stdio.h>
#include <iostream> 
#include <sys/types.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>
#include "FileDesc.h"

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
    this->address.sin_family = AF_INET;   
    this->address.sin_addr.s_addr = INADDR_ANY;   
    this->address.sin_port = htons( port );
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
    struct timeval timeVal;
    timeVal.tv_sec = 0;
    timeVal.tv_usec = 10; // timeout after 10 microseconds
    fd_set readfds;
    
    char readMsg[1024];
    int readMsgSize;

    if (listen(this->socketFD, 3) < 0)   
        throw std::runtime_error("Listen failed\n");
        
    
    while(true)
    {   
        //clear the socket set  
        FD_ZERO(&readfds);   
        //add master socket to set  
        FD_SET(this->socketFD, &readfds);    
        
        for(int clientSocket : clientSocketList)
        {
            if(clientSocket > 0)   
                FD_SET(clientSocket , &readfds);
            if(clientSocket > maxSocket)
                maxSocket = clientSocket; 
        }

        socketActivity = select(maxSocket + 1 , &readfds, NULL, NULL, &timeVal);   
    
        if ((socketActivity < 0) && (errno!=EINTR))   
            printf("socketActivty select error");      
            
        //If something happened on the server socket, then its an incoming connection  
        if (FD_ISSET(this->socketFD, &readfds))   
        {   
            int addrLen = sizeof(this->address);
            if ((newSocket = accept(this->socketFD, (struct sockaddr *)&this->address, (socklen_t*)&addrLen)) < 0)   
                throw std::runtime_error("Accept failed\n");
            
            //inform user of socket number - used in send and receive commands  
            printf("Connected! Socket FD: %d, ip: %s, port: %d\n", newSocket, inet_ntoa(address.sin_addr), ntohs (address.sin_port));
        
            //send new connection menu message
            this->serverSend(newSocket, this->menuMsg);
                
            //add new socket to array of sockets
            clientSocketList.push_back(newSocket);
            
        }   
            
        //else its some IO operation on some other socket
        for(auto it=clientSocketList.begin(); it!=clientSocketList.end(); it++)
        {
            int clientSocket = *it;
            if(clientSocket == 0) 
                continue;  
                
            if (FD_ISSET(clientSocket, &readfds))   
            {   
                //Check if it was for closing , and also read the incoming message  
                memset(readMsg, 0, 1024);
                if ((readMsgSize = read(clientSocket, readMsg, 1024)) == 0)   
                {   
                    //Somebody disconnected , get his details and print
                    int addrLen = sizeof(this->address);
                    getpeername(clientSocket , (struct sockaddr*)&this->address, (socklen_t*)&addrLen);   
                    printf("Disconnected! ip %s, port %d\n", inet_ntoa(this->address.sin_addr), ntohs(this->address.sin_port));   
                    
                    //Close the socket and mark as 0 in list for reus
                    close( clientSocket );
                    clientSocketList.erase(it--);
                    
                }   
                else  
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
    std::string rMsg(readMsg, readMsgSize);
    int curpos = rMsg.find("\n"); //returns -1 if \n not found
    while (curpos != -1)
    {
        std::string cmd;
        cmd = rMsg.substr(0, curpos+1);
        rMsg.erase(0, curpos+1);

        std::cout << "Responding to: " + cmd;
        std::string sendMsg = "Unknown command: " + cmd;

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
            for (int i=0;i < clientSocketList.size(); i++)
            {
                if (clientSocketList.at(i) == clientSocket) //needs to be different than above due to buf overflows of erasure of vectors
                {                                           //for this implementation cannot be single kill() method
                    int addrLen = sizeof(this->address);
                    getpeername(clientSocket , (struct sockaddr*)&this->address, (socklen_t*)&addrLen);   
                    printf("Disconnected! ip %s, port %d\n", inet_ntoa(this->address.sin_addr), ntohs(this->address.sin_port));
                    close(clientSocket);
                    clientSocketList.at(i)=0;
                    break;
                }
            }
        }

        if (cmd != "exit\n")
            serverSend(clientSocket, sendMsg);

        curpos = rMsg.find("\n");
    }
}