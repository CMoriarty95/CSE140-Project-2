#include "tips.h"

/* The following two functions are defined in util.c */

/* finds the highest 1 bit, and returns its position, else 0xFFFFFFFF */
unsigned int uint_log2(word w);

/* return random int from 0..x-1 */
int randomint(int x);

/*
  This function allows the lfu information to be displayed

	assoc_index - the cache unit that contains the block to be modified
	block_index - the index of the block to be modified

  returns a string representation of the lfu information
 */
char* lfu_to_string(int assoc_index, int block_index)
{
	/* Buffer to print lfu information -- increase size as needed. */
	static char buffer[9];
	sprintf(buffer, "%u", cache[assoc_index].block[block_index].accessCount);

	return buffer;
}

/*
  This function allows the lru information to be displayed

	assoc_index - the cache unit that contains the block to be modified
	block_index - the index of the block to be modified

  returns a string representation of the lru information
 */
char* lru_to_string(int assoc_index, int block_index)
{
	/* Buffer to print lru information -- increase size as needed. */
	static char buffer[9];
	sprintf(buffer, "%u", cache[assoc_index].block[block_index].lru.value);

	return buffer;
}

/*
  This function initializes the lfu information

	assoc_index - the cache unit that contains the block to be modified
	block_number - the index of the block to be modified

*/
void init_lfu(int assoc_index, int block_index)
{
	cache[assoc_index].block[block_index].accessCount = 0;
}

/*
  This function initializes the lru information

	assoc_index - the cache unit that contains the block to be modified
	block_number - the index of the block to be modified

*/
void init_lru(int assoc_index, int block_index)
{
	cache[assoc_index].block[block_index].lru.value = 0;
}

/*
  This is the primary function you are filling out,
  You are free to add helper functions if you need them

  @param addr 32-bit byte address
  @param data a pointer to a SINGLE word (32-bits of data)
  @param we   if we == READ, then data used to return
			  information back to CPU

			  if we == WRITE, then data used to
			  update Cache/DRAM
*/
void accessMemory(address addr, word* data, WriteEnable we)
{

	/* handle the case of no cache at all - leave this in */
	if (assoc == 0) {
		accessDRAM(addr, (byte*)data, WORD_SIZE, we);
		return;
	}

	/* Declare variables here */
	//Declare bit sizes for each category
	int offsetBits = uint_log2(block_size);

	int indexBits;
	if (assoc == set_count)
		indexBits = 0;
	else
		indexBits = uint_log2(set_count);
	//int indexBits = uint_log2(set_count/assoc); may need to re-examine this part

	int tagBits = 32 - offsetBits - indexBits;


	//Parse through address given to find values for offset,
	//index (if applicable), and tag
	unsigned int offset = addr << (tagBits + indexBits);
	offset = offset >> (tagBits + indexBits);

	unsigned int index;
	if (assoc == set_count)
		index = 0;
	else
	{
		index = addr << (tagBits);
		index = index >> (tagBits + offsetBits);
	}

	unsigned int tag = addr >> (offsetBits + indexBits);

	int block = 0;

	//Checking for write mode
	bool foundData = false;

	for (int i = 0; i < assoc; i++)
	{
		if (tag == cache[index].block[i].tag && cache[index].block[i].valid == 1)
		{
			printf("Block found at Index %d, Block %d", index, i);
			block = i;
			foundData = true;
			break;
		}
	}

	if (we == READ)
	{
		//go to the proper cache location and check if data is in cache 
		
		if (foundData)
		{

			fetchBlock(*data, index, block, offset);

		}

		else
		{

			getDRAMBlock(addr, *data, we);

			if (policy == LRU) 
			{
				int maxValue = 0, LRUBlock = 0;

				for (int i = 0; i < assoc; i++)
				{

					if (cache[index].block[i].lru.value > maxValue)
					{

						maxValue = cache[index].block[i].lru.value;
						LRUBlock = i;

					}
					
				}

				if (cache[index].block[LRUBlock].dirty == DIRTY)
				{

					replaceDirty(LRUBlock, index, indexBits, offsetBits);

				}
				
				memcpy((void *)cache[index].block[LRUBlock].data, (void *)data, block_size);
				
				//reset block data
				cache[index].block[LRUBlock].valid = VALID;
				cache[index].block[LRUBlock].dirty = VIRGIN;
				cache[index].block[LRUBlock].tag = tag;
				cache[index].block[LRUBlock].accessCount = 0;
				cache[index].block[LRUBlock].lru.value = 0;


			}

			else
			{

				int randomBlock = randomint(assoc);

				if (cache[index].block[randomBlock].dirty == DIRTY)
				{

					replaceDirty(randomBlock, index, indexBits, offsetBits);

				}

				memcpy((void *)cache[index].block[randomBlock].data, (void *)data, block_size);

				//reset block data
				cache[index].block[randomBlock].valid = VALID;
				cache[index].block[randomBlock].dirty = VIRGIN;
				cache[index].block[randomBlock].tag = tag;
				cache[index].block[randomBlock].accessCount = 0;
				cache[index].block[randomBlock].lru.value = 0;
			}

			fetchBlock(*data, index, block, offset);

		}

	}

	else
	{
		//do some other shit
		if (MemorySyncPolicy == WRITE_BACK)
		{
			//do this shit
		}

		else
		{
			//do that shit
		}
	}

	/*
	You need to read/write between memory (via the accessDRAM() function) and
	the cache (via the cache[] global structure defined in tips.h)

	Remember to read tips.h for all the global variables that tell you the
	cache parameters

	The same code should handle random, LFU, and LRU policies. Test the policy
	variable (see tips.h) to decide which policy to execute. The LRU policy
	should be written such that no two blocks (when their valid bit is VALID)
	will ever be a candidate for replacement. In the case of a tie in the
	least number of accesses for LFU, you use the LRU information to determine
	which block to replace.

	Your cache should be able to support write-through mode (any writes to
	the cache get immediately copied to main memory also) and write-back mode
	(and writes to the cache only gets copied to main memory when the block
	is kicked out of the cache.

	Also, cache should do allocate-on-write. This means, a write operation
	will bring in an entire block if the block is not already in the cache.

	To properly work with the GUI, the code needs to tell the GUI code
	when to redraw and when to flash things. Descriptions of the animation
	functions can be found in tips.h
	*/

	/* Start adding code here */


	/* This call to accessDRAM occurs when you modify any of the
	   cache parameters. It is provided as a stop gap solution.
	   At some point, ONCE YOU HAVE MORE OF YOUR CACHELOGIC IN PLACE,
	   THIS LINE SHOULD BE REMOVED.
	*/
	accessDRAM(addr, (byte*)data, WORD_SIZE, we);
}

void fetchBlock(word* data, int index, int block, int offset)
{

	memcpy((void*)data, (void*)cache[index].block[block].data[offset], 4);

	for (int i = 0; i < assoc; i++)
	{

		cache[index].block[i].lru.value++;

	}

	cache[index].block[block].lru.value = 0;

}

void replaceDirty(int dirtyBlock, int index, int indexBits, int offsetBits)
{

	address dirty = 0;
	unsigned int dirtyIndex = 0;
	unsigned int dirtyTag = 0;

	dirtyIndex = index << offsetBits;
	dirtyTag = cache[index].block[dirtyBlock].tag;

	dirtyTag = dirtyTag << (offsetBits + indexBits);

	dirty = dirtyTag + dirtyIndex;

	getDRAMBlock(dirty, (byte *)cache[index].block[dirtyBlock].data, block_size, WRITE);

}

void getDRAMBlock(address addr, word* data, WriteEnable we)
{

	for (int i = 0; i < block_size; i += 4)
	{

		accessDRAM(addr + i, (byte*)data + i, WORD_SIZE, we);	//if something goes wrong, maybe check the typecast

	}

}
