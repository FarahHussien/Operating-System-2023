#include <inc/lib.h>

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

struct Va_no_pages_MARKED

{
	uint32 va;
	uint32 no_of_pages_marked;
};

struct Va_no_pages_MARKED marked_va[USER_HEAP_MAX  / PAGE_SIZE];
uint32 marked_count = 0;
uint32 total_pages = (USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE;


int FirstTimeFlag = 1;
void InitializeUHeap()
{
	if(FirstTimeFlag)
	{
#if UHP_USE_BUDDY
		initialize_buddy();
		cprintf("BUDDY SYSTEM IS INITIALIZED\n");
#endif
		FirstTimeFlag = 0;
	}
}


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
void* malloc(uint32 size)
{
	//==============================================================
	//TODO: [PROJECT'23.MS2 - #09] [2] USER HEAP - malloc() [User Side]
	// Write your code here, remove the panic and write your code
//	panic("malloc() is not implemented yet...!!");
//	return NULL;

	//==============================================================
	//DON'T CHANGE THIS CODE========================================


	//PERM_MARKED
	//sys_get_perm(i)
	//marked_va[marked_count]->no_of_pages_marked = 1;

	//sys_gethardlimit();


	InitializeUHeap();
/*		panic("malloc() is not implemented yet...!!");
		return NULL;*/

  uint32 limit = myEnv->hardl;
  uint32 start_page_alloc=limit+PAGE_SIZE;

	if (size == 0)
	{
		return NULL;
	}
	else
	{

        bool is_found=0;
		uint32 round_size = ROUNDUP(size, PAGE_SIZE);
		uint32 no_of_pages = ROUNDUP(size, PAGE_SIZE)/PAGE_SIZE;
		uint32 last_add;
		uint32 begin_add;

		if(size <= DYN_ALLOC_MAX_BLOCK_SIZE ) // BLOCK ALLOCATOR
		{

			return alloc_block_FF(size);
		}
		else  // PAGE ALLOCATOR
		{

			if(sys_isUHeapPlacementStrategyFIRSTFIT())
			{
				uint32 found_counter=0;

				for(uint32 i = start_page_alloc; i < USER_HEAP_MAX; i += PAGE_SIZE)
				{

					uint32 perm = sys_get_perm(i);
					if((perm&PERM_MARKED)==0x000|| (perm == -1))
					{
						found_counter++;
					}
					else
					{
						found_counter=0;
					}
					if(found_counter==no_of_pages)
					{
						is_found=1;
						last_add=i;
						marked_count++;
						break;
					}
				}
				if(is_found==1)
				{
					begin_add=(last_add-(round_size-PAGE_SIZE));
					struct Va_no_pages_MARKED temp;
					temp.va=begin_add;
					temp.no_of_pages_marked = no_of_pages;
					sys_allocate_user_mem(begin_add,round_size);
					marked_va[marked_count]=temp;
					return (void*)begin_add;

				}
				else
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


}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #11] [2] USER HEAP - free() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");
	uint32 size =0;
	if (virtual_address==NULL)
	{
		return;
	}
	if((uint32)virtual_address>= myEnv->start&& (uint32)virtual_address< myEnv->hardl )
	{
		free_block(virtual_address);
	}
	else if((uint32)virtual_address >= myEnv->hardl + PAGE_SIZE && (uint32)virtual_address < USER_HEAP_MAX)
	{
		for(int i = 0; i <= marked_count; i++) //Loop on the array
				{
					if((uint32)virtual_address == marked_va[i].va)
					{
						size= (marked_va[i].no_of_pages_marked)* PAGE_SIZE;
						break;
					}

				}

		 sys_free_user_mem((uint32)virtual_address,size);

	}
	}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================
	panic("smalloc() is not implemented yet...!!");
	return NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================
	// Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");
	return NULL;
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================

	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}


//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");
}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
