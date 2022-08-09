#include <thread>
#include <chrono>

#include "../server.hpp"

void Server::processPerfStats() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}