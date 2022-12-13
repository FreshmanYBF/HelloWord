#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

/*
*		Test volatile 
* This code can't show the volatile effect
*/
pthread_mutex_t mutex;
int x = 0;
void* addOne(void *args)
{
	pthread_mutex_lock(&mutex);
	x++;
	printf("------------> x = %d\r\n", x);
	pthread_mutex_unlock(&mutex);
	return args;
}

void test_volatile()
{
	pthread_t tid = -1;
	pthread_mutex_init(&mutex, NULL);
	for (int i = 0; i < 30000; ++i){
		pthread_create(&tid, NULL, addOne, NULL);
		pthread_join(tid, NULL);
	}
}	

/*
*		Test pthread_key_t 
* pthread_key_t as a global index, but different threads can set/get this own value by same key
* just like every thread has a personal KV DataBase, even if not so big
*/
pthread_key_t key;

void* thread1(void* args)
{
	int value1 = 1;
	void* ptr = pthread_getspecific(key);
	if (!ptr){
		pthread_setspecific(key, &value1);
	}
	ptr = pthread_getspecific(key);
	if (!ptr){
		printf("------------->thread1 pthread_setspecific invalid\r\n");
	}
	else{
		printf("------------->thread1 get value %d\r\n", *((int*)ptr));
	}

	ptr = pthread_getspecific(key);
	if (!ptr){
		printf("------------->thread1 pthread_setspecific invalid\r\n");
	}
	else{
		printf("------------->thread1 get value %d\r\n", *((int*)ptr));
	}

	ptr = pthread_getspecific(key);
	if (!ptr){
		printf("------------->thread1 pthread_setspecific invalid\r\n");
	}
	else{
		printf("------------->thread1 get value %d\r\n", *((int*)ptr));
	}

	return NULL;
}

void* thread2(void* args)
{
	int value2 = 2;
	void* ptr = pthread_getspecific(key);
	if (!ptr){
		pthread_setspecific(key, &value2);
	}
	ptr = pthread_getspecific(key);
	if (!ptr){
		printf("------------->thread2 pthread_setspecific invalid\r\n");
	}
	else{
		printf("------------->thread2 get value %d\r\n", *((int*)ptr));
	}
	return NULL;
}

void* thread3(void* args)
{
	void* ptr = pthread_getspecific(key);
	if (!ptr){
		printf("------------->thread3 pthread_setspecific invalid\r\n");
	}
	else{
		printf("------------->thread3 get value %d\r\n", *((int*)ptr));
	}
	return NULL;
}

void test_pthread_key_t()
{
	pthread_t tid = -1;
	pthread_key_create(&key, NULL);
	
	pthread_create(&tid, NULL, thread1, NULL);
	pthread_join(tid, NULL);

	pthread_create(&tid, NULL, thread2, NULL);
	pthread_join(tid, NULL);

	pthread_create(&tid, NULL, thread3, NULL);
	pthread_join(tid, NULL);

	sleep(10);
	pthread_key_delete(key);
}


int main()
{
	
	return 0;
}
