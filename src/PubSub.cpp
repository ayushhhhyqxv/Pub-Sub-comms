#include "PubSub.h"

int PubSub::subscribe(const std::string& channel) {
    PubSubLockGuard lock(mtx);
    int clientId = nextClientId++;
    channels[channel].insert(clientId);
    return clientId;
}

void PubSub::unsubscribe(int clientId, const std::string& channel) {
    PubSubLockGuard lock(mtx);
    auto it = channels.find(channel);
    if (it != channels.end()) {
        it->second.erase(clientId);
    }
}

void PubSub::publish(const std::string& channel, const std::string& message) {
    PubSubLockGuard lock(mtx);
    auto it = channels.find(channel);
    if (it != channels.end()) {
        for (int clientId : it->second) {
            clientMessages[clientId].push_back("[" + channel + "] " + message);
        }
    }
}

std::vector<std::string> PubSub::getMessages(int clientId) {
    PubSubLockGuard lock(mtx);
    auto it = clientMessages.find(clientId);
    if (it != clientMessages.end()) {
        std::vector<std::string> messages = it->second;
        it->second.clear();
        return messages;
    }
    return {};
}