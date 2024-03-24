//����n�����ĺ�
//1.����ۼ�
//2.�������Ż��㷨
#include<iostream>
#include<Windows.h>
#include<omp.h>
#include<thread>
using namespace std;

long normal(int* a, int n) {
	long result=0;
	for (int i = 0; i < n; i++) {
		result += a[i];
	}
	return result;
}

long optimize(int* a, int m, int n) {
	if (m + 1 == n) {
		return long(a[m] + a[n]);
	}
	else if (m == n) { return long(a[m]); }
	else {
		long sum1=0, sum2=0;
		int mid1 = (m + n) / 2,mid2=mid1+1;
#pragma omp task shared(sum1) firstprivate(m,mid1)
		{
			sum1=optimize(a, m, (m + n) / 2);
		}
#pragma omp task shared(sum2) firstprivate(mid2,n)
		{
			sum2=optimize(a, (m + n) / 2 + 1, n);
		}

#pragma omp taskwait
		return sum1 + sum2;
	}
}

int optimize2(int* a, int n) {
	int sum = 0;
#pragma omp parallel for reduction(+:sum)
	for (int i = 0; i < n; i++) {
		sum += a[i];
	}
	return sum;
}


int main0() {
	int n; volatile long result1,result2,result3;
	std::cout << "please input scale:";
	cin >> n;

	int* a = new int[n];
	for (int i = 0; i < n; i++) {
		a[i] = i/10 + 1;
	}//��ʼ��

	long long head1, tail1, freq1;
	long long head2, tail2, freq2;

	//normal
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq1);//��ȡʱ��Ƶ��
	QueryPerformanceCounter((LARGE_INTEGER*)&head1);//��ʱ��ʼ
	for (int i = 0; i < 1000; i++) {
		result1=normal(a, n);
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&tail1);//��ʱ����
	cout << "--result:" << result1 << endl;
	std::cout << "normal:" << (tail1 - head1) * 1000.0 / freq1 << "ms" << endl;
	//���г���
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq2);//��ȡʱ��Ƶ��
	QueryPerformanceCounter((LARGE_INTEGER*)&head2);//��ʱ��ʼ
	//��ȡ���߳���
	int t= thread::hardware_concurrency();

#pragma omp parallel num_threads(t)
		{
#pragma omp single
			{
				for (int i = 0; i < 100; i++) {
					result2=optimize(a, 0, n - 1);
				}
			}
		}
	QueryPerformanceCounter((LARGE_INTEGER*)&tail2);//��ʱ����
	cout << "--result:" << result2 << endl;
	std::cout << "�ݹ�optimized:" << (tail2 - head2) * 10000.0 / freq2 << "ms" << endl;

	QueryPerformanceFrequency((LARGE_INTEGER*)&freq2);//��ȡʱ��Ƶ��
	QueryPerformanceCounter((LARGE_INTEGER*)&head2);//��ʱ��ʼ
	t = thread::hardware_concurrency();
	for (int i = 0; i < 100; i++) {
		result3=optimize2(a, n);
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&tail2);//��ʱ����
	cout << "--result:" << result3<< endl;
	std::cout << "�ϰ�optimized:" << (tail2 - head2) * 10000.0 / freq2 << "ms" << endl;

	return 0;
}

int main() {
	int p=1;
	while (p == 1) {
		main0();
		cout <<endl<<endl<< "continue?";
		cin >> p;
	}
}
