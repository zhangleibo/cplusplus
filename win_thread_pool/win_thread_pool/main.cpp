#include "./threadpool/InterfaceAdapter.h"
#include <iostream>
#include "./threadpool/ThreadPool.h"
#include <time.h>
#include <set>
using namespace std;


#define CATCH_EXCEPTION(exp) try\
{\
	exp; \
}\
	catch (std::exception& e)\
{\
	printf("Exception file:%s, line:%d, %s\n", __FILE__, __LINE__, e.what()); \
}\
	catch (...)\
{\
	printf(" unknown exception.\n"); \
}


typedef struct MyStruct
{
	int i;
	std::string str;
}Stru;

class T
{
public:
	void Run()
	{
		time_t t_start, t_end;
		t_start = time(NULL);

		size_t iThreadNum = 10;
		CThreadPool thread_pool(iThreadNum);

		thread_pool.Start();
		Stru s;
		int i = 0;
		for (;;) {
			s.i = i++;
			thread_pool.Add(NewAdapterImp(this, &T::printEx, s));
		}

		t_end = time(NULL);
		printf("time: %.3f s\n", difftime(t_end, t_start));

		thread_pool.Stop();
	}

	void printEx(Stru s)
	{
		Sleep(5);
		cout << "ID:" << GetCurrentThreadId() << " " << "Num:" << s.i << endl;
	}
};

//思路：管理一个任务队列，一个线程队列，然后每次取一个任务分配给一个线程去做，循环往复。
int main()
{
	T t;
	t.Run();
	return 0;
}