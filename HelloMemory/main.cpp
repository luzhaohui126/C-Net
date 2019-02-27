
#include<stdlib.h>
#include<thread>
#include<mutex>
#include<memory>
#include<memory>

#include"Alloctor.h"
#include"MemoryMgr.hpp"
#include"CellTimestamp.hpp"
#include"CellObjectPool.hpp"

//std::mutex m;
const int tCount = 8;
const int mCount = 100000;
const int nCount = mCount/tCount;

class classA : public objectPoolBase<classA,100000>
{
public:
	classA() {
		nums = 0;
		printf("classA\n");
	}
	classA(int n) {
		nums = n;
		printf("classA\n");
	}
	~classA() {
		printf("~classA\n");
	}
	int nums = 0;
private:
};
class classB : public objectPoolBase<classB,10000>
{
public:
classB(int n, int m) {
	nums1 = n;
	nums2 = m;
	printf("classB\n");
}
~classB() {
	printf("~classB\n");
}
int nums1 = 0;
int nums2 = 0;
private:

};

void workFunc(int index) {
	//	char* data[nCount];
	classA* data[nCount];
	//	size_t size;

	for (int i = 0; i < nCount; i++)
	{
		//		size = (rand() % 128) + 1;
		//		data[i] = new char[size];
		data[i] = new classA(5);
	}
	for (int i = 0; i < nCount; i++)
	{
		delete data[i];
	}
}

//#define SHA(x,y) std::shared_ptr<y> x std::make_shared<y>()
void func(std::shared_ptr<classA> a) {
	printf("%d\n", a.use_count());
	a->nums++;
}
int main(int argc, char *argv[]) {

	//char* data[nCount];
	//for (size_t i = 0; i < nCount; i++)
	//{
	//	data[i] = new char[(rand() % 1000) + 1];
	//}
	//for (size_t i = 0; i < nCount; i++)
	//{
	//	delete[] data[i];
	//}

/*	CellTimestamp tTime;
	std::thread t[tCount];
	for (size_t i = 0; i < tCount; i++)
	{
		t[i] = std::thread(workFunc, (int)i);
	}
	for (size_t i = 0; i < tCount; i++)
	{
		t[i].join();
	}
	printf("time %f\n", tTime.getElapsedTimeInMilliSec());*/
	//classA* a = new classA();
	////delete a;

	//std::shared_ptr<int> b = std::make_shared<int>();
	//*b = 100;
	//printf("b=%d\n", *b);

	//std::shared_ptr<classA> aa = std::make_shared<classA>();
	//printf("%d\n", aa.use_count());
	//func(aa);
	//printf("%d\n", aa.use_count());
	//printf("classA nums=%d\n", aa->nums);
	//printf("%d\n", aa.use_count());

//	char *a = new char[100];
//	delete a;

//	char *b = new char[50];
//	delete b;

	//std::shared_ptr<classA> s1 = std::make_shared<classA>(5);

	//std::shared_ptr<classB> s2(new classB(5,6));

//	classA* a1 = new classA();
//	delete a1;

//	classB* b1 = new classB(5,6);
//	delete b1;

	//classA* a2 = classA::createObject(6);
	//classA::destroyObject(a2);

//	classB* b1 = classB::createObject(5,6);
//	classB::destroyObject(b1);

	time_t _oldTime = CellTime::getNowInMilliSec();
	time_t _tTime = 0;
	while (true) {
		auto nowTime = CellTime::getNowInMilliSec();
		auto dt = nowTime - _oldTime;
		if (dt >= 1000) {
			printf("_tTime=%d %d\n", _tTime, dt);
			//Sleep(93);
			_tTime += dt;
			_oldTime = nowTime;
		}
	}
	return 0;
}
