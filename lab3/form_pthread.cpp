#include <iostream>  
#include <vector>  
#include <pthread.h>  
#include <unordered_map>  
#include <algorithm>  
#include <numeric>  
#include <chrono> 
#include <stdlib.h>
#include <sstream>
#include <fstream>
using namespace std;

vector<vector<int>> li;
std::vector<int> result;
pthread_mutex_t amutex = PTHREAD_MUTEX_INITIALIZER;

bool compareByLength(const vector<int>& a, const vector<int>& b) {
    return a.size() < b.size(); // 使用小于号进行升序排序  
}

void print(std::vector<int> l) {
    for (std::vector<int>::iterator it = l.begin(); it != l.end(); it++) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    return;
}
 
void processVector(std::vector<int>& vec, int n) {
    std::unordered_map<int, int> elementCounts;
    for (int num : vec) {
        elementCounts[num]++;
    }
    vec.erase(
        std::remove_if(vec.begin(), vec.end(),
            [&elementCounts, n](int num) {
                if (elementCounts[num] < n) { return true; }
                else { return false; }
            }
        ),
        vec.end()
                );
    for (auto it = vec.begin(); it != vec.end();) {
        if (elementCounts[*it] > 1) {
            elementCounts[*it]--;
            it = vec.erase(it);
        }
        else { ++it; }
    }
}

// 线程工作函数  
void* worker(void* arg) {
    auto* data = static_cast<std::pair<int, int>*>(arg);
    int start = data->first;
    int end = data->second;

    std::vector<int> subre;
    for (int i = start; i < end; ++i) {
        subre = li[i]; // 初始化subre为当前子列表  
    }

    // 遍历剩余的子列表，标记不在当前子列表中的元素  
    for (int j = start + 1; j < end; ++j) {
        subre.erase(std::remove_if(subre.begin(), subre.end(),
            [&](int num) {
                for (int t = 0; t < li[j].size(); ++t) {
                    if (num == li[j][t]) {
                        return false; 
                    }
                }
                return true;
            }), subre.end());
    }

    pthread_mutex_lock(&amutex);
    result.insert(result.end(), subre.begin(), subre.end());
    pthread_mutex_unlock(&amutex);

    return nullptr;
}

std::vector<int> form() {
    int numthread = std::min<int>(8, li.size());

    pthread_t threads[numthread];
    std::pair<int, int> ranges[numthread];

    int sz = li.size() / numthread;
    for (int i = 0; i < numthread; ++i) {
        ranges[i].first = i * sz;
        ranges[i].second = (i == numthread - 1) ? li.size() : ranges[i].first + sz;

        pthread_create(&threads[i], nullptr, worker, &ranges[i]);
    }

    // 等待所有线程完成  
    for (int i = 0; i < numthread; ++i) {
        pthread_join(threads[i], nullptr);
    }

    // 在所有线程完成后处理结果  
    processVector(result, numthread);

    return result;
}

int main() {
    string path = "test.txt";
    fstream f;
    f.open(path, ios::in);
    if (!f.is_open()) {
        cout << "failed to open " << path << endl;
        return 0;
    }

    string line;
    string num_str;

    while (getline(f, line)) {
        istringstream iss(line);
        vector<int> tmp;
        while (iss >> num_str) {
            tmp.push_back(stoi(num_str)); // 将字符串转换为整数并添加到vector中

        }
        li.push_back(tmp);
    }
    sort(li.begin(), li.end(), compareByLength);

    cout << "-----按表求交结果-----" << endl;
    auto start = std::chrono::high_resolution_clock::now();
    auto stop = std::chrono::high_resolution_clock::now();
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10; i++) {
        form();
    }
    stop = std::chrono::high_resolution_clock::now();
    chrono::duration<double> diff = stop - start;
    cout << diff.count()/10 << " s" << endl;
    print(result);

    return 0;
}
