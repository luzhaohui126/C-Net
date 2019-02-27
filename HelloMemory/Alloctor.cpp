
#include"Alloctor.h"
#include"MemoryMgr.hpp"

void *operator new(const size_t size) {
	return MemoryMgr::getInstance().allocMem(size);
}
void operator delete(void *p) {
	MemoryMgr::getInstance().freeMem(p);
}
void *operator new[](const size_t size) {
	return MemoryMgr::getInstance().allocMem(size);
}
void operator delete[](void *p) {
	MemoryMgr::getInstance().freeMem(p);
}
//void *mem_alloc(const size_t size) {
//	return MemoryMgr::getInstance().allocMem(size);
//}
//void mem_free(void *p) {
//	MemoryMgr::getInstance().freeMem(p);
//}
