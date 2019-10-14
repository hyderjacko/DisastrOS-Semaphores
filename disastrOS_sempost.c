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
	
	printf("[SEM_POST_INFO] Process N°%d want to free a shared resource: semPost launched on semaphore N°%d.\n", running->pid,sem->id);
	//Now we increase semaphore counter
	//and check if it's <= 0
	sem->count++;
	if(sem->count <= 0){
		//we put the current waiting process in the ready list
		List_insert(&ready_list, ready_list.last, (ListItem*)running);
		running->status = Ready;
		printf("[SEM_POST_INFO] Process N°%d moved to ready list.\n",running->pid);
		//we need another process to run:
		//removing the first descriptor from waiting descriptor list
		SemDescriptorPtr* sem_dscr_ptr = (SemDescriptorPtr*)List_detach(&sem->waiting_descriptors, (ListItem*)sem->waiting_descriptors.first);
		
		//put it in descriptors list
		List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*)sem_dscr_ptr);
		
		//removing the relative process from waiting list
		List_detach(&waiting_list, (ListItem*)sem_dscr_ptr->descriptor->pcb);
		
		//now we update the running process
		running = sem_dscr_ptr->descriptor->pcb;
		printf("[SEM_POST_INFO] Process N°%d now is running.\n",running->pid);
	}
	
	printf("[SEM_POST_INFO] Process has left the resource.\n");
	running->syscall_retvalue = 0;
	return;
}
