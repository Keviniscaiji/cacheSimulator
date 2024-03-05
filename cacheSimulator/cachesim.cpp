#include "cachesim.hpp"
#include <iostream>
#include <vector>
#include <cstdint>
#include <cmath>
using namespace std;
/**
 * Subroutine for initializing the cache simulator. You many add and initialize any global or heap
 * variables as needed.
 * TODO: You're responsible for completing this routine
 */
cacheL1* globalCacheL1 = nullptr;
cacheL2* globalCacheL2 = nullptr;
int c1,c2,b1,b2,s1,s2;
int timeCount = 0;


void sim_setup(sim_config_t *config) {
    c1 = config->l1_config.c;
    s1 = config->l1_config.s;
    b1 = config->l1_config.b;

    int L1BlockNum = 1 << (c1 - b1);
    int L1SetsNum = (1 << (s1));

    c2 = config->l2_config.c;
    s2 = config->l2_config.s;
    b2 = config->l2_config.b;

    int L2BlockNum = 1 << (c2 - b2);
    int L2SetsNum = (1 << (s2));

//    std::cout <<"L1: "<< c1 << " " << b1 << " "  << s1 << std::endl;
//    std::cout <<"L2: "<< c2 << " "  << b2 << " "  << s2 << std::endl;

    globalCacheL1 = new cacheL1(config, L1BlockNum, L1SetsNum);
    globalCacheL2 = new cacheL2(config, L2BlockNum, L2SetsNum);
}
/**
 * Subroutine that simulates the cache one trace event at a time.
 * TODO: You're responsible for completing this routine
 */
void sim_access(char rw, uint64_t addr, sim_stats_t* stats) {
#ifdef DEBUG
    printf("\nTime: %d. Address: 0x%lx. Read/Write: %c \n",timeCount,addr,rw);
#endif

    if (rw == READ){
        stats->reads++;
        globalCacheL1->read(addr,stats);
    }else{
        stats->writes++;
        globalCacheL1->write(addr,stats);
    }
    timeCount ++;
}
/**
 * Subroutine for cleaning up any outstanding memory operations and calculating overall statistics
 * such as miss rate or average access time.
 * TODO: You're responsible for completing this routine
 */
void sim_finish(sim_stats_t *stats) {
    stats->hit_ratio_l1 = static_cast<double>(stats->hits_l1) / stats->accesses_l1;
    stats->miss_ratio_l1 = 1.0 - stats->hit_ratio_l1;

    stats->read_hit_ratio_l2 = static_cast<double>(stats->read_hits_l2) / stats->reads_l2;
    stats->read_miss_ratio_l2 = 1.0 - stats->read_hit_ratio_l2;

    double HTL2 = L2_HIT_K3 + (L2_HIT_K4 * (c2 - b2 - s2)) + L2_HIT_K5 * (std::max(3.0, static_cast<double>(s2)) - 3);
    if(globalCacheL2->l2config.disabled) {
        stats->avg_access_time_l2 = DRAM_ACCESS_TIME;
    } else {
        stats->avg_access_time_l2 = HTL2 + (stats->read_miss_ratio_l2 * DRAM_ACCESS_TIME);
    }
    double HTL1 = L1_HIT_K0 + (L1_HIT_K1 * (c1 - b1 - s1)) + L1_HIT_K2 * (std::max(3.0, static_cast<double>(s1)) - 3);
    if(globalCacheL2->l2config.disabled) {
        stats->avg_access_time_l1 = HTL1 + (stats->miss_ratio_l1 * DRAM_ACCESS_TIME);
    } else {
        double L1MissTimeL2Hit = HTL2;
        double L1MissTimeL2Miss = HTL2 + DRAM_ACCESS_TIME;
        stats->avg_access_time_l1 = HTL1 + stats->miss_ratio_l1 * ((1 - stats->read_miss_ratio_l2) * L1MissTimeL2Hit + stats->read_miss_ratio_l2 * L1MissTimeL2Miss);
    }

    delete globalCacheL1;
    delete globalCacheL2;
}
cacheL1::cacheL1(sim_config_t *config, int b_num, int s_num):cache(b_num, std::vector<block>(s_num)){
//    std::cout <<"init cache L1 "<< b_num <<" "<< s_num << std::endl;
    l1config = config->l1_config;
    currentTime = pow(2,l1config.c - l1config.b - l1config.s);
    initializeCache();
}
void cacheL1::read(uint64_t addr, sim_stats_t* stats) {
    stats->accesses_l1 ++;
    uint64_t index = (addr >> l1config.b) & ((1 << (l1config.c - l1config.b - l1config.s)) - 1);
    uint64_t tag = addr >> (l1config.c - l1config.s);

#ifdef DEBUG
    printf("L1 decomposed address 0x%" PRIx64 " -> Tag: 0x%" PRIx64 " and Index: 0x%" PRIx64 "\n",addr,tag,index);
#endif
    bool hit = false;
    std::vector<block> &row = cache[index];
    for (int i = 0; i < row.size(); ++i) {
        if(row[i].valid && row[i].tag == tag) {
#ifdef DEBUG
            printf("L1 hit\n");
#endif
            hit = true;
            handleCacheHit(row, i, currentTime, l1config);
            break;
#ifdef DEBUG
            printf("In L1, moving Tag: 0x%" PRIx64 " and Index: 0x%" PRIx64 " to MRU position\n", tag,index);
#endif
        }
    }
    if(!hit){
#ifdef DEBUG
        printf("L1 miss\n");
#endif
        globalCacheL2->read(addr,stats);

        int replaceIdx = findReplacementIndex(row, l1config);
        if (row[replaceIdx].dirty) {
            uint64_t reconstructed_addr = (row[replaceIdx].tag << (l1config.c - l1config.s)) | (index << l1config.b);
            globalCacheL2->write(reconstructed_addr, stats);
            row[replaceIdx].dirty = false;
        }
#ifdef DEBUG
        printf("Evict from L1: block with valid=%lx and index=%lx",row[replaceIdx].valid,index);
#endif
        row[replaceIdx].tag = tag;
        row[replaceIdx].valid = true;
        cleanMRU(row);
        row[replaceIdx].MRU = true;
        row[replaceIdx].timestamp = currentTime;
        row[replaceIdx].use = 1;
        currentTime++;
    }
    if (hit) {
        stats->hits_l1++;
    } else {
        stats->misses_l1++;
    }
#ifdef DEBUG
    printf("\n");
#endif
}
void cacheL1::write(uint64_t addr, sim_stats_t* stats) {
    stats->accesses_l1 ++;
    uint64_t index = (addr >> l1config.b) & ((1 << (l1config.c - l1config.b - l1config.s)) - 1);
    uint64_t tag = addr >> (l1config.c - l1config.s);
    bool hit = false;
    std::vector<block> &row = cache[index];
    for (int i = 0; i < cache[index].size(); ++i) {
        if (row[i].valid && row[i].tag == tag) {
            hit = true;
            handleCacheHit(row, i, currentTime, l1config);
            row[i].dirty = true;
#ifdef DEBUG
            printf("In L1, moving Tag: 0x%" PRIx64 " and Index: 0x%" PRIx64 " to MRU position\n", tag,i);
#endif
            break;
        }
    }
// not hit read for l1
    if (!hit) {
        globalCacheL2->read(addr,stats);
        int replaceIdx = findReplacementIndex(row, l1config);
        // perform write back operation
        if (row[replaceIdx].dirty) {
            uint64_t reconstructed_addr = (row[replaceIdx].tag << (l1config.c - l1config.s)) | (index << l1config.b);
            globalCacheL2->write(reconstructed_addr, stats);
            row[replaceIdx].dirty = false;
        }
#ifdef DEBUG
        printf("Evict from L1: block with valid=%lx and index=%lx\n",row[replaceIdx].valid,index);
#endif
        row[replaceIdx].tag = tag;
        row[replaceIdx].valid = true;
        cleanMRU(row);
        row[replaceIdx].MRU = true;
        row[replaceIdx].timestamp = currentTime;
        row[replaceIdx].use = 0;
        row[replaceIdx].dirty = true;
        currentTime++;
    }
    if (hit) {
        stats->hits_l1++;
    } else {
        stats->misses_l1++;
    }
#ifdef DEBUG
    printf("\n");
#endif
}
cacheL2::cacheL2(sim_config_t *config, int b_num, int s_num) : l2config(config->l2_config),
                                                               cache(b_num, std::vector<block>(s_num))  {
    previousMissAddr = 0x0;
    currentTime = pow(2,(config->l2_config.c - config->l1_config.b + 1));
    initializeCache();
//    std::cout <<"init cache L2 "<< b_num <<" "<< s_num << std::endl;
}
void cacheL2::read(uint64_t addr, sim_stats_t* stats) {
    stats->accesses_l2++;
    stats->reads_l2++;
    if (l2config.disabled) {
        stats->read_misses_l2++;
        return;
    }
    uint64_t index = (addr >> l2config.b) & ((1 << (l2config.c - l2config.b - l2config.s)) - 1);
    uint64_t tag = addr >> (l2config.c - l2config.s);
#ifdef DEBUG
    printf("L2 decomposed address 0x%lx -> Tag: 0x%lx and Index: 0x%lx\n", addr, tag, index);
#endif
    bool hit = false;
    vector<block> &row = cache[index];
    for (int i = 0; i < row.size(); ++i) {
        if (row[i].valid && row[i].tag == tag) {
            hit = true;
            row[i].timestamp = currentTime;
            cleanMRU(row);
            row[i].valid = true;
            row[i].MRU = true;
            row[i].use ++;
            currentTime ++;
#ifdef DEBUG
            printf("In L2, moving Tag: 0x%" PRIx64 " and Index: 0x%" PRIx64 " to MRU position\n", tag, index);
#endif
            break;
        }
    }

    if (!hit) {
//        int replaceIdx = 0;
//        uint64_t minVal = UINT64_MAX;
        int replaceIdx = findReplacementIndex(row, l2config);
        cleanMRU(row);
        row[replaceIdx].MRU = true;
        row[replaceIdx].use = 1;
        row[replaceIdx].valid = true;
        row[replaceIdx].tag = tag;
        row[replaceIdx].timestamp = currentTime;
        currentTime++;
#ifdef DEBUG
        printf("L2 read miss\n");
#endif

        bool prefetchHit = false;
        if (!l2config.prefetcher_disabled) {
            auto [prefetchTag, prefetchIdx] = calculatePrefetchAddress(index, tag);
            vector<block> &newRow = cache[prefetchIdx];
            // if the prefetch one hit
            for (int i = 0; i < newRow.size(); i++) {
                if (newRow[i].valid && newRow[i].tag == prefetchTag) {
                    prefetchHit = true;
                    break;
                }
            }

            // if the prefetched block not hit
            if (!prefetchHit) {
                stats->prefetches_l2++;
//                uint64_t targetIdx = 0;
//                uint64_t targetValue = UINT64_MAX;
                // LRU MIP
                int targetIdx = findReplacementIndex(newRow, l2config);
#ifdef DEBUG
                printf("Evict from L2: block with valid=%lx and index=%lx\n",row[targetIdx].valid,index);
#endif
                prefetchIntoCache(newRow, targetIdx, prefetchTag, l2config, currentTime);
             }
        }
    }

    if(hit) {
#ifdef DEBUG
        printf("L2 read hit\n");
#endif
        stats->read_hits_l2++;
    } else {
#ifdef DEBUG
        printf("L2 did not find block in cache on write, writing through to memory anyway\n");
#endif
        stats->read_misses_l2++;
    }
}
void cacheL2::write(uint64_t addr, sim_stats_t* stats) {
    stats->accesses_l2++;
    stats->writes_l2 ++;
    if(l2config.disabled){
#ifdef DEBUG
        printf("L2 is disabled, writing through to memory\n");
#endif
        return;
    }
    uint64_t index = (addr >> l2config.b) & ((1 << (l2config.c - l2config.b - l2config.s)) - 1);
    uint64_t tag = addr >> (l2config.c - l2config.s);
    vector<block> &row = cache[index];
    for (int i = 0; i < row.size(); ++i) {
        if (row[i].valid && row[i].tag == tag) {

#ifdef DEBUG
            printf("L2 found block in cache on write\n");
            printf("In L2, moving Tag: 0x%" PRIx64 " and Index: 0x%" PRIx64 " to MRU position\n", tag,index);
#endif
            cleanMRU(row);
            row[i].MRU = true;
            row[i].timestamp = currentTime;
            row[i].use ++;
            row[i].valid = true;
            row[i].dirty = true;
            currentTime++;
            break;
        }
    }
}
void cacheL1::initializeCache() {
    for (int i = 0; i < cache.size(); ++i) {
        for (int j = 0; j < cache[i].size(); ++j) {
            cache[i][j].timestamp = -1;
            cache[i][j].dirty = false;
            cache[i][j].valid = false;
            cache[i][j].MRU = false;
            cache[i][j].tag = -1;
            cache[i][j].use = -1;
        }
    }
}
void cacheL2::initializeCache() {
    for (int i = 0; i < cache.size(); ++i) {
        for (int j = 0; j < cache[i].size(); ++j) {
            cache[i][j].timestamp = -1;
            cache[i][j].dirty = false;
            cache[i][j].valid = false;
            cache[i][j].MRU = false;
            cache[i][j].tag = -1;
            cache[i][j].use = -1;
        }
    }
}
pair<uint64_t, uint64_t> cacheL2:: calculatePrefetchAddress(uint64_t index, uint64_t tag) {
    uint64_t curVal = (tag << (l2config.c - l2config.b - l2config.s)) | index;
    uint64_t prefetchVal;
    if (l2config.strided_prefetch_disabled) {
        prefetchVal = curVal + 1;
    } else {
        prefetchVal = curVal + (curVal - previousMissAddr);
        previousMissAddr = curVal;
    }
//    printf("old address : 0x%lx \n",curVal);
//    printf("tag : 0x%lx \n",tag);
//    printf("index : 0x%lx \n",index);


    uint64_t indexBits = l2config.c - l2config.b - l2config.s;
    uint64_t prefetchTag = prefetchVal >> indexBits;
    uint64_t indexMask = (1ULL << indexBits) - 1;
    uint64_t prefetchIdx = (prefetchVal) & indexMask;

//    printf("prefetch : 0x%lx \n",prefetchVal << l2config.b);
//    printf("prefetch tag: 0x%lx \n",prefetchTag);
//    printf("prefetch idx: 0x%lx \n",prefetchIdx);
//    printf("From debug output: 0x%lx\n\n", (0xac1fd6dafe80));



#ifdef DEBUG
    printf("Prefetch block with address 0x%lx from memory to L2 \n", (prefetchTag << (l2config.c - l2config.b - l2config.s) | prefetchIdx) << l2config.b);
#endif
    return make_pair(prefetchTag,prefetchIdx);
}
void handleCacheHit(std::vector<block>& row, int hitIndex, uint64_t& currentTime, cache_config_t& l1config) {
    row[hitIndex].timestamp = currentTime;
    cleanMRU(row);
    row[hitIndex].MRU = true;
    row[hitIndex].use++;
    row[hitIndex].valid = true;
    currentTime++;
}
void cleanMRU(vector<block > &row){
    for(int i = 0; i < row.size(); i++){
        row[i].MRU = false;
    }
}
void prefetchIntoCache(vector<block>& newRow, int targetIdx, uint64_t prefetchTag, cache_config_t l2config, uint64_t& currentTime) {
    if (l2config.replace_policy == REPLACE_POLICY_LRU) {
        if (l2config.prefetch_insert_policy == INSERT_POLICY_MIP) {
            newRow[targetIdx].valid = true;
            newRow[targetIdx].timestamp = currentTime;
            newRow[targetIdx].tag = prefetchTag;
            currentTime++;
        } else {
            uint64_t lowestTime = INT64_MAX;
            for (int i = 0; i < newRow.size(); i++) {
                if (newRow[i].valid && newRow[i].timestamp < lowestTime) {
                    lowestTime = newRow[i].timestamp;
                }
            }
            newRow[targetIdx].valid = true;
            newRow[targetIdx].tag = prefetchTag;
            if (lowestTime == INT64_MAX) newRow[targetIdx].timestamp = currentTime;
            else newRow[targetIdx].timestamp = lowestTime - 1;
            currentTime++;
        }
    } else {
        if (l2config.prefetch_insert_policy == INSERT_POLICY_MIP) {
            cleanMRU(newRow);
            newRow[targetIdx].MRU = true;
            newRow[targetIdx].valid = true;
            newRow[targetIdx].tag = prefetchTag;
            newRow[targetIdx].use = 0;
        } else {
            newRow[targetIdx].MRU = false;
            newRow[targetIdx].valid = true;
            newRow[targetIdx].tag = prefetchTag;
            newRow[targetIdx].use = 0;
        }
    }
}
int findReplacementIndex(std::vector<block> row, cache_config_t config) {
    int replaceIdx = -1; // Initialize with an invalid index
    uint64_t minValue = UINT64_MAX;
    int rowSize = row.size();
    for (int i = 0; i < rowSize; ++i) {
        if (!row[i].valid) {
            return i;
        } else if (config.replace_policy == REPLACE_POLICY_LRU) {
            if (row[i].timestamp < minValue) {
                replaceIdx = i;
                minValue = row[i].timestamp;
            }
        } else {
            if (!row[i].MRU && (row[i].use < minValue||(row[i].use == row[replaceIdx].use&&row[i].tag < row[replaceIdx].tag))) {
                replaceIdx = i;
                minValue = row[i].use;
            }
        }
    }

    return replaceIdx; // Return the found index or -1 if all entries are valid and no other condition met
}


