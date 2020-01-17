#ifndef TCPSERVER_H
#define TCPSERVER_H
#pragma once
#include <vector>
#include "Server.h"
#include <arpa/inet.h>
#include "FileDesc.h"
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
#include <exception>
#include <fcntl.h>

class TCPServer : public Server 
{
public:
   TCPServer();
   ~TCPServer();

   void bindSvr(const char *ip_addr, unsigned short port);
   void listenSvr();
   void shutdown();
   void serverSend(int socketFD, std::string &msg);
   void handleMsg(char readMsg[], int readMsgSize, int clientSocket);

private:
   int socketFD;
   std::string menuMsg = "Your menu options are:\r\n\thello - replies back\r\n\t1-5 - sings a song\r\n\tpasswd - "
                         "a friendly reminder\r\n\texit - closes the connection\r\n\tmenu - prints this menu\r\n";
   struct sockaddr_in address;
   std::vector<int> clientSocketList;
};


#endif
