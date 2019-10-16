#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semClose(){
  // do stuff :)
    
	//First we have to remove semaphore descriptor
	//We take semaphore descriptor from the file descriptor of the
	//semaphore we have to close (Using SemDescriptorList_byId);
	//Than we remove it from semaphore descriptor list
	//and free allocating memory
	int fd = running->syscall_args[0];
	SemDescriptor* sem_dscr = SemDescriptorList_byFd(&running->sem_descriptors, fd);
	List_detach(&running->sem_descriptors, (ListItem*)sem_dscr);
	Semaphore* sem = sem_dscr->semaphore;
	SemDescriptorPtr* sem_dscr_ptr = (SemDescriptorPtr*)List_detach(&sem->descriptors, (ListItem*)sem_dscr->ptr);
	
	printf("[SEM_CLOSE_INFO] Closing semaphore N°%d...\n",sem->id);
	SemDescriptorPtr_free(sem_dscr_ptr);
	SemDescriptor_free(sem_dscr);
	
	//Checking if other processes are using the semaphore first of remove it
	if(sem->descriptors.size == 0 && sem->waiting_descriptors.size == 0){
		//Now we can remove the semaphore from semaphore list and destroy it
		List_detach(&semaphores_list, (ListItem*)sem);
		printf("[SEM_CLOSE_INFO] Semaphore N°%d closed.\n",sem->id);
		Semaphore_free(sem);
	}
	
	//updating file descriptor
	running->last_sem_fd--;
	
	running->syscall_retvalue = 0;
	return;
}
