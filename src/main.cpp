#include "RedisServer.h"
#include <iostream>

int main() {
    std::cout << "=== Redis-like Server ===" << std::endl;
    std::cout << "Building with GCC " << __VERSION__ << std::endl;
    
    RedisServer server(6379);
    
    if (!server.start()) {
        std::cerr << "Failed to start server!" << std::endl;
        return 1;
    }
    
    std::cout << "Server started successfully!" << std::endl;
    std::cout << "Press 'q' to quit the server" << std::endl;
    
    server.run();
    
    std::cout << "Thank you for using !" << std::endl;
    return 0;
}