#include "server.hpp"

void Server::initializeAddrInfo(const char *hostname, const char *port_num) {
    std::memset(&hostInfo, 0, sizeof(hostInfo)); //reset/clear struct
    hostInfo.ai_family = AF_UNSPEC; //could be IPv4 or IPv6
    hostInfo.ai_socktype = SOCK_STREAM; //Using TCP
    hostInfo.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hostInfo.ai_protocol = 0;          /* Any protocol */
    hostInfo.ai_canonname = NULL;
    hostInfo.ai_addr = NULL;
    hostInfo.ai_next = NULL;

    if ( (status = getaddrinfo(hostname, port_num, &hostInfo, &servInfo)) != 0)  {
        std::cerr << "hostname is: " << hostname << std::endl;
        std::cerr << "getaddrinfo error" << gai_strerror(status) << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Server::openSocket() {
    serverFd = socket(servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol);
    if (serverFd < 0) {
        std::cerr << "open socket error" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Server::bindSockToNetworkInterface() {
    //Tell OS to set port free again immediately(otherwise it will normally take a while to do so by the OS and 
    // will lead to Address already in use error)
    int optval = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(optval)) != 0) {
        std::cerr << "setsockopt error" << std::endl;
        exit(EXIT_FAILURE);
    }
    // // bind socket with a port on local machine(ai_addr)
    if ( (status = bind(serverFd, servInfo->ai_addr, servInfo->ai_addrlen)) < 0) {
        std::cerr << "bind error" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Server::listenForIncomingConnection() {
    if (listen (serverFd, MAX_NUM_CLIENTS) < 0) {
        std::cerr << "listen error" << std::endl;
        exit(EXIT_FAILURE);
    }
}

int Server::acceptIncomingConnection(ConnectionInfo & info) {
    struct sockaddr_storage sock_addr;
    socklen_t sock_addr_len = sizeof(sock_addr);
    int clientFd = accept(serverFd, (struct sockaddr *) &sock_addr, &sock_addr_len);
    if (clientFd < 0) {
        std::cerr << "accept error" << std::endl;
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in * client_addr = (sockaddr_in *)&sock_addr;
    std::string ip = inet_ntoa(client_addr->sin_addr);
    int port = ntohs(client_addr->sin_port);
    info.ip = ip;
    info.port = port;
    std::cout << "server: got connection from =" << inet_ntoa(client_addr->sin_addr) << " and port = " << ntohs(client_addr->sin_port) << std::endl;
    return clientFd;
}

int Server::getPortNum() {
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(serverFd, (struct sockaddr *)&sin, &len) == -1) {
        std::cerr << "getsockname failed!" << std::endl;
        exit(EXIT_FAILURE);
    }
    return ntohs(sin.sin_port);
}