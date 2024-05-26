#include<iostream>
#include<vector>
#include<fstream>
#include<sstream>
#include<string>
#include<iterator>
#include<algorithm>
#include<Windows.h>
using namespace std;

void print(std::vector<int> l) {
	for (std::vector<int>::iterator it = l.begin(); it != l.end(); it++) {
		std::cout << *it << " ";
	}
	std::cout << std::endl;
	return;
}

bool find(int e, vector<int> list) {
	for (vector<int>::iterator it = list.begin(); it != list.end(); it++) {
		if (*it == e) {
			return true;
		}
	}
	return false;
}

bool compareByLength(const vector<int>& a, const vector<int>& b) {
	return a.size() < b.size(); // 使用小于号进行升序排序  
}

//按表求交
vector<int> form(vector<vector<int>> list) {
	vector<int> result = list[0];
	for (vector<vector<int>>::iterator t=next(list.begin());t!=list.end();t++) {
		vector<int> tmp = *t;
		for (vector<int>::iterator it = result.begin(); it != result.end();)
		{
			if (!find(*it, tmp)) {
				it=result.erase(it);
			}
			else {
				it++;
			}
		}
	}
	return result;
}
//按元素求交
vector<int> elem(vector<vector<int>>& li) {
	std::vector<std::vector<int>> list = li;
	vector<int>result;
	while (list[0].size() != 0) {
		int e = list[0][0];
		auto iter0 = remove(list[0].begin(), list[0].end(), e);
		list[0].erase(iter0, list[0].end());
		int s = 1;
		bool found = true;
		for (; s <list.size(); s++) {
			found = find(e, list[s]);
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

	cout << "-----串行按表求交结果-----" << endl;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq1);//获取时钟频率
	QueryPerformanceCounter((LARGE_INTEGER*)&head1);//计时开始
	for (int i = 0; i < 1; i++) {
		form(list);
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&tail1);//计时结束
	std::cout << "time1:" << (tail1 - head1) * 100.0 / freq1 << "ms" << endl;
	print(form(list));

	cout << "-----串行按元素求交结果-----" << endl;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq2);//获取时钟频率
	QueryPerformanceCounter((LARGE_INTEGER*)&head2);//计时开始
	for (int i = 0; i < 1; i++) {
		elem(list);
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&tail2);//计时结束
	std::cout << "time2:" << (tail2 - head2) * 100.0 / freq2 << "ms" << endl;
	print(elem(list));

	cout <<endl<< "end";
}
