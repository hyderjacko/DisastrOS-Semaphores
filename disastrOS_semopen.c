#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semOpen(){
  // do stuff :)
	
	// We take id semaphore from current process
	int id = running->syscall_args[0];
	if(id < 0){
		printf("ERROR! Negative id, cannot initialized semaphore!\n");
		running->syscall_retvalue = -1;
		return;
	}
	
	//We take processes counter from current process
	int count = running->syscall_args[1];
	
	//Checking if semaphore exists already (using SemaphoreList_byId)
	//*** aggiunta variabile semaphores_list in disastrOS_globals.h ***
	Semaphore* sem = SemaphoreList_byId(&semaphores_list, id);
	if(!sem){
		//Allocating semaphore and add it in semaphores list
		sem = Semaphore_alloc(id, count);
		List_insert(&semaphores_list, semaphores_list->last, sem);
	}
	
	//Allocating semaphore descriptor for current process
	SemDescriptor* sem_dscr = SemDescriptor_alloc(running->last_sem_fd, sem, running);
	
	//Set return value of the process to the semaphore file descriptor
	running->syscall_retvalue = sem_dscr->fd;
	return;
}
