#include <xmmintrin.h>  //SSE
#include<Windows.h>
#include <emmintrin.h>  //SSE2
#include <pmmintrin.h>  //sse3
#include <immintrin.h>
#include<iostream>
#include<algorithm>
#include<fstream>
#include<sstream>
#include<iterator>
#include<vector>
#include<stdlib.h>
using namespace std;

#include <vector>  
#include <emmintrin.h> // 包含 SSE2 指令集的头文件  

//按表求交
std::vector<int> sse_form(std::vector<std::vector<int>>& list) {
    std::vector<int> result = list[0];
    for (size_t i = 1; i < list.size(); ++i) {
        std::vector<int> temp_result;/////!!!!!!
        int t = 0;
        temp_result.reserve(result.size()); // 预分配内存以提高效率 !!! 
        for (int j = 0; j < result.size(); ++j) {
            bool found = false;
            __m128i cur_i = _mm_set1_epi32(result[j]);
            for (size_t t = 0; t < list[i].size(); t += 4) {
                __m128i cur_j;
                if (t + 4 <= list[i].size()) {
                    cur_j = _mm_loadu_si128((__m128i*) & list[i][t]);
                }
                else {
                    // 处理剩余不足 4 个元素的情况  
                    int remaining = list[i].size() - t;
                    int data[4] = { 0 };
                    std::copy(list[i].begin() + t, list[i].begin() + t + remaining, data);
                    cur_j = _mm_loadu_si128((__m128i*)data);
                }
                __m128i cmp = _mm_cmpeq_epi32(cur_i, cur_j);
                int mask = _mm_movemask_epi8(cmp);

                if (mask != 0) {
                    found = true;
                    break;
                }
                // 如果当前元素比 result[j] 大，则后面的元素也一定大，无需继续比较  
                if (t + 4 <= list[i].size() && list[i][t] > result[j]) {
                    break;
                }
            }
            // 处理剩余不足 4 个元素的情况的末尾比较  
            if (!found && t < list[i].size()) {
                for (; t < list[i].size(); ++t) {
                    if (result[j] == list[i][t]) {
                        found = true;
                        break;
                    }
                    if (result[j] < list[i][t]) {
                        break;
                    }
                }
            }
            if (found) {
                temp_result.push_back(result[j]);
            }
        }
        result = std::move(temp_result); // 将临时结果移动到最终结果中  
    }
    return result;
}

vector<int> sse_form2(vector<vector<int>> list) {
    vector<int> result = list[0];
    for (int i = 1; i < list.size(); i++) {
        int count = 0, t = 0;
        for (int j = 0; j < result.size(); j++) {
            bool found = false;

            __m128i cur_i = _mm_set1_epi32(result[j]);

            for (; t < list[i].size() - 3; t += 4) {
                __m128i cur_j = _mm_loadu_si128((__m128i*) & list[i][t]);
                __m128i cmp = _mm_cmpeq_epi32(cur_i, cur_j);
                int mask = _mm_movemask_epi8(cmp);

                if (mask != 0) {
                    found = true;
                    break;
                }
                else if (result[j] < list[i][t]) {//后面的都会比当前元素大，不用比了
                    break;
                }
            }
            if (!found && (t >= list[i].size() - 3)) {//处理最后不能配成4个的
                for (; t < list[i].size(); t++) {
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
                result[count++] = result[j];//用匹配上的覆盖前面的
            }
        }
        if (count < result.size()) {//后面的都是多余的了
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

bool sse_find(int e, const std::vector<int>& vec) {
    if (vec.empty()) return false;

    const int* data = &vec[0];
    const int size = vec.size();
    const int alignment = 16; // SSE对齐要求  
    const int mask = alignment - 1;
    const int* aligned_data = (const int*)(((size_t)data & ~mask));
    int offset = data - aligned_data;
    int remaining = size - offset;

    __m128i val = _mm_set1_epi32(e); // 创建一个包含重复元素e的SSE向量  

    for (; offset < size; offset += 4, aligned_data += 4) {
        __m128i comp = _mm_cmpeq_epi32(_mm_loadu_si128((__m128i const*)aligned_data), val);
        unsigned int mask = _mm_movemask_epi8(comp);
        if (mask != 0) {
            return true; // 如果找到匹配项，则返回true  
        }
    }

    return false; // 如果没有找到匹配项，则返回false  
}

// 主函数：计算交集，使用SSE加速查找  
std::vector<int> sse_elem(std::vector<std::vector<int>>& li) {
    std::vector<std::vector<int>> list = li;
    std::vector<int> result;

    while (!list[0].empty()) {
        int e = list[0][0];
        list[0].erase(std::remove(list[0].begin(), list[0].end(), e), list[0].end());

        bool found = true;
        size_t s = 1;
        for (; s < list.size(); ++s) {
            found = sse_find(e, list[s]);
            if (!found) {
                break;
            }
            list[s].erase(std::remove(list[s].begin(), list[s].end(), e), list[s].end());
        }

        if (found && s == list.size()) {
            result.push_back(e);
        }

        par_sort(list);
    }
    return result;
}

////////////////////////////////


class QueryItem {
public:
    int cursor;// 当前读到哪了
    int length;// 倒排索引总长度
    int key;// 关键字值
};

//按元素求交
vector<int> elem(vector<vector<int>>& li) {
	vector<int>result0;
    QueryItem* l = new QueryItem[li.size()];
    for (int i = 0; i < li.size(); i++) {
        l[i].cursor = 0;
        l[i].key = i;
        l[i].length = li[i].size();
    }
    while (l[0].cursor < l[0].length) {
        bool found = true;
        int s = 1;
        unsigned int e = li[l[0].key][l[0].cursor];
        __m128i cur_i = _mm_set1_epi32(e);

        while ((s != li.size()) && (found == true)) {
            found = false;
            while (l[s].cursor < l[s].length - 3) {
                __m128i cur_j = _mm_load_si128((__m128i*) & li[l[s].key][l[s].cursor]);
                __m128i result = _mm_set1_epi32(0);
                result = _mm_cmpeq_epi32(cur_i, cur_j);
                int mask = _mm_movemask_epi8(result);
                if (mask != 0) {
                    found = true;
                    break;
                }
                else if (e < li[l[s].key][l[s].cursor]) {
                    break;
                }
                l[s].cursor += 4;
            }
            if (!found) {
                while (l[s].cursor < l[s].length) {
                    if (e == li[l[s].key][l[s].cursor]) {
                        found = true;
                        break;
                    }
                    else if (e < li[l[s].key][l[s].cursor]) {
                        break;
                    }
                    l[s].cursor++;
                }
            }
            s++;
        }
        l[0].cursor++;
        if (s == li.size() && found) {
            result0.push_back(e);
        }
    }
    return result0;
}

vector<int> elem2(vector<vector<int>>& li) {
    vector<int>result0;
    QueryItem* l = new QueryItem[li.size()];
    for (int i = 0; i < li.size(); i++) {
        l[i].cursor = 0;
        l[i].key = i;
        l[i].length = li[i].size();
    }
    while (l[0].cursor < l[0].length) {
        std::vector<int>tmp;//!!!!!!1
        tmp.reserve(li[0].size());//!!!
        bool found = true;
        int s = 1;
        unsigned int e = li[l[0].key][l[0].cursor];
        __m128i cur_i = _mm_set1_epi32(e);

        while ((s != li.size()) && (found == true)) {
            found = false;
            while (l[s].cursor < l[s].length) {
                __m128i cur_j;
                if (l[s].cursor + 4 <= l[s].length) {
                    cur_j = _mm_load_si128((__m128i*) & li[l[s].key][l[s].cursor]);
                }
                else {
                    int remaining = l[s].length - l[s].cursor;
                    int data[4] = { 0 };
                    std::copy(li[s].begin() + l[s].cursor, li[s].begin() + l[s].cursor + remaining,data);
                    cur_j = _mm_loadu_si128((__m128i*)data);
                }
                __m128i result=_mm_cmpeq_epi32(cur_i, cur_j);
                int mask = _mm_movemask_epi8(result);
                if (mask != 0) {
                    found = true;
                    break;
                }
                if (e < li[l[s].key][l[s].cursor]) {//???????
                    break;
                }
                l[s].cursor += 4;
            }
            if (!found && l[s].cursor<l[s].length) {
                while (l[s].cursor < l[s].length) {
                    if (e == li[l[s].key][l[s].cursor]) {
                        found = true;
                        break;
                    }
                    else if (e < li[l[s].key][l[s].cursor]) {
                        break;
                    }
                    l[s].cursor++;
                }
            }
            s++;
        }
        l[0].cursor++;
        if (s == li.size() && found) {
            tmp.push_back(e);
        }
        result0 = std::move(tmp);
    }
    return result0;
}



void print(std::vector<int> l) {
    for (std::vector<int>::iterator it = l.begin(); it != l.end(); it++) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    return;
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
    vector<vector<int>> list;

    long long head1, tail1, freq1;
    long long head2, tail2, freq2;

    while (getline(f, line)) {
        istringstream iss(line);
        vector<int> tmp;
        while (iss >> num_str) {
            tmp.push_back(stoi(num_str)); // 将字符串转换为整数并添加到vector中

        }
        list.push_back(tmp);
    }
    sort(list.begin(), list.end(), compareByLength);

    QueryPerformanceFrequency((LARGE_INTEGER*)&freq1);//获取时钟频率
    QueryPerformanceCounter((LARGE_INTEGER*)&head1);//计时开始
    for (int i = 0; i < 100; i++) {
        sse_form(list);
    }
    QueryPerformanceCounter((LARGE_INTEGER*)&tail1);//计时结束
    std::cout << "time1:" << (tail1 - head1) * 1000000.0 / freq1 << "ms" << endl;
    print(sse_form(list));

    QueryPerformanceFrequency((LARGE_INTEGER*)&freq2);//获取时钟频率
    QueryPerformanceCounter((LARGE_INTEGER*)&head2);//计时开始
    for (int i = 0; i < 100; i++) {
        elem(list);
    }
    QueryPerformanceCounter((LARGE_INTEGER*)&tail2);//计时结束
    std::cout << "time2:" << (tail2 - head2) * 10000.0 / freq2 << "ms" << endl;
    print(elem(list));

    return 0;
}


