#include "RedisServer.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <conio.h>  
#include <thread>

RedisServer::RedisServer(int port) : port(port), running(false), serverSocket(INVALID_SOCKET) {}

RedisServer::~RedisServer() {
    stop();
}

bool RedisServer::start() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return false;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        WSACleanup();
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        std::cerr << "Listen failed" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    running = true;
    std::cout << "Redis-like server started on port " << port << std::endl;
    std::cout << "Server supports: SET, GET, DEL, INCR, HSET, HGET, LPUSH, RPUSH, INFO, etc." << std::endl;
    return true;
}

void RedisServer::stop() {
    running = false;
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }
    WSACleanup();
}

std::string RedisServer::processCommand(const std::string& command) {
    std::stringstream ss(command);
    std::string cmd;
    ss >> cmd;
    
    // Convert to uppercase
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
    
    if (cmd == "SET") {
        std::string key, value;
        ss >> key >> value;
        return dataStore.set(key, value);
    }
    else if (cmd == "GET") {
        std::string key;
        ss >> key;
        return dataStore.get(key);
    }
    else if (cmd == "DEL") {
        std::string key;
        ss >> key;
        int result = dataStore.del(key);
        return std::to_string(result);
    }
    else if (cmd == "EXISTS") {
        std::string key;
        ss >> key;
        return dataStore.exists(key) ? "1" : "0";
    }
    else if (cmd == "INCR") {
        std::string key;
        ss >> key;
        int result = dataStore.incr(key);
        return result >= 0 ? std::to_string(result) : "ERROR";
    }
    else if (cmd == "DECR") {
        std::string key;
        ss >> key;
        int result = dataStore.decr(key);
        return result >= 0 ? std::to_string(result) : "ERROR";
    }
    else if (cmd == "HSET") {
        std::string key, field, value;
        ss >> key >> field >> value;
        return dataStore.hset(key, field, value);
    }
    else if (cmd == "HGET") {
        std::string key, field;
        ss >> key >> field;
        return dataStore.hget(key, field);
    }
    else if (cmd == "HGETALL") {
        std::string key;
        ss >> key;
        return dataStore.hgetall(key);
    }
    else if (cmd == "LPUSH") {
        std::string key, value;
        ss >> key >> value;
        return dataStore.lpush(key, value);
    }
    else if (cmd == "RPUSH") {
        std::string key, value;
        ss >> key >> value;
        return dataStore.rpush(key, value);
    }
    else if (cmd == "LPOP") {
        std::string key;
        ss >> key;
        return dataStore.lpop(key);
    }
    else if (cmd == "RPOP") {
        std::string key;
        ss >> key;
        return dataStore.rpop(key);
    }
    else if (cmd == "LRANGE") {
        std::string key;
        int start, stop;
        ss >> key >> start >> stop;
        return dataStore.lrange(key, start, stop);
    }
    else if (cmd == "SADD") {
        std::string key, member;
        ss >> key >> member;
        return dataStore.sadd(key, member);
    }
    else if (cmd == "SMEMBERS") {
        std::string key;
        ss >> key;
        return dataStore.smembers(key);
    }
    else if (cmd == "SISMEMBER") {
        std::string key, member;
        ss >> key >> member;
        return dataStore.sismember(key, member);
    }
    else if (cmd == "KEYS") {
        std::string pattern;
        ss >> pattern;
        return dataStore.keys(pattern);
    }
    else if (cmd == "DBSIZE") {
        return std::to_string(dataStore.dbsize());
    }
    else if (cmd == "INFO") {
        return dataStore.info();
    }
    else if (cmd == "TTL") {
        std::string key;
        ss >> key;
        int result = dataStore.ttl(key);
        return std::to_string(result);
    }
    else if (cmd == "EXPIRE") {
        std::string key;
        int seconds;
        ss >> key >> seconds;
        int result = dataStore.expire(key, seconds);
        return std::to_string(result);
    }
    else if (cmd == "PING") {
        return "PONG";
    }
    else if (cmd == "QUIT") {
        return "BYE";
    }
    else if (cmd == "HELP") {
        return "Available commands: SET, GET, DEL, EXISTS, INCR, DECR, HSET, HGET, HGETALL, LPUSH, RPUSH, LPOP, RPOP, LRANGE, SADD, SMEMBERS, SISMEMBER, KEYS, DBSIZE, INFO, TTL, EXPIRE, PING, QUIT";
    }
    else {
        return "ERROR: Unknown command '" + cmd + "'. Type HELP for available commands.";
    }
}



void RedisServer::startConsoleUI() {
    std::cout << "\n=== Redis Server Console ===" << std::endl;
    std::cout << "Type commands (SET key value, GET key, INFO, HELP, QUIT)" << std::endl;
    
    std::string command;
    while (running) {
        std::cout << "> ";
        std::getline(std::cin, command);
        
        if (command == "QUIT") {
            running = false;
            break;
        }
        
        if (!command.empty()) {
            std::string response = processCommand(command);
            std::cout << response << std::endl;
        }
    }
}

void RedisServer::handleClient(SOCKET clientSocket) {
    char buffer[1024];
    
    std::cout << "Client connected! Ready for commands." << std::endl;
    
    std::string welcome = "Welcome to Redis-like Server! Type HELP for commands.\n";
    send(clientSocket, welcome.c_str(), welcome.length(), 0);
    
    while (running) {
        memset(buffer, 0, sizeof(buffer));
        
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::string command(buffer);
            
            // Remove carriage return and newline characters
            command.erase(std::remove(command.begin(), command.end(), '\r'), command.end());
            command.erase(std::remove(command.begin(), command.end(), '\n'), command.end());
            
            if (!command.empty()) {
                std::cout << "Client command: " << command << std::endl;
                std::string response = processCommand(command) + "\n";
                
                // Send ONLY the command response back to client
                send(clientSocket, response.c_str(), response.length(), 0);
                
                if (command == "QUIT") {
                    break;
                }
            }
        } else if (bytesReceived == 0) {
            // Client disconnected gracefully
            std::cout << "Client disconnected" << std::endl;
            break;
        } else {
            // Error occurred
            std::cout << "Client connection error: " << WSAGetLastError() << std::endl;
            break;
        }
    }
    
    closesocket(clientSocket);
    std::cout << "Client handling finished" << std::endl;
}

void RedisServer::run() {
    std::cout << "\n=== Redis Server ===" << std::endl;
    std::cout << "Choose mode:" << std::endl;
    std::cout << "1. Console mode (type 'console')" << std::endl;
    std::cout << "2. Network mode (type 'network')" << std::endl;
    std::cout << "> ";
    
    std::string mode;
    std::getline(std::cin, mode);
    
    if (mode == "console") {
        // Console-only mode
        startConsoleUI();
    } else {
        // Network-only mode (no console interference)
        std::cout << "Network server started on port " << port << std::endl;
        std::cout << "Waiting for clients... (Press Ctrl+C to stop)" << std::endl;
        
        while (running) {
            SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket != INVALID_SOCKET) {
                std::cout << "New client connected!" << std::endl;
                handleClient(clientSocket);
            }
            
            // Check if we should stop
            if (_kbhit()) {
                char ch = _getch();
                if (ch == 'q' || ch == 'Q') {
                    running = false;
                    break;
                }
            }
        }
    }
    
    std::cout << "Server shutdown complete." << std::endl;
}