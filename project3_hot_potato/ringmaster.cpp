#include "server.hpp"
#include "potato.hpp"

#include "potato.hpp"
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>

#include "server.hpp"

class Ringmaster : public Server
{
public:
    int numPlayers;
    int numHops;
    std::vector<int> playerFds;
    std::vector<std::string> playerIps;
    std::vector<int> playerPorts;

public:
    Potato potato;
 
    /* Initialize a Ringmaster server */
    Ringmaster(const char *portNum, int totalPlayers, int totalHops) : numPlayers(totalPlayers),
                                                                       numHops(totalHops), potato(totalHops)
    {
        playerFds.resize(totalPlayers);
        playerIps.resize(totalPlayers);
        playerPorts.resize(totalPlayers);
        initializeAddrInfo(NULL, portNum);
        openSocket();
        bindSockToNetworkInterface();
        listenForIncomingConnection();
    }

    /* Print information about the Potato Ringmaster */
    void printRingmasterInfo()
    {
        std::cout << "Potato Ringmaster\n"
                  << "Players = " << numPlayers << "\nHops = " << numHops << std::endl;
    }

    /* Build the ring of players */
    void buildPlayerRing()
    {
        // Accept all players and get their info
        for (int i = 0, playerId = 0; i < numPlayers; i++, playerId++)
        {
            std::string ip;
            int playerFd = acceptIncomingConnection(info);
            //int playerFd = serverAccept(ip);
            // Master sends player id and players num to this player
            send(playerFd, &playerId, sizeof(playerId), 0);
            send(playerFd, &numPlayers, sizeof(playerId), 0);
            // Receive the player's port
            recv(playerFd, &playerPorts[i], sizeof(playerPorts[i]), 0);
            playerIps[i] = ip;
            playerFds[i] = playerFd;
            std::cout << "Player " << i << " is ready to play " << std::endl;
        }

        // Connect players to their left neighbors
        for (int i = 0; i < numPlayers; i++)
        {
            int leftPlayerId = (i - 1 + numPlayers) % numPlayers;
            send(playerFds[i], &(playerPorts[leftPlayerId]), sizeof(playerPorts[leftPlayerId]), 0);
            int ipLength = sizeof(playerIps[leftPlayerId]);
            send(playerFds[i], &ipLength, sizeof(ipLength), 0);
            send(playerFds[i], playerIps[leftPlayerId].c_str(), sizeof(playerIps[leftPlayerId]), 0);
        }
    }

    /* Play the game with the Potato */
    void playGameWithPotato()
    {
        // If there are no hops, end the game after building the ring
        if (numHops == 0)
        {
            return;
        }

        // Send the potato to a randomly selected first player
        srand((unsigned int)time(NULL) + numPlayers);
        int randomPlayer = rand() % numPlayers;
        std::cout << "Ready to start the game, sending potato to player " << randomPlayer << std::endl;

        if (send(playerFds[randomPlayer], &potato, sizeof(potato), 0) == -1)
        {
            std::cerr << "Potato is broken!" << std::endl;
        }

        fd_set readFds;
        int maxFd = *std::max_element(playerFds.begin(), playerFds.end());
        FD_ZERO(&readFds);

        for (int i = 0; i < playerFds.size(); i++)
        {
            FD_SET(playerFds[i], &readFds);
        }

        int selectResult = select(maxFd + 1, &readFds, NULL, NULL, NULL);
        if (selectResult == -1)
        {
            perror("select"); // select() error
            return;
        }

        // Catch the last potato
        for (int i = 0; i < playerFds.size(); i++)
        {
            if (FD_ISSET(playerFds[i], &readFds))
            {
                int receivedBytes = recv(playerFds[i], &potato, sizeof(potato), MSG_WAITALL);
                if (receivedBytes != sizeof(potato))
                {
                    std::cerr << "Received a broken potato with length " << receivedBytes << std::endl;
                    return;
                }

                // Send potato with remainHops = 0 to all players, shutting down the game
                for (int j = 0; j < playerFds.size(); j++)
                {
                    if (send(playerFds[j], &potato, sizeof(potato), 0) == -1)
                    {
                        perror("Send potato failed");
                        return;
                    }
                }
                break;
            }
        }
    }

    /* Run the Ringmaster logic */
    void runRingmaster()
    {
        printRingmasterInfo();
        buildPlayerRing();
        playGameWithPotato();
        potato.printPath();
    }

    virtual ~Ringmaster()
    {
        for (int i = 0; i < numPlayers; i++)
        {
            close(playerFds[i]);
        }
        close(serverFd);
    }
};

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Incorrect command line argument formatting. Should be: ./ringmaster <port_num> <num_players> <num_hops>" << std::endl;
        return EXIT_FAILURE;
    }

    const char * portNumStr = argv[1];
    int numPlayers = atoi(argv[2]);
    int numHops = atoi(argv[3]);

    if (numPlayers <= 1)
    {
        std::cerr << "The number of players should be greater than 1!" << std::endl;
        exit(EXIT_FAILURE);
    }

    Ringmaster *ringmaster = new Ringmaster(portNumStr, numPlayers, numHops);
    ringmaster->runRingmaster();
    delete ringmaster;

    return EXIT_SUCCESS;
}

