#include <arm_neon.h>  
#include <vector>  
#include <iterator>  
#include <algorithm>
#include<fstream>
#include<sstream>
#include<string>
#include<iostream>
using namespace std;

void print(std::vector<int> l) {
    for (std::vector<int>::iterator it = l.begin(); it != l.end(); it++) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    return;
}

bool compareByLength(const vector<int>& a, const vector<int>& b) {
    return a.size() < b.size();
}

std::vector<int> neon_form(std::vector<std::vector<int>>& list) {
    std::vector<int> result = list[0];
    for (size_t i = 1; i < list.size(); ++i) {
        std::vector<int> temp_result;
        temp_result.reserve(result.size());
        for (size_t j = 0; j < result.size(); ++j) {
            bool found = false;
            int32x4_t cur_i = vdupq_n_s32(result[j]);
            size_t t = 0;
            for (; t < list[i].size() - 3; t += 4) {
                int32x4_t cur_j = vld1q_s32(&list[i][t]);
                uint32x4_t cmp = vceqq_s32(cur_i, cur_j);
                uint32x4_t any_non_zero = vmovn_u32(vpaddq_u32(cmp, cmp));
                uint32_t mask_val = vaddv_u32(any_non_zero); 
                if (mask_val != 0) {
                    found = true;
                    break;
                }
                if (result[j] < list[i][t]) {
                    break;
                }
            }
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
        result.swap(temp_result);
    }
    return result;
}

bool neon_find(int e, std::vector<int>& list) {
    if (list.size() % 4 != 0) {
        return neon_find(e, list);
    }
    int32x4_t target = vdupq_n_s32(e); 
    for (size_t i = 0; i < list.size(); i += 4) {
        int32x4_t data = vld1q_s32(&list[i]);
        uint32x4_t cmp = vceqq_s32(data, target); 
        uint32_t mask = vmovemask_u32(cmp); 
        if (mask != 0) { 
            return true;
        }
    }
    return false; 
}

vector<int> neon_elem(vector<vector<int>>& list) {
    vector<int>result;
    while (list[0].size() != 0) {
        int e = list[0][0];
        auto iter0 = remove(list[0].begin(), list[0].end(), e);
        list[0].erase(iter0, list[0].end());
        int s = 1;
        bool found = true;
        for (; s < list.size(); s++) {
            found = neon_find(e, list[s]);
            if (found == true) {
                auto iter = remove(list[s].begin(), list[s].end(), e);
                list[s].erase(iter, list[s].end());
            }
            else {
                break;
            }
        }
        if ((s == list.size()) && (found == true)) {
            result.push_back(e);
        }
        sort(list.begin(), list.end(), compareByLength);
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
    string line;
    string num_str;
    vector<vector<int>> list;

    while (getline(f, line)) {
        istringstream iss(line);
        vector<int> tmp;
        while (iss >> num_str) {
            tmp.push_back(stoi(num_str)); 

        }
        list.push_back(tmp);
    }
    sort(list.begin(), list.end(), compareByLength);

    cout << "---初始输入倒排索引表---" << endl;
    for (vector<vector<int>>::iterator it = list.begin(); it != list.end(); it++)
    {
        print(*it);
    }
    cout << endl;
    cout << "-----NEON按表求交结果-----" << endl;
    print(neon_form(list));
    cout << "-----NEON按元素求交结果-----" << endl;
    print(neon_elem(list));

}