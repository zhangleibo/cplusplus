#include <windows.h>  
#include <conio.h>  
#include <stdio.h>
#include <tchar.h>

#include "AutoMutex.h"

#define PIPE_BUFF_SIZE 1024

class CPipeClient
{
public:
	static CPipeClient* GetInstance()
	{
		static CPipeClient Obj;
		return &Obj;
	}
private:
	CPipeClient() : m_hPipe(NULL), m_bOk(false)
	{

	}
	~CPipeClient()
	{
		if (m_bOk)
		{
			if (NULL != m_hPipe)
			{
				::CloseHandle(m_hPipe);
				m_hPipe = NULL;
			}
		}

		m_bOk = false;
	}

public:
	int Output(const char* format, ...)
	{
		CLock lock(m_mutex);

		if (!m_bOk)
		{
			if (!ConnectNamedPipe())
			{
				return 0;
			}
		}

		char szbuff[PIPE_BUFF_SIZE] = { 0 };

		if (NULL == format)
		{
			return 0;
		}

		va_list st;
		va_start(st, format);
		vsnprintf(szbuff, PIPE_BUFF_SIZE, format, st);
		va_end(st);

		return SetInfoToPipe(szbuff);
	}

private:
	bool ConnectNamedPipe()
	{
		BOOL fSuccess;
		DWORD dwMode;
		LPCSTR lpszPipename = "\\\\.\\pipe\\{BB705DBE-211F-4370-86AA-20A6644D048A}";

		// Try to open a named pipe; wait for it, if necessary.   

		do 
		{
			m_hPipe = CreateFileA(
				lpszPipename,   // pipe name   
				GENERIC_READ |  // read and write access   
				GENERIC_WRITE,
				0,              // no sharing   
				NULL,           // default security attributes  
				OPEN_EXISTING,  // opens existing pipe   
				0,              // default attributes   
				NULL);          // no template file   

			// Break if the pipe handle is valid.   

			if (m_hPipe != INVALID_HANDLE_VALUE)
			{
				break;
			}

			// Exit if an error other than ERROR_PIPE_BUSY occurs.   

			if (GetLastError() != ERROR_PIPE_BUSY)
			{
				return m_bOk;
			}

			// All pipe instances are busy, so wait for 0.2 seconds.   

			if (!WaitNamedPipeA(lpszPipename, 200))
			{
				return m_bOk;
			}

		} while (1);

		// The pipe connected; change to message-read mode.   

		dwMode = PIPE_READMODE_MESSAGE;
		fSuccess = SetNamedPipeHandleState(
			m_hPipe,    // pipe handle   
			&dwMode,  // new pipe mode   
			NULL,     // don't set maximum bytes   
			NULL);    // don't set maximum time   
		if (!fSuccess)
		{
			return m_bOk;
		}

		m_bOk = true;

		return m_bOk;
	}

	int SetInfoToPipe(char szbuff[])
	{
		if (!m_bOk)
		{
			return 0;
		}

		// Send a message to the pipe server.   
		BOOL fSuccess;
		DWORD cbWritten;
		fSuccess = WriteFile(
			m_hPipe,                  // pipe handle   
			szbuff,             // message   
			(strlen(szbuff) + 1)*sizeof(char), // message length   
			&cbWritten,             // bytes written   
			NULL);                  // not overlapped   

		if (!fSuccess)
		{
			m_bOk = false;
			if (NULL != m_hPipe)
			{
				::CloseHandle(m_hPipe);
				m_hPipe = NULL;
			}
			return 0;
		}

		return 1;
	}

		HANDLE m_hPipe;
		CMutex m_mutex;
		bool m_bOk;
};
