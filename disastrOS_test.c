#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS.h"

#define BUFFER_SIZE 50

/** Semaphores:
		sem_cs_cons: critical section in consumer
		sem_cs_prod: critical section in producer
		sem_to_empty: control how much resource to produce
		sem_to_fill: control how much resource to consume
  **/
int sem_cs_prod, sem_cs_cons, sem_to_empty, sem_to_fill;
int buf[BUFFER_SIZE];
int i_prod = 0, i_cons = 0;
int to_produce = 1;

// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

void producer() {
		
	//Controlling if anyone is producing
	disastrOS_semWait(sem_to_empty);
	disastrOS_semWait(sem_cs_prod);
		
	buf[i_prod] = to_produce;
	to_produce++;
	i_prod = (i_prod+1) % BUFFER_SIZE;
	disastrOS_sleep(10);
	
	//Process finish to product the value
	disastrOS_semPost(sem_cs_prod);
	disastrOS_semPost(sem_to_fill);
}

void consumer() {
	
	//Controlling if anyone is consuming
	disastrOS_semWait(sem_to_fill);
	disastrOS_semWait(sem_cs_cons);
	
	int val = buf[i_cons];
	i_cons++;
	
	disastrOS_sleep(10);
	
	printf("[CONSUMER_INFO] Consuming the value %d...\n", i_cons);
	printf("[SEMAPHORE_INFO] Child N°%d consumed the value %d.\n", disastrOS_getpid(), val);
	
	//Process finish to consume the value
	disastrOS_semPost(sem_cs_cons);
	disastrOS_semPost(sem_to_empty); 	
}

void childFunction(void* args){
	printf("Hello, I am the child function %d\n",disastrOS_getpid());
	printf("I will iterate a bit, before terminating\n");
	int type=0;
	int mode=0;
	int fd=disastrOS_openResource(disastrOS_getpid(),type,mode);
	printf("fd=%d\n", fd);
	
	//Opening semaphores
	sem_cs_prod = disastrOS_semOpen(2, 1);
	sem_cs_cons = disastrOS_semOpen(1, 1);
	sem_to_empty = disastrOS_semOpen(4, BUFFER_SIZE);
	sem_to_fill = disastrOS_semOpen(3, 0);

  
	for(int i = 1; i <= 10; i++){
	
		if(disastrOS_getpid() <= 5){
			producer();
		}
		else{
			consumer();
		}
	
	}
	
	//Closing semaphores
	disastrOS_semClose(sem_cs_prod);
	disastrOS_semClose(sem_cs_cons);
	disastrOS_semClose(sem_to_empty);
	disastrOS_semClose(sem_to_fill);

	printf("PID: %d, terminating\n", disastrOS_getpid());

	for (int i=0; i<(disastrOS_getpid()+1); ++i){
		printf("PID: %d, iterate %d\n", disastrOS_getpid(), i);
		disastrOS_sleep((20-disastrOS_getpid())*5);
	}
	disastrOS_exit(disastrOS_getpid()+1);
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started\n");
  disastrOS_spawn(sleeperFunction, 0);
  

  printf("I feel like to spawn 10 nice threads\n");
  int alive_children=0;
  for (int i=0; i<10; ++i) {
    int type=0;
    int mode=DSOS_CREATE;
    printf("mode: %d\n", mode);
    printf("opening resource (and creating if necessary)\n");
    int fd=disastrOS_openResource(i,type,mode);
    printf("fd=%d\n", fd);
    disastrOS_spawn(childFunction, 0);
    alive_children++;
  }

  disastrOS_printStatus();
  int retval;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ 
    disastrOS_printStatus();
    printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
	   pid, retval, alive_children);
    --alive_children;
  }
  printf("shutdown!");
  disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue
  printf("the function pointer is: %p\n", childFunction);
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
