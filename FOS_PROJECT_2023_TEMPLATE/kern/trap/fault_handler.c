/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include "../cpu/sched.h"
#include "../disk/pagefile_manager.h"
#include "../mem/memory_manager.h"
#include "kern/mem/kheap.h"

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE)
{
	assert(LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE ;
}
void setPageReplacmentAlgorithmCLOCK(){_PageRepAlgoType = PG_REP_CLOCK;}
void setPageReplacmentAlgorithmFIFO(){_PageRepAlgoType = PG_REP_FIFO;}
void setPageReplacmentAlgorithmModifiedCLOCK(){_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;}
/*2018*/ void setPageReplacmentAlgorithmDynamicLocal(){_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;}
/*2021*/ void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps){_PageRepAlgoType = PG_REP_NchanceCLOCK;  page_WS_max_sweeps = PageWSMaxSweeps;}

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE){return _PageRepAlgoType == LRU_TYPE ? 1 : 0;}
uint32 isPageReplacmentAlgorithmCLOCK(){if(_PageRepAlgoType == PG_REP_CLOCK) return 1; return 0;}
uint32 isPageReplacmentAlgorithmFIFO(){if(_PageRepAlgoType == PG_REP_FIFO) return 1; return 0;}
uint32 isPageReplacmentAlgorithmModifiedCLOCK(){if(_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1; return 0;}
/*2018*/ uint32 isPageReplacmentAlgorithmDynamicLocal(){if(_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL) return 1; return 0;}
/*2021*/ uint32 isPageReplacmentAlgorithmNchanceCLOCK(){if(_PageRepAlgoType == PG_REP_NchanceCLOCK) return 1; return 0;}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt){_EnableModifiedBuffer = enableIt;}
uint8 isModifiedBufferEnabled(){  return _EnableModifiedBuffer ; }

void enableBuffering(uint32 enableIt){_EnableBuffering = enableIt;}
uint8 isBufferingEnabled(){  return _EnableBuffering ; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length;}
uint32 getModifiedBufferLength() { return _ModifiedBufferLength;}

//===============================
// FAULT HANDLERS
//===============================

//Handle the table fault
void table_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}

//Handle the page fault

void page_fault_handler(struct Env * curenv, uint32 fault_va)
{
#if USE_KHEAP
		struct WorkingSetElement *victimWSElement = NULL;
		uint32 wsSize = LIST_SIZE(&(curenv->page_WS_list));
#else
		int iWS =curenv->page_last_WS_index;
		uint32 wsSize = env_page_ws_get_size(curenv);
#endif


		//cprintf("REPLACEMENT=========================WS Size = %d\n", wsSize );
		//refer to the project presentation and documentation for details
		if(isPageReplacmentAlgorithmFIFO())
		{
			//TODO: [PROJECT'23.MS3 - #1] [1] PAGE FAULT HANDLER - FIFO Replacement
			// Write your code here, remove the panic and write your code
			//panic("page_fault_handler() FIFO Replacement is not implemented yet...!!");
			if(wsSize < (curenv->page_WS_max_size))
			{

				struct FrameInfo *ptr_frame_info;
				bool uHeap  = ((fault_va >= USER_HEAP_START) && (fault_va <USER_HEAP_MAX));
				bool uStack = ((fault_va >=USTACKBOTTOM)     && (fault_va< USTACKTOP));
				int ret = allocate_frame(&ptr_frame_info);
				if(ret == 0)
				{
					map_frame(curenv->env_page_directory,ptr_frame_info,fault_va,PERM_PRESENT|PERM_USER|PERM_WRITEABLE);
					ptr_frame_info->va = fault_va;
				}
				int retRF = pf_read_env_page(curenv,(void *)fault_va);
				if((retRF == 0) || ((retRF == E_PAGE_NOT_EXIST_IN_PF) && (uHeap || uStack)))
				{
					struct WorkingSetElement* pointerToElemnt = env_page_ws_list_create_element(curenv,fault_va);
					LIST_INSERT_TAIL(&(curenv->page_WS_list) , pointerToElemnt);

					int ListS = LIST_SIZE(&(curenv->page_WS_list));
					cprintf("inserting in list\n");

					if(ListS == (curenv->page_WS_max_size))
					{
						curenv->page_last_WS_element = LIST_FIRST(&curenv->page_WS_list);
					}
					else
						curenv->page_last_WS_element = NULL;
				}
				else
				{
					cprintf("Killed here1\n");
					sched_kill_env(curenv->env_id);
				}
			}
			else{
					struct FrameInfo *ptr_frame_info;
					cprintf("\n in fifo replacment \n");
					bool uHeap  = ((fault_va >= USER_HEAP_START) && (fault_va <USER_HEAP_MAX));
					bool uStack = ((fault_va >=USTACKBOTTOM)     && (fault_va< USTACKTOP));
					cprintf("\n in fifo replacment xxxx \n");
					int ret = allocate_frame(&ptr_frame_info);
					if(ret == 0)
					{
						map_frame(curenv->env_page_directory,ptr_frame_info,fault_va,PERM_PRESENT|PERM_USER|PERM_WRITEABLE);
						ptr_frame_info->va = fault_va;
					}
					int retRF = pf_read_env_page(curenv,(void *)fault_va);
					cprintf("\n in fifo replacmenttttt \n");
					if((retRF == 0) || ((retRF == E_PAGE_NOT_EXIST_IN_PF) && (uHeap || uStack)))
					{

						struct WorkingSetElement *victim  = LIST_FIRST(&curenv->page_WS_list);
						if(victim != NULL)
						{
							uint32 victimVA = victim->virtual_address;
							struct WorkingSetElement *secondElemntInList2 = victim->prev_next_info.le_next;
							int perms = pt_get_page_permissions(curenv->env_page_directory, victimVA); //victim
							cprintf("\n in fifo 2 \n");
							//uint32 **ptr_page_table = NULL;  //error
							cprintf("\n in fifo 3 \n");
							uint32 *ptr_page =NULL;
							//struct FrameInfo *ptr_victim_frame_info = NULL;
							//ptr_victim_frame_info->va = victimVA;
							cprintf("\n in fifo 4 \n");

							if(perms & PERM_MODIFIED)
							{
								//uint32 **ptr_page_table = NULL;  //error

								struct FrameInfo *ptr_frame_info = get_frame_info(curenv->env_page_directory
																								 ,victimVA,&ptr_page);
								int ret = pf_update_env_page(curenv,victimVA,ptr_frame_info);
								if(ret == 0)
								{
									cprintf("\n page updated successfully \n");
								}
						   }

						//free_frame(ptr_frame_info); //unmap
						unmap_frame(curenv->env_page_directory,victimVA);
						LIST_REMOVE(&curenv->page_WS_list, victim);
						cprintf("\n to kfree \n");
						kfree(victim);


							struct WorkingSetElement* pointerToElemnt = env_page_ws_list_create_element(curenv,fault_va);
							LIST_INSERT_TAIL(&(curenv->page_WS_list) , pointerToElemnt);
							cprintf("\n inserted in list \n");
							curenv->page_last_WS_element = LIST_FIRST(&curenv->page_WS_list);
					}
						else
							cprintf(" \n empty ws \n");
					}

			else
			{


				cprintf("Killed here2\n");
				sched_kill_env(curenv->env_id);

			}




					//struct WorkingSetElement *secondElemntInList = LIST_NEXT (&firstElemntInList);


					}

			}


//////////////////////////////////////////////////////////////////////////////////////////////////////


		if(isPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX))
					{
						//TODO: [PROJECT'23.MS3 - #2] [1] PAGE FAULT HANDLER - LRU Replacement
						// Write your code here, remove the panic and write your code
						//panic("page_fault_handler() LRU Replacement is not implemented yet...!!");

						//TODO: [PROJECT'23.MS3 - BONUS] [1] PAGE FAULT HANDLER - O(1) implementation of LRU replacement
		//				if(fault_va == NULL)
		//				{
		//					return;
		//				}

						if(LIST_SIZE(&(curenv->SecondList)) + LIST_SIZE(&curenv->ActiveList) < curenv->page_WS_max_size) // Placement
					   {

							if(curenv->ActiveList.size < curenv->ActiveListSize) //SecondList Empty addresss not in SecondList
							{
								struct FrameInfo *ptr_frame_info;

								//cprintf("\n in LRU Placement not in second list 1 \n");

								bool uHeap  = ((fault_va >= USER_HEAP_START) && (fault_va <USER_HEAP_MAX));
								bool uStack = ((fault_va >=USTACKBOTTOM)     && (fault_va< USTACKTOP));

								//cprintf("\n in LRU Placement not in second list 2 \n");

								int ret = allocate_frame(&ptr_frame_info);
								cprintf("LRU Placement 11 \n");

								if(ret == 0)
								{
									cprintf("LRU Placement 12 (Allocated the frame) \n");

									map_frame(curenv->env_page_directory,ptr_frame_info,fault_va,PERM_PRESENT|PERM_USER|PERM_WRITEABLE);
									ptr_frame_info->va = fault_va;

									cprintf("LRU Placement 13 (Mapped the frame) \n");

								}
								int retRF = pf_read_env_page(curenv,(void *)fault_va);

								cprintf("LRU Placement 14 (read from disk) \n");

								if(!(fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX) && !(fault_va < USTACKTOP && fault_va >= USTACKBOTTOM))
								{

									cprintf("Killed here ya dana\n");
									sched_kill_env(curenv->env_id);
								}

								else
								{
									// Insert new_element , Set Present bit 1
									//cprintf("LRU Placement 21 (else)\n");
									struct WorkingSetElement* new_element = env_page_ws_list_create_element(curenv, fault_va);
									//cprintf("LRU Placement 22 (Created new_element 2) \n");

									LIST_INSERT_HEAD(&(curenv->ActiveList) , new_element);
									//cprintf("LRU Placement 23 \n");
								}


						}// space in ActiveList / Empty SecondList

						else // ActiveList FULL --> Check on SecondList
						{
								struct WorkingSetElement* ptr;
								struct WorkingSetElement* in_second_list;
								int found_in_second = 0;

								if(curenv->SecondList.size != 0) //Check if fault_va in SecondList
								{
									LIST_FOREACH(ptr, &(curenv->SecondList))
									{
										 cprintf("LRU Placement 2 (INSIDE the LOOP) \n");

										 if(ptr->virtual_address == fault_va) // in the second list
										 {
											 cprintf("LRU Placement 3 (found in second list) \n");
											 found_in_second = 1;
											 in_second_list = ptr;

											 break;
										 }
									} // loop
								}

								if(found_in_second == 1) //fault_va in SecondList
								{
									 LIST_REMOVE(&(curenv->SecondList) , in_second_list);


									 cprintf("LRU Placement 4 (Removed from SecondList) \n");


									 cprintf("LRU Placement 7 (No Space in ActiveList) \n");
									 struct WorkingSetElement* active_list_tail = LIST_LAST(&(curenv->ActiveList));

									 LIST_REMOVE(&(curenv->ActiveList) , active_list_tail);
									 LIST_INSERT_HEAD(&(curenv->SecondList), active_list_tail);

									 pt_set_page_permissions(curenv->env_page_directory, active_list_tail->virtual_address, 0, PERM_PRESENT); // Set present bit 0

									 cprintf("LRU Placement 8 (Removed tail from  ActiveList to insert) \n");

									 LIST_INSERT_HEAD(&(curenv->ActiveList) , in_second_list);
									 cprintf("LRU Placement 9 (Inserted in ActiveList) \n");


									 pt_set_page_permissions(curenv->env_page_directory, in_second_list->virtual_address, PERM_PRESENT, 0); // Set present bit 1

								}
								else // NOT in SeconList
								{
									struct FrameInfo *ptr_frame_info;

									//cprintf("\n in LRU Placement not in second list 1 \n");

									bool uHeap  = ((fault_va >= USER_HEAP_START) && (fault_va <USER_HEAP_MAX));
									bool uStack = ((fault_va >=USTACKBOTTOM)     && (fault_va< USTACKTOP));

									//cprintf("\n in LRU Placement not in second list 2 \n");

									int ret = allocate_frame(&ptr_frame_info);
									cprintf("LRU Placement 11 \n");

									if(ret == 0)
									{
										cprintf("LRU Placement 12 (Allocated the frame) \n");

										map_frame(curenv->env_page_directory,ptr_frame_info,fault_va,PERM_PRESENT|PERM_USER|PERM_WRITEABLE);
										ptr_frame_info->va = fault_va;

										cprintf("LRU Placement 13 (Mapped the frame) \n");

									}
									int retRF = pf_read_env_page(curenv,(void *)fault_va);

									cprintf("LRU Placement 14 (read from disk) \n");


									if(!(fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX) && !(fault_va < USTACKTOP && fault_va >= USTACKBOTTOM))
									{
										cprintf("Killed here ya dana\n");
										sched_kill_env(curenv->env_id);
									}
									else
									{
										//cprintf("LRU Placement 16 (ActiveList Full) \n");

										struct WorkingSetElement* active_list_tail = LIST_LAST(&(curenv->ActiveList));

										LIST_REMOVE(&(curenv->ActiveList) , active_list_tail);
										//cprintf("LRU Placement 17 (Removed active tail) \n");

										LIST_INSERT_HEAD(&(curenv->SecondList), active_list_tail);
										//cprintf("LRU Placement 18 (Inserted tail in secondList) \n");

										pt_set_page_permissions(curenv->env_page_directory, active_list_tail->virtual_address, 0, PERM_PRESENT); // Set present bit 0

										struct WorkingSetElement* new_element = env_page_ws_list_create_element(curenv , fault_va);
										//cprintf("LRU Placement 19 (Created new_element) \n");

										//pt_set_page_permissions(curenv->env_page_directory, fault_va, PERM_PRESENT, 0); // Set Present bit 1
										LIST_INSERT_HEAD(&(curenv->ActiveList) , new_element);
										//cprintf("LRU Placement 20 (Inserted new_element in ActiveList) \n");

									}

								}// not in second

						}                          //end ActiveList FULL


					   }// END PLACEMENT
						else
								{
						//			cprintf("In LRU REPLACEMENTTTTTTTTTTT 1111 \n");

									fault_va = ROUNDDOWN(fault_va, PAGE_SIZE);

										struct WorkingSetElement* ptr;
										struct WorkingSetElement* in_second_list;
										int found_in_second = 0;

											//Check if fault_va in SecondList

											LIST_FOREACH(ptr, &(curenv->SecondList))
											{
												 //cprintf("LRU REEEEEPlacement (INSIDE the LOOP) \n");

												 if(ptr->virtual_address == fault_va) // in the second list
												 {
													 //cprintf("LRU REEEEPlacement  (found in second list) \n");
													 found_in_second = 1;
													 in_second_list = ptr;

													 break;
												 }
											} // loop


										if(found_in_second == 1) //fault_va in SecondList
										{
											 LIST_REMOVE(&(curenv->SecondList) , in_second_list);


											// cprintf("LRU REEEEEPlacement  (Removed from SecondList) \n");

											// cprintf("LRU REEEEEPlacement  (No Space in ActiveList) \n");
											 struct WorkingSetElement* active_list_tail = LIST_LAST(&(curenv->ActiveList));

											 LIST_REMOVE(&(curenv->ActiveList) , active_list_tail);
											 LIST_INSERT_HEAD(&(curenv->SecondList), active_list_tail);

											 pt_set_page_permissions(curenv->env_page_directory, active_list_tail->virtual_address, 0, PERM_PRESENT); // Set present bit 0

											// cprintf("LRU REEEEEPlacement  (Removed tail from  ActiveList to insert) \n");

											 LIST_INSERT_HEAD(&(curenv->ActiveList) , in_second_list);
											// cprintf("LRU REEEEEPlacement  (Inserted in ActiveList) \n");


											 pt_set_page_permissions(curenv->env_page_directory, in_second_list->virtual_address, PERM_PRESENT, 0); // Set present bit 1

										}
										else // NOT in SecondList
										{
											struct FrameInfo *ptr_frame_info;
											bool uHeap  = ((fault_va >= USER_HEAP_START) && (fault_va <USER_HEAP_MAX));
											bool uStack = ((fault_va >=USTACKBOTTOM)     && (fault_va< USTACKTOP));


											int ret = allocate_frame(&ptr_frame_info);
											//cprintf("LRU Placement 11 \n");

											//cprintf("LRU REPLACEMENTTTT 22222 (Allocated the frame) \n");

											map_frame(curenv->env_page_directory,ptr_frame_info,fault_va,PERM_PRESENT|PERM_USER|PERM_WRITEABLE);


											//cprintf("LRU REPLACEMENTTTT 3333333 (Mapped the frame) \n");


											int retRF = pf_read_env_page(curenv,(void *)fault_va);

											//cprintf("LRU REPLACEMENTTTT 4444444 (read from disk) \n");

											if(retRF == E_PAGE_NOT_EXIST_IN_PF)
											{
												if(!(fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX) && !(fault_va < USTACKTOP && fault_va >= USTACKBOTTOM))
												{
													cprintf("Killed here ya dana\n");//////////heeeeeenaaaaaaaaaaaaaa
													sched_kill_env(curenv->env_id);
												}
											}
											 struct WorkingSetElement* victim = LIST_LAST(&(curenv->SecondList));

											  uint32 page_perm = pt_get_page_permissions(curenv->env_page_directory, victim->virtual_address);

											   if(page_perm & PERM_MODIFIED) // Modified = 1 of victim -> write it on the disk
											   {


												   uint32 * ptr_page_table = NULL;
												   struct FrameInfo *ptr_frame_info = get_frame_info(curenv->env_page_directory,victim->virtual_address,&ptr_page_table);
												   int ret = pf_update_env_page(curenv,victim->virtual_address, ptr_frame_info); //Update page on disk
											   }

											   env_page_ws_invalidate(curenv, victim->virtual_address);


											   struct WorkingSetElement* active_list_tail = LIST_LAST(&(curenv->ActiveList));

											  LIST_REMOVE(&(curenv->ActiveList) , active_list_tail);
											  LIST_INSERT_HEAD(&(curenv->SecondList) , active_list_tail);

											  pt_set_page_permissions(curenv->env_page_directory, active_list_tail->virtual_address, 0, PERM_PRESENT); // Set Present bit 0

											  struct WorkingSetElement* new_element = env_page_ws_list_create_element(curenv , fault_va);
											  LIST_INSERT_HEAD(&(curenv->ActiveList) , new_element);
									}

								}//END REPLACEMENTTT

/////////////////////////////////////////////////////////////////////////////////////////////////////
	}
}


void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	panic("this function is not required...!!");
}






