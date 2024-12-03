#include "cache.hh"
#include "methods.hh" 
#include <cmath>
  
Cache cache;

bool CacheConfig::loadConfig(const std::string &filename) 
{
    std::ifstream file(filename);
    if (!file) 
    {
        std::cerr << "Failed to open config file." << std::endl;
        return false;
    }
    
    file >> cacheSize;
    file >> blockSize;
    file >> associativity;
    file >> replacementPolicy;
    file >> writeBackPolicy;
    
    file.close();
    return true;
}

void CacheConfig::printConfig() const 
{
    std::cout << "Cache Size: " << cacheSize << "\n";
    std::cout << "Block Size: " << blockSize << "\n";
    std::cout << "Associativity: " << associativity << "\n";
    std::cout << "Replacement Policy: " << replacementPolicy << "\n";
    std::cout << "Write Back Policy: " << writeBackPolicy << "\n";
}

void logCacheAccess(uint64_t address, const std::string& result, int is_write, int tag, bool dirty, std::string file_name, const CacheConfig config)
{
    size_t dot_pos = file_name.find('.'); 
    if (dot_pos != std::string::npos) {
        if(dot_pos == 0)
        {
            file_name.clear();
        }
        else
        {
            file_name = file_name.substr(0, dot_pos);  // Remove comment portion
        }
    }

    file_name += ".output";

    std::ofstream log_file(file_name, std::ios::app);
    int num_sets = 0;
    if(config.associativity != 0)
    {
        num_sets = config.cacheSize / (config.blockSize * config.associativity);
    }
    else 
    {
        num_sets = 1;
    }
    int index = (address / config.blockSize) % num_sets;

    log_file << (!is_write ? "R" : "W") << ": Address: 0x" << std::hex << address 
            << ", Set: 0x" << index
            << ", " << result 
            << ", Tag: 0x" << tag 
            << ", " << (dirty ? "Dirty" : "Clean") 
            << std::endl;
    
    log_file.close();
}

std::tuple<bool, uint64_t, int64_t> CacheSet::access(bool isWrite, uint64_t address,  CacheConfig config, int access_size = 8, uint64_t data = 0, std::string &file_name = cache.file_name) 
{
    int num_sets, index, tag, blockOffset, num_blocks;

    if(config.associativity != 0)
    {
        num_sets = config.cacheSize / (config.blockSize * config.associativity);
        index = (address / config.blockSize) % num_sets;
        tag = address / (config.blockSize * num_sets);
        blockOffset = address % config.blockSize;
    }
    else
    {
        num_sets = 1; 
        index = 0;
        num_blocks = config.cacheSize / config.blockSize;
        tag = address / config.blockSize; 
        blockOffset = address % config.blockSize; 
    }

    auto it = entryMap.find(tag);

    if(it != entryMap.end()){
        CacheEntry& entry = *(it->second);

        if(entry.valid)
        {
            if(isWrite)
            {
                entry.dirty = true;
                writeToCacheData(entry.data, data, address, access_size);
            }

            if(replacementPolicy == "LRU")
            {
                entries.splice(entries.begin(), entries, it->second);
            }
            else if(replacementPolicy == "FIFO")
            {
                fifoQueue.push_back(tag);
            }

            logCacheAccess(address, "Hit", isWrite, entry.tag, entry.dirty, file_name, config);

            uint64_t ret_data = 0;
            int64_t s_ret_data = 0;

            for(int i=0;i<entry.data.size();i++)
            {
                ret_data += (entry.data[i] << (i * 8));
                s_ret_data += (entry.data[i] << (i * 8));
            }

            return {true, ret_data, s_ret_data};
        }
        else 
        {
            if(entries.size() == associativity || entries.size() == num_blocks)
            {
                evict(num_sets, index, access_size, config);
            }

            CacheEntry newEntry(tag, true, isWrite, config.blockSize, blockOffset);
            loadFromMemory(newEntry.data, address);

            if(isWrite)
            {
                newEntry.dirty = true;
                writeToCacheData(newEntry.data, data, address, access_size);
            }

            entries.emplace_front(newEntry);
            entryMap[tag] = entries.begin();
            
            logCacheAccess(address, "Miss", isWrite, newEntry.tag, newEntry.dirty, file_name, config);

            uint64_t ret_data = 0;
            int64_t s_ret_data = 0;

            for(int i=0;i<newEntry.data.size();i++)
            {
                ret_data += (newEntry.data[i] << (i * 8));
                s_ret_data += (newEntry.data[i] << (i * 8));
            }

            return {false, ret_data, s_ret_data};
        }
    }
    else
    {
        if(entries.size() == associativity || entries.size() == num_blocks)
        {
            evict(num_sets, index, access_size, config);
        }

        CacheEntry newEntry(tag, true, isWrite, config.blockSize, blockOffset);
        loadFromMemory(newEntry.data, address);

        if(isWrite)
        {
            newEntry.dirty = true;
            writeToCacheData(newEntry.data, data, address, access_size);
        }

        entries.emplace_front(newEntry);
        entryMap[tag] = entries.begin();
        
        logCacheAccess(address, "Miss", isWrite, newEntry.tag, newEntry.dirty, file_name, config);
        
        uint64_t ret_data = 0;
        int64_t s_ret_data = 0;

        for(int i=0;i<newEntry.data.size();i++)
        {
            ret_data += (newEntry.data[i] << (i * 8));
            s_ret_data += (newEntry.data[i] << (i * 8));
        }
        
        return {false, ret_data, s_ret_data};
    }
    
}

void CacheSet::evict(int num_sets, int index, int block_size, CacheConfig &config) 
{
    if (entries.empty()) return;
    
    if (replacementPolicy == "LRU" ) 
    {
        // FIFO and LRU evict the last element in the list
        CacheEntry evicted = entries.back();
        if (writePolicy == "WB" && evicted.dirty && config.associativity != 0) 
        {
            uint64_t address = (evicted.tag << static_cast<int>(log2(num_sets) + log2(block_size))) + (index << static_cast<int>(log2(block_size))) + evicted.offset;
            // Write back to memory if dirty
            writeToMemory(address, evicted.data, config.blockSize); 
        }
        else if(writePolicy == "WB" && evicted.dirty && config.associativity == 0)
        {
            uint64_t address = (evicted.tag << static_cast<int>( log2(block_size))) + evicted.offset;
            // Write back to memory if dirty
            writeToMemory(address, evicted.data, config.blockSize); 
        }

        entryMap.erase(evicted.tag);
        entries.pop_back();
    } 
    else if(replacementPolicy == "FIFO")
    {
        int evictedTag = fifoQueue.front();
        CacheEntry evicted = *(entryMap.find(evictedTag) ->second);

        if (writePolicy == "WB" && evicted.dirty && config.associativity != 0) 
        {
            uint64_t address = (evicted.tag << static_cast<int>(log2(num_sets) + log2(block_size))) + (index << static_cast<int>(log2(block_size))) + evicted.offset;
            // Write back to memory if dirty
            writeToMemory(address, evicted.data, config.blockSize); 
        }
        else if(writePolicy == "WB" && evicted.dirty && config.associativity == 0)
        {
            uint64_t address = (evicted.tag << static_cast<int>( log2(block_size))) + evicted.offset;
            // Write back to memory if dirty
            writeToMemory(address, evicted.data, config.blockSize); 
        }

        fifoQueue.pop_front();
        entryMap.erase(evictedTag);
    }
    else if (replacementPolicy == "RANDOM") 
    {
        // For RANDOM, remove a random entry
        int randomIndex = std::rand() % entries.size();
        auto it = entries.begin();
        std::advance(it, randomIndex);
        
        if (writePolicy == "WB" && it->dirty && config.associativity != 0) 
        {
            uint64_t address = (it->tag << static_cast<int>(log2(num_sets) + log2(block_size))) + (index << static_cast<int>(log2(block_size))) + it->offset;
            writeToMemory(address, it->data, config.blockSize); // Write back if dirty
        }
        else if(writePolicy == "WB" && it->dirty && config.associativity == 0)
        {
            uint64_t address = (it->tag << static_cast<int>( log2(block_size))) + it->offset;
            // Write back to memory if dirty
            writeToMemory(address, it->data, config.blockSize); 
        }

        entryMap.erase(it->tag);
        entries.erase(it);
    }
}

void CacheSet::writeToMemory(uint64_t address, std::vector<uint8_t> data, int block_size)
{
    int offset = address % data.size();

    for(int i=0;(i ) <data.size();i++)
    {
        memory.store_byte((address), data[i + offset]);
        address += 1;
    }
}

void CacheSet::writeToCacheData(std::vector<uint8_t>& cacheData, uint64_t data, uint64_t address, int access_size)
{
    int offset = address % cacheData.size();
    
    for(int i = 0; i < access_size; ++i) 
    {
        cacheData[offset + i] = (data) & 0xFF;
        data = data >> 8;        
    }
}

void CacheSet::loadFromMemory(std::vector<uint8_t>& cacheData, uint64_t address)
{
    for(size_t i = 0; i < cacheData.size(); ++i) 
    {
        cacheData[i] = memory.read_byte_unsigned(address + i);  // Reading data from memory
    }
}


// -----------------------------------------------------------------------------
// NEEDS A REWORK FOR INVALIDATE ----------- NEED TO SAVE THE DATA TO MEMORY BEFORE INVALIDATING
// --------------------------------------------------------------------------------
void CacheSet::invalidate(int index, CacheConfig config) 
{   
    int num_sets;
    if(config.associativity != 0)
    {
        num_sets = config.cacheSize / (config.blockSize * config.associativity);
    }
    else 
    {
        num_sets = 1;
    }
    
    for(auto& entry : entries)
    {
        if(entry.valid && entry.dirty && config.associativity != 0)
        {
            uint64_t address = (entry.tag << static_cast<int>(log2(num_sets) + log2(config.blockSize))) + (index << static_cast<int>(log2(config.blockSize))) + entry.offset;
            writeToMemory(address, entry.data, entry.data.size());
        }
        else if(writePolicy == "WB" && entry.dirty && config.associativity == 0)
        {
            uint64_t address = (entry.tag << static_cast<int>( log2(config.blockSize))) + entry.offset;
            // Write back to memory if dirty
            writeToMemory(address, entry.data, config.blockSize); 
        }
    }
    entries.clear();
    fifoQueue.clear();
    entryMap.clear();
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void Cache::stats()
{
    std::cout  << "D-cache statistics: "
                  << "Accesses=" << access_count
                  << ", Hit=" << hit_count
                  << ", Miss=" << miss_count
                  << ", Hit Rate=" << (hit_count / static_cast<double>(access_count)) << std::endl;
}

bool Cache::enable(const std::string &file)
{
    if(enabled)
    {
        return false;
    }

    enabled = config.loadConfig(file);
    if(enabled)
    {
        initialize_cache();
    }

    return enabled;
}

void Cache::disable()
{
    enabled = false;
    sets.clear();
}

void Cache::initialize_cache()
{
    int indices;
    if(config.associativity != 0)
    {
        indices = config.cacheSize / (config.blockSize * config.associativity);
    }
    else 
    {
        indices = 1;
    }
    sets = std::vector<CacheSet>(indices, CacheSet(config.associativity, config.replacementPolicy, config.writeBackPolicy));
}

void Cache::status()
{
    std::cout << (enabled ? "Enabled" : "Disabled") << std::endl;
    if(enabled)
    {
        config.printConfig();
    }
}

void Cache::invalidate()
{
    for(int index=0;index<sets.size();index++)
    {
        sets[index].invalidate(index, config);
    }

    access_count = 0;
    hit_count = 0;
    miss_count = 0;
    cache.enabled = false;
    cache.file_name.clear();
}

void Cache::dump(const std::string &file)
{
    std::ofstream output(file);

    if(!output)
    {
        return;
    }

    for (size_t i = 0; i < sets.size(); ++i) 
    {
        const auto &set = sets[i];
        for (const auto &entry : set.entries) 
        {
            if (entry.valid) 
            {
                output << "Set: 0x" << std::hex << i 
                        << ", Tag: 0x" << entry.tag 
                        << (entry.dirty ? ", Dirty\n" : ", Clean\n");
            }
        }
    }

    output.close();
}

std::tuple<uint64_t, int64_t> Cache::access_cache(uint64_t address, bool is_write, int access_size, uint64_t data = 0, std::string &file_name = cache.file_name)
{
    if(!enabled) return {0,0};
    if(access_size > config.blockSize)
    {
        std::cerr << "Access size greater than block size of the cache" << std::endl;
    }

    access_count++;

    int num_sets, index, tag, blockOffset, num_blocks;

    if(config.associativity != 0)
    {
        num_sets = config.cacheSize / (config.blockSize * config.associativity);
        index = (address / config.blockSize) % num_sets;
        tag = address / (config.blockSize * num_sets);
        blockOffset = address % config.blockSize;
    }
    else
    {
        index = 0;
        num_blocks = config.cacheSize / config.blockSize;
        tag = address / config.blockSize; 
        blockOffset = address % config.blockSize; 
    }

    auto [hit, us_data, s_data] = sets[index].access(is_write, address, config, access_size, data, file_name);

    if(hit)
    {
        hit_count++;
    }
    else
    {
        miss_count++;
    }

    return {us_data, s_data};
}

void Cache::clear_output(std::string file_name)
{
    size_t dot_pos = file_name.find('.'); 
    if (dot_pos != std::string::npos) {
        if(dot_pos == 0)
        {
            file_name.clear();
        }
        else
        {
            file_name = file_name.substr(0, dot_pos);  // Remove comment portion
        }
    }

    file_name += ".output";
    std::ofstream file(file_name, std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open log file " << file_name << std::endl;
        return;
    }
    file.close(); 
}