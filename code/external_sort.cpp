#include "header.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <chrono>

constexpr int BUFFER_LIMIT = 500000;    //工作区缓冲大小限制
constexpr int K = 5;    //指定k路归并
unsigned long long WPL = 0; //带权路径长度

struct MergeBlock {
    std::string file_name;
    int weight;
    MergeBlock(std::string f_name, const int& weight) : file_name(std::move(f_name)), weight(weight) {}
};

struct cmp {
    bool operator()(const MergeBlock& a, const MergeBlock& b) const {
        return a.weight > b.weight;
    }
};

using myPQ = std::priority_queue<MergeBlock, std::vector<MergeBlock>, cmp>;

myPQ generate_merge_blocks(std::ifstream& input) {
    myPQ blocks;
    std::vector<int> workspace(BUFFER_LIMIT);
    int workspace_size = 0; // 当前工作区中有效元素的数量
    
    int val;
    while (workspace_size < BUFFER_LIMIT && IntReader(input, val)) {
        workspace[workspace_size++] = val;
    }

    if (workspace_size == 0) {
        return blocks; // 文件为空
    }

    int file_index = 0;
    bool input_exhausted = false;

    // 外层循环：每次循环生成一个完整的归并段
    while (workspace_size > 0) {
        std::string block_file_name("tmp/merge_block_" + std::to_string(file_index) + ".tmp");
        std::ofstream block_file(block_file_name);
        int count = 0;

        // 将当前工作区的所有元素构建成一个最小堆
        int heap_size = workspace_size;
        std::make_heap(workspace.begin(), workspace.begin() + heap_size, std::greater<>{});

        // 内层循环：不断从堆中取数，直到堆变空，从而完成一个归并段
        while (heap_size > 0) {
            int min_val = workspace[0];

            block_file << min_val << '\n';
            count++;
            
            // 将堆顶元素与堆的最后一个元素交换，并缩小堆的大小
            std::pop_heap(workspace.begin(), workspace.begin() + heap_size, std::greater<>{});
            heap_size--;

            // 尝试补充一个新元素到工作区
            if (!input_exhausted && IntReader(input, val)) {
                workspace[heap_size] = val;
                if (val >= min_val) {
                    //认为val属于当前归并段
                    heap_size++;
                    std::push_heap(workspace.begin(), workspace.begin() + heap_size, std::greater<>{});
                } //else: val属于下一个归并段.
                
            } else {
            // 尝试失败. 输入文件已读完.    
                input_exhausted = true;

                //处理一下空出来的部分. (没有val能够填充它, 我们选择workspace中最后的那个元素来填充)
                if (heap_size < workspace_size - 1) {
                    workspace[heap_size] = workspace[workspace_size - 1];
                }
                workspace_size--;
            }
        }
        
        block_file.close();
        blocks.emplace(block_file_name, count);
        file_index++;
    }
    
    return blocks;
}


class LoserTree {
private:
    std::vector<int> tree;
    std::vector<int> leaves;
    std::vector<std::ifstream*> blocks;
    int k;

    void adjust(int s) {
        int parent = (s + k) / 2;
        while (parent > 0) {
            if (leaves[s] > leaves[tree[parent]]) {
                std::swap(s, tree[parent]);
            }
            parent /= 2;
        }
        tree[0] = s;
    }
public:
    LoserTree(const std::vector<std::ifstream*>& blocks_) : k(blocks_.size()), blocks(blocks_) {
        for (int i = 0; i < k; i++) {
            int val;
            if (IntReader(*blocks[i], val)) {
                leaves.push_back(val);
            } else {
                leaves.push_back(INT_MAX);
            }
        }
        tree.resize(k, 0);
        leaves.push_back(INT_MIN);
        for (int i = 0; i < k; i++) tree[i] = k;
        for (int i = k - 1; i >= 0; i--) adjust(i);
    }
    
    int extractMin() {
        int minIdx = tree[0];
        if (leaves[minIdx] == INT_MIN) return INT_MAX;
        int minVal = leaves[minIdx];
        int val;
        if (IntReader(*blocks[minIdx], val)) {
            leaves[minIdx] = val;
        } else {
            leaves[minIdx] = INT_MAX;
        }
        adjust(minIdx);
        return minVal;
    }
};

void Merge(myPQ& blocks, const int& k, const int& mergeIndex) {
    std::string merge_file_name("tmp/merge" + std::to_string(mergeIndex) + ".tmp");
    std::vector<std::ifstream*> blocks_to_merge;
    
    for (int i = 0; i < k; i++) {
        if (blocks.top().weight != 0) {
            std::ifstream* ifs = new std::ifstream(blocks.top().file_name);
            blocks_to_merge.push_back(ifs);
            WPL += blocks.top().weight;
        }
        blocks.pop();
    }
    LoserTree loser_tree(blocks_to_merge);

    std::ofstream merge_file(merge_file_name);
    int minVal = loser_tree.extractMin();
    int count = 0;
    while (minVal != INT_MAX) {
        merge_file << minVal << '\n';
        count++;
        minVal = loser_tree.extractMin();
    }

    blocks.emplace(merge_file_name, count);

    for (int i = 0; i < blocks_to_merge.size(); i++) {
        delete blocks_to_merge[i];
    }
}

std::string ExternalSort(std::ifstream &input) {
    /** TODO:使用置换选择排序,生成初始归并段. */
    myPQ blocks = generate_merge_blocks(input);
    /** TODO_END */

    /** TODO:归并(最佳归并树). */
    std::cout << "Start merging, k = " + std::to_string(K) + '\n';
    auto start = std::chrono::steady_clock::now();
    
    //添加虚段
    const int check = (blocks.size() - 1) % (K - 1);
    if (check != 0) {
        const int num = K - 1 - check;
        for (int i = 0; i < num; i++) {
            blocks.emplace("mono_block", 0);
        }
    }

    int mergeIndex = 0;
    while (blocks.size() > 1) {
        Merge(blocks, K, mergeIndex);
        mergeIndex++;
    }
    std::string result = "tmp/merge" + std::to_string(mergeIndex - 1) + ".tmp";

    auto end = std::chrono::steady_clock::now();
    auto duration = end - start;
    auto seconds = std::chrono::duration<double>(duration);
    std::cout << "Merging duration: " << seconds.count() << '\n';
    std::cout << "WPL: " << WPL << '\n';
    /** TODO_END */
    
    return result;
}