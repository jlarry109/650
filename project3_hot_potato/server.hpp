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

/**
 * @brief The ConnectionInfo class represents the IP address and port number of a connection.
 */
 class ConnectionInfo {
        public:
            std::string ip;/**< The IP address of the connection. */
            int port;/**< The port number of the connection. */

            ConnectionInfo() : ip(), port() {}
            ConnectionInfo(const std::string & ip, const int port) {
                this->ip = ip;
                this->port = port;
            }
            ~ ConnectionInfo() {}

    };

/**
 * @brief The Server class manages the creation, binding, and handling of server sockets.
 */
class Server {
public: 
   struct addrinfo hostInfo; /**< Address information structure for the host. */
   struct addrinfo *servInfo; /**< Points to the results of getaddrinfo. */
   int serverFd;            /**< Socket file descriptor for the server. */
   int status;   /**< Status variable to track errors. */
   ConnectionInfo info;   /**< Connection information for accepted connections. */

    /**
     * @brief Initializes address information using the specified hostname and port number.
     * @param hostname The hostname to bind the server to.
     * @param portNum The port number to bind the server to.
     */
   void initializeAddrInfo(const char *hostname, const char * portNum);
    /**
     * @brief Opens a socket for the server.
     */
   void openSocket();
   /**
     * @brief Binds the socket to the network interface specified in the address information.
     */
   void bindSockToNetworkInterface();
    /**
     * @brief Listens for incoming connections on the server socket.
     */
   void listenForIncomingConnection(); 
   /**
     * @brief Accepts an incoming connection and retrieves connection information.
     * @param info Reference to the ConnectionInfo object to store connection details.
     * @return The file descriptor of the accepted connection.
     */
   int acceptIncomingConnection(ConnectionInfo & info);
   /**
     * @brief Gets the port number to which the server socket is bound.
     * @return The port number.
     */
   int getPortNum();

   /**
     * @brief Default constructor for the Server class.
     */
   Server() : hostInfo(), servInfo(nullptr), serverFd(-1), status(-1), info() {}
   
   /**
     * @brief Destructor for the Server class, closes the server socket.
     */
   virtual ~Server() {
      close(serverFd);
   }
};
#endif