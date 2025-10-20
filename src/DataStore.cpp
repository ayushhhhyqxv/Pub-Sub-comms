#include "DataStore.h" // not using namespace std here .

void DataStore::cleanupExpired() {
    time_t now = time(nullptr);
    auto it = expiry.begin();
    while (it != expiry.end()) {
        if (now >= it->second) {
            strings.erase(it->first);
            hashes.erase(it->first);
            lists.erase(it->first);
            sets.erase(it->first);
            sortedSets.erase(it->first);
            it = expiry.erase(it);
        } else {
            ++it;
        }
    }
}


std::string DataStore::set(const std::string& key, const std::string& value, int ttl) {
    SimpleLockGuard lock(mtx);
    cleanupExpired();
    
    strings[key] = value;
    if (ttl > 0) {
        expiry[key] = time(nullptr) + ttl;
    } else {
        expiry.erase(key);
    }
    return "OK";
}

std::string DataStore::get(const std::string& key) {
    SimpleLockGuard lock(mtx);
    cleanupExpired();
    
    auto it = strings.find(key);
    if (it == strings.end()) return "(nil)";
    return it->second;
}

int DataStore::del(const std::string& key) {
    SimpleLockGuard lock(mtx);
    int count = 0;
    
    if (strings.erase(key)) count++;
    if (hashes.erase(key)) count++;
    if (lists.erase(key)) count++;
    if (sets.erase(key)) count++;
    if (sortedSets.erase(key)) count++;
    expiry.erase(key);
    
    return count;
}

bool DataStore::exists(const std::string& key) {
    SimpleLockGuard lock(mtx);
    cleanupExpired();
    
    return strings.count(key) || hashes.count(key) || lists.count(key) || 
           sets.count(key) || sortedSets.count(key);
}

int DataStore::incr(const std::string& key) {
    SimpleLockGuard lock(mtx);
    cleanupExpired();
    
    try {
        if (strings.find(key) == strings.end()) {
            strings[key] = "0";
        }
        int value = std::stoi(strings[key]);
        value++;
        strings[key] = std::to_string(value);
        return value;
    } catch (...) {
        return -1;
    }
}

int DataStore::decr(const std::string& key) {
    SimpleLockGuard lock(mtx);
    cleanupExpired();
    
    try {
        if (strings.find(key) == strings.end()) {
            strings[key] = "0";
        }
        int value = std::stoi(strings[key]);
        value--;
        strings[key] = std::to_string(value);
        return value;
    } catch (...) {
        return -1;
    }
}


std::string DataStore::hset(const std::string& key, const std::string& field, const std::string& value) {
    SimpleLockGuard lock(mtx);
    cleanupExpired();
    
    hashes[key][field] = value;
    return "1";
}

std::string DataStore::hget(const std::string& key, const std::string& field) {
    SimpleLockGuard lock(mtx);
    cleanupExpired();
    
    auto hash_it = hashes.find(key);
    if (hash_it == hashes.end()) return "(nil)";
    
    auto field_it = hash_it->second.find(field);
    if (field_it == hash_it->second.end()) return "(nil)";
    
    return field_it->second;
}

std::string DataStore::hgetall(const std::string& key) {
    SimpleLockGuard lock(mtx);
    cleanupExpired();
    
    auto hash_it = hashes.find(key);
    if (hash_it == hashes.end()) return "(empty)";
    
    std::string result;
    for (const auto& pair : hash_it->second) {
        result += pair.first + ": " + pair.second + "\n";
    }
    return result.empty() ? "(empty)" : result;
}


std::string DataStore::lpush(const std::string& key, const std::string& value) {
    SimpleLockGuard lock(mtx);
    cleanupExpired();
    
    lists[key].insert(lists[key].begin(), value);
    return std::to_string(lists[key].size());
}

std::string DataStore::rpush(const std::string& key, const std::string& value) {
    SimpleLockGuard lock(mtx);
    cleanupExpired();
    
    lists[key].push_back(value);
    return std::to_string(lists[key].size());
}

std::string DataStore::lpop(const std::string& key) {
    SimpleLockGuard lock(mtx);
    cleanupExpired();
    
    auto it = lists.find(key);
    if (it == lists.end() || it->second.empty()) return "(nil)";
    
    std::string value = it->second.front();
    it->second.erase(it->second.begin());
    return value;
}

std::string DataStore::rpop(const std::string& key) {
    SimpleLockGuard lock(mtx);
    cleanupExpired();
    
    auto it = lists.find(key);
    if (it == lists.end() || it->second.empty()) return "(nil)";
    
    std::string value = it->second.back();
    it->second.pop_back();
    return value;
}

std::string DataStore::lrange(const std::string& key, int start, int stop) {
    SimpleLockGuard lock(mtx);
    cleanupExpired();
    
    auto it = lists.find(key);
    if (it == lists.end()) return "(empty)";
    
    const auto& list = it->second;
    if (start < 0) start = list.size() + start;
    if (stop < 0) stop = list.size() + stop;
    if (start < 0) start = 0;
    if (stop >= list.size()) stop = list.size() - 1;
    
    std::string result;
    for (int i = start; i <= stop && i < list.size(); ++i) {
        result += std::to_string(i) + ") " + list[i] + "\n";
    }
    return result.empty() ? "(empty)" : result;
}

// Set operations
std::string DataStore::sadd(const std::string& key, const std::string& member) {
    SimpleLockGuard lock(mtx);
    cleanupExpired();
    
    auto result = sets[key].insert(member);
    return result.second ? "1" : "0";
}

std::string DataStore::smembers(const std::string& key) {
    SimpleLockGuard lock(mtx);
    cleanupExpired();
    
    auto it = sets.find(key);
    if (it == sets.end()) return "(empty)";
    
    std::string result;
    for (const auto& member : it->second) {
        result += member + "\n";
    }
    return result.empty() ? "(empty)" : result;
}

std::string DataStore::sismember(const std::string& key, const std::string& member) {
    SimpleLockGuard lock(mtx);
    cleanupExpired();
    
    auto it = sets.find(key);
    if (it == sets.end()) return "0";
    
    return it->second.count(member) ? "1" : "0";
}

// Key operations
std::string DataStore::keys(const std::string& pattern) {
    SimpleLockGuard lock(mtx);
    cleanupExpired();
    
    std::string result;
    for (const auto& pair : strings) result += pair.first + "\n";
    for (const auto& pair : hashes) result += pair.first + "\n";
    for (const auto& pair : lists) result += pair.first + "\n";
    for (const auto& pair : sets) result += pair.first + "\n";
    for (const auto& pair : sortedSets) result += pair.first + "\n";
    
    return result.empty() ? "(empty)" : result;
}

int DataStore::ttl(const std::string& key) {
    SimpleLockGuard lock(mtx);
    auto it = expiry.find(key);
    if (it == expiry.end()) return -1;
    
    time_t now = time(nullptr);
    if (now >= it->second) {
        del(key);
        return -2;
    }
    
    return it->second - now;
}

int DataStore::expire(const std::string& key, int seconds) {
    SimpleLockGuard lock(mtx);
    if (!exists(key)) return 0;
    
    expiry[key] = time(nullptr) + seconds;
    return 1;
}

int DataStore::dbsize() {
    SimpleLockGuard lock(mtx);
    cleanupExpired();
    
    std::set<std::string> allKeys;
    for (const auto& pair : strings) allKeys.insert(pair.first);
    for (const auto& pair : hashes) allKeys.insert(pair.first);
    for (const auto& pair : lists) allKeys.insert(pair.first);
    for (const auto& pair : sets) allKeys.insert(pair.first);
    for (const auto& pair : sortedSets) allKeys.insert(pair.first);
    
    return allKeys.size();
}

std::string DataStore::info() {
    SimpleLockGuard lock(mtx);
    cleanupExpired();
    
    std::stringstream ss;
    ss << "Redis Server Info:\n";
    ss << "Database Size: " << dbsize() << " keys\n";
    ss << "Strings: " << strings.size() << "\n";
    ss << "Hashes: " << hashes.size() << "\n";
    ss << "Lists: " << lists.size() << "\n";
    ss << "Sets: " << sets.size() << "\n";
    ss << "Sorted Sets: " << sortedSets.size() << "\n";
    
    return ss.str();
}