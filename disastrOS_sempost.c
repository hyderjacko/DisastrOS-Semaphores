#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semPost(){
  // do stuff :)
	//We take semaphore descriptor from file descriptor,
	//than we take the semaphore
	int fd = running->syscall_args[0];
	SemDescriptor* sem_dscr = SemDescriptorList_byFd(&running->sem_descriptors, fd);
	Semaphore* sem = sem_dscr->semaphore;
	
	//Now we increase semaphore counter
	//and check if it's <= 0
	sem->count++;
	if(sem->count <= 0){
		//we put the current waiting process in the ready list
		List_insert(&ready_list, &ready_list->last, (ListItem*)running);
		running->status = Ready;
	}
	
	running->syscall_retvalue = 0;
	return;
}
