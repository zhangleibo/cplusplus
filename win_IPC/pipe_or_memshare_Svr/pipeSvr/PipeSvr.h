#include <windows.h>  
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <process.h>
#include <string>
#include <deque>

#include "BoundedBlockingQueue.h"

#define PIPE_BUFF_SIZE 1024

typedef void (*PCALLBACK)(std::string );

unsigned int __stdcall InstanceThread(LPVOID);

class CPipeSvr;

typedef struct _Paramer
{
	CPipeSvr* pPipeSvr;
	HANDLE hPipe;

	_Paramer(CPipeSvr* pPipe, HANDLE hdl) : pPipeSvr(pPipe), hPipe(hdl)
	{

	}
}PARAMER, *PPARAMER;

class CPipeSvr
{
public:
	CPipeSvr() : m_blockQueue(100), m_pCallBack(NULL)
	{

	}

	~CPipeSvr()
	{
		m_pCallBack = NULL;
	}

	bool Run(PCALLBACK pCallBack)
	{
		bool bOk = false;

		do
		{
			if (NULL == pCallBack)
			{
				break;
			}

			m_pCallBack = pCallBack;

			HANDLE hdl = (HANDLE)_beginthreadex(NULL, 0, GetInfo, (PVOID)this, 0, NULL);
			if (NULL != hdl)
			{
				::CloseHandle(hdl);
			}
			else
			{
				break;
			}

			::CloseHandle((HANDLE)_beginthreadex(NULL, 0, Work, this, 0, NULL));

			bOk = true;

		} while (0);

		return bOk;
	}

private:
	static unsigned int __stdcall GetInfo(PVOID pData)
	{
		CPipeSvr* pThis = (CPipeSvr*)pData;

		if ((NULL == pThis) || (NULL == pThis->m_pCallBack))
		{
			return 1;
		}

		while (true)
		{
			pThis->m_pCallBack(pThis->Get());
		}

		return 0;
	}

	static unsigned int __stdcall Work(PVOID pData)
	{
		BOOL fConnected;
		unsigned int dwThreadId;
		HANDLE hPipe, hThread;
		LPCSTR lpszPipename = "\\\\.\\pipe\\{BB705DBE-211F-4370-86AA-20A6644D048A}";

		// The run loop creates an instance of the named pipe and   
		// then waits for a client to connect to it. When the client   
		// connects, a thread is created to handle communications   
		// with that client, and the loop is repeated.   
		if (NULL == pData)
		{
			return 0;
		}

		while (true)
		{
			hPipe = CreateNamedPipeA(
				lpszPipename,             // pipe name   
				PIPE_ACCESS_DUPLEX,       // read/write access   
				PIPE_TYPE_MESSAGE |       // message type pipe   
				PIPE_READMODE_MESSAGE |   // message-read mode   
				PIPE_WAIT,                // blocking mode   
				PIPE_UNLIMITED_INSTANCES, // max. instances    
				PIPE_BUFF_SIZE,                  // output buffer size   
				PIPE_BUFF_SIZE,                  // input buffer size   
				0,                        // client time-out   
				NULL);                    // default security attribute   

			if (hPipe == INVALID_HANDLE_VALUE)
			{
				return 0;
			}

			// Wait for the client to connect; if it succeeds,   
			// the function returns a nonzero value. If the function  
			// returns zero, GetLastError returns ERROR_PIPE_CONNECTED.   

			fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

			if (fConnected)
			{
				PARAMER paramer((CPipeSvr*)pData, hPipe);

				// Create a thread for this client.   
				hThread = (HANDLE)_beginthreadex(
					NULL,              // no security attribute   
					0,                 // default stack size   
					InstanceThread,    // thread proc  
					(LPVOID)&paramer,    // thread parameter   
					0,                 // not suspended   
					&dwThreadId);      // returns thread ID   

				if (hThread == NULL)
				{
					return 0;
				}
				else
				{
					CloseHandle(hThread);
				}
			}
			else
			{
				// The client could not connect, so close the pipe.   
				CloseHandle(hPipe);
			}
		}
		return 1;
	}

	static unsigned int __stdcall InstanceThread(LPVOID lpvParam)
	{
		char chRequest[PIPE_BUFF_SIZE];
		DWORD cbBytesRead;
		BOOL fSuccess;
		HANDLE hPipe;

		// The thread's parameter is a handle to a pipe instance.   
		PPARAMER pParamer = (PPARAMER)lpvParam;
		if (NULL == pParamer)
		{
			return 0;
		}

		hPipe = pParamer->hPipe;

		while (true)
		{
			// Read client requests from the pipe.   
			fSuccess = ReadFile(
				hPipe,        // handle to pipe   
				chRequest,    // buffer to receive data   
				PIPE_BUFF_SIZE*sizeof(TCHAR), // size of buffer   
				&cbBytesRead, // number of bytes read   
				NULL);        // not overlapped I/O   

			if (!fSuccess || cbBytesRead == 0)
				break;

			//TO DO:
			pParamer->pPipeSvr->Set(chRequest);
		}

		// Flush the pipe to allow the client to read the pipe's contents   
		// before disconnecting. Then disconnect the pipe, and close the   
		// handle to this pipe instance.   

		FlushFileBuffers(hPipe);
		DisconnectNamedPipe(hPipe);
		CloseHandle(hPipe);

		return 1;
	}

public:
	// block get
	std::string Get() { return m_blockQueue.Pop(); }
private:
	void Set(std::string str) { m_blockQueue.Push(str); }

	CBoundedBlockingQueue<std::string> m_blockQueue;
	PCALLBACK m_pCallBack;
};
