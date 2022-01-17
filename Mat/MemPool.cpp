#include "MemPool.h"
#include <stdio.h>
#include "SystemLog.h"

static void* default_allocator(size_t sizeInBytes)
{
	return malloc(sizeInBytes);
}

static void default_deallocator(void* ptr)
{
	return free(ptr);
}

std::list<MemPool::MemBlock> MemPool::allocList;
std::list<MemPool::MemBlock> MemPool::memBlkList;
std::list<MemPool::MemBlock>::iterator MemPool::currentBlock;

std::list<MemPool::MemBlock> MemPool::allocList_b;
std::list<MemPool::MemBlock> MemPool::memBlkList_b;
std::list<MemPool::MemBlock>::iterator MemPool::currentBlock_b;
func_allocator  MemPool::allocator = default_allocator;
func_deallocator MemPool::deallocator = default_deallocator;
mutex MemPool::_lock;
int MemPool::refCount = 0;
unsigned int nMemPoolsum = 0;
unsigned int nMemPoolsum_b = 0;
void MemPool::PreAllocate(size_t sizeInBytes, unsigned int isbigalloc)
{
	if (isbigalloc)
	{
		std::list<MemBlock>::iterator it = FindNextBlock(currentBlock_b, sizeInBytes);
		MemBlock block = *it;
		it->bVailable = false;
		it->nBlockSize = sizeInBytes;
		void* ptr = it->ptr;
		if (block.nBlockSize > sizeInBytes)
		{
			MemBlock newBlock;
			newBlock.bVailable = true;
			newBlock.nBlockSize = block.nBlockSize - sizeInBytes;
			newBlock.ptr = (void*)((char*)block.ptr + sizeInBytes);
			it++;
			memBlkList_b.insert(it, newBlock);
		}
		currentBlock_b = it;
		nMemPoolsum_b += sizeInBytes;
	}
	else
	{
		std::list<MemBlock>::iterator it = FindNextBlock(currentBlock, sizeInBytes);
		MemBlock block = *it;
		it->bVailable = false;
		it->nBlockSize = sizeInBytes;
		void* ptr = it->ptr;
		if (block.nBlockSize > sizeInBytes)
		{
			MemBlock newBlock;
			newBlock.bVailable = true;
			newBlock.nBlockSize = block.nBlockSize - sizeInBytes;
			newBlock.ptr = (void*)((char*)block.ptr + sizeInBytes);
			it++;
			memBlkList.insert(it, newBlock);
		}
		currentBlock = it;
		nMemPoolsum += sizeInBytes;
	}
}

bool MemPool::Create(size_t sizeInBytes)
{
	AutoLock mutex(_lock);
	if (refCount <= 0)
	{
		refCount = 0;
		allocList.clear();
		memBlkList.clear();
		if (sizeInBytes > NEED_MALLOC_BIG_SIZE)
		{
			allocList_b.clear();
			memBlkList_b.clear();
		}
	}
	if (sizeInBytes > NEED_MALLOC_BIG_SIZE)
	{
		Extend_b(BLOCK_BIG_SIZE(sizeInBytes));
		sizeInBytes -= BLOCK_BIG_SIZE(sizeInBytes);
	}
	ExtendMemory(sizeInBytes);
	refCount++;

	if (refCount == 1)
	{
		currentBlock = memBlkList.begin();
		currentBlock_b = memBlkList_b.begin();
	}

	return true;
}

bool MemPool::Release()
{
	AutoLock mutex(_lock);
	refCount--;
	if (refCount > 0) return true;

	for (std::list<MemBlock>::iterator it = allocList.begin(); it != allocList.end(); it++)
	{
		if (it->ptr)
		{
			deallocator((uint8_t *)it->ptr - PREALLOC_SIZE);
			it->ptr = NULL;
			it->nBlockSize = 0;
		}
	}
	for (std::list<MemBlock>::iterator it = allocList_b.begin(); it != allocList_b.end(); it++)
	{
		if (it->ptr)
		{
			deallocator((uint8_t *)it->ptr - PREALLOC_SIZE);
			it->ptr = NULL;
			it->nBlockSize = 0;
		}
	}
	allocList.clear();
	memBlkList.clear();
	allocList_b.clear();
	memBlkList_b.clear();
	refCount = 0;
	currentBlock = memBlkList.begin();
	currentBlock = memBlkList_b.begin();

	return true;
}

void* MemPool::Allocate(size_t sizeInBytes)
{
	AutoLock mutex(_lock);
	sizeInBytes = (sizeInBytes + 63) / 64 * 64;

	if (sizeInBytes > USED_BIG_SIZE)
	{
		std::list<MemBlock>::iterator it = FindNextBlock(currentBlock_b, sizeInBytes);
		if (it == memBlkList_b.end())
		{
			int nExtendSize = 2 * sizeInBytes;
			if (nExtendSize < (40 << 20)) nExtendSize = (40 << 20);
			if (!Extend_b(nExtendSize)) return NULL;
			currentBlock_b = memBlkList_b.end();
			currentBlock_b--;
			it = FindNextBlock(currentBlock_b, sizeInBytes);
#if 0
			MemBlock block = *it;
			it->bVailable = false;
			it->nBlockSize = PREALLOC_SIZE;
			void* ptr = it->ptr;
			if (block.nBlockSize > PREALLOC_SIZE)
			{
				MemBlock newBlock;
				newBlock.bVailable = true;
				newBlock.nBlockSize = block.nBlockSize - PREALLOC_SIZE;
				newBlock.ptr = (void*)((char*)block.ptr + PREALLOC_SIZE);
				it++;
				memBlkList_b.insert(it, newBlock);
			}
			currentBlock_b = it;
			nMemPoolsum_b += PREALLOC_SIZE;
#endif			
			it = FindNextBlock(currentBlock_b, sizeInBytes);
			LOGI("USED_BIG_SIZE Alllocate MemPool ExtendImage !!!!\n");
		}
		if (it != memBlkList_b.end())
		{
			MemBlock block = *it;
			it->bVailable = false;
			it->nBlockSize = sizeInBytes;
			void* ptr = it->ptr;
			if (block.nBlockSize > sizeInBytes)
			{
				MemBlock newBlock;
				newBlock.bVailable = true;
				newBlock.nBlockSize = block.nBlockSize - sizeInBytes;
				newBlock.ptr = (void*)((char*)block.ptr + sizeInBytes);
				it++;
				memBlkList_b.insert(it, newBlock);
			}
			currentBlock_b = it;
			nMemPoolsum_b += sizeInBytes;
			return ptr;
		}
	}
	else
	{
		std::list<MemBlock>::iterator it = FindNextBlock(currentBlock, sizeInBytes);

		if (it == memBlkList.end())
		{
			//Memory pool ExtendImage.
			int nExtendSize = 2 * sizeInBytes;
			if (nExtendSize < (20 << 20)) nExtendSize = (20 << 20);
			if (!ExtendMemory(nExtendSize)) return NULL;
			currentBlock = memBlkList.end();
			currentBlock--;
			it = FindNextBlock(currentBlock, sizeInBytes);
#if 0
			MemBlock block = *it;
			it->bVailable = false;
			it->nBlockSize = PREALLOC_SIZE;
			void* ptr = it->ptr;
			if (block.nBlockSize > PREALLOC_SIZE)
			{
				MemBlock newBlock;
				newBlock.bVailable = true;
				newBlock.nBlockSize = block.nBlockSize - PREALLOC_SIZE;
				newBlock.ptr = (void*)((char*)block.ptr + PREALLOC_SIZE);
				it++;
				memBlkList.insert(it, newBlock);
			}
			currentBlock = it;
			nMemPoolsum += PREALLOC_SIZE;
			it = FindNextBlock(currentBlock, sizeInBytes);
#endif			
			LOGI("Alllocate MemPool ExtendImage !!!!\n");
		}

		if (it != memBlkList.end())
		{
			MemBlock block = *it;

			it->bVailable = false;
			it->nBlockSize = sizeInBytes;
			void* ptr = it->ptr;
			if (block.nBlockSize > sizeInBytes)
			{
				MemBlock newBlock;
				newBlock.bVailable = true;
				newBlock.nBlockSize = block.nBlockSize - sizeInBytes;
				newBlock.ptr = (void*)((char*)block.ptr + sizeInBytes);
				it++;
				memBlkList.insert(it, newBlock);
			}
			currentBlock = it;
			nMemPoolsum += sizeInBytes;
			//printf("################# alloc #################################\n");
			//PrintMemPool();

			return ptr;
		}
	}

	return NULL;
}

// 1. find memory
// 2. 合并后面的空闲memory
// 3. 合并前面的空闲memory
// 4. 如果当前块是独立块，把这个blocklist释放掉
void MemPool::Deallocate(void* ptr)
{
	AutoLock mutex(_lock);
	int is_bigblock = 0;
	std::list<MemBlock>::iterator block_b;

	if (ptr == NULL) return;
	if (!memBlkList_b.empty())
	{
		std::list<MemBlock>::iterator start_b = memBlkList_b.begin();
		block_b = start_b;
		do {
			if ((!block_b->bVailable) && (block_b->ptr == ptr))
			{
				block_b->bVailable = true;
				is_bigblock = 1;
				break;
			}
			block_b++;
			if (block_b == memBlkList_b.end()) break;
		} while (block_b != start_b);
	}
	if (is_bigblock)
	{
		if (block_b == memBlkList_b.end() || block_b->ptr != ptr || block_b->bVailable == false) return;
		std::list<MemBlock>::iterator cur = block_b;
		block_b++;
		currentBlock_b = cur;
		nMemPoolsum_b -= (unsigned int)currentBlock_b->nBlockSize;
		if (block_b != memBlkList_b.end() && block_b->bVailable == true && (char*)block_b->ptr == (char*)cur->ptr + cur->nBlockSize)
		{
			cur->nBlockSize += block_b->nBlockSize;
			memBlkList_b.erase(block_b);
		}
		block_b = cur;
		if (block_b != memBlkList_b.begin()) cur--;
		if (block_b != memBlkList_b.begin() && cur->bVailable == true && (char*)block_b->ptr == (char*)cur->ptr + cur->nBlockSize)
		{
			cur->nBlockSize += block_b->nBlockSize;
			memBlkList_b.erase(block_b);
			currentBlock_b = cur;
		}
		std::list<MemBlock>::iterator it = allocList_b.begin();
		while (it != allocList_b.end())
		{
			std::list<MemBlock>::iterator it1 = it++;
			if (it1->ptr == currentBlock_b->ptr && it1->nBlockSize == currentBlock_b->nBlockSize && currentBlock_b->bVailable == true)
			{
				std::list<MemBlock>::iterator del = currentBlock_b;
				currentBlock_b++;
				if (currentBlock_b == memBlkList_b.end()) currentBlock_b = memBlkList_b.begin();
				memBlkList_b.erase(del);
				deallocator((uint8_t *)it1->ptr - PREALLOC_SIZE);
				allocList_b.erase(it1);
			}
		}
	}
	else
	{
		if (memBlkList.empty()) return;

		std::list<MemBlock>::iterator start = currentBlock;
		if (start == memBlkList.end()) start = memBlkList.begin();

		//Find and mark the memory
		std::list<MemBlock>::iterator block = start;
		do {
			if ((!block->bVailable) && (block->ptr == ptr))
			{
				block->bVailable = true;
				break;
			}
			block++;
			if (block == memBlkList.end()) block = memBlkList.begin();

		} while (block != start);

		if (block == memBlkList.end() || block->ptr != ptr || block->bVailable == false) return;

		//Memoy De-fragment
		std::list<MemBlock>::iterator cur = block;
		block++;
		currentBlock = cur;
		nMemPoolsum -= (unsigned int)currentBlock->nBlockSize;
		if (block != memBlkList.end() && block->bVailable == true && (char*)block->ptr == (char*)cur->ptr + cur->nBlockSize)
		{
			cur->nBlockSize += block->nBlockSize;
			memBlkList.erase(block);
		}

		block = cur;
		if (block != memBlkList.begin()) cur--;
		if (block != memBlkList.begin() && cur->bVailable == true && (char*)block->ptr == (char*)cur->ptr + cur->nBlockSize)
		{
			cur->nBlockSize += block->nBlockSize;
			memBlkList.erase(block);
			currentBlock = cur;
		}
#if 1
		//Auto release used memory.
		std::list<MemBlock>::iterator it = allocList.begin();
		while (it != allocList.end())
		{
			std::list<MemBlock>::iterator it1 = it++;
			if (it1->ptr == currentBlock->ptr && it1->nBlockSize == currentBlock->nBlockSize && currentBlock->bVailable == true)
			{
				std::list<MemBlock>::iterator del = currentBlock;
				currentBlock++;
				if (currentBlock == memBlkList.end()) currentBlock = memBlkList.begin();
				memBlkList.erase(del);

				deallocator((uint8_t *)it1->ptr - PREALLOC_SIZE);
				allocList.erase(it1);
			}
		}
#endif
	}
	//printf("----------------- dealloc -----------------------------\n");
   // PrintMemPool();
}

std::list<MemPool::MemBlock>::iterator MemPool::FindNextBlock(std::list<MemPool::MemBlock>::iterator start, size_t sizeInBytes)
{
	if (sizeInBytes > USED_BIG_SIZE)
	{
		if (memBlkList_b.empty()) return memBlkList_b.end();
		if (start == memBlkList_b.end()) start = memBlkList_b.begin();
		std::list<MemBlock>::iterator block = memBlkList_b.begin();
		do
		{
			if (block->bVailable && block->nBlockSize >= sizeInBytes) break;
			block++;
			if (block == memBlkList_b.end()) block = memBlkList_b.begin();
		} while (block != memBlkList_b.begin());
		if (false == block->bVailable || sizeInBytes > block->nBlockSize) block = memBlkList_b.end();
		return block;
	}
	else
	{
		if (memBlkList.empty()) { return memBlkList.end(); }

		if (start == memBlkList.end()) start = memBlkList.begin();
		std::list<MemBlock>::iterator block = memBlkList.begin();
		do
		{
			if (block->bVailable && block->nBlockSize >= sizeInBytes) break;
			block++;
			if (block == memBlkList.end()) block = memBlkList.begin();

		} while (block != memBlkList.begin());

		if (false == block->bVailable || sizeInBytes > block->nBlockSize) block = memBlkList.end();

		return block;
	}
}
void MemPool::SetAllocator(func_allocator all, func_deallocator deall)
{
	AutoLock mutex(_lock);
	if (all == NULL || deall == NULL)
	{
		all = default_allocator;
		deall = default_deallocator;
	}

	allocator = all;
	deallocator = deall;
}
void MemPool::PrintMemPool()
{
	std::list<MemBlock>::iterator block = memBlkList.begin();
	std::list<MemBlock>::iterator startblock = memBlkList.begin();
	printf("  No.   start  size  status\n");
	printf("---------------------------\n");
	int i = 0;
	int fragmentcount = 0;
	while (block != memBlkList.end())
	{
		LOGI("%4d  %8lx  %8x %d\n", i, (unsigned long)block->ptr, (unsigned int)block->nBlockSize, block->bVailable);
		block++;
		i++;
	}
	block--;
	LOGI(" ******WARNING******realsize  = %ld  %8lx %8lx \n", (unsigned long)block->ptr - (unsigned long)startblock->ptr, (unsigned long)block->ptr, (unsigned long)startblock->ptr);
	printf("\n");
#if 0
	std::list<MemBlock>::iterator block_b = memBlkList_b.begin();
	std::list<MemBlock>::iterator startblock_b = memBlkList_b.begin();
	printf("  No.   start  size  status\n");
	printf("---------------------------\n");
	while (block_b != memBlkList_b.end())
	{
		LOGI("%4d  %8lx  %8x %d\n", i, (unsigned long)block_b->ptr, (unsigned int)block_b->nBlockSize, block_b->bVailable);
		block_b++;
		i++;
	}
	block_b--;
	LOGI(" ******WARNING******big realsize  = %ld  %8lx %8lx \n", (unsigned long)block_b->ptr - (unsigned long)startblock_b->ptr, (unsigned long)block_b->ptr, (unsigned long)startblock_b->ptr);
	printf("\n");
#endif
}

void MemPool::SumMemPool()
{
	std::list<MemBlock>::iterator block = memBlkList.begin();
	printf("  No.   start  size  status\n");
	printf("---------------------------\n");
	int sum = 0;
	while (block != memBlkList.end())
	{
		char * ptr = (char *)block->ptr;
		for (int i = 0; i < block->nBlockSize - 4095; i += 4096)
		{
			sum += ptr[i];
			//           printf("%4d  %8lx  %8x %d\n", i, (unsigned long)block->ptr, (unsigned int)block->nBlockSize, block->bVailable);
		}
		block++;
	}
	printf("sum = %d\n", sum);
}
bool MemPool::ExtendMemory(size_t sizeInBytes)
{
	void* ptr = allocator(sizeInBytes);
	if (!ptr) return false;
	int nx = 800000;
	int ny = sizeInBytes / nx;
	#pragma omp parallel for schedule(dynamic,16) 
	for (int y=0;y<ny;y++)
	{
		uint8_t *pptr = (uint8_t *)ptr + y * nx;
		//int b = nx;
		//while (b--)
		//	*pptr++ = 0;
		for (int x=0;x<nx/16;x+=16)
		{
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;
			*pptr++ = 0;
		}
	}
	//memset(ptr, 0, sizeInBytes);
	MemBlock block;
	block.ptr = (uint8_t *)ptr + PREALLOC_SIZE;
	block.nBlockSize = sizeInBytes;
	block.bVailable = true;
	allocList.push_back(block);
	memBlkList.push_back(block);
	return true;
}
bool MemPool::Extend_b(size_t sizeInBytes)
{
	void* ptr = allocator(sizeInBytes);
	if (!ptr) return false;
	MemBlock block;
	block.ptr = (uint8_t *)ptr + PREALLOC_SIZE;
	block.nBlockSize = sizeInBytes;
	block.bVailable = true;
	allocList_b.push_back(block);
	memBlkList_b.push_back(block);
	return true;
}
