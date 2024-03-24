//�������n*n�����ÿһ��������������ڻ�
//1.���з���
//2.cache�Ż�
#include<iostream>
#include<Windows.h>
#include<stdlib.h>
using namespace std;

int* normal(int **a, int* b, int n) {
	int* result=new int[n];
	for (int i = 0; i < n; i++) {
		result[i] = 0;
	}
	for (int i = 0; i < n; i++) {//��j�е�i��
		for (int j = 0; j < n; j++) {
			result[i] += b[j] * a[j][i];
		}
	}
	return result;
}

int* cache(int** a, int* b, int n) {
	int* result = new int[n];

	for (int i = 0; i < n; i++) {
		result[i] = 0;//��ʼ��
	}
	for (int i = 0; i < n; i++) {//��j�е�i��
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
	}//��ʼ��

	long long head1, tail1, freq1;
	long long head2, tail2, freq2;
	long long now;

	QueryPerformanceFrequency((LARGE_INTEGER*)&freq1);//��ȡʱ��Ƶ��
	QueryPerformanceCounter((LARGE_INTEGER*)&head1);//��ʱ��ʼ
	for (int i = 0; i < 1000; i++) {
		normal(a, b, n);
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&tail1);//��ʱ����
	cout << "normal:" << (tail1 - head1) * 1.0 / freq1 << "ms" <<endl;

	QueryPerformanceFrequency((LARGE_INTEGER*)&freq2);//��ȡʱ��Ƶ��
	QueryPerformanceCounter((LARGE_INTEGER*)&head2);//��ʱ��ʼ
	for (int i = 0; i < 1000; i++) {
		cache(a, b, n);
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&tail2);//��ʱ����
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
