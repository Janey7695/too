#ifndef _1_H_
#define  _1_H_
#include <pthread.h>


#define MIN_WAIT_TASK_NUM 4
#define DEFAULT_THREAD_NUM 2

typedef struct {
	void *(*taskfunction)(void *);
	void *arg;
}threadpool_task_t;

typedef struct {
	
	/* thread pool manage*/
	pthread_mutex_t lock;
	pthread_mutex_t thread_counter;
	pthread_cond_t queue_not_full;
	pthread_cond_t queue_not_empty;
	pthread_t *threads;
	pthread_t admin_tid;
	threadpool_task_t *task_queue;
	/*thread pool information*/
	int min_thr_num;
	int max_thr_num;
	int live_thr_num;
	int busy_thr_num;
	int wait_thr_num;
	/*task queue information*/
	int queue_front;
	int queue_rear;
	int queue_size;
	int queue_max_size;
	
	int shutdown;
	
}threadpool_t;

threadpool_t *threadpool_create(int min_thr_num,int max_thr_num,int queue_max_size);
int threadpool_add_task(threadpool_t *pool,void *(*taskfunction)(void *arg),void *arg);
int threadpool_destroy(threadpool_t *pool);

#endif
