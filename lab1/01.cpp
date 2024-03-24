//计算给定n*n矩阵的每一列与给定向量的内积
//1.逐列访问
//2.cache优化
#include<iostream>
#include<Windows.h>
#include<stdlib.h>
using namespace std;

int* normal(int **a, int* b, int n) {
	int* result=new int[n];
	for (int i = 0; i < n; i++) {
		result[i] = 0;
	}
	for (int i = 0; i < n; i++) {//第j行第i列
		for (int j = 0; j < n; j++) {
			result[i] += b[j] * a[j][i];
		}
	}
	return result;
}

int* cache(int** a, int* b, int n) {
	int* result = new int[n];

	for (int i = 0; i < n; i++) {
		result[i] = 0;//初始化
	}
	for (int i = 0; i < n; i++) {//第j列第i行
		for (int j = 0; j < n; j++) {
			result[i] += b[i] * a[i][j];
		}
	}
	return result;
}

int main0() {
	int n; 
	cout << "please input scale:";
	cin >> n;
	int** a = new int*[n];
	for (int i = 0; i < n; i++) {
		a[i] = new int[n];
	}

	int* b = new int[n];
	for (int i = 1; i <= n; i++) {
		b[i - 1] = i;
		for (int j = 0; j < n; j++) {
			a[i - 1][j] = i;
		}
	}//初始化

	long long head1, tail1, freq1;
	long long head2, tail2, freq2;
	long long now;

	QueryPerformanceFrequency((LARGE_INTEGER*)&freq1);//获取时钟频率
	QueryPerformanceCounter((LARGE_INTEGER*)&head1);//计时开始
	for (int i = 0; i < 1000; i++) {
		normal(a, b, n);
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&tail1);//计时结束
	cout << "normal:" << (tail1 - head1) * 1.0 / freq1 << "ms" <<endl;

	QueryPerformanceFrequency((LARGE_INTEGER*)&freq2);//获取时钟频率
	QueryPerformanceCounter((LARGE_INTEGER*)&head2);//计时开始
	for (int i = 0; i < 1000; i++) {
		cache(a, b, n);
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&tail2);//计时结束
	cout << "cache optimized:" << (tail2 - head2) * 1.0 / freq2 << "ms" << endl;
	return 0;
}

int main() {
	int p = 1;
	while (p == 1) {
		main0();
		cout << endl << endl << "continue?";
		cin >> p;
	}
}
