#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"
int arr_count = 0;
uint32 total_pages=0;
struct Va_no_pages va_arr[KERNEL_HEAP_MAX/PAGE_SIZE];
int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	//TODO: [PROJECT'23.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator()
	//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
	//All pages in the given range should be allocated
	//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
	//Return:
	//	On success: 0
	//	Otherwise (if no memory OR initial size exceed the given limit): E_NO_MEM

	//Comment the following line(s) before start coding...
	//panic("not implemented yet");

	start = ROUNDDOWN(daStart,PAGE_SIZE);
	hlimit = ROUNDUP(daLimit,PAGE_SIZE);
	brk = ROUNDUP((initSizeToAllocate+start),PAGE_SIZE);

	total_pages= (KERNEL_HEAP_MAX-(hlimit+PAGE_SIZE))/PAGE_SIZE;

	if(brk>hlimit)
	{
		return E_NO_MEM;
	}
	else {

		uint32 num_of_pages = ROUNDUP((brk-start)/PAGE_SIZE,PAGE_SIZE);
		struct FrameInfo*ptr_frame =NULL;
		for(uint32 i=start ; i<brk;i+=PAGE_SIZE){
		int return_AllocFrame = allocate_frame(&ptr_frame);
		if (return_AllocFrame != E_NO_MEM){
			 map_frame(ptr_page_directory, ptr_frame, i, PERM_WRITEABLE | PERM_PRESENT);
			 ptr_frame->va = i;
		}
		else
			return E_NO_MEM;

		}
		initialize_dynamic_allocator(start,initSizeToAllocate);
		 return 0;
		}
}

void* sbrk(int increment)
{
	//TODO: [PROJECT'23.MS2 - #02] [1] KERNEL HEAP - sbrk()
	/* increment > 0: move the segment break of the kernel to increase the size of its heap,
	 * 				you should allocate pages and map them into the kernel virtual address space as necessary,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * increment = 0: just return the current position of the segment break
	 * increment < 0: move the segment break of the kernel to decrease the size of its heap,
	 * 				you should deallocate pages that no longer contain part of the heap as necessary.
	 * 				and returns the address of the new break (i.e. the end of the current heap space).
	 *
	 * NOTES:
	 * 	1) You should only have to allocate or deallocate pages if the segment break crosses a page boundary
	 * 	2) New segment break should be aligned on page-boundary to avoid "No Man's Land" problem
	 * 	3) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, kernel should panic(...)
	 */
	uint32 noPages = ROUNDUP(increment, PAGE_SIZE)/PAGE_SIZE;
	if(increment > 0)
	{

		if((brk + increment) <= hlimit)
		{
			if(noPages!=0){
			struct FrameInfo *ptr_frame_info;
			uint32 newbrk = ROUNDUP((brk+increment),PAGE_SIZE);
            uint32 oldbrk = brk;

			for(int i = brk; i<newbrk;i+=PAGE_SIZE )
			{

			int ret =allocate_frame(&ptr_frame_info);
            if(ret==0){
				map_frame(ptr_page_directory,ptr_frame_info,i,PERM_WRITEABLE | PERM_PRESENT);
				ptr_frame_info->va = i;
                }
            else
            	return (void*)-1;
			}

			brk+= (noPages*PAGE_SIZE);
			return (void*)oldbrk;
		}
		}
	}
	else if(increment == 0)
	{
		return (void*) brk;
	}
	else if(increment < 0 &&(brk+increment)>=start)
	{
		for(uint32 i = ROUNDUP(brk,PAGE_SIZE); i<ROUNDUP((brk+increment),PAGE_SIZE) ; i-=PAGE_SIZE)
		{
			uint32 *ptr_table = NULL;
			struct FrameInfo* ptr_frame_info = get_frame_info(ptr_page_directory,i,&ptr_table);
			ptr_frame_info->va=0;
			unmap_frame(ptr_page_directory,i);

		}

        brk+=increment;
		return (void*)brk;
	}

return (void*)-1;
}

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'23.MS2 - #03] [1] KERNEL HEAP - kmalloc()
	//refer to the project presentation and documentation for details
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

	//change this "return" according to your answer
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	//return NULL;
//cprintf("============================kmalloc=========================\n");
	uint32 size_of_page_alloc=KERNEL_HEAP_MAX-(start);
	if (size == 0 || size >=size_of_page_alloc ){
		return NULL;
	}
	if(size <= DYN_ALLOC_MAX_BLOCK_SIZE) // BLOCK ALLOCATOR
	{
		return alloc_block_FF(size);
	}
	else if(size > DYN_ALLOC_MAX_BLOCK_SIZE)//PAGE ALLOCATOR
	{
		uint32 no_of_pages = ROUNDUP(size, PAGE_SIZE)/PAGE_SIZE;

		if(isKHeapPlacementStrategyFIRSTFIT())
		{
			uint32 strt_page = PAGE_SIZE + hlimit;//search in virtual memory to allocate pages

			if(size <= PAGE_SIZE && total_pages>=1) //Allocate 1 page, 1 Frame only without internal fragamentation
			{
				for(uint32 i = strt_page; i < KERNEL_HEAP_MAX; i+= PAGE_SIZE)
				{
					uint32 *ptr_table = NULL;
					struct FrameInfo* ptr_frame_info =get_frame_info(ptr_page_directory,i,&ptr_table);
					if(ptr_frame_info == NULL) //not mapping to a frame (empty page)
					{

						struct FrameInfo*ptr_frame =NULL;
						allocate_frame(&ptr_frame);
						map_frame(ptr_page_directory,ptr_frame,i,PERM_WRITEABLE | PERM_PRESENT);
						ptr_frame->va=i;
						va_arr[arr_count].va = i;
						va_arr[arr_count].no_of_pages = 1;
						arr_count++;
						total_pages--;
						return (void*)i;

					}
				}
			}
			else if (size > PAGE_SIZE && total_pages>=no_of_pages)
			{
				uint32 count = 0;
				bool found = 0;
				for(uint32 i = strt_page; i < KERNEL_HEAP_MAX; i += PAGE_SIZE)
				{
					uint32 *ptr_table =NULL;
					struct FrameInfo* ptr_frame_info= get_frame_info(ptr_page_directory,i,&ptr_table);

					if(ptr_frame_info == NULL)
					{
						count ++;
						if (count == no_of_pages)
						{
							va_arr[arr_count].va = i - ((no_of_pages - 1) * PAGE_SIZE);
							va_arr[arr_count].no_of_pages = no_of_pages;
							found = 1;
							break;
						}

					}
					else if(ptr_frame_info != NULL)
					{
						count = 0;
					}
				}

				if(found == 1){
					for(uint32 i = va_arr[arr_count].va; i <va_arr[arr_count].va+(no_of_pages*PAGE_SIZE); i += PAGE_SIZE)
					{
						struct FrameInfo*ptr_frame = NULL;
						allocate_frame(&ptr_frame);
						map_frame(ptr_page_directory, ptr_frame, i, PERM_WRITEABLE | PERM_PRESENT);
						ptr_frame->va = i;
				   }
				total_pages-=va_arr[arr_count].no_of_pages ;
				arr_count++;
				return (void*)va_arr[arr_count-1].va;
			}
			}
		}
	}
	return NULL;
}
void kfree(void* virtual_address)
{
	//cprintf("========================kfree====================\n");
	//TODO: [PROJECT'23.MS2 - #04] [1] KERNEL HEAP - kfree()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");
	if(virtual_address == NULL)
		return;

	if((uint32)virtual_address < hlimit && (uint32)virtual_address > start) //Block Allocator
	{

			free_block(virtual_address);
	}
		// Page allocator
	else if((uint32)virtual_address >= hlimit + PAGE_SIZE && (uint32)virtual_address < KERNEL_HEAP_MAX)
	{
			for(int i = 0; i < arr_count; i++) //Loop on the array
			{
				if((uint32)virtual_address == va_arr[i].va)
				{
					for(int j = 0; j < va_arr[i].no_of_pages; j++)
					{
						uint32 *ptr_table = NULL;
						struct FrameInfo* ptr_frame_info = get_frame_info(ptr_page_directory,(uint32)virtual_address,&ptr_table);
						ptr_frame_info->va=0;
						unmap_frame(ptr_page_directory,(uint32)virtual_address);
						virtual_address += PAGE_SIZE;
					}
					total_pages+=va_arr[i].no_of_pages;
					va_arr[i].no_of_pages = 0;
					va_arr[i].va = 0;
					return;
				}
			}
			return; //va not found in arrray (not allocated)
	}
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//cprintf("==============================va==================\n");
	//TODO: [PROJECT'23.MS2 - #05] [1] KERNEL HEAP - kheap_virtual_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

	//change this "return" according to your answer
	struct FrameInfo* ptr =  to_frame_info(physical_address);
	uint32 physical_page_address =to_physical_address(ptr);
	int offset =physical_address-physical_page_address;

	if(ptr->va==0)
	{
		offset =0;
	}
	return ptr->va+offset;
}
unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//cprintf("==================================pa====================\n");
	//TODO: [PROJECT'23.MS2 - #06] [1] KERNEL HEAP - kheap_physical_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	//change this "return" according to your answer

	uint32 *ptr_page_table = NULL;
	int ret = get_page_table(ptr_page_directory,virtual_address,&ptr_page_table) ;

	//uint32 tIndex = PTX(virtual_address);
	uint32 page_table_entry = ptr_page_table[PTX(virtual_address)];
	if( page_table_entry != 0)
	{
		return (page_table_entry & 0xFFFFF000) + (virtual_address & 0x00000FFF);
	}
	return 0;
}


void kfreeall()
{
	panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
	panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
	panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'23.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc()
	// Write your code here, remove the panic and write your code
	return NULL;
	panic("krealloc() is not implemented yet...!!");
}
