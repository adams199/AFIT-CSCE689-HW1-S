#ifndef TCPSERVER_H
#define TCPSERVER_H
#pragma once
#include <vector>
#include "Server.h"
#include <arpa/inet.h>
#include "FileDesc.h"
//#include "TCPConn.h"

class TCPServer : public Server 
{
public:
   TCPServer();
   ~TCPServer();

   void bindSvr(const char *ip_addr, unsigned short port);
   void listenSvr();
   void shutdown();

private:
   serverSocketFD* serverFD;
};


#endif
