#include "too.h"
#include <stdio.h>
#include <unistd.h>

void do_work(void *arg){
	int number = *(int *)arg;
	printf("hello %d \r\n",number);
	//printf("hello  \r\n");
}


int work_number = 5;
int main(){
	threadpool_t *thp = threadpool_create(4, 10, 10);

	while(work_number>0){
		threadpool_add_task(thp, do_work, &work_number);
		work_number--;
		printf("hey!\r\n");
	}
	while(1);
	return 0;
}
