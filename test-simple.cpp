#include <winsock2.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

int main() {
    std::cout << "=== Redis Test Client ===" << std::endl;
    
    // Initialized Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSAStartup failed!" << std::endl;
        return 1;
    }
    
    // Create socket for conn
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cout << "Socket creation failed!" << std::endl;
        WSACleanup();
        return 1;
    }
    
    // Server address req
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(6379);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // Connect to server
    std::cout << "Connecting to server... ";
    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "FAILED!" << std::endl;
        std::cout << "Make sure server is running: ./redis-server.exe" << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    std::cout << "SUCCESS!" << std::endl;
    
    char buffer[1024];
    std::string command;
    
    // Read and display welcome message ONCE conn
    int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        std::cout << "Server: " << buffer;
    }
    
    std::cout << "Type Redis commands (type 'QUIT' to exit)" << std::endl;
    std::cout << "==========================================" << std::endl;
    
    // Main interactive loop repl
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, command);
        
        // Check for quit command
        if (command == "QUIT" || command == "quit") {
            std::cout << "Goodbye!" << std::endl;
            break;
        }
        
        // Skip empty commands
        if (command.empty()) {
            continue;
        }
        
        // Send command to server + nstop
        std::string fullCommand = command + "\n";
        send(sock, fullCommand.c_str(), fullCommand.length(), 0);
        
        // Wait for and receive response
        bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            std::cout << buffer;
        } else if (bytes == 0) {
            std::cout << "Server disconnected" << std::endl;
            break;
        } else {
            std::cout << "No response from server" << std::endl;
        }
    }
    
    // Cleanup
    closesocket(sock);
    WSACleanup();
    return 0;
}