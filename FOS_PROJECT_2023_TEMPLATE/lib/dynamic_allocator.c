/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"
struct MemBlock_LIST DynamicAllocator;

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
uint32 get_block_size(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->size ;
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
int8 is_free_block(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->is_free ;
}

//===========================================
// 3) ALLOCATE BLOCK BASED ON GIVEN STRATEGY:
//===========================================
void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockMetaData* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", blk->size, blk->is_free) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
bool is_initialized=0;
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{

	//=========================================
	//DON'T CHANGE THESE LINES=================
	if (initSizeOfAllocatedSpace == 0)
		return ;
	is_initialized=1;
	//=========================================
	//=========================================

	//TODO: [PROJECT'23.MS1 - #5] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator()
	//panic("initialize_dynamic_allocator is not implemented yet");

	LIST_INIT(&DynamicAllocator);

	struct BlockMetaData* block = (struct BlockMetaData*) daStart;

	block->size = initSizeOfAllocatedSpace;
	block->is_free = 1;

	LIST_INSERT_HEAD( &DynamicAllocator, block);

}

//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
//=========================================
void *alloc_block_FF(uint32 size)
{
    // TODO: [PROJECT'23.MS1 - #6] [3] DYNAMIC ALLOCATOR - alloc_block_FF()

    if (!is_initialized)
    {
    uint32 required_size = size + sizeOfMetaData();
    uint32 da_start = (uint32)sbrk(required_size);
    //get new break since it's page aligned! thus, the size can be more than the required one
    uint32 da_break = (uint32)sbrk(0);
    initialize_dynamic_allocator(da_start, da_break - da_start);
    }
    if (size == 0)
            return NULL;
    struct BlockMetaData* blk_pointer;
    LIST_FOREACH(blk_pointer, &DynamicAllocator)
    {

        if(blk_pointer->is_free == 1 && blk_pointer->size > (size + sizeOfMetaData()))
        {

        	blk_pointer->is_free = 0;
        	if(blk_pointer->size > (size + sizeOfMetaData()+ sizeOfMetaData()))
        	{
        		struct BlockMetaData* new_block;
        		uint32 ad =(uint32) blk_pointer + size + sizeOfMetaData();
        		new_block = (struct BlockMetaData *)ad;
        		new_block->size=blk_pointer->size - size - sizeOfMetaData();
        		blk_pointer->size=size + sizeOfMetaData();
        		new_block->is_free=1;
        		LIST_INSERT_AFTER(&DynamicAllocator, blk_pointer, new_block);

        	}
        	return blk_pointer+1;
        }
        else if(blk_pointer->is_free == 1 && blk_pointer->size == (size + sizeOfMetaData()))
        {
            blk_pointer->is_free = 0;
            return (blk_pointer + 1);
        }
    }


if(DynamicAllocator.lh_last->is_free==1)
{
	uint32 req_size =(size+sizeOfMetaData())-DynamicAllocator.lh_last->size;
	uint32 t = ROUNDUP(req_size+DynamicAllocator.lh_last->size,PAGE_SIZE);
	if(sbrk(req_size) != (void*)-1)
	{
		DynamicAllocator.lh_last->is_free=0;
		if(t==size+sizeOfMetaData())
		{
			DynamicAllocator.lh_last->size=t;
			return DynamicAllocator.lh_last+1;
		}
		else
		{
			if(t>size+sizeOfMetaData())
			{
				struct BlockMetaData* temp2 = DynamicAllocator.lh_last ;
				uint32 size_temp = t -size-sizeOfMetaData() ;
				if(size_temp>=sizeOfMetaData())
				{
					struct BlockMetaData* temp ;
					uint32 ad = (uint32)DynamicAllocator.lh_last + size+sizeOfMetaData();
					temp = (struct BlockMetaData *)ad;
					temp->size = size_temp;
					temp->is_free=1;
					DynamicAllocator.lh_last->size=size+sizeOfMetaData()+sizeOfMetaData();
					LIST_INSERT_AFTER(&DynamicAllocator, DynamicAllocator.lh_last, temp);
				}
				else
				{
					temp2->size=t;
				}

				return temp2+1;
			}
			else
			{
				DynamicAllocator.lh_last->size=t;
				return DynamicAllocator.lh_last+1;
			}
		}

	}
	else
	{
		return NULL;
	}
}
else
{

	uint32 return_size =ROUNDUP(size+ sizeOfMetaData(),PAGE_SIZE);
	uint32 return_size2 =size+ sizeOfMetaData();
	uint32 da_break = (uint32)DynamicAllocator.lh_last + DynamicAllocator.lh_last->size;
	if (sbrk(return_size2) != (void*)-1)
	{

		if(return_size == size + sizeOfMetaData())
		{

			struct BlockMetaData* new_block = (struct BlockMetaData*)da_break;
			new_block->size = return_size;
			new_block->is_free = 0;

			LIST_INSERT_AFTER(&DynamicAllocator, DynamicAllocator.lh_last, new_block);
			return (new_block + 1);
		}
		else if(return_size > size + sizeOfMetaData()) //s_return > size + META
		{

			struct BlockMetaData* new_block;
			new_block = (struct BlockMetaData*)da_break;
			new_block->is_free = 0;
			uint32 a = (uint32)new_block + size + sizeOfMetaData();
			LIST_INSERT_AFTER(&DynamicAllocator, DynamicAllocator.lh_last, new_block);
			if(return_size >= (size + sizeOfMetaData()+ sizeOfMetaData()))
			{
				struct BlockMetaData* new_block2;
				uint32 ad =(uint32) new_block + size + sizeOfMetaData();
				new_block2 = (struct BlockMetaData *)ad;
				new_block2->size=return_size - size - sizeOfMetaData();
				new_block->size=size + sizeOfMetaData();
				new_block2->is_free=1;
				LIST_INSERT_AFTER(&DynamicAllocator, new_block, new_block2);
				return new_block+1;
			}
			else
			{
				new_block->size=return_size;
				return new_block+1;
			}
		}else
		{
			return NULL;
		}
	    }
	else
	{
		return NULL;
	}
}





}
//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT'23.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF()
	panic("alloc_block_BF is not implemented yet");
	return NULL;
}

//=========================================
// [6] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}

//===================================================
// [8] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{
	//cprintf("====================freeblock================\n");
	//TODO: [PROJECT'23.MS1 - #7] [3] DYNAMIC ALLOCATOR - free_block()
	struct BlockMetaData* free_blk = (struct BlockMetaData*)(va-(sizeOfMetaData()));

	if(va == NULL){
		// must do nothing
		return;
	}

		// must free it
		//no merge

	int next=0,prev=0;
	if(LIST_NEXT(free_blk) != NULL){// LAW HOWA MESH BE NULL
				next = 1;
			}
			if(LIST_PREV(free_blk)!=NULL){
				prev=1;
			}




		struct BlockMetaData* block_next = LIST_NEXT(free_blk);
		struct BlockMetaData* block_prev = LIST_PREV(free_blk);

		if( ((block_next == NULL ) && (block_prev == NULL))  || //ONLY ONE ELEMENT WA5EED EL HEAP KOLHA
			((block_next == NULL) && ((prev == 1) && ( block_prev->is_free != 1)) ) || //LAW AKHER ELEMENT FE LIST
			((block_prev == NULL) && ((next == 1) && ( block_next->is_free != 1)) ) ||//LAW AWEL ELEMENT FE LIST
			(( (next == 1) && ( block_next->is_free != 1) )&&( (prev == 1) && ( block_prev->is_free != 1) ))

		  )// MAFEESH MERGING
						{

				             free_blk->is_free = 1;
							    return ;
						}




		//merge
		// if block is the last merge with previous
		else if(block_next == NULL && (prev == 1 && block_prev->is_free == 1)){
			uint32 s = free_blk->size ;

			block_prev->size += s;
			block_prev->is_free = 1;

			free_blk->size = 0;
			free_blk->is_free = 0;
			LIST_REMOVE(&DynamicAllocator,free_blk);

			return;
		}
		// prev is free only
		else if((prev == 1 && block_prev->is_free == 1) && (next == 1 && block_next->is_free != 1)){

			  uint32 s = free_blk->size;

			  block_prev->size+=s;
			  block_prev->is_free = 1;

			  free_blk->size = 0;
			  free_blk->is_free = 0;
			  LIST_REMOVE(&DynamicAllocator,free_blk);
			  return;
		}
		// if block is the first so merge with next
		else if((block_prev == NULL) && ( (next == 1) && block_next->is_free == 1)){
			uint32 s = block_next->size;

			free_blk->is_free = 1;
			free_blk->size += s;

			block_next->size = 0;
			block_next->is_free = 0;
			LIST_REMOVE(&DynamicAllocator,block_next); // remove el nxt free block
			return;
		}
		//next is free only
		else if(( (next ==1) && block_next->is_free == 1) && ((prev == 1) && block_prev->is_free != 1)){

		   uint32 s = block_next->size;

		   free_blk->is_free = 1;
		   free_blk->size += s;

		   block_next->size = 0;
		   block_next->is_free = 0;
		   LIST_REMOVE(&DynamicAllocator,block_next);

		   return;
		}
		// prev is free only
		else if(( (prev == 1) && block_prev->is_free == 1) && ( (next ==1) && block_next->is_free != 1)){
			  uint32 s = free_blk->size;

			  block_prev->size+=s;
			  block_prev->is_free = 1;


			  free_blk->size = 0;
			  free_blk->is_free = 0;
			  LIST_REMOVE(&DynamicAllocator,free_blk);

			  return;
		}
		// previous and next are free
		else if(( (next == 1) && block_next->is_free == 1 ) && ( prev == 1 && block_prev->is_free == 1)){

			//to remove next block


			uint32 s1 = block_next->size;

			free_blk->size+=s1;

			block_next->size = 0;
			block_next->is_free = 0;


			LIST_REMOVE(&DynamicAllocator,block_next);

			//to remove block
			uint32 s2 =(free_blk->size);

			free_blk->is_free = 0;
			free_blk->size = 0;

			block_prev->size +=s2;

			block_prev->is_free = 1;

			LIST_REMOVE(&DynamicAllocator,free_blk);
			return;
		}

		}
//=========================================
// [4] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'23.MS1 - #8] [3] DYNAMIC ALLOCATOR - realloc_block_FF()
		//panic("realloc_block_FF is not implemented yet");

		struct BlockMetaData* block = (struct BlockMetaData*)(va-(sizeOfMetaData()));
		int next=0,prev=0;
			if(LIST_NEXT(block) != NULL){// LAW HOWA MESH BE NULL
						next = 1;
					}
					if(LIST_PREV(block)!=NULL){
						prev=1;
					}

		//uint32 Original_size= get_block_size(va);
		//struct BlockMetaData* block = (struct BlockMetaData*)(va-(sizeOfMetaData()));
		//struct BlockMetaData* next_block = block->prev_next_info.le_next;
		//uint32 next_size= get_block_size(block->prev_next_info.le_next);

	    if(va == NULL && new_size != 0)
						{
							return alloc_block_FF(new_size);
						}
	    if(va == NULL && new_size == 0)
							{
							return NULL;
							}

		if(va != NULL && new_size == 0)
		{
			 free_block(va);
	         return NULL;
		}

		else if(va != NULL && new_size != 0)
		{
			uint32 Original_size= block->size;
			uint32 next_size= LIST_NEXT(block)->size;


			//if((new_size +sizeOfMetaData()) > Original_size)


				if( next && block->prev_next_info.le_next->is_free==1 && (new_size+sizeOfMetaData()) <=next_size+Original_size)
				{

					uint32 s = Original_size+ LIST_NEXT(block)->size - (new_size +sizeOfMetaData());

					block->size = new_size+sizeOfMetaData();

					LIST_NEXT(block)->size=0;
					LIST_NEXT(block)->is_free=0;

				    LIST_REMOVE(&DynamicAllocator ,LIST_NEXT(block));

					if(s > sizeof(struct BlockMetaData))
					            {
						struct BlockMetaData* new_block = (struct BlockMetaData*) ((int)va + block->size);
										new_block->size=s;
										new_block->is_free=1;
					       LIST_INSERT_AFTER(&DynamicAllocator, block, new_block);


					            }
	                        return (block+1);

				}
				else
				{


					if(new_size+ sizeOfMetaData()<Original_size){

						block->size =new_size+sizeOfMetaData();

						struct BlockMetaData* new_block_1 = (struct BlockMetaData*) ((int)block + block->size);
						new_block_1->is_free = 1;

						new_block_1->size = Original_size -(new_size+sizeOfMetaData());
						LIST_INSERT_AFTER(&DynamicAllocator, block, new_block_1);
						return (block+1);
					}


					void *x = alloc_block_FF(new_size);
					if(x)
						{


						free_block(block+1);
						return x;
						}
					else{
					    return NULL;
					}
				}


	        }


		return NULL;
}
