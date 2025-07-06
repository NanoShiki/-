#include "header.h"
#include <string>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <random>
#include <windows.h>
#include <psapi.h>
#include <thread>
#include <algorithm>
const int N = 200;

unsigned int seed = 0;

void GenerateNumbers(std::string& input_file_name) {
    std::ofstream unsorted(input_file_name);
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    std::mt19937 engine(seed = static_cast<unsigned int>(timestamp.count()));
    std::uniform_int_distribution<> distrib(0, 1 << 30);
    for (int i = 0; i < N; i++) {
        unsorted << distrib(engine) << '\n';
    }
    unsorted.close();
}

// void OutputScore(int score) {
//    Hidden
// }

int CalcScore(unsigned long long memory_cost) {
    return std::min(100, (int)(N * 40. / memory_cost));
}

bool Check(std::string& input_file_name, std::string& output_file_name) {
    std::ifstream sorted(output_file_name);
    int a, b;
    std::vector<int> arra, arrb;
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    std::mt19937 engine(seed);
    std::uniform_int_distribution<> distrib(0, 1 << 30);
    for (int i = 0; i < N; i++) {
        arra.emplace_back(distrib(engine));
    }
    while (IntReader(sorted, b)) {
        arrb.emplace_back(b);
    }
    if (arra.size() < arrb.size()) {
        std::cerr << "[SortError]: The number of sorted numbers is more than number of unsorted numbers.\n";
        sorted.close();
        return false;
    } else if (arra.size() > arrb.size()) {
        std::cerr << "[SortError]: The number of sorted numbers is less than number of unsorted numbers.\n";
        sorted.close();
        return false;
    }
    std::sort(arra.begin(), arra.end());
    for (int i = 0; i < arra.size(); i++) {
        if (arra[i] != arrb[i]) {
            std::cerr << "[SortError]: Wrong answer.\n";
            sorted.close();
            return false;
        }
    }
    sorted.close();
    return true;
}

int main() {
    std::string intput_file_name = "unsorted.input";
    GenerateNumbers(intput_file_name);
    unsigned long long memory_cost = 0;
    HANDLE handle = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS pmc;
    bool sort_end = false;
    std::ifstream input(intput_file_name);
    std::thread monitor([&memory_cost, &handle, &pmc, &sort_end](){
        while (!sort_end) {
            GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
            memory_cost = std::max(memory_cost, pmc.WorkingSetSize);
        }
    });
    std::string output_file_name = ExternalSort(input);
    sort_end = true;
    monitor.join();
    if (Check(intput_file_name, output_file_name)) {
        std::cerr << "Sorting result correct.\n";
        std::cerr << "Memory cost: " << memory_cost / 1024. / 1024. << " MB\n";
        auto score = CalcScore(memory_cost);
        std::cerr << "Score: " << score << '\n';
    } else {
        std::cerr << "External sorting failed. Please fix your code.\n";
    }
    return 0;
}
