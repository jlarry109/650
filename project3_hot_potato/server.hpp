#ifndef SERVER_H
#define SERVER_H
#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

constexpr unsigned int MAX_NUM_CLIENTS = 1048576;
 class ConnectionInfo {
        public:
            std::string ip;
            int port;

            ConnectionInfo() : ip(), port() {}
            ConnectionInfo(const std::string & ip, const int port) {
                this->ip = ip;
                this->port = port;
            }
            ~ ConnectionInfo() {}

    };
class Server {
public: 
   struct addrinfo hostInfo;
   struct addrinfo *servInfo; // will point to the results
   int serverFd;             // socket fd, still listening for more new connections
   int status;
   ConnectionInfo info;

   void initializeAddrInfo(const char *hostname, const char * portNum);
   void openSocket();
   void bindSockToNetworkInterface();
   void listenForIncomingConnection(); 
   int acceptIncomingConnection(ConnectionInfo & info);
   int getPortNum();
   Server() : hostInfo(), servInfo(nullptr), serverFd(-1), status(-1), info() {}
   virtual ~Server() {
      close(serverFd);
   }
};
#endif