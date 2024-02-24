#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include "server.hpp"
#include "potato.hpp"
#include <algorithm>

class Player : public Server {
public:
    int playerId;
    int playerNum;
    const char *machineName;
    const char *masterPort; // Master's port
    //int playerPort;
    int masterFd; // File descriptor to ringmaster
    int leftFd;
    int rightFd;

public:
    Player(const char *machineName, const char *portNum) : machineName(machineName), masterPort(portNum),
                                                             leftFd(-1), rightFd(-1)
    {
        initializeAddrInfo(machineName, masterPort);
        // Connect to ringmaster
        buildMasterConnect();
        // Receive player_id and playerNum
        recv(masterFd, &playerId, sizeof(playerId), 0);
        recv(masterFd, &playerNum, sizeof(playerNum), 0);
    }
    virtual ~Player()
    {
        // close(clientFd);
        close(masterFd);
        close(leftFd);
        close(rightFd);
    }
    
    /* Build socket to connect to ringmaster */
    void buildMasterConnect()
    {
        masterFd = socket(servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol);
        if (masterFd == -1)
        {
            fprintf(stderr, "Failed to create socket!\n");
            exit(EXIT_FAILURE);
        }
        status = connect(masterFd, servInfo->ai_addr, servInfo->ai_addrlen);
        if (status == -1)
        {
            std::cerr << "Error: cannot connect to master" << std::endl;
            exit(EXIT_FAILURE);
        }
        freeaddrinfo(servInfo);
    }

    /* Build socket to connect to previous (left) neighbor */
    int buildNeighborConnect(const char *hostName, const char *portNum)
    {
        std::memset(&hostInfo, 0, sizeof(hostInfo)); // make sure the struct is empty
        hostInfo.ai_family = AF_UNSPEC;              // don't care IPv4 or IPv6
        hostInfo.ai_socktype = SOCK_STREAM;          // TCP stream sockets
        
        // Get all address information into servInfo
        if ((status = getaddrinfo(hostName, portNum, &hostInfo, &servInfo)) != 0) {
            fprintf(stderr, "hostname is: %s", hostName);
            std::cout << std::endl;
            std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
            exit(EXIT_FAILURE);
        }

        int neighborFd = socket(servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol);
        if (neighborFd == -1)
        {
            fprintf(stderr, "Failed to create socket!\n");
            exit(EXIT_FAILURE);
        }

        status = connect(neighborFd, servInfo->ai_addr, servInfo->ai_addrlen);
        if (status == -1)
        {
            std::cerr << "Error: cannot connect to left neighbor" << std::endl;
            exit(EXIT_FAILURE);
        }

        freeaddrinfo(servInfo);
        return neighborFd;
    }

    /* Listen for potato and forward accordingly */
    void listenAndForwardPotato()
    {
        Potato potato(0);
        fd_set readFds;
        srand((unsigned int)time(NULL) + playerId);
        std::vector<int> listenFds{leftFd, rightFd, masterFd};
        int nfds = *(std::max_element(listenFds.begin(), listenFds.end()));

        // Catch the potato
        while (true)
        {
            FD_ZERO(&readFds);
            for (int i = 0; i < listenFds.size(); i++)
            {
                FD_SET(listenFds[i], &readFds);
            }

            int rv = select(nfds + 1, &readFds, NULL, NULL, NULL);
            if (rv == -1)
            {
                perror("select"); // select() error
                return;
            }

            int numBytes = 0;
            for (int i = 0; i < listenFds.size(); i++)
            {
                if (FD_ISSET(listenFds[i], &readFds))
                {
                    numBytes = recv(listenFds[i], &potato, sizeof(potato), MSG_WAITALL);
                    if (numBytes == -1)
                    {
                        std::cerr << "Received a broken potato" << std::endl;
                        return;
                    }
                    break;
                }
            }

            // If receive potato with 0 hop from master, shut down
            if (potato.ttl == 0 || numBytes == 0)
            {
                return;
            }

            if (--potato.ttl == 0) // Send potato to master if the final hop
            {
                potato.path[potato.count++] = playerId;
                if (send(masterFd, &potato, sizeof(potato), 0) == -1)
                {
                    std::cerr << "Send potato to master failed!" << std::endl;
                }
                std::cout << "I'm it" << std::endl;
                return;
            }
            else
            {
                potato.path[potato.count++] = playerId;
                int picked = rand() % 2;
                std::cout << "Sending potato to ";
                picked == 0 ? std::cout << (playerId + playerNum - 1) % playerNum : std::cout << (playerId + 1) % playerNum;
                std::cout << std::endl;

                if (send(listenFds[picked], &potato, sizeof(potato), 0) == -1)
                {
                    std::cerr << "Send potato to neighbor failed!" << std::endl;
                }
            }
        }
    }

    /* Run the player logic */
    void runPlayer()
    {
        // Player build as a server
        initializeAddrInfo(NULL, "");
        openSocket();
        bindSockToNetworkInterface();
        listenForIncomingConnection();

        // Send port to master
        info.port = getPortNum();
        send(masterFd, &info.port, sizeof(info.port), 0);
        std::cout << "Connected as player " << playerId << " out of " << playerNum
                  << " total players" << std::endl;

        // Receive left neighbor's fd and port from master
        int leftPort = 0;
        int ipLen = 0;
        recv(masterFd, &leftPort, sizeof(leftPort), 0);
        recv(masterFd, &ipLen, sizeof(ipLen), 0);

        char buf[ipLen];
        recv(masterFd, buf, sizeof(buf), MSG_WAITALL);
        std::string leftIp(buf);
        std::cout << "--------------\nleft ip: " << leftIp << std::endl;
        // connect to left neighbor as its client
        // Connect to left neighbor as its client
        leftFd = buildNeighborConnect("localhost"/*leftIp.c_str()*/, std::to_string(leftPort).c_str());
        
        ConnectionInfo neighborInfo;
        // Accept neighbor connection
        rightFd = acceptIncomingConnection(neighborInfo);
        
        // Listen and forward the potato
        listenAndForwardPotato();
    }
};

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "The argument format should be: ./player <machine_name> <port_num>" << std::endl;
        return EXIT_FAILURE;
    }

    const char *machineName = argv[1];
    const char *portNum = argv[2];

    Player *player = new Player(machineName, portNum);
    player->runPlayer();
    delete player;

    return EXIT_SUCCESS;
}
