#ifndef TCPSERVER_H
#define TCPSERVER_H
#include "Server.h"    
#include <sys/types.h>   
#include <netinet/in.h>  
#include <sys/time.h>
#include <string>
#include <vector> 


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
