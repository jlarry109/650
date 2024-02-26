#ifndef POTATO_H 
#define POTATO_H
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <iostream>

constexpr std::size_t MAX_NUM_HOPS = 512;

/**
 * @brief Represents a Potato that is passed around in the game.
 */
class Potato {

    public:
        std::size_t ttl;  ///< Time to live: remaining hops
        int path [MAX_NUM_HOPS];///< Array to store the path of the potato
        std::size_t count; ///< Number of hops made by the potato

        /**
     * @brief Default constructor for Potato, initializes ttl and count.
     */
        Potato() : ttl(0), count(0) {
            memset(path, 0, sizeof(path));
        }

        /**
     * @brief Parameterized constructor for Potato, initializes ttl,
     * path, and count based on the specified number of hops.
     * @param num_hops The total number of hops for the potato.
     */
        Potato(size_t num_hops) : ttl(num_hops), path(), count(0) {
            std::memset(path, 0, sizeof(path));
        }

        /**
     * @brief Destructor for Potato.
     */
        ~Potato () {}

        /**
     * @brief Prints the trace of the potato's path to the standard output.
     */
        void printPath() {
            std::cout << "Trace of potato:" << std::endl;
            for (std::size_t i = 0; i < count; i++) {
                std::cout << path[i];
                if (i == count - 1) {
                    std::cout << std::endl;
                }
                else {
                    std::cout << ",";
                }
            }
        }
    };
    #endif