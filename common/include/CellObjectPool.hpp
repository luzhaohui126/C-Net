#pragma once

#include<assert.h>
#include<stdlib.h>
#include<thread>
#include<mutex>

//#define OPRINT
#ifdef OPRINT
#include<stdio.h>
#define xPrintf(...) printf(__VA_ARGS__)
#else
#define xPrintf(...)
#endif OPRINT

template<class Type,size_t nPoolSize>
class CellobjectPool
{
public:
	CellobjectPool() {
		_pBuf = nullptr;
		_pHeader = nullptr;
	}
	~CellobjectPool() {
		if (_pBuf) {
			delete[] _pBuf;
			_pBuf = nullptr;
		}
	}
public:
	class NodeHeader
	{
	 public:
		//下一块位置
		NodeHeader* pNext;
		//内存块编号
		int nID;
		//引用次数
		char nRef;
		//是否在内存池中
		bool bPool;
	private:
		//预留
		char c1;
		char c2;
	};
	//申请
	void *allocObj(size_t nSize) {
		std::lock_guard<std::mutex> lg(_mutex);
		if (!_pBuf) {
			initPool();
		}
		NodeHeader *pReturn = nullptr;
		if (nullptr == _pHeader) {
			//没有可用块
			pReturn = (NodeHeader *)new char [sizeof(Type) + sizeof(NodeHeader)];
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pNext = nullptr;
			printf("overpool allocObj: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, (int)nSize);
		}
		else {
			//
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}
		xPrintf("allocObj: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, (int)nSize);
		return ((char *)pReturn + sizeof(NodeHeader));
	}
	//释放
	void freeObj(void *pObj) {
		NodeHeader *pBlock = (NodeHeader *)((char *)pObj - sizeof(NodeHeader));
		assert(1 == pBlock->nRef);
		pBlock->nRef = 0;
		if (pBlock->bPool) {
			std::lock_guard<std::mutex> lg(_mutex);
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else {
			delete pBlock;
		}
	}
	//初始化
	void initPool(){
		xPrintf("initPool:_nSzie=%d,_nBlockSzie=%d\n", (int)sizeof(Type), (int)nPoolSize);
		assert(nullptr == _pBuf);
		if (_pBuf)return;
		//对象池大小
		size_t realSize = sizeof(Type) + sizeof(NodeHeader);
		size_t nSize = realSize * nPoolSize;
		//申请内存
		_pBuf = new char[nSize];
		//
		_pHeader = (NodeHeader*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pNext = nullptr;
		NodeHeader* pLast = _pHeader;
		for (size_t i = 1; i < nPoolSize; i++)
		{
			NodeHeader* pTemp = (NodeHeader*)(_pBuf + i * realSize);
			pTemp->bPool = true;
			pTemp->nID = (int)i;
			pTemp->nRef = 0;
			pTemp->pNext = nullptr;
			pLast->pNext = pTemp;
			pLast = pTemp;
		}
	}
protected:
	//头部位置
	NodeHeader *_pHeader;
	//对象池指针
	char *_pBuf;
	//
	std::mutex _mutex;
};

template<class Type, size_t nPoolSize>
class objectPoolBase
{
public:
	objectPoolBase() {

	}
	~objectPoolBase() {

	}
	void *operator new(const size_t size) {
		//return malloc(size);
		return objectPool().allocObj(size);
	}
	void operator delete(void *p) {
		objectPool().freeObj(p);
		//free(p);
	}
	template<typename ...Args>
	static Type* createObject(Args ...args) {
		Type* obj = new Type(args...);
		return obj;
	}
	static void destroyObject(Type* obj) {
		delete obj;
	}
private:
	typedef CellobjectPool<Type, nPoolSize> classTypePool;
	static classTypePool& objectPool() {
		static classTypePool sPoll;
		return sPoll;
	}
};
