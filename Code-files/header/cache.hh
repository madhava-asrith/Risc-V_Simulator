#ifndef CACHE_HH
#define CACHE_HH

#include <map>
#include <random>
#include <deque>
#include <fstream>
#include "methods.hh"


struct CacheConfig 
{
    int cacheSize;
    int blockSize;
    int associativity; 
    std::string replacementPolicy;
    std::string writeBackPolicy;

    bool loadConfig(const std::string &filename);
    void printConfig() const;
};

struct CacheEntry 
{
    int tag;
    bool valid;
    bool dirty;
    int offset;
    std::vector<uint8_t> data;

    CacheEntry(int t, bool v, bool d, int blocksize, int offset) : tag(t), valid(v), dirty(d), data(blocksize,0), offset(offset) {}
};

class CacheSet 
{
    int associativity;
    std::string replacementPolicy;  
    std::string writePolicy;

public:
    std::deque<int> fifoQueue;
    std::list<CacheEntry> entries;
    std::unordered_map<int, std::list<CacheEntry>::iterator> entryMap;

    CacheSet(int assoc, const std::string &policy, const std::string wrtPolicy) 
        : associativity(assoc), replacementPolicy(policy), writePolicy(wrtPolicy) {}

    std::tuple<bool, uint64_t, int64_t>  access(bool isWrite,  uint64_t address, CacheConfig config, int access_size, uint64_t data, std::string &logFile);
    void evict(int num_sets, int index, int block_size, CacheConfig &config);
    void invalidate(int index, CacheConfig config);
    void writeToMemory(uint64_t address, std::vector<uint8_t> data, int size);
    void writeToCacheData(std::vector<uint8_t>& cacheData, uint64_t data, uint64_t address, int access_size);
    void loadFromMemory(std::vector<uint8_t>& cacheData, uint64_t address);
    friend void logCacheAccess(uint64_t address, const std::string& result, int is_write, int tag, bool dirty, std::string logFile, const CacheConfig config);
};

class Cache
{
    std::vector<CacheSet> sets;
    int access_count = 0, hit_count = 0, miss_count = 0;

    public:
    CacheConfig config;
    bool enabled;
    std::string file_name;

    Cache() : enabled(false){}

    bool enable(const std::string &file);
    void disable();
    void initialize_cache();
    void status();
    void invalidate();
    std::tuple<uint64_t, int64_t> access_cache(uint64_t address, bool is_write, int access_size, uint64_t data, std::string &file_name);
    void stats();
    void dump(const std::string &filename);
    void clear_output(std::string file_name);
};

extern Cache cache;
#endif