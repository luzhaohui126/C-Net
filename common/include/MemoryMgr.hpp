#pragma once

#include<assert.h>
#include<stdlib.h>
#include<stdio.h>
#include<mutex>
#include<atomic>
#include"CellTimestamp.hpp"

//#define MPRINT
#ifdef MPRINT
#include<stdio.h>
#define xPrintf(...) printf(__VA_ARGS__)
#else
#define xPrintf(...)
#endif MPRINT

#define MAX_MEMORY_SIZE 256

class MemoryAlloc;
//内存块 最小单元
class MemoryBlock
{
public:
	//所属最大内存
	MemoryAlloc* pAlloc;
	//下一块位置
	MemoryBlock* pNext;
	//内存块编号
	int nID;
	//引用次数
	int nRef;
	//内存大小
	time_t nSize;
	//是否在内存池中
	bool bPool;
private:
	//预留
	char c1;
	char c2;
	char c3;
};

//内存池
class MemoryAlloc
{
public:
	MemoryAlloc() {
		_pBuf = nullptr;
		_pHeader = nullptr;
		_nSize = 0;
		_nBlockNums = 0;
		_nBlockUseNums = 0;
		xPrintf("MemoryAlloc");
	}
	~MemoryAlloc() {
		if (_pBuf) {
			xPrintf("~MemoryAlloc:%llx\n", _pBuf);
			free(_pBuf);
		}
	}
	//申请内存
	void *allocMem(const size_t nSize) {
		std::lock_guard<std::mutex> lg(_mutex);
		if (!_pBuf) {
			initMemory();
		}
		MemoryBlock *pReturn = nullptr;
		if (nullptr == _pHeader) {
			//没有可用块,超出内存池
			pReturn = (MemoryBlock *)malloc(nSize+sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->nSize = nSize;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			printf("overpool allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, (int)nSize);
		}
		else {
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
			pReturn->nSize = nSize;
			_nBlockUseNums++;
		}
		xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, (int)nSize);
		return ((char *)pReturn +sizeof(MemoryBlock));
	}
	//释放内存
	void freeMem(void *pMem) {
		MemoryBlock *pBlock = (MemoryBlock *)((char *)pMem - sizeof(MemoryBlock));
		assert(1 == pBlock->nRef);
		if (pBlock->bPool) {
			std::lock_guard<std::mutex> lg(_mutex);
			if (--pBlock->nRef) {
				return;
			}
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
			_nBlockUseNums--;
		}
		else {
			if (--pBlock->nRef) {
				return;
			}
			free(pBlock);
		}
	}
	//初始化
	void initMemory() {
		assert(nullptr == _pBuf);
		if (_pBuf)return;
		size_t realSize = _nSize + sizeof(MemoryBlock);
		size_t size = realSize*_nBlockNums;
		_pBuf = (char *)malloc(size);
		xPrintf("initMemory:%llx,_nSzie=%d,_nBlockSzie=%d size=%d\n",_pBuf, (int)_nSize, (int)_nBlockNums,(int)size);
		//
		_pHeader = (MemoryBlock *)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pAlloc = this;
		_pHeader->pNext = nullptr;
		MemoryBlock *pLast=_pHeader;
		for (size_t i = 1; i < _nBlockNums; i++)
		{
			MemoryBlock *pTemp = (MemoryBlock *)(_pBuf+i*realSize);
			pTemp->bPool = true;
			pTemp->nID = (int)i;
			pTemp->nRef = 0;
			pTemp->pAlloc = this;
			pTemp->pNext = nullptr;
			pLast->pNext = pTemp;
			pLast = pTemp;
		}
		//MemoryBlock *pTemp = _pHeader;
		//for (size_t i = 0; i < 10; i++)
		//{
		//	printf("%d-", pTemp->nID);
		//	pTemp = pTemp->pNext;
		//}
		//printf("\n");
	}
	int getSize() { return (int)_nSize; }
	int getBlockNums() { return (int)_nBlockNums; }
	int getBlockUseNums() { return (int)_nBlockUseNums; }
protected:
	//内存池指针
	char *_pBuf;
	//头部位置
	MemoryBlock *_pHeader;
	//内存单元大小
	size_t _nSize;
	//内存单元数量
	size_t _nBlockNums;
	std::atomic_int _nBlockUseNums;
	std::mutex _mutex;
};

template<size_t nSize,size_t nBlockNums>
class MemoryAlloctor : public MemoryAlloc
{
public:
	MemoryAlloctor() {
		const size_t n = sizeof(void *);
		_nSize = (nSize / n)*n + (nSize%n ? n : 0);
		_nBlockNums = nBlockNums;
	}
	~MemoryAlloctor() {

	}
};

//内存管理工具
class MemoryMgr
{
private:
	MemoryMgr() {
		init(0, 64, &_mem64);
		init(65, 128, &_mem128);
 		init(129, 256, &_mem256);
		//init(257, 512, &_mem512);
		//init(513, 1024, &_mem1024);
		_nOtherUseNums = 0;
	}
	~MemoryMgr() {

	}
public:
	static MemoryMgr& getInstance() {
		static MemoryMgr mgr;
		return mgr;
	}
	//申请内存
	void *allocMem(const size_t nSize) {
		if (nSize <= MAX_MEMORY_SIZE) {
			return _szAlloc[nSize]->allocMem(nSize);
		}
		else {
			//_nOtherUseNums++;
			MemoryBlock *pRet = (MemoryBlock *)malloc(nSize+sizeof(MemoryBlock));
			pRet->bPool = false;
			pRet->nID = -1;
			pRet->nRef = 1;
			pRet->pAlloc = nullptr;
			pRet->pNext = nullptr;
			pRet->nSize = nSize;
			xPrintf("allocMem: %llx, id=%d, size=%d\n", pRet, pRet->nID, (int)nSize);
			return ((char *)pRet + sizeof(MemoryBlock));
		}
	}
	//释放内存
	void freeMem(void *pMem) {
		MemoryBlock *pBlock = (MemoryBlock*)((char *)pMem-sizeof(MemoryBlock));
		xPrintf("freeMem: %llx, id=%d size=%d\n", pBlock, pBlock->nID, (int)pBlock->nSize);
		if (pBlock->bPool) {
			pBlock->pAlloc->freeMem(pMem);
		}
		else {
			if (--pBlock->nRef == 0) {
				//_nOtherUseNums--;
				free(pBlock);
			}
		}
//		auto t1 = _tTime.getElapsedSecond();
//		if (t1 >= 1.0) {
//			printf("current alloc<%d> %d %d\n", pBlock->pAlloc->getSize(),pBlock->pAlloc->getBlockUseNums(), getOtherUseNums());
//			_tTime.update();
//		}
	}
	//增加内存引用计数
	void addRef(void* pMem) {
		MemoryBlock *pBlock = (MemoryBlock*)((char *)pMem - sizeof(MemoryBlock));
		++pBlock->nRef;
	}
	int getOtherUseNums() { return (int)_nOtherUseNums; }
private:
	void init(int nBegin, int nEnd, MemoryAlloc* pMemA) {
		for (size_t n = nBegin; n <= nEnd; n++) {
			_szAlloc[n] = pMemA;
		}
	}
	MemoryAlloctor<64, 100000> _mem64;
	MemoryAlloctor<128, 100000> _mem128;
	MemoryAlloctor<256, 10000> _mem256;
	//MemoryAlloctor<512, 10000> _mem512;
	//MemoryAlloctor<1024, 10000> _mem1024;
	MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE + 1];
	CellTimestamp _tTime;
	std::atomic_int _nOtherUseNums;
};
