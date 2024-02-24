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
class Potato {

    public:
        std::size_t ttl; //time to leave: remaining hops
        int path [MAX_NUM_HOPS];
        std::size_t count;

        Potato() : ttl(0), count(0) {
            memset(path, 0, sizeof(path));
        }
        Potato(size_t num_hops) : ttl(num_hops), path(), count(0) {
            memset(path, 0, sizeof(path));
        }
        ~Potato () {}

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

        // Serialize the Potato object to a string
    /*std::string serialize1() const {
        std::ostringstream oss;
        oss << ttl << ",";
        for (std::size_t i = 0; i < count; i++) {
            oss << path[i];
            if (i != count - 1) {
                oss << ",";
            }
        }
        return oss.str();
    }

    // Deserialize the string to populate the Potato object
    void deserialize1(const std::string& data) {
        std::istringstream iss(data);
        char comma;
        iss >> ttl >> comma;
        path.clear();
        while (iss) {
            std::size_t value;
            iss >> value;
            path.push_back(value);
            iss >> comma;
        }
        count = path.size();
    }
     // Serialize the Potato object into a buffer
    void serialize(char* buffer, std::size_t bufferSize) const {
        // Ensure the buffer size is sufficient
        if (bufferSize < sizeof(std::size_t) * (path.size() + 2)) {
            // Handle the error, for example:
            std::cerr << "Error: Insufficient buffer size for serialization!" << std::endl;
            return;
        }

        // Copy ttl, count, and path into the buffer
        std::size_t* ptr = reinterpret_cast<std::size_t*>(buffer);
        *ptr++ = ttl;
        *ptr++ = count;
        std::memcpy(ptr, path.data(), sizeof(std::size_t) * path.size());
    }

    // Deserialize data from a buffer into the Potato object
    void deserialize(const char* buffer, std::size_t bufferSize) {
        // Ensure the buffer size is sufficient
        if (bufferSize < sizeof(std::size_t) * 2) {
            // Handle the error, for example:
            std::cerr << "Error: Insufficient buffer size for deserialization!" << std::endl;
            return;
        }

        // Copy ttl and count from the buffer
        const std::size_t* ptr = reinterpret_cast<const std::size_t*>(buffer);
        ttl = *ptr++;
        count = *ptr++;

        // Copy path from the buffer
        path.resize(count);
        std::memcpy(path.data(), ptr, sizeof(std::size_t) * count);
    }*/
    };
    #endif