
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

/*

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>


mqd_t mq_open(const char *name, int oflag);
mqd_t mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr);

mq_open() 创建一个POSIX消息队列，或者打开一个存在的队列。 以“name”标识。
----->mq_overview()

oflag
    O_RDONLY   打开队列，仅从中接收消息
	O_WRONLY   打开队列，仅向中发送消息
	O_RDWR     可发也可收
	O_CLOEXEC  close-on-exec
	O_CREAT    创建一个消息队列。（UserID为调用进程ID，groupID为调用进程group）
	O_EXCL     和O_CREAT一起使用，如果队列已经存在则报错EEXIST
	O_NONBLOCK 一般mq_receive和mq_send是阻塞，使用这个则可以不阻塞

mq_attr
    struct mq_attr{
		long mq_flags;
		long mq_maxmsg;  //
		long mq_msgsize;  //
		long mq_curmsgs;  //
	};
	mq_open时，仅mq_maxmsg和mq_msgsize有用，剩余的被忽略


mq_close
mq_getattr
mq_notify
mq_open
mq_receive
mq_send
mq_setattr
mq_timedreceive
mq_timedsend
mq_unlink



#include <mqueue.h>]
#include <time.h>

ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);
ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio, const struct timespec *abs_timeout);

队列把最老的优先级最高的消息抛出

	
mqd_t mq_open

*/

int main()
{
	struct mq_attr attr;
	attr.mq_maxmsg = 5;
	attr.mq_msgsize = 48;
	
	mode_t mt = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	
	int ret = mq_unlink("/mqtest");
	if (ret == 0) printf("unlink successful\r\n");
	#if 0
	mqd_t qt = mq_open("/mqtest", O_RDWR | O_CREAT | O_EXCL, mt, &attr);
	
	if (qt < 0){
		printf("create mqtest failed, %d - %s\r\n", errno, strerror(errno));
	}
	#endif
	
	printf("I am OK\r\n");
	return 0;
}
