#include<iostream>
#include<vector>
#include<fstream>
#include<sstream>
#include<string>
#include<iterator>
#include<algorithm>
#include<chrono>
#include<bitset>
#include <immintrin.h> // AVX intrinsics
#include <cstdint>
#include<omp.h>
using namespace std;

int siz;

bool compareByLength(const vector<int>& a, const vector<int>& b) {
	return a.size() < b.size(); // 使用小于号进行升序排序  
}

void print(vector<int> l) {
	for (vector<int>::iterator it = l.begin(); it != l.end(); it++) {
		cout << *it << " ";
	}
	cout << endl;
	return;
}


bitset<64> simd(vector<bitset<64>> inputVec) {
	bitset<64> result(0xFFFFFFFFFFFFFFFF); // 初始化为全1，即-1，用于按位与操作

	size_t dataSize = inputVec.size();
	if (dataSize % 8 != 0) {
		inputVec.resize(dataSize + (8 - dataSize % 8), bitset<64>(-1));
		dataSize = inputVec.size();//更新
	}

	alignas(32) vector<__m256i> inputData(dataSize / 4);
	for (size_t i = 0; i < dataSize; i += 4) {
		uint64_t data[4] = {
			inputVec[i + 3].to_ullong(),
			inputVec[i + 2].to_ullong(),
			inputVec[i + 1].to_ullong(),
			inputVec[i].to_ullong()
		};
		inputData[i / 4] = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(data));
	}

	

	vector<__m256i> threadAccumulators(omp_get_max_threads());

#pragma omp parallel
	{
		int threadNum = omp_get_thread_num();
		// 在每个线程内部累积
		__m256i localAccumulator = _mm256_set1_epi64x(-1);
		for (size_t i = threadNum; i < inputData.size(); i += omp_get_num_threads()) {
			localAccumulator = _mm256_and_si256(localAccumulator, inputData[i]);
		}
		threadAccumulators[threadNum] = localAccumulator;
	}

	__m256i globalAccumulator = _mm256_set1_epi64x(-1);
	for (const auto& acc : threadAccumulators) {
		globalAccumulator = _mm256_and_si256(globalAccumulator, acc);
	}

	int64_t a1 = _mm256_extract_epi64(globalAccumulator, 0);
	int64_t a2 = _mm256_extract_epi64(globalAccumulator, 1);
	int64_t a3 = _mm256_extract_epi64(globalAccumulator, 2);
	int64_t a4 = _mm256_extract_epi64(globalAccumulator, 3);
	a1 = a1 & a2 & a3 & a4;

	return bitset<64>(a1); // 返回64位的最终按位与结果
}

vector<int> union_res(vector<vector<bitset<64>>> b) {

	vector<bitset<64>> res(b.size());
		for (int i = 0; i < b.size(); i++) {
			if (b[i].size() == siz) {//减少不必要的运算
				res[i] = simd(b[i]);
			}
		}

	vector<int> result;

	vector<vector<int>> localResults(omp_get_max_threads());
#pragma omp parallel
	{
		int threadNum = omp_get_thread_num();
		localResults[threadNum].reserve(64); // 预先分配空间以减少动态分配开销

#pragma omp for
		for (int i = 0; i < static_cast<int>(res.size()); ++i) {
			if (b[i].size() < siz) { continue; }
			for (size_t j = 0; j < 64; ++j) {
				if (res[i].test(j)) {
					localResults[threadNum].push_back(j + i * 64);
				}
			}
		}
	}

	// 合并所有线程的局部结果
	for (auto& localResult : localResults) {
		result.insert(result.end(), localResult.begin(), localResult.end());
	}
	return result;
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
		cout << "-----" << list.size() << endl;

		siz = list.size();
		auto st1 = std::chrono::high_resolution_clock::now();
		auto sto1 = std::chrono::high_resolution_clock::now();
		vector<vector<bitset<64>>> bitmap(160);
		for (const auto& sublist : list) {
			bitset<64> tmp;
			for (int i = 0; i < sublist.size(); i++) {
				if (i != 0) {
					if (sublist[i - 1] / 64 != sublist[i] / 64) {
						bitmap[sublist[i - 1] / 64].push_back(tmp);
						tmp.reset();
					}
				}
				tmp.set(sublist[i] % 64);
			}
			bitmap[sublist[sublist.size() - 1] / 64].push_back(tmp);
			tmp.reset();
		}
		//cout << "--位图生成完成--" << endl;
		sto1 = std::chrono::high_resolution_clock::now();
		chrono::duration<double> di1 = sto1 - st1;
		cout << di1.count()<< " s" << endl;


			vector<int> result;
			auto start = std::chrono::high_resolution_clock::now();
			auto stop = std::chrono::high_resolution_clock::now();
			cout << "-----求交结果-----" << endl;
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