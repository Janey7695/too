#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include "too.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


void *threadpool_thread(void *arg);
void *admin_thread(void *arg);
int is_thread_alive(pthread_t tid);
void threadpool_free(threadpool_t *pool);


threadpool_t *threadpool_create(
	int min_thr_num,
	int max_thr_num,
	int queue_max_size
)
{
	int i;
	threadpool_t *pool = NULL;

	if((pool = tOO_MALLOC(threadpool_t, 1)) == NULL){
		DEBUG_ERROR("malloc threadpool faild in %s",__func__);
		return NULL;
	}
	pool->min_thr_num = min_thr_num;
	pool->max_thr_num = max_thr_num;
	pool->busy_thr_num = 0;
	pool->live_thr_num = min_thr_num;
	pool->wait_thr_num = 0;
	pool->queue_front = 0;
	pool->queue_rear = 0;
	pool->queue_size = 0;
	pool->queue_max_size = queue_max_size;

	pool->threads = tOO_MALLOC(pthread_t, max_thr_num);
	if(pool->threads == NULL){
		DEBUG_ERROR("malloc threads faild in %s",__func__);
		threadpool_free(pool);
		return NULL;
	}	
	memset(pool->threads, 0, sizeof(pthread_t)*max_thr_num);

	pool->task_queue = tOO_MALLOC(threadpool_task_t, queue_max_size);
	for (i=0; i<queue_max_size; i++) {
		pool->task_queue[i].taskfunction = NULL;
		pool->task_queue[i].arg = NULL;
	}
	if(pool->task_queue == NULL){
		DEBUG_ERROR("malloc task_queue faild in %s",__func__);
		threadpool_free(pool);
		return NULL;
	}

	if (pthread_mutex_init(&(pool->lock), NULL)!=0 || 
					pthread_mutex_init(&(pool->thread_counter), NULL)!=0||
					pthread_cond_init(&(pool->queue_not_empty), NULL)!=0||
					pthread_cond_init(&(pool->queue_not_full), NULL)!=0) {
		DEBUG_ERROR("init lock or cond faild in %s",__func__)	;
		threadpool_free(pool);
		return NULL;
	}
	for (i=0; i<min_thr_num; i++) {
		pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void *)pool);
		DEBUG_WARN("starting thread 0x%x ...",(unsigned int)pool->threads[i]);
	}
	pthread_create(&(pool->admin_tid), NULL, admin_thread, (void *)pool);
	return pool;
}

int threadpool_destroy(threadpool_t *pool)
{
	int i;
	if (pool == NULL) {
		return -1;
	}
	pool->shutdown = 1;
	pthread_join(pool->admin_tid, NULL);
	for (i=0; i<pool->live_thr_num; i++) {
		pthread_cond_broadcast(&(pool->queue_not_empty));
	}
	for (i=0; i<pool->live_thr_num; i++) {
		pthread_join(pool->threads[i], NULL);
	}
	threadpool_free(pool);
	return 0;
}

void *threadpool_thread(void *arg)
{
	threadpool_t *pool = (threadpool_t *)arg;
	threadpool_task_t task;
	while(1){
		pthread_mutex_lock(&(pool->lock));

		
		while ((pool->queue_size == 0) && (!pool->shutdown)) {
			DEBUG_WARN("not task in the queue,thread 0x%x is waiting",(unsigned int)pthread_self());
			pthread_cond_wait(&(pool->queue_not_empty), &(pool->lock));

			if(pool->wait_thr_num > 0){
				pool->wait_thr_num--;

				if (pool->live_thr_num > pool->min_thr_num) {
					DEBUG_WARN("thread 0x%x is exiting",(unsigned int)pthread_self());
					pool->live_thr_num--;
					pthread_mutex_unlock(&(pool->lock));
					pthread_exit(NULL);
				}

			}

		}	

		if (pool->shutdown) {
			pthread_mutex_unlock(&(pool->lock));
			DEBUG_WARN("pool will be shutdown,thread 0x%x is exiting",(unsigned int)pthread_self());
			pthread_exit(NULL);
		}
		
		task.taskfunction = pool->task_queue[pool->queue_front].taskfunction;
		task.arg = pool->task_queue[pool->queue_front].arg;
		pool->queue_front = (pool->queue_front + 1) % pool->queue_max_size;
		pool->queue_size--;
		pthread_cond_broadcast(&(pool->queue_not_full));
		pthread_mutex_unlock(&(pool->lock));

		DEBUG_NORMAL("thread 0x%x start working",(unsigned int)pthread_self());
		pthread_mutex_lock(&(pool->thread_counter));
		pool->busy_thr_num++;
		pthread_mutex_unlock(&(pool->thread_counter));

		(*(task.taskfunction))(task.arg);

		DEBUG_SUCCESS("thread 0x%x finish work",(unsigned int)pthread_self());
		pthread_mutex_lock(&(pool->thread_counter));
		pool->busy_thr_num--;
		pthread_mutex_unlock(&(pool->thread_counter));
		
	}
	pthread_exit(NULL);
}

int threadpool_add_task(
	threadpool_t *pool,
	void *(*taskfunction)(void *arg),
	void *arg
)
{
	pthread_mutex_lock(&(pool->lock));
	
	while ((pool->queue_size == pool->queue_max_size) && (!pool->shutdown)) {
		pthread_cond_wait(&(pool->queue_not_full), &(pool->lock));
	}	
	
	if (pool->shutdown) {
		pthread_mutex_unlock(&(pool->lock));
		return -1;
	}

	if (pool->task_queue[pool->queue_rear].arg!=NULL) {
		//free(pool->task_queue[pool->queue_rear].arg);
		pool->task_queue[pool->queue_rear].arg = NULL;
	}

	pool->task_queue[pool->queue_rear].taskfunction = taskfunction;
	pool->task_queue[pool->queue_rear].arg = arg;
	pool->queue_rear = (pool->queue_rear + 1) % pool->queue_max_size;
	pool->queue_size++;
	pthread_cond_signal(&(pool->queue_not_empty));
	pthread_mutex_unlock(&(pool->lock));
	return 0;
}

void *admin_thread(void *arg)
{
	int i;
	threadpool_t *pool = (threadpool_t *)arg;
	while (!pool->shutdown) {
		sleep(1);

		DEBUG_NORMAL("admin thread -----");
//		pthread_mutex_lock(&(pool->lock));
		int queue_size = pool->queue_size;
		int live_thr_num = pool->live_thr_num;
//		pthread_mutex_unlock(&(pool->lock));
		
		pthread_mutex_lock(&(pool->thread_counter));
		int busy_thr_num = pool->busy_thr_num;
		pthread_mutex_unlock(&(pool->thread_counter));

		DEBUG_SUCCESS("Current live %d threads,busy %d threads,%d task in the queue",live_thr_num,busy_thr_num,queue_size);

		if (queue_size >= MIN_WAIT_TASK_NUM && live_thr_num <= pool->max_thr_num) {
			DEBUG_WARN("admin will add thread");
			pthread_mutex_lock(&(pool->lock));
			int add = 0;
			for (i=0; i<pool->max_thr_num && add < DEFAULT_THREAD_NUM && pool->live_thr_num < pool->max_thr_num; i++) {
				if (pool->threads[i]==0 || !is_thread_alive(pool->threads[i])) {
					pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void *)pool);
					add++;
					pool->live_thr_num++;

				}	
			}
			pthread_mutex_unlock(&(pool->lock));
		}
		if ((busy_thr_num*2)<live_thr_num && live_thr_num > pool->min_thr_num) {
			DEBUG_SUCCESS("Current live %d threads,busy %d threads",live_thr_num,busy_thr_num);
			pthread_mutex_lock(&(pool->lock));
			pool->wait_thr_num = DEFAULT_THREAD_NUM;
			pthread_mutex_unlock(&(pool->lock));
			
			for (i=0; i<DEFAULT_THREAD_NUM; i++) {
				pthread_cond_signal(&(pool->queue_not_empty))	;
				DEBUG_WARN("admin send delete signal to the free thread");
			}
		}
	}
	return NULL;
}

void threadpool_free(threadpool_t *pool)
{
	if(pool->task_queue) free(pool->task_queue);
	if(pool->threads){
		free(pool->threads);
		pthread_mutex_lock(&(pool->lock));
		pthread_mutex_destroy(&(pool->lock));
		pthread_mutex_lock(&(pool->thread_counter));
		pthread_mutex_destroy(&(pool->thread_counter));

		pthread_cond_destroy(&(pool->queue_not_empty));
		pthread_cond_destroy(&(pool->queue_not_full));
		
	} 
	if(pool) free(pool);
	pool = NULL;
}

int is_thread_alive(pthread_t tid)
{
	int kill_rc = pthread_kill(tid, 0);
	if (kill_rc == ESRCH) {
		return 0;
	}
	return 1;
}

