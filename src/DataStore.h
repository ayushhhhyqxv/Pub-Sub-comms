#ifndef DATASTORE_H
#define DATASTORE_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <set>
#include <ctime>
#include <sstream>

class SimpleMutex {
private:
    bool locked;
public:
    SimpleMutex() : locked(false) {}
    void lock() { while(locked) {} locked = true; }
    void unlock() { locked = false; }
};

class SimpleLockGuard {
private:
    SimpleMutex& mutex;
public:
    SimpleLockGuard(SimpleMutex& mtx) : mutex(mtx) { mutex.lock(); }
    ~SimpleLockGuard() { mutex.unlock(); }
};

class DataStore {
private:

    std::unordered_map<std::string, std::string> strings;
    
    // Hash data ( HSET sathi)
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> hashes;
    
    // List data
    std::unordered_map<std::string, std::vector<std::string>> lists;
    
    // Set data
    std::unordered_map<std::string, std::set<std::string>> sets;
    
    // Sorted Set data with scores
    std::unordered_map<std::string, std::map<double, std::set<std::string>>> sortedSets;
    
    // Expiry times
    std::unordered_map<std::string, time_t> expiry;
    
    SimpleMutex mtx;

    void cleanupExpired();

public:
    // String operations
    std::string set(const std::string& key, const std::string& value, int ttl = 0);
    std::string get(const std::string& key);
    int del(const std::string& key);
    bool exists(const std::string& key);
    int incr(const std::string& key);
    int decr(const std::string& key);
    
    // Hash operations
    std::string hset(const std::string& key, const std::string& field, const std::string& value);
    std::string hget(const std::string& key, const std::string& field);
    std::string hgetall(const std::string& key);
    
    // List operations
    std::string lpush(const std::string& key, const std::string& value);
    std::string rpush(const std::string& key, const std::string& value);
    std::string lpop(const std::string& key);
    std::string rpop(const std::string& key);
    std::string lrange(const std::string& key, int start, int stop);
    
    // Set operations
    std::string sadd(const std::string& key, const std::string& member);
    std::string smembers(const std::string& key);
    std::string sismember(const std::string& key, const std::string& member);
    
    // Key operations
    std::string keys(const std::string& pattern);
    int ttl(const std::string& key);
    int expire(const std::string& key, int seconds);
    
    
    int dbsize();
    std::string info();
};

#endif // export use for regarding datastore.cpp file