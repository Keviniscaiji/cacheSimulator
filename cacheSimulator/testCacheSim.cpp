#include "cachesim.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <cmath>
#include <sstream>
#include "cachesim.cpp"

// #include <iostream>
#include <string>
#include <algorithm>
using namespace std;
struct test_config_t
{
    int c_l1, b_l1, s_l1, c_l2, s_l2;
    bool prefetcher_disabled, useL2;
};

int main()
{
    string inputData;
    getline(cin, inputData);
    istringstream iss(inputData);
    string token;

    int b, s, c1, c2;
    bool prefetch, useL2;

    std::getline(iss, token, ',');
    b = std::stoi(token);

    std::getline(iss, token, ',');
    s = std::stoi(token);

    std::getline(iss, token, ',');
    c1 = std::stoi(token);

    std::getline(iss, token, ',');
    c2 = std::stoi(token);

    std::getline(iss, token, ',');
    prefetch = token == "true" ? true : false;

    std::getline(iss, token, ',');
    useL2 = token == "true" ? true : false;




    test_config_t config = {c1, b, s, c2, s, !prefetch, !useL2};
    cache_config_t l1Config = {
        .disabled = false,
        .prefetcher_disabled = config.prefetcher_disabled,
        .strided_prefetch_disabled = true,
        .c = config.c_l1,
        .b = config.b_l1,
        .s = config.s_l1,
        .replace_policy = REPLACE_POLICY_LRU,
        .prefetch_insert_policy = INSERT_POLICY_MIP,
        .write_strat = WRITE_STRAT_WBWA};

    cache_config_t l2Config = {
        .disabled = config.useL2,
        .prefetcher_disabled = config.prefetcher_disabled,
        .strided_prefetch_disabled = true,
        .c = config.c_l2,
        .b = config.b_l1, // Assuming L2 uses the same block size as L1
        .s = config.s_l2,
        .replace_policy = REPLACE_POLICY_LRU,
        .prefetch_insert_policy = INSERT_POLICY_MIP,
        .write_strat = WRITE_STRAT_WTWNA};

    sim_config_t simConfig = {
        .l1_config = l1Config,
        .l2_config = l2Config};

    cout << "L1, C=" << config.c_l1 << ", B=" << config.b_l1 << ", S=" << config.s_l1
         << ",L2, C=" << config.c_l2 << ", S=" << config.s_l2 << endl;

    sim_setup(&simConfig);
    string line;
    sim_stats_t stats;
    memset(&stats, 0, sizeof stats);
    // Process each line in the trace file
    while (true)
    {
        string line;
        std::getline(std::cin, line);
        if (line == "stop")
        {
            break;
        }

        if (line.empty())
        {
            continue; 
        }

        if (line[0] == 'R' || line[0] == 'W')
        {
            std::istringstream iss(line.substr(2)); 
            uint64_t address;
            if (iss >> std::hex >> address)
            { // 以十六进制形式读取地址
                sim_access(line[0], address, &stats);
                cout << "hit L1: " << stats.hits_l1 << " miss L1: " << stats.misses_l1 << " acc L1: " << stats.accesses_l1 << endl;
                if(stats.accesses_l2 != 0){
                    cout << " hit L2: " << stats.read_hits_l2 << " miss L2: " << stats.read_misses_l2 << " acc L2: " << stats.accesses_l2 << endl;
                }
            }
            else
            {
                std::cout << "Invalid input format. Please enter R/W followed by address.\n";
            }
        }
        else
        {
            std::cout << "Invalid operation. Please start with 'R' or 'W'.\n";
        }
    }
    sim_finish(&stats);

    // Output relevant statistics for this configuration
    cout << "hit ratio l1" << stats.hit_ratio_l1 <<  "hit ratio l2" << stats.read_hit_ratio_l2 << "AAT l1" << stats.avg_access_time_l1 
    << "AAT l2" << stats.avg_access_time_l2 << endl;
    return 0;
}