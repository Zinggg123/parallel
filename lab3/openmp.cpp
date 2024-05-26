//linux版本的openmp

#include<iostream>
#include<vector>
#include<omp.h>
#include<algorithm>
#include<fstream>
#include<sstream>
#include <chrono> 
#include<unordered_map>
using namespace std;

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
                else if (elementCounts[num] > n) { elementCounts[num] = n; }
                return false; // 保留  
            }
        ),
        vec.end()
                );
    for (auto it = vec.begin(); it != vec.end();) {
        if (elementCounts[*it] == 0) { it; }
        else if (elementCounts[*it] > 1) {
            elementCounts[*it]--;
            it = vec.erase(it);
        }
        else { ++it; }
    }
}

vector<int> form(vector <vector<int>> list) {
    vector<int> result;
    int numthread = min<int>(64, list.size());
#pragma omp parallel num_threads(numthread)
    {
        int threadId = omp_get_thread_num();
        int sz = list.size() / numthread;
        int start = threadId * sz;
        int end = (threadId == numthread - 1) ? list.size() : start + sz;

        vector<int> subre = list[start];
        for (int i = start + 1; i < end; i++) {
            for (int j = 0; j < subre.size(); j++) {
                bool r = false;
                for (int t = 0; t < list[i].size(); t++) {
                    if (list[i][t] == subre[j]) { r = true; }
                }
                if (r == false) { subre[j] = -1; }
            }
            subre.erase(std::remove_if(subre.begin(), subre.end(), [](int i) { return i == -1; }), subre.end());
        }
#pragma omp critical
        {
            result.insert(result.end(), subre.begin(), subre.end());
        }
    }
    processVector(result, numthread);
    return result;
}

vector<int> elem(vector<vector<int>> list) {
    vector<int> result;
    int numthread = min<int>(64, list.size());
#pragma omp parallel num_threads(numthread)
    {
        int threadId = omp_get_thread_num();
        int sz = list.size() / numthread;
        int start = threadId * sz;
        int end = (threadId == numthread - 1) ? list.size() : start + sz;

        vector<int> subre;
        int tmp;
        while (list[start].size()) {
            //cout << threadId << "okk" << endl; print(list[start]);
            tmp = list[start][0];
            auto iter = remove(list[start].begin(), list[start].end(), tmp);
            list[start].erase(iter, list[start].end());
            int s = start;
            bool found = false;
            for (; s < end; s++) {
                for (int i = 0; i < list[s].size(); i++) {
                    if (list[s][i] == tmp) {
                        found = true;
                        iter = remove(list[s].begin(), list[s].end(), tmp);
                        list[s].erase(iter, list[s].end());
                    }
                }
            }
            if ((s == end) && (found == true)) {
                subre.push_back(tmp);
            }
        }
#pragma omp critical
        {
            result.insert(result.end(), subre.begin(), subre.end());
        }
    }
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
    };

    string line;
    string num_str;
    vector<vector<int>> list;

    while (getline(f, line)) {
        istringstream iss(line);
        vector<int> tmp;
        while (iss >> num_str) {
            tmp.push_back(stoi(num_str)); // 将字符串转换为整数并添加到vector中

        }
        list.push_back(tmp);
    }
    sort(list.begin(), list.end(), compareByLength);

    auto start = std::chrono::high_resolution_clock::now();
    auto stop = std::chrono::high_resolution_clock::now();
    cout << "-----按表求交结果-----" << endl;
    print(form(list));
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1; i++) {
        form(list);
    }
    stop = std::chrono::high_resolution_clock::now();
    chrono::duration<double> diff = stop - start;
    cout << diff.count()/1 << " s" << endl;
    

    cout << "-----按元素求交结果-----" << endl;
    print(elem(list));
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10; i++) {
        elem(list);
    }
    stop = std::chrono::high_resolution_clock::now();
    diff = stop - start;
    cout << diff.count()/10 << " s" << endl;
    

    cout << "end";
}