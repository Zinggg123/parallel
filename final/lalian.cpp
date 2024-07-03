#include<iostream>
#include<vector>
#include<fstream>
#include<sstream>
#include<string>
#include<iterator>
#include<algorithm>
#include<chrono>
#include<omp.h>
using namespace std;

bool compareByLength(const vector<int>& a, const vector<int>& b) {
	return a.size() < b.size(); // 使用小于号进行升序排序  
}

void print(vector<int> l) {
	for (vector<int>::iterator it = l.begin(); it != l.end(); it++) {
		cout << *it << " ";
	}
	cout << std::endl;
	return;
}

vector<int> lalian(vector<vector<int>> b) {
	//获取各列表的头元素
	vector<vector<int>::const_iterator> iter;
	int minvalue = 0;
	for (int i = 0; i < b.size(); i++) {
		iter.push_back(b[i].begin());
		if (*iter[i] > minvalue) {
			//此时要取各列表中最大的值，因为他们都是自己列表中最小的
			minvalue = *iter[i];
		}
	}

	//拉链法求交
	vector<int> result;
	while (true) {
		bool finish = false;
		for (int i = 0; i < iter.size(); i++) {
			while (*iter[i] < minvalue) {
				++iter[i];//把它加到大于等于min
				if (iter[i] == b[i].end()) {
					finish = true;
					break;
				}
			}
			if (finish == true) { break; }
			if (*iter[i] > minvalue) {
				minvalue = *iter[i];
				//更新min，因为b[i]里无匹配值，放弃这个minvalue
				//跳出这个循环，再来一遍
				break;
			}
			if (i == iter.size() - 1) {
				//现在还没跳出，说明在所有列表中都找到了匹配
				result.push_back(*iter[i]);
				++iter[0];//更新一下最小值
				if (iter[0] == b[0].end()) {
					finish = true;
					break;
				}
				else {
					minvalue = *iter[0];
				}
			}
		}
		if (finish == true) {
			break;
		}
	}

	return result;
}

vector<int> union_res(vector<vector<vector<int>>> b) {
	vector<int> res;
	vector<vector<int>> localres(b.size());

#pragma omp parallel
	{
#pragma omp for
		for (int i = 0; i < b.size(); ++i) {
			localres[i] = lalian(b[i]); // 单个桶的交集计算
		}
	}

	for (const auto& lres : localres) {
		for (const auto& num : lres) {
			res.push_back(num);
		}
	}

	return res;
}


vector<int> union_res0(vector<vector<vector<int>>> b) {
	vector<int>res;

	for (int i = 0; i < b.size(); i++) {
		vector<int> tmp;
		tmp = lalian(b[i]);
		for (const auto& num : tmp) {
			res.push_back(num);
		}
	}
	return res;
}

int main() {
	vector<string> p(12);
	for (int i = 0; i < 12; i++) {
		p[i] = "./data/10000/test" + to_string(i) + ".txt";
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
		cout << list.size() << endl;
		//cout << "--读取完成--" << endl;

		auto st1 = std::chrono::high_resolution_clock::now();
		auto st2 = std::chrono::high_resolution_clock::now();
		//分桶水平切分
		vector<vector<vector<int>>>bucket(8);
		for (const auto& sublist : list) {
			vector<int>tmp;//一个sublist有一个tmp，当当前元素超出当前bucket范围就存储并清空tmp，循环利用
			for (int i = 0; i < sublist.size(); i++) {
				if (i != 0) {
					if (sublist[i] / 1250 != sublist[i - 1] / 1250) {
						bucket[sublist[i - 1] / 1250].push_back(tmp);
						tmp.clear();
					}
				}
				tmp.push_back(sublist[i]);
			}
			bucket[7].push_back(tmp);
		}
		//cout << "--分桶完成--" << endl;
		st2 = std::chrono::high_resolution_clock::now();
		chrono::duration<double> di1 = st2 - st1;
		cout << di1.count() / 10 << " s" << endl;

		vector<int> result;
		auto start = std::chrono::high_resolution_clock::now();
		auto stop = std::chrono::high_resolution_clock::now();
		cout << "-----拉链求交结果-----" << endl;
		for (int i = 0; i < 10; i++) {
			result = union_res(bucket);
		}
		stop = std::chrono::high_resolution_clock::now();
		chrono::duration<double> diff = stop - start;
		cout << diff.count() / 10 << " s" << endl;
		print(result);

		cout << "-----串行拉链求交结果-----" << endl;
		start = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < 10; i++) {
			result = union_res0(bucket);
		}
		stop = std::chrono::high_resolution_clock::now();
		diff = stop - start;
		cout << diff.count() / 10 << " s" << endl;
		print(result);

		cout << "end"<<endl<<endl;
	}
}