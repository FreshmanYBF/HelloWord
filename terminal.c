#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
//对终端进行读写
	//标准模式和非标准模式
	//	标准模式：用户按下回车键后，程序才能读到终端的输入。此时可以使用退格键或删除键来纠正输入中的错误
	//	非标准模式：应用程序对用户输入字符的处理拥有更大的权限
	
	//Linux终端处理程序能够把中断字符转换成对应的信号从而自动替用户完成对退格键和删除键的处理
	//Linux用一个单独的“换行符”（LF，10）来表示一行的结束；其他操作系统用“回车符”（CR，13）和“换行符”（LF，10）两个字符的结合来表示一行的结束。
	
	//如果想知道标准输出是否被重定向，只需要检查底层文件描述符是否关联到一个终端即可
	
	// /dev/tty始终指向当前终端或当前的登录会话

//关联返回1，不关联返回0——是否管理到一个终端
int tc_is_associate_tty(int fd)
{
	// #include <unistd.h>
	// int isatty(int fd);
	//如果fd连接到一个终端则返回1，否则返回0
	return isatty(fd);
}




//终端驱动程序和通用终端接口
//Linux提供了一组编程接口用来控制终端驱动程序的行为，从而使得更好地控制终端的输入和输出
	//	可以通过一组函数调用（通用终端接口，GTI，General Terminal Interface）来控制终端
	//	可控制的功能：
		//		（1）行编辑：是否允许用退格键进行编辑
		//		（2）缓存：是立即读取字符，还是等待一段可配置的延迟之后再读取它们
		//		（3）回显：允许控制字符的回显，例如读取密码时
		//		（4）回车/换行：定义如何在输入/输出时映射回车/换行，比如打印\n字符时应该如何处理
		//		（5）线速：用于调制解调器或通过串行线连接的终端

//termios
	//通过设置termios类型的数据结构中的值和使用一小组函数调用，就可以对终端接口进行控制
	//	termios调用定义在termios.h头文件中，通常需要链接到curses或其他标准函数库
	//	termios可以被调整来影响终端的值按照不同的模式被分成如下几组
		// 输入模式：
		// 输出模式：
		// 控制模式：
		// 本地模式：
		// 特殊控制模式：
		
	// #include <termios.h>
	// struct termios{
	//		tcflag_t c_iflag;
	//		tcflag_t c_oflag;
	//		tcflag_t c_cflag;
	//		tcflag_t c_lflag;
	//		cc_t     c_cc[NCCS];
	//}


#define TCSA_NOW TCSANOW
#define TCSA_DRAIN TCSADRAIN
#define TCSA_FLUSH TCSAFLUSH
// int tcgetattr(int fd, struct termios *termios_p);
// int tcsetattr(int fd, int actions, const struct termios *termios_p);
	// actions 控制修改方式
		// TCSANOW：立刻对值进行修改
		// TCSADRAIN：等当前的输出完成后再对值进行修改
		// TCSAFLUSH：等当前的输出完成后再对 值进行修改，但丢弃还未从read调用返回的当前可用的任何输出
//0成功，-1失败
int tc_get_fdAterm_attr(int fd, struct termios *termios_p){
	return tcgetattr(fd, termios_p);
}

int tc_set_fdAterm_attr(int fd, int actions, const struct termios *termios_p){
	return tcsetattr(fd, actions, termios_p);
}

//设置输入模式
//	输入模式控制输入数据（终端控制程序从串口或键盘接收到的字符）在被传递给程序之前的处理方式。
	//通过设置c_iflag进行控制，所有标志都被定义为宏，并可通过按位与或的方式来结合起来
	// BRKINT：当在输入行中检测到一个终止状态（连接丢失）时，产生一个中断。BROKEN_INTERRUPT
	// IGNBRK：忽略输入行中的终止状态。IGNORE_BROKEN
	// ICRNL：将接收到的回车符转换为新行符。INPUT_CR_TO_NEWLINE
	// IGNCR：忽略接收到的回车符。IGNORE_CR
	// INLCR：将接收到的新行符转换为回车符。INPUT_NEWLINE_TO_CR
	// IGNPAR：忽略奇偶校验错误的字符。一串二进制串中出现奇偶校验错误，此时知道错误的位置。IGNORE_PARITY_CHECK
	// INPCK：对接收到的字符执行奇偶校验。INPUT_PARITY_CHECK
	// PARMRK：对奇偶校验错误做出标记。PARITY_CHECK_MARK
	// ISTRIP：将所有接收到的字符裁剪为7比特。INPUT_STRIP_7BIT
	// IXOFF：对输入启用软件流控。INPUT_FLOW_CONTROL_OFF
	// IXON：对输出启用软件流控。INPUT_FLOW_CONTROL_ON
	// 如果BRKINT和IGNBRK标志都未被设置，则输入中的终止状态就被读取为NULL字符。
#define INPUT_BROKEN_INTERRUPT 			BRKINT
#define INPUT_IGNORE_BROKEN 			IGNBRK
#define INPUT_CR_TO_NEWLINE 			ICRNL
#define INPUT_IGNORE_CR 				IGNCR
#define INPUT_NEWLINE_TO_CR 			INLCR
#define INPUT_IGNORE_PARITY_WRONG_CHAR 	IGNPAR
#define INPUT_PARITY_CHECK 				INPCK
#define INPUT_PARITY_WRONG_MARK 		PARMARK
#define INPUT_STRIP_7BIT 				ISTRIP
#define INPUT_XOFF 						IXOFF
#define INPUT_XON 						IXON
int tc_set_fdAterm_imode(int fd, int actions, int imode){
	struct termios st_termios;
	int ret = 0;
	ret = tc_get_fdAterm_attr(fd, &st_termios);
	if (ret != 1){
		//
		return ret;
	}
	
	st_termios.c_iflag |= imode;
	
	ret = tc_set_fdAterm_attr(fd, actions, &st_termios);
	return ret;
}

int tc_clear_fdAterm_imode(int fd, int actions, int imode){
	struct termios st_termios;
	int ret = 0;
	ret = tc_get_fdAterm_attr(fd, &st_termios);
	if (ret != 1){
		//
		return ret;
	}
	
	st_termios.c_iflag &= ~imode;
	
	ret = tc_set_fdAterm_attr(fd, actions, &st_termios);
	return ret;
}



//设置输出模式
//输出模式控制输出字符的处理方式，即由程序发送出去的字符在传递到串行口或屏幕之前是如何处理的
// 其许多处理方式正好与输入模式对应
// 其他几个标志，主要用于慢速终端，因为这些终端在处理回车符等字符时需要花费一定的时间
//		目前来说终端的速度比以前快很多，且使用terminfo数据库处理会更有效
	// OPOST：打开输出处理功能
	// ONLCR：将输出中的换行符转换为回车/换行符
	// OCRNL：将输出中的回车符转换为新行符
	// ONOCR：在第0列不输出和回车符
	// ONLRET：不输出回车符
	// OFILL：发送填充字符以提供延时
	// OFDEL：用DEL而不是NULL字符作为填充字符
	// NLDLY：新行符延时选择
	// CRDLY：回车符延时选择
	// TABDLY：制表符延时选择
	// BSDLY：退格符延时选择
	// VTDLY：垂直制表符延时选择
	// FFDLY：换页符延时选择
	// 如果没有设置OPOST，则所有其他标志都被忽略
#define OUTPUT_POST 				OPOST
#define OUTPUT_NEWLINE_TO_CR 		ONLCR
#define OUTPUT_CR_TO_NEWLINE 		OCRNL 
#define OUTPUT_NO_CR 				ONOCR 
#define OUTPUT_NEWLINE_RETURN 		ONLRET 
#define OUTPUT_FILL 				OFILL 
#define OUTPUT_FILL_DEL 			OFDEL 
#define OUTPUT_CR_DELAY 			CRDLY 
#define OUTPUT_NEWLINE_DELAY 		NLDLY 
#define OUTPUT_TABLE_DELAY 			TABDLY 
#define OUTPUT_BACK_SPACE_DELAY 	BSDLY 
#define OUTPUT_VERTICAL_DELAY 		VTDLY 
#define OUTPUT_PAGE_FEED_DELAY 		FFDLY 
int tc_set_fdAterm_omode(int fd, int actions, int omode){
	struct termios st_termios;
	int ret = 0;
	ret = tc_get_fdAterm_attr(fd, &st_termios);
	if (ret != 1){
		//
		return ret;
	}
	
	st_termios.c_oflag |= omode;
	
	ret = tc_set_fdAterm_attr(fd, actions, &st_termios);
	return ret;
}

int tc_clear_fdAterm_omode(int fd, int actions, int omode){
	struct termios st_termios;
	int ret = 0;
	ret = tc_get_fdAterm_attr(fd, &st_termios);
	if (ret != 1){
		//
		return ret;
	}
	
	st_termios.c_oflag &= ~omode;
	
	ret = tc_set_fdAterm_attr(fd, actions, &st_termios);
	return ret;
}


//设置控制模式
//控制模式控制终端的硬件特性
// 可控制的特性如下
	// CLOCAL：忽略所有调制解调器的状态行
	// CREAD：启用字符接收器
	// CS5：发送或接收字符时使用5比特
	// CS6：发送或接收字符时使用6比特
	// CS7：发送或接收字符时使用7比特
	// CS8：发送后接收字符时使用8比特
	// CSTOPB：每个字符使用两个停止位而不是一个
	// HUPCL：关闭时挂断调制解调器
	// PARENB：启用奇偶校验码的生成和检测功能
	// PARODD：使用奇校验而不是偶校验
	// 如果设置了HUPCL标志，当终端驱动程序检测到与终端对应的最后一个文件描述符被关闭时，它将通过设置调制解调器的而控制线来挂断电话线路

#define CTRL_IGNORE_LOCAL 			CLOCAL 
#define CTRL_READ_ON 				OREAD
#define CTRL_RCVSND_5BITS 			CS5
#define CTRL_RCVSND_6BITS 			CS6 
#define CTRL_RCVSND_7BITS 			CS7 
#define CTRL_RCVSND_8BITS 			CS8 
#define CTRL_STOP_2BITS 			CSTOPB 
#define CTRL_HUNG_UP_TERM 			HUPCL 
#define CTRL_PARITY_NB 				PARENB 
#define CTRL_PARITY_ODD 			PARODD 
int tc_set_fdAterm_cmode(int fd, int actions, int cmode){
	struct termios st_termios;
	int ret = 0;
	ret = tc_get_fdAterm_attr(fd, &st_termios);
	if (ret != 1){
		//
		return ret;
	}
	
	st_termios.c_iflag |= cmode;
	
	ret = tc_set_fdAterm_attr(fd, actions, &st_termios);
	return ret;
}

int tc_clear_fdAterm_cmode(int fd, int actions, int cmode){
	struct termios st_termios;
	int ret = 0;
	ret = tc_get_fdAterm_attr(fd, &st_termios);
	if (ret != 1){
		//
		return ret;
	}
	
	st_termios.c_iflag &= ~cmode;
	
	ret = tc_set_fdAterm_attr(fd, actions, &st_termios);
	return ret;
}




//设置本地模式
// 本地模式控制终端的各种特性
// 本地模式控制包括
	// ECHO：启用输入字符的本地回显功能
	// ECHOE：接收到ERASE时执行退格、空格、退格的动作组合
	// ECHOK：接收到KILL字符时执行行删除操作
	// ECHONL：回显新行符
	// ICANON：启用标准输入处理
	// IEXTEN：启用基于特定实现的函数
	// ISIG：启用信号
	// NOFLSH：禁止清空队列
	// TOSTOP：在试图进行写操作之前给后台发送进程发送一个信号
	// 最重要的两个标志：ECHO和ICANON。前者的作用是抑制键入字符的回显，而后者是将终端在两个截然不同的接收字符处理模式间进行切换。
	//									如果设置了ICANON标志，就启用标准输入行处理模式，否则，就启用非标准模式
	//									Canonical mode 规范模式或 Standard mode标准模式
#define LOCAL_ECHO 							ECHO 
#define LOCAL_ECHOE 						ECHOE 
#define LOCAL_ECHOK 						ECHOK 
#define LOCAL_ECHO_NEWLINE 					ECHONL 
#define LOCAL_INPUT_CANON 					ICANON 
#define LOCAL_INPUT_EXTEND 					IEXTEN 
#define LOCAL_INPUT_SIGNAL 					ISIG 
#define LOCAL_NO_FLUSH 						NOFLUSH 
#define LOCAL_SIGNAL_W_TO_STOP 			`	TOSTOP
int tc_set_fdAterm_lmode(int fd, int actions, int lmode){
	struct termios st_termios;
	int ret = 0;
	ret = tc_get_fdAterm_attr(fd, &st_termios);
	if (ret != 1){
		//
		return ret;
	}
	
	st_termios.c_lflag |= lmode;
	
	ret = tc_set_fdAterm_attr(fd, actions, &st_termios);
	return ret;
}

int tc_clear_fdAterm_lmode(int fd, int actions, int lmode){
	struct termios st_termios;
	int ret = 0;
	ret = tc_get_fdAterm_attr(fd, &st_termios);
	if (ret != 1){
		//
		return ret;
	}
	
	st_termios.c_lflag &= ~lmode;
	
	ret = tc_set_fdAterm_attr(fd, actions, &st_termios);
	return ret;
}


//设置特殊字符模式
// 特殊控制字符是一些字符组合，如Ctrl+C，当用户键入这样的组合键时，终端会采取一些特殊的处理方式。（比如键盘）
	// termios结构中的c_cc数组成员将各种特殊控制字符映射到对应的支持函数。
	//	每个字符的位置（它在数组中的下标）是由一个宏定义的，但并不限制这些字符必须是控制字符
	// 根据终端是否被设置为标准模式，c_cc数组有两种差别很大的用法
		// 标准模式 
			// VEOF：EOF字符。将输入行中的全部字符传递给正在读取输入的应用程序。如果输入行为空，read调用将返回0，就好像在文件结尾调用read一样。
			// VEOL：EOL字符。该字符的作用类似行结束符，效果和常用的新行符相同。
			// VERASE：ERASE字符。该字符使终端驱动程序删除输入行的最后一个字符。
			// VINTR：INTR字符。该字符使终端驱动程序向与终端相连的进程发送SIGINT信号。
			// VKILL：KILL字符
			// VQUIT：QUIT字符。该字符使终端程序向与终端相连的进程发送SIQUIT信号。
			// VSUSP：SUSP字符。发送SIGSUSP信号，将当前应用程序挂起
			// VSTART：START字符。字符重新启动被STOP字符暂停的输出，让通常被设置为ASCII的XON字符。
			// VSTOP：STOP字符。作用是截流，阻止向终端的进一步输出。它用于支持XON/XOFF流控。通常被设置为ASCII的XOFF字符。
		// 非标准模式 
			// VINTR：INTR字符 
			// VMIN：MIN值
			// VQUIT：QUIT字符
			// VSUSP：SUSP字符
			// VTIME：TIME字符 
			// VSTART：START字符 
			// VSTOP：STOP字符
				// TIME和MIN
					// 二者用来控制对输入的读取
					// MIN = 0 和 TIME = 0。read调用立刻返回。如果有等待处理的字符，它们就会被返回；如果没有，read返回0，并且不读取任何字符
					// MIN = 0 和 TIME > 0。只要有字符可以处理或者经过TIME个十分之一秒的时间间隔，read调用就返回。如果因超时未读到任何字符，read返回0，否则read返回读取的字符数目
					// MIN > 0 和 TIME = 0。read调用一直等待，直到有MIN个字符可以读取才返回，返回值是读取的字符数量。到的文件尾时返回0。
					// MIN > 0 和 TIME > 0。当read被调用时，它会等待接收一个字符。在接收到第一个字符及后续的每个字符后，一个字符间隔定时器被启动（如果定时器已在运行，则重启它）。
					//							当有MIN个字符可读或两个字符之间的时间间隔超过TIME个十分之一秒时，read调用返回。这个功能可用于区分是单独按下Escape键盘还是按下一个以Escap键开始的功能组合键。
					//							但是在网络通信或处理器的高负载将使得类似的定时器失去作用。
/*
int tc_set_fdAterm_ccmode(int fd, int actions, int ccmode){
	struct termios st_termios;
	int ret = 0;
	ret = tc_get_fdAterm_attr(fd, &st_termios);
	if (ret != 1){
		//
		return ret;
	}
	
	st_termios.c_cc |= ccmode;
	
	ret = tc_set_fdAterm_attr(fd, actions, &st_termios);
	return ret;
}
*/

//终端速度
// termios可用于控制终端速度，输入速度和输出速度是分开处理的。

/*
#define NCCS		18
struct termios
{
    unsigned short c_iflag;  // 输入模式标志
    unsigned short c_oflag;  // 输出模式标志
    unsigned short c_cflag;  // 控制模式标志
    unsigned short c_lflag;  // 本地模式标志
    unsigned char c_line;    // 线路规程
    unsigned char c_cc[NCC]; // 控制特性
    speed_t c_ispeed;        // 输入速度
    speed_t c_ospeed;        // 输出速度
}

			B0
            B50
            B75
            B110
            B134
            B150
            B200
            B300
            B600
            B1200
            B1800
            B2400
            B4800
            B9600
            B19200
            B38400
            B57600
            B115200
            B230400

*/



speed_t tc_get_fdAterm_ispeed(int fd)
{
	int ret = -1;
	struct termios st_termios;
	ret = tc_get_fdAterm_attr(fd, &st_termios);
	if (ret != 0){
		return 0;
	}
	
	return cfgetispeed(&st_termios);
}

int tc_set_fdAterm_ispeed(int fd, speed_t speed)
{
	int ret = -1;
	struct termios st_termios;
	ret = tc_get_fdAterm_attr(fd, &st_termios);
	if (ret != 0){
		return -1;
	}
	
	ret = cfsetispeed(&st_termios, speed);
	
	ret = tc_set_fdAterm_attr(fd, TCSANOW, &st_termios);
	return ret;
}

speed_t tc_get_fdAterm_ospeed(int fd)
{
	int ret = -1;
	struct termios st_termios;
	ret = tc_get_fdAterm_attr(fd, &st_termios);
	if (ret != 0){
		return 0;
	}
	
	return cfgetospeed(&st_termios);
}

int tc_set_fdAterm_ospeed(int fd, speed_t speed)
{
	int ret = -1;
	struct termios st_termios;
	ret = tc_get_fdAterm_attr(fd, &st_termios);
	if (ret != 0){
		return -1;
	}
	
	ret = cfsetospeed(&st_termios, speed);
	
	ret = tc_set_fdAterm_attr(fd, TCSANOW, &st_termios);
	return ret;
}



//其他控制终端的函数
//等待，直到所有排队的输出都已发送完毕
int tc_iodrain(int fd)
{
	return tcdrain(fd);
}

//TCOOFF：输出被挂起。
//TCOON：重新启动以前被挂起的输出。
//TCIOFF：系统发送一个 STOP 字符，这将使终端设备停止发送数据。
//TCION：系统发送一个 START 字符，使终端设备恢复发送数据。

#define TC_OUTPUT_OFF TCOOFF 
#define TC_OUPUT_ON TCOON 
#define TC_INPUT_OFF TCIOFF 
#define TC_INPUT_ON TCION 
//用于暂停或重新开始输出
int tc_oflow_off(int fd)
{
	return tcflow(fd, TCOOFF);
}
int tc_oflow_on(int fd)
{
	return tcflow(fd, TCOON);
}
int tc_iflow_off(int fd)
{
	return tcflow(fd, TCIOFF);
}
int tc_iflow_on(int fd)
{
	return tcflow(fd, TCION);
}

//清空输入或输出或两者都清空
//TCIFLUSH：冲洗输入队列。
//TCOFLUSH：冲洗输出队列。
//TCIOFLUSH：冲洗输入和输出队列
#define TC_INPUT_FLUSH 		TCIFLUSH
#define TC_OUTPUT_FLUSH 	TCOFLUSH 
#define TC_IO_FLUSH 		TCIOFLUSH
int tc_oflush(int fd)
{
	return tcflush(fd, TCOFLUSH);
}

int tc_iflush(int fd)
{
	return tcflush(fd, TCIFLUSH);
}

int tc_ioflush(int fd)
{
	return tcflush(fd, TCIOFLUSH);
}



#ifdef DEBUG_TEST

static struct termios initial_settings, new_settings;
static int peek_character = -1;

void init_keyboard()
{
	int ret = 0;
	ret = tc_get_fdAterm_attr(0, &initial_settings);
	
	new_settings = initial_settings;
	new_settings.c_lflag &= ~ICANON;  //关闭标准模式，启用非标准模式
	new_settings.c_lflag &= ~ECHO;   //禁止回显
	new_settings.c_lflag &= ~ISIG;  //关闭信号
	
	new_settings.c_cc[VMIN] = 1;
	new_settings.c_cc[VTIME] = 0;
	
	ret = tc_set_fdAterm_attr(0, TCSANOW, &new_settings);
}

void close_keyboard()
{
	int ret = 0;
	ret = tc_set_fdAterm_attr(0, TCSANOW, &initial_settings);
}

int readch()
{
	char ch;
	if (peek_character != -1)
	{
		ch = peek_character;
		peek_character = -1;
		return ch;
	}
	read(0, &ch, 1);
	printf("----------------->ch = %d\r\n", ch);
	return ch;
}

int khbit()
{
	char ch;
	int nread;
	int ret = -1;
	static int count = 1;
	
	if (peek_character != -1){
		return 1;
	}
	
	new_settings.c_cc[VMIN] = 0;
	ret = tc_set_fdAterm_attr(0, TCSANOW, &new_settings);  //有按键按下，则下面的read会立即读到
	
	nread = read(0, &ch, 1);
	printf("---------------->count= %d, nread = %d\r\n", count++, nread);
	new_settings.c_cc[VMIN] = 1;
	ret = tc_set_fdAterm_attr(0, TCSANOW, &new_settings);  //没有按键按下，则以后的read会一直等待
	
	if (nread == 1){
		peek_character = ch;
		return 1;
	}
	return 0;
}


#endif

#ifdef DEBUG
#define DEBUG_PRINT(format, arg...) printf(format, ##arg)
#else
#define DEBUG_PRINT(format, arg...) 
#endif

#define do_once(x) ({do{ printf("print once %d\r\n", x); }while(0);})

#define min_t(type, x, y)	({type __x = (x); type __y = (y); __x < __y ? __x : __y;})

#define min_st(type, st1, st2, member) ({type __x = st1.member; type __y = st2.member; __x < __y ? st1 : st2;})

struct DataSet{
	int member1;
	float member2;
};

#define offset_T(type, member) &(((type*)0)->member)

#define min_v2(x, y) ({const typeof(x) _x = (x); const typeof(y) _y = (y); (void) (&_x == &_y); _x < _y ? x : y;})


int main(int argc, char **argv)
{
	#ifdef DEBUG_TEST
	int ch = 0;
	init_keyboard();
	
	while (ch != 'q'){
		printf("looping\n");
		sleep(1);
		if (khbit()){
			ch = readch();
			printf("you hit %c\n", ch);
		}
	}
	close_keyboard();
	#endif
	
	#ifdef STDIN_TEST
	int ret = -1;
	int fd = -1;
	fd = fileno(stdin);
	struct termios st_termios;
	ret = tcgetattr(fd, &st_termios);
	if (ret < 0){
		printf("------------->tcgetattr failed\r\n");
	}
	printf("------------------>0x%x\r\n", st_termios.c_iflag);
	printf("------------------>0x%x\r\n", st_termios.c_oflag);
	printf("------------------>0x%x\r\n", st_termios.c_cflag);
	printf("------------------>0x%x\r\n", st_termios.c_lflag);
	printf("------------------>%d\r\n", st_termios.c_cc[VMIN]);
	printf("------------------>%d\r\n", st_termios.c_cc[VTIME]);
	
	if (st_termios.c_lflag & ICANON){
		printf("this is canonical mode\r\n");
	}
	#endif
	
	#ifdef VAR_ARRAY
	int n = 10;
	scanf("%d", &n);
	int array[n];
	int i = 0;
	for (;i < n; ++i){
		array[i] = i;
	}
	for (i = 0; i < n; ++i){
		printf("%3d", array[i]);
	}
	#endif
	
	
	return 0;
}
