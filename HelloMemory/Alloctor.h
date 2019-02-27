#pragma once

void *operator new(const size_t size);
void operator delete(void *p);
void *operator new[](const size_t size);
void operator delete[](void *p);
//void *mem_alloc(const size_t size);
//void mem_free(void *p);

