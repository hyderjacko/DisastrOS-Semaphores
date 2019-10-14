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
	
	printf("[PRODUCER_INFO] Child N°%d want to produce.\n", disastrOS_getpid());
	//Controlling if anyone is producing
	disastrOS_semWait(sem_to_empty);
	disastrOS_semWait(sem_cs_prod);
	printf("[PRODUCER_INFO] Child N°%d now is producing.\n", disastrOS_getpid());
	
	printf("[PRODUCER_INFO] Producing in cell N°%d.\n",i_prod);
	buf[i_prod] = to_produce;
	printf("[PRODUCER_INFO] Child N°%d produced the value %d.\n",disastrOS_getpid(),to_produce);
	to_produce++;
	i_prod = (i_prod+1) % BUFFER_SIZE;
	
	disastrOS_sleep(10);
	
	//Process finish to product the value
	printf("[PRODUCER_INFO] Child N°%d finished to produce.\n", disastrOS_getpid());
	disastrOS_semPost(sem_cs_prod);
	disastrOS_semPost(sem_to_fill);
	
	disastrOS_printStatus();
}

void consumer() {
	
	printf("[CONSUMER_INFO] Child N°%d want to consume.\n", disastrOS_getpid());
	//Controlling if anyone is consuming
	disastrOS_semWait(sem_to_fill);
	disastrOS_semWait(sem_cs_cons);
	printf("[CONSUMER_INFO] Child N°%d now is consuming.\n", disastrOS_getpid());
	
	int val = buf[i_cons];
	printf("[CONSUMER_INFO] Consuming the value in cell %d...\n", i_cons);
	printf("[CONSUMER_INFO] Child N°%d consumed the value %d.\n", disastrOS_getpid(), val);
	i_cons++;
	
	disastrOS_sleep(10);
	
	//Process finish to consume the value
	printf("[CONSUMER_INFO] Child N°%d finished to consume.\n", disastrOS_getpid());
	disastrOS_semPost(sem_cs_cons);
	disastrOS_semPost(sem_to_empty);
	
	disastrOS_printStatus();
}

void childFunction(void* args){
	printf("Hello, I am the child function %d\n",disastrOS_getpid());
	printf("I will iterate a bit, before terminating\n");
	int type=0;
	int mode=0;
	int fd=disastrOS_openResource(disastrOS_getpid(),type,mode);
	printf("fd=%d\n", fd);
	
	//Opening semaphores
	printf("[CHILD_INFO] Opening semaphores (or creating if don't exist)...\n");
	sem_cs_prod = disastrOS_semOpen(1, 1);
	sem_cs_cons = disastrOS_semOpen(2, 1);
	sem_to_empty = disastrOS_semOpen(3, BUFFER_SIZE);
	sem_to_fill = disastrOS_semOpen(4, 0);


	for(int i = 1; i <= 10; i++){
		
		printf("[CHILD_INFO] PID: %d, iteration: %d...\n", disastrOS_getpid(), i);

		if(disastrOS_getpid() <= 5){
			printf("[CHILD_INFO] Role of child N°%d: producer.\n", disastrOS_getpid());
			producer();
		}
		else{
			printf("[CHILD_INFO] Role of child N°%d: consumer.\n", disastrOS_getpid());
			consumer();
		}
	
	}
	
	//Closing semaphores
	printf("Closing semaphores...\n");
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
