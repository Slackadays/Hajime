#include <thread>
#include <chrono>

#include "../server.hpp"
#include "../constants.hpp"

void Server::processPerfStats() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(counterInterval));
    }
}