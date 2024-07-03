#include<iostream>
#include<vector>
#include<fstream>
#include<sstream>
#include<string>
#include<iterator>
#include<algorithm>
#include<chrono>
#include<bitset>
#include <immintrin.h> 
#include<omp.h>
#include <unordered_map>
using namespace std;

class rbm {
public:
	vector<int> index;//桶编号
	vector<bitset<256>> buc;//桶
	int num;//桶数量

	rbm() :num(0), index(), buc() {}

	void add(int n) {
		int b = n / 256;//高八位
		int c = n % 256;//低八位

		auto it = lower_bound(index.begin(), index.end(), b);//找到≥n的最小索引
		if (it == index.end()) {//在末尾创建一个新的桶
			index.push_back(b);
			buc.push_back(0);
			buc[buc.size() - 1].set(c);
		}
		else if (*it != b) {//在该it的位置上插入
			int i = distance(index.begin(), it);
			index.insert(it, b);
			buc.insert(buc.begin() + i, 0);
			buc[i].set(c);
		}
		else {//直接在buc对应bitset中置位
			int i = distance(index.begin(), it);
			buc[i].set(c);
		}

		num++;
	}
};

void print(vector<int> l) {
	for (vector<int>::iterator it = l.begin(); it != l.end(); it++) {
		cout << *it << " ";
	}
	cout << std::endl;
	return;
}

vector<int> lalian(vector<rbm> bitmap) {
	vector<int> iter(bitmap.size());
	int minvalue = 0;
	for (int i = 0; i < bitmap.size(); i++) {
		iter[i]=0;
		if (bitmap[i].index[0] > minvalue) { minvalue = bitmap[i].index[0]; }
	}
	vector<int> result;
	while (1) {
		bool finish = false;
		for (int i = 0; i < iter.size(); i++) {
			while (bitmap[i].index[iter[i]] < minvalue) {
				++iter[i];
				if (iter[i] == bitmap[i].index.size()-1) {
					finish = true;
					break;
				}
			}
			if (finish == true) { break; }
			if (bitmap[i].index[iter[i]] > minvalue) {
				minvalue = bitmap[i].index[iter[i]];
				break;
			}
			if (i == iter.size() - 1) {
				bitset<256> tmp = bitmap[0].buc[iter[0]];
				for (int k = 1; k < iter.size(); k++) {
					tmp = tmp & bitmap[k].buc[iter[k]];
				}
				if (tmp.any()) {
					for (int k = 0; k< 256; k++) {
						if (tmp.test(k)) {
							result.push_back(bitmap[0].index[iter[0]]*256+ k);
						}
					}
				}
				++iter[0];
				if (iter[0] == bitmap[0].index.size()) {
					finish = true;
					break;
				}
				else { minvalue = bitmap[0].index[iter[0]];}
			}
		}
		if (finish == true) { break; }
	}
	return result;
}

vector<int> union_res(vector<rbm> b) {
	vector<int> res;
	vector<vector<int>> localres(b.size());

	int chunk_size = b.size() / omp_get_max_threads(); 
	if (chunk_size == 0) chunk_size = 1; 

#pragma omp parallel
	{
		int thread_id = omp_get_thread_num();
		int start = thread_id * chunk_size;
		int end = std::min(start + chunk_size, static_cast<int>(b.size()));

		vector<rbm> sub_b(b.begin() + start, b.begin() + end); // 获取当前线程处理的子集
		localres[thread_id] = lalian(sub_b); // 对子集调用lalian
	}

	// 合并所有线程的结果
	for (const auto& sublist : localres) {
		res.insert(res.end(), sublist.begin(), sublist.end());
	}

	// 统计每个元素出现的次数
	std::unordered_map<int, int> countMap;
	for (const int& elem : res) {countMap[elem]++;}

	// 删除出现次数小于chunk_size的元素
	res.erase(std::remove_if(res.begin(), res.end(), [&](const int& elem) {
		return countMap[elem] < omp_get_max_threads();}), res.end());

	sort(res.begin(), res.end());
	res.erase(unique(res.begin(), res.end()), res.end());

	return res;
}

int main() {
		vector<string> p(12);
		for (int i = 0; i < 12; i++) {
			p[i] = "./data/5000/test" + to_string(i) + ".txt";
		}
		for (int i = 0; i < 12; i++) {
			string path = p[i];
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
					tmp.push_back(stoi(num_str)); // 将字符串转换为整数并添加到vector中

				}
				list.push_back(tmp);
			}
			//cout << "--读取完成--" << endl;
			cout << "-----"<<list.size() << endl;

			auto st1 = std::chrono::high_resolution_clock::now();
			auto sto1 = std::chrono::high_resolution_clock::now();
			vector<rbm> bitmap;//数据集有多少行就会有多少个rbm。。
			for (const auto& sublist : list) {
				rbm tmp;
				for (int i = 0; i < sublist.size(); i++) {
					tmp.add(sublist[i]);
				}
				bitmap.push_back(tmp);
			}
			sto1 = std::chrono::high_resolution_clock::now();
			chrono::duration<double> di1 = sto1 - st1;
			cout << di1.count()<< " s" << endl;

			vector<int> result;
			auto start = std::chrono::high_resolution_clock::now();
			auto stop = std::chrono::high_resolution_clock::now();
			cout << "-----拉链求交结果-----" << endl;
			for (int i = 0; i < 30; i++) {
				result = union_res(bitmap);
			}
			stop = std::chrono::high_resolution_clock::now();
			chrono::duration<double> diff = stop - start;
			cout << diff.count() / 30 << " s" << endl;
			print(result);
			cout << "end" << endl << endl;
		}
}