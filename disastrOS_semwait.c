#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semWait(){
  // do stuff :)
	
	//We take semaphore descriptor from file descriptor, than we take
	//the semaphore
	int fd = running->syscall_args[0];
	SemDescriptor* sem_dscr = SemDescriptorList_byFd(&running->sem_descriptors, fd);
	Semaphore* sem = sem_dscr->semaphore;
	
	//Now we have to decrement counter of process and check if it's < 0
	sem->count--;
	if(sem->count < 0){
		//Process can't access to the resource,
		//so we put it and its descriptor in the waiting list
		List_detach(&sem->descriptors, (ListItem*)(&sem_dscr->ptr));
		List_insert(&sem->waiting_descriptors, sem->waiting_descriptors.last, (ListItem*)&sem_dscr->ptr);
		List_insert(&waiting_list, waiting_list.last, (ListItem*)running);
		running->status = Waiting;
		
		//Now we run the first process in the ready list
		PCB* ready_process = (PCB*)List_detach(&ready_list, (ListItem*)ready_list.first);
		running = ready_process;
	}
	
	running->syscall_retvalue = 0;
}
