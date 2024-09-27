#include <fcntl.h>
//======================================================//
//					共享文件IPC通信
//======================================================//
//使用文件作为IPC通信的媒介

//第一种： 以原子操作的方式创建“锁文件”——在创建锁文件时，系统将不允许任何其他事情发生。以这种方式来确保它所创建的文件是唯一的。
// 锁文件只是“建议锁”，充当指示器的作用，程序通过判断锁文件的状态指示来操作文件，并不是强制锁
// 锁文件通常被放置在一个特定位置，并带有一个与被控制资源相关的文件名
//系统调用对于用户来说是原子操作，其内部存在全局锁
int ipc_create_lock_file(const char *lock_file)
{
	return open(lock_file, O_RDWR | O_CREAT | O_EXCL, 0777);
}

int ipc_delete_lock_file(const char *lock_file, int fd)
{
	(void)close(fd);
	(void)unlink(lock_file);
}

//第二种： 允许锁定文件的一部分，从而可以独享对这一部分内容的访问
// 文件段锁定/文件区域锁定
// Linux提供了两种系统调用实现：fcntl和lockf

/*
#include <fcntl.h>

int fcntl(int fildes, int command, ...);

	command:
		F_GETLK
		F_SETLK
		F_SETLKW - 等待，直到获取锁
		//当使用这些命令时，第三个参数必须是一个指向flock结构的指针
		// int fcntl(int fd, int command, struct flock *flock_structure);
		flock：
			short l_type		F_RDLCK：共享锁（读锁）；F_UNLCK：解锁；F_WRLCK：独占锁（写锁）
			short l_whence		SEEK_SET，SEEK_CUR，SEEK_END
			short l_start
			short l_len
			short l_pid			记录持有锁的进程
	文件中的每个字节在任一时刻只能拥有一种类型的锁
*/
// F_GETLK
//获取文件的flock信息
//成功返回非-1；失败返回-1
int f_getlock(int fd, struct flock *flock_p)
{
	return fcntl(fd, F_GETLK, flock_p);
}

int f_getrdlock(int fd, short whence, short start, short len, struct flock *flock_p)
{
	flock_p->l_type = F_RDLCK;
	flock_p->l_whence = whence;
	flock_p->l_start = start;
	flock_p->l_len = len;
	return f_getlock(fd, flock_p);
}

int f_getwrlock(int fd, short whence, short start, short len)
{
	return 0;
}

int f_getunlock(int fd, short whence, short start, short len)
{
	return 0;
}

//F_SETLK
//对文件的某个区域加锁或解锁
//当无法获取锁时，这个调用将等待直到可以为止
int f_rdlock(int fd, short whence, short start, short len)
{
	int ret = -1;
	struct flock st_flock;
	ret = f_get_flock(fd,&st_flock);
	if (-1 == ret){
		return ret;
	}
	
	st_flock.l_type = F_RDLCK;
	st_flock.l_whence = whence;
	st_flock.l_start = start;
	st_flock.l_len = len;
	
	ret = fcntl(fd, F_SETLK, &st_flock);
	return ret;
}

int f_wrlock(int fd, short whence, short start, short len)
{
	int ret = -1;
	struct flock st_flock;
	ret = f_get_flock(fd,&st_flock);
	if (-1 == ret){
		return ret;
	}
	
	st_flock.l_type = F_WRLCK;
	st_flock.l_whence = whence;
	st_flock.l_start = start;
	st_flock.l_len = len;
	
	ret = fcntl(fd, F_SETLK, &st_flock);
	return ret;
}

int f_unlock(int fd, short whence, short start, short len)
{
	int ret = -1;
	struct flock st_flock;
	ret = f_get_flock(fd, &st_flock);
	if (-1 == ret){
		return ret;
	}
	
	st_flock.l_type = F_UNLCK;
	
	ret = fcntl(fd, F_SETLK, &st_flock);
	return ret;
}

//F_SETLKW
int f_rdlockw(int fd, short whence, short start, short len)
{
	int ret = -1;
	struct flock st_flock;
	ret = f_get_flock(fd,&st_flock);
	if (-1 == ret){
		return ret;
	}
	
	st_flock.l_type = F_RDLCK;
	st_flock.l_whence = whence;
	st_flock.l_start = start;
	st_flock.l_len = len;
	
	ret = fcntl(fd, F_SETLKW, &st_flock);
	return ret;
}

int f_wrlockw(int fd, short whence, short start, short len)
{
	int ret = -1;
	struct flock st_flock;
	ret = f_get_flock(fd,&st_flock);
	if (-1 == ret){
		return ret;
	}
	
	st_flock.l_type = F_WRLCK;
	st_flock.l_whence = whence;
	st_flock.l_start = start;
	st_flock.l_len = len;
	
	ret = fcntl(fd, F_SETLKW, &st_flock);
	return ret;
}

int f_unlockw(int fd, short whence, short start, short len)
{
	int ret = -1;
	struct flock st_flock;
	ret = f_get_flock(fd, &st_flock);
	if (-1 == ret){
		return ret;
	}
	
	st_flock.l_type = F_UNLCK;
	
	ret = fcntl(fd, F_SETLKW, &st_flock);
	return ret;
}



int main()
{
	int  ret = -1;
	struct flock st_flock;
	
	//锁定状态下的读写操作
	//当对文件区域加锁之后，必须使用底层的read和write调用来访问文件中的数据
	//而不要使用更高级的fread和fwrite，因为fread和fwrite会缓存数据。例如一次读取100个字节，但是fwrite一次读取了BUFFSIZE个字节放在缓存区，然后只返回100个字节
	
	
	
	
	return 0;
}
