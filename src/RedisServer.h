#ifndef REDISSERVER_H
#define REDISSERVER_H

#include "DataStore.h"
#include "PubSub.h"
#include <winsock2.h>
#include <string>
#include <atomic>

#pragma comment(lib, "ws2_32.lib")

class RedisServer {
private:
    DataStore dataStore;
    PubSub pubSub;
    std::atomic<bool> running;
    int port;
    SOCKET serverSocket;

    void handleClient(SOCKET clientSocket);
    std::string processCommand(const std::string& command);
    void startConsoleUI();

public:
    RedisServer(int port = 6379);
    ~RedisServer();
    
    bool start();
    void stop();
    void run();
};

#endif