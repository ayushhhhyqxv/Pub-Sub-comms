#ifndef PUBSUB_H
#define PUBSUB_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>

// mutex for PubSub , so that each thread is isolated and locked for that context 
class PubSubMutex {
private:
    bool locked;
public:
    PubSubMutex() : locked(false) {}
    void lock() { while(locked) {} locked = true; }
    void unlock() { locked = false; }
};

class PubSubLockGuard {
private:
    PubSubMutex& mutex;
public:
    PubSubLockGuard(PubSubMutex& mtx) : mutex(mtx) { mutex.lock(); }
    ~PubSubLockGuard() { mutex.unlock(); }
};

class PubSub {
private:
    std::unordered_map<std::string, std::set<int>> channels;
    std::unordered_map<int, std::vector<std::string>> clientMessages;
    PubSubMutex mtx;
    int nextClientId;

public:
    PubSub() : nextClientId(1) {}
    
    int subscribe(const std::string& channel);
    void unsubscribe(int clientId, const std::string& channel);
    void publish(const std::string& channel, const std::string& message);
    std::vector<std::string> getMessages(int clientId);
};

#endif