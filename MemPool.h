#ifndef __MEM_POOL_H__
#define __MEM_POOL_H__

#include <stdlib.h>
#include <mutex> 
#include <list>

typedef void* (*func_allocator)(size_t);
typedef void (*func_deallocator)(void*);
using namespace std;
#define NEED_MALLOC_BIG_SIZE (4000 << 20)
#define PREALLOC_SIZE 128
#ifdef MAINIMG_NSIZE_13M
#define USED_BIG_SIZE (9 << 20)  // 9M
#define BLOCK_BIG_SIZE(x) (x*3/5 + (1 << 20))   // 13M picture > 85M
#else
#define USED_BIG_SIZE (4000 << 20)  // 
#define BLOCK_BIG_SIZE(x) (x*3/5 + (4 << 20))  
#endif
class MemPool
{
public:
	static bool Create(size_t sizeInBytes);
	static bool Release();
	static void* Allocate(size_t sizeInBytes);
	static void PreAllocate(size_t sizeInBytes, unsigned int isbigalloc);	
	static void Deallocate(void* ptr);
	static void PrintMemPool();
    static void SumMemPool();
    static void SetAllocator(func_allocator all = NULL, func_deallocator deall = NULL);

public:
	struct MemBlock
	{
		void* ptr;
		size_t nBlockSize;
		bool  bVailable;
	};

private:
	static std::list<MemBlock>::iterator FindNextBlock(std::list<MemBlock>::iterator start, size_t sizeInBytes);
	static bool Extend_b(size_t sizeInBytes);
	static bool ExtendMemory(size_t sizeInBytes);

private:
	class AutoLock
	{
	public:
		AutoLock(mutex& _mutex):_lock(_mutex)
		{
			_lock.lock();
		}
		~AutoLock()
		{
			_lock.unlock();
		}
	private:
		mutex& _lock;
	};

	static func_allocator  allocator;
	static func_deallocator deallocator;
	static mutex _lock;
	static int       refCount;

	static std::list<MemBlock> allocList;
	static std::list<MemBlock> memBlkList;
	static std::list<MemBlock>::iterator currentBlock;
	static std::list<MemBlock> allocList_b;
	static std::list<MemBlock> memBlkList_b;	
	static std::list<MemBlock>::iterator currentBlock_b;	
};

#define MEMPOLL 1
template <class T>
T *MergerBufnew(int SizeInBytes)
{
	T *m_pImgData = NULL;
#ifdef MEMPOLL
	size_t len = SizeInBytes * sizeof(T) + 128;
	if (m_pImgData != NULL)
	{
		MemPool::Deallocate(m_pImgData);
	}
	m_pImgData = (T*)MemPool::Allocate(len);
	if (m_pImgData == NULL) return NULL;
	return m_pImgData;
#else
	if (m_pImgData != NULL)
	{
		delete[] m_pImgData;
		m_pImgData = NULL;
	}
	m_pImgData = new T[SizeInBytes];
	return m_pImgData;
#endif
}
template <class T>
void MergerBufdelete(T *m_pImgData)
{
#ifdef MEMPOLL
	if (m_pImgData != NULL)
		MemPool::Deallocate(m_pImgData);
	m_pImgData = NULL;
#else
	if (m_pImgData != NULL)delete[] m_pImgData;
#endif
}
#endif // !__MEM_POOL_H__