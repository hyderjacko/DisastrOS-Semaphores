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
	SemDescriptorPtr* sem_dscr_ptr = sem_dscr->ptr;
	printf("[SEM_WAIT_INFO] process N°%d tring to access to shared resource: semWait launched on semaphore N°%d.\n", running->pid,sem->id);
	//Now we have to decrement counter of process and check if it's < 0
	sem->count--;
	if(sem->count < 0){
		//Process can't access to the resource,
		//so we put it and its descriptor in the waiting list
		List_detach(&sem->descriptors, (ListItem*)sem_dscr_ptr);
		List_insert(&sem->waiting_descriptors, sem->waiting_descriptors.last, (ListItem*)sem_dscr_ptr);
		List_insert(&waiting_list, waiting_list.last, (ListItem*)running);
		running->status = Waiting;
		
		printf("[SEM_WAIT_INFO] Process N°%d moved to waiting list.\n",running->pid);
		//Now we run the first process in the ready list
		PCB* ready_process = (PCB*)List_detach(&ready_list, (ListItem*)ready_list.first);
		running = (PCB*)ready_process;
	}
	else{
		printf("[SEM_WAIT_INFO] Process N°%d can access to resource.\n",running->pid);
	}
	running->syscall_retvalue = 0;
}
