
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


typedef pthread_mutex_t mutex_t;
typedef pthread_cond_t condition_t;


/* 互斥量 */
#if 0
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);
int pthread_mutex_lock(pthtread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);

//互斥量默认属性是fast，但是有一个缺点：
//			如果程序试图对一个已经加了锁的互斥量调用pthread_mutex_lock，程序就会阻塞
//			而拥有互斥量的这个线程就是现在被阻塞的线程，所以互斥量永远不会被解锁，程序就进入死锁状态
//			可以通过改变互斥量的属性来解决：（1）让它检查这种情况并返回一个错误（2）使用递归属性
#endif

/* 条件变量 */

#if 0
int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *cond_attr);
 
int pthread_condattr_init(pthread_condattr_t *condattr);
int pthread_condattr_setclock(pthread_condattr_t *condattr, int clock); //CLOCK_MONOTONIC(单调的无变化的)
 
int pthread_cond_signal(pthread_cond_t *cond);

//通知状态发生变化，并不一定是条件满足
int pthread_cond_broadcast(pthread_cond_t *cond);

//等待
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

//等待超时
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);

//销毁
int pthread_cond_destroy(pthread_cond_t *cond);

#endif


/* 线程属性 */ 
#if 0
int pthread_attr_init(pthread_attr_t *attr);	//初始化一个线程对象
int pthread_attr_destroy(pthread_attr_t *attr);

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
int pthread_attr_getdetachstate(pthread_attr_t *attr, int *detachstate);

//控制线程调度方式。他的取值可以是SCHED_OTHER，SCHED_RP（循环）和SCHED_FIFO（先进先出）
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);	//设置优先级
int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy);	//设置优先级

//
int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param);
int pthread_attr_getschedparam(pthread_attr_t *attr, struct sched_param *param);

int sched_get_priority_max( int policy );
int sched_get_priority_min( int policy );

int pthread_setaffinity_np(pthread_t thread, size_t cpusetsize，const cpu_set_t *cpuset);
int pthread_getaffinity_np(pthread_t thread, size_t cpusetsize, cpu_set_t *cpuset);
从函数名以及参数名都很明了，唯一需要点解释下的可能就是cpu_set_t这个结构体了。这个结构体的理解类似于select中的fd_set，可以理解为cpu集，也是通过约定好的宏来进行清除、设置以及判断：
//初始化，设为空
void CPU_ZERO (cpu_set_t *set); 
//将某个cpu加入cpu集中 
void CPU_SET (int cpu, cpu_set_t *set); 
//将某个cpu从cpu集中移出 
void CPU_CLR (int cpu, cpu_set_t *set); 
//判断某个cpu是否已在cpu集中设置了 
int CPU_ISSET (int cpu, const cpu_set_t *set);

//PTHREAD_EXPLICIT_SCHED（由调度属性明确地设置）和PTHREAD_INHERIT_SCHED（沿用创建者锁使用的属性）
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inherit);
int pthread_attr_getinheritsched(pthread_attr_t *attr, int *inherit);

//线程调度计算方式
//PTHREAD_SCOPE_SYSTEM
int pthread_attr_setscope(pthread_attr_t *attr, int scope);
int pthread_attr_getscope(pthread_attr_t *attr, int *scope);

//控制线程创建的栈大小
int pthread_attr_setstacksize(pthread_attr_t *attr, int scope);
int pthread_attr_getstacksize(pthread_attr_t *attr, int *scope);

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg);

#endif

/* 启动新的进程 */
#if 0
//替换进程映像
//把当前进程替换为一个新进程
char **environ;  //可用来把一个值传递到新的程序环境中
int execl(const char *path, const char *arg0, ..., (char*)0); // 参数个数可变，以一个空指针结束
int execlp(const char *file, const char *arg0, ..., (char *)0);  //通过搜索环境变量PATH来查找新程序的可执行路径
int execle(const char *path, const char *arg0, ..., (char *)0, char *const envp[]);  //

int execv(const char *path, char *const argv[]);  //argv会传递给main函数
int execvp(const char *file, char *const argv[]);
int execve(const char *path, char *const argv[], char *const envp[]);

//复制进程映像
pid_t fork(void);  //复制当前进程，在进程表中创建一个新的表项，新表项中的许多属性与当前进程是相同的。
					//返回字进程的PID

//等待一个进程
pid_t wait(int *stat_loc);	//暂停父进程，直到它的子进程结束。返回子进程的PID
							//当子进程结束，其表项不会被立即释放，直到父进程结束或调用wait才会释放。

//等待某个特定进程结束
//WOHANG：防止调用者执行挂起
pid_t waitpid(pid_t pid, int *stat_loc, int options);	
	//如果子进程没有结束或意外终止，就返回0；否则返回子进程的PID
	waitpid(child_pid, (int*)0, WNOHANG);

#endif



char *gmem = NULL;
int gcount = 0;
pthread_cond_t gcond;
pthread_mutex_t gmutex;
pthread_mutex_t gcountmutex;

void init_env()
{
	//初始化cond
	pthread_condattr_t condattr;
	pthread_condattr_init(&condattr);
	pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
	pthread_cond_init(&gcond, &condattr);
	pthread_condattr_destroy(&condattr);
	
	//初始化互斥量
	pthread_mutexattr_t mutexattr;
	pthread_mutexattr_init(&mutexattr);
	
	pthread_mutex_init(&gmutex, &mutexattr);
	pthread_mutexattr_destroy(&mutexattr);
	
	//初始化互斥量
	pthread_mutexattr_t mutexattr2;
	pthread_mutexattr_init(&mutexattr2);
	
	pthread_mutex_init(&gcountmutex, &mutexattr2);
	pthread_mutexattr_destroy(&mutexattr2);
	
}

int get_gmem(char **pgmem)
{
	pthread_mutex_lock(&gmutex);
	if (gmem != NULL){
		*pgmem = gmem;
		pthread_mutex_unlock(&gmutex);
		return 0;
	}
	pthread_mutex_unlock(&gmutex);
	return -1;
}

int add_count()
{
	pthread_mutex_lock(&gcountmutex);
	++gcount;
	pthread_mutex_unlock(&gcountmutex);
}

int sub_count()
{
	pthread_mutex_lock(&gcountmutex);
	--gcount;
	printf("------------------->gcount = %d\n", gcount);
	pthread_mutex_unlock(&gcountmutex);
}

int zero_count()
{
	pthread_mutex_lock(&gcountmutex);
	if (gcount == 0){
		pthread_mutex_unlock(&gcountmutex);
		return 1;
	}
	pthread_mutex_unlock(&gcountmutex);
	return 0;
}


void *master_task(void *arg)
{
	//上锁
	pthread_mutex_lock(&gmutex);
	gmem = (char*)malloc(256);
	snprintf(gmem, 256, "I am avaliable\r\n");
	//等待
	pthread_cond_wait(&gcond, &gmutex);
	//释放
	free(gmem);
	gmem = NULL;
	printf("----------------->free gmem\n");
	pthread_mutex_unlock(&gmutex);
}

void *slave_task(void *arg)
{
	add_count();
	char *pgmem = NULL;
	int ret = 0;
	ret = get_gmem(&pgmem);
	if (ret == 0){
		printf("I am slave_task, gmem = %s\n", pgmem);
	}
	sleep(2);
	sub_count();
	if (zero_count()){
		pthread_cond_signal(&gcond);
	}
}

void start_ps()
{
	char *const ps_argv[] = {"ps", "ax", 0};
	
	
	char *const ps_envp = {"PATH=/bin:/usr/bin", "TERM=console", 0};
	
	execl("/bin/ps", "ps", "ax", 0);
	execlp("ps", "ps", "ax", 0);
	execle("/bin/ps", "ps", "ax", 0, ps_envp);
	
	execv("/bin/ps", ps_argv);
	execvp("ps", ps_argv);
	execve("/bin/ps", ps_argv, ps_envp);
}


int main()
{
	pthread_t tidm;
	pthread_t tids;
	void *retval = NULL;
	init_env();
	
	pthread_attr_t threadattr;
	pthread_attr_init(&threadattr);
	pthread_attr_setdetachstate(&threadattr, 0);
	
	pthread_create(&tidm, &threadattr, master_task, NULL);
	printf("--------------------->tidm = %ld\n", tidm);
	for (int i = 0; i < 100; ++i){
		pthread_create(&tids, &threadattr, slave_task, NULL);
	}
	pthread_attr_destroy(&threadattr);
	pthread_join(tidm, &retval);
	
	return 0;
}
