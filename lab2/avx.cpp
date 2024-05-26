#include <vector>  
#include <immintrin.h> // AVX指令集头文件
#include<iostream>
#include<algorithm>
#include<fstream>
#include<sstream>
#include<string>
#include<iterator>
#include<Windows.h>
#include"h.h"
using namespace std;

void print(std::vector<int> l) {
    for (std::vector<int>::iterator it = l.begin(); it != l.end(); it++) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    return;
}

std::vector<int> avx_form(std::vector<std::vector<int>>& list) {
    std::vector<int> result = list[0];
    for (size_t i = 1; i < list.size(); ++i) {
        size_t count = 0;
        for (size_t j = 0; j < result.size(); ++j) {
            bool found = false;
            __m256i cur_i = _mm256_set1_epi32(result[j]);

            size_t t = 0;
            for (; t < list[i].size(); t += 8) { // AVX每次处理8个整数  
                if (t + 8 > list[i].size()) {
                    break; // 如果不足8个整数，则跳出循环  
                }
                __m256i cur_j = _mm256_loadu_si256((__m256i*) & list[i][t]);
                __m256i cmp = _mm256_cmpeq_epi32(cur_i, cur_j);
                unsigned int mask = _mm256_movemask_epi8(cmp);

                if (mask != 0) {
                    found = true;
                    break;
                }
                else if (result[j] < list[i][t]) {
                    break; // 后面的数都比当前的大，无需比较  
                }
            }

            if (!found) {
                // 如果最后几个数不足8个，则手动检查  
                for (; t<list[i].size(); ++t) {
                    if (result[j] == list[i][t]) {
                        found = true;
                        break;
                    }
                    else if (result[j] < list[i][t]) {
                        break;
                    }
                }
            }

            if (found) {
                result[count++] = result[j];
            }
        }
        if (count < result.size()) {
            result.erase(result.begin() + count, result.end());
        }
    }
    return result;
}



//按元素求交
bool compareByLength(const std::vector<int>& a, const std::vector<int>& b) {
    return a.size() < b.size(); // 使用小于号进行升序排序  
}

void par_sort(std::vector<std::vector<int>>& list) {
#pragma omp parallel for  
    for (int i = 0; i < list.size(); ++i) {
        sort(list[i].begin(), list[i].end()); // 先对每个内部的vector进行排序  
    }

    // 然后对整个list按照长度进行排序  
    sort(list.begin(), list.end(), compareByLength);
}


//////////////////////////////////////

bool avx_find(int e, const std::vector<int>& vec) {
    if (vec.empty()) return false;

    const int* data = vec.data();
    const size_t alignment = 32;
    const size_t mask = alignment - 1;
    const int* aligned_data = reinterpret_cast<const int*>((reinterpret_cast<size_t>(data) & ~mask));
    int offset = data - aligned_data;

    __m256i val = _mm256_set1_epi32(e); // 创建一个包含重复元素e的AVX向量  

    // 使用AVX指令集加速查找  
    for (; offset + 7 < vec.size(); offset += 8, aligned_data += 8) { // 检查offset + 7以确保不会越界  
        __m256i loaded = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(aligned_data));
        __m256i comp = _mm256_cmpeq_epi32(loaded, val);
        unsigned int mask = _mm256_movemask_epi8(comp);
        if (mask != 0) {
            return true; // 如果找到匹配项，则返回true  
        }
    }

    // 处理剩余不足8个整数的部分  
    for (; offset < vec.size(); ++offset) {
        if (data[offset] == e) {
            return true;
        }
    }

    return false; // 如果没有找到匹配项，则返回false  
}

bool avx_find2(int e, const std::vector<int>& list) {
    const int SIMD_WIDTH = sizeof(__m256i) / sizeof(int); // 假设使用__m256i类型，它是AVX2的256位整数向量  
    const int N = list.size();
    const int alignedSize = (N / SIMD_WIDTH) * SIMD_WIDTH; // 计算可以完整使用AVX的元素数量  
    __m256i cmpval = _mm256_set1_epi32(e); // 创建一个包含重复元素e的AVX向量  

    for (int i = 0; i < alignedSize; i += SIMD_WIDTH) {
        __m256i data = _mm256_loadu_si256((__m256i const*)(&list[i])); // 从list中加载数据到AVX向量  
        __m256i mask = _mm256_cmpeq_epi32(data, cmpval); // 比较AVX向量中的元素和e  
        if (_mm256_testz_si256(mask, mask) == 0) { // 如果mask中有非零元素，说明找到了e  
            return true;
        }
    }

    // 处理剩余的元素（如果列表大小不是SIMD宽度的倍数）  
    for (int i = alignedSize; i < N; ++i) {
        if (list[i] == e) {
            return true;
        }
    }

    return false;
}

std::vector<int> avx_elem(std::vector<std::vector<int>>& list) {
    std::vector<int> result;

    while (!list[0].empty()) {
        int e = list[0][0];
        list[0].erase(std::remove(list[0].begin(), list[0].end(), e), list[0].end());

        bool found = true;
        size_t s = 1;
        for (; s < list.size(); ++s) {
            found = avx_find2(e, list[s]);
            if (!found) {
                break;
            }
            list[s].erase(std::remove(list[s].begin(), list[s].end(), e), list[s].end());
        }

        if (found && s == list.size()) {
            result.push_back(e);
        }
        // 排序是为了保证下一个元素仍然是所有子列表中的最小元素  
        par_sort(list);
    }
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

    long long head1, tail1, freq1;
    long long head2, tail2, freq2;

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

    cout << "avx_form:" << endl;
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq1);//获取时钟频率
    QueryPerformanceCounter((LARGE_INTEGER*)&head1);//计时开始
    for (int i = 0; i < 100; i++) {
        avx_form(list);
    }
    QueryPerformanceCounter((LARGE_INTEGER*)&tail1);//计时结束
    std::cout << "time1:" << (tail1 - head1) * 10000.0 / freq1 << "ms" << endl;
    print(avx_form(list));

    std::vector<vector<int>> li[100];
    for (int i = 0; i < 100; i++) {
        li[i] = list;
    }

    cout << "avx_elem:" << endl;
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq2);//获取时钟频率
    QueryPerformanceCounter((LARGE_INTEGER*)&head2);//计时开始
    for (int i = 0; i < 100; i++) {
        avx_elem(li[i]);
    }
    QueryPerformanceCounter((LARGE_INTEGER*)&tail2);//计时结束
    std::cout << "time2:" << (tail2 - head2) * 10000.0 / freq2 << "ms" << endl;
    print(avx_elem(list));

}