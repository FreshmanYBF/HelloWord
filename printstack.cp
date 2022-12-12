#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <cxxabi.h>

/** Print a demangled stack backtrace of the caller function to FILE* out. */
static inline void print_stacktrace(FILE *out = stderr, unsigned int max_frames = 63)
{
    fprintf(out, "stack trace:\n");

    // storage array for stack trace address data
    void* addrlist[max_frames+1];

    // retrieve current stack addresses
    int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

    if (addrlen == 0) {
	fprintf(out, "  <empty, possibly corrupt>\n");
	return;
    }

    // resolve addresses into strings containing "filename(function+address)",
    // this array must be free()-ed
    char** symbollist = backtrace_symbols(addrlist, addrlen);

    // allocate string which will be filled with the demangled function name
    size_t funcnamesize = 256;
    char* funcname = (char*)malloc(funcnamesize);

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    for (int i = 1; i < addrlen; i++)
    {
	char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

	// find parentheses and +address offset surrounding the mangled name:
	// ./module(function+0x15c) [0x8048a6d]
	for (char *p = symbollist[i]; *p; ++p)
	{
	    if (*p == '(')
		begin_name = p;
	    else if (*p == '+')
		begin_offset = p;
	    else if (*p == ')' && begin_offset) {
		end_offset = p;
		break;
	    }
	}

	if (begin_name && begin_offset && end_offset
	    && begin_name < begin_offset)
	{
	    *begin_name++ = '\0';
	    *begin_offset++ = '\0';
	    *end_offset = '\0';

	    // mangled name is now in [begin_name, begin_offset) and caller
	    // offset in [begin_offset, end_offset). now apply
	    // __cxa_demangle():

	    int status;
	    char* ret = abi::__cxa_demangle(begin_name,
					    funcname, &funcnamesize, &status);
	    if (status == 0) {
		funcname = ret; // use possibly realloc()-ed string
		fprintf(out, "  %s : %s+%s\n",
			symbollist[i], funcname, begin_offset);
	    }
	    else {
		// demangling failed. Output function name as a C function with
		// no arguments.
		fprintf(out, "  %s : %s()+%s\n",
			symbollist[i], begin_name, begin_offset);
	    }
	}
	else
	{
	    // couldn't parse the line? print the whole line.
	    fprintf(out, "  %s\n", symbollist[i]);
	}
    }

    free(funcname);
    free(symbollist);
}

string stackTrace(bool demangle)
{
	string stack;
	const int max_frames = 200;
	void* frame[max_frames];
	int nptrs = ::backtrace(frame, max_frames);  //返回去有多少栈帧
	char** strings = ::backtrace_symbols(frame, nptrs);  //将这些栈帧都转成字符, 返回一个二维数组 —— 是否需要释放

	printf("----------------->nptrs = %d\r\n", nptrs);

	if (strings)
	{
		size_t len = 256;
		char* demangled = demangle ? static_cast<char*>(::malloc(len)) : nullptr;
		for (int i = 1; i < nptrs; ++i)  // skipping the 0-th,w which is this function
		{
			if (demangled)
			{
				char* left_par = nullptr;
				char* plus = nullptr;

				printf("------------------->string[%d] = %s\r\n", i, strings[i]);
				//在每个堆栈信息中找 ( 和 +
				for (char* p = strings[i]; *p; ++p)
				{
					if (*p == '(')
						left_par = p;
					else if (*p == '+')
						plus = p;
				}
				
				//有 ( 和 +，则进行demangle
				if (left_par && plus)
				{
					*plus = '\0';
					int status = 0;
					char* ret = abi::__cxa_demangle(left_par + 1, demangled, &len, &status);
					*plus = '+';
					printf("------------------->status = %d\r\n", status);
					if (status == 0)
					{
						demangled = ret;  //ret could realloc()
						stack.append(strings[i], left_par + 1);
						stack.append(demangled);
						stack.append(plus);
						stack.push_back('\n');
						continue;
					}
				}
			}
			stack.append(strings[i]);
			stack.push_back('\n');
		}
		//fallback to mangled names
		free(demangled);
		free(strings);
	}
	return stack;
}

extern "C"{

void print()
{
	printf("func is run\r\n");
}

void xxxx(){
	print_stacktrace();
}

void yyyy()
{
	print();
	xxxx();
}

void zzzz()
{
	print();
	yyyy();
}

void base4()
{
	zzzz();
}
}

int main()
{
	base4();
	return 0;
}
