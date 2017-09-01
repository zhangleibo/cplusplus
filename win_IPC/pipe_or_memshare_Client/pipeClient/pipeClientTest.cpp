#include "PipeClient.h"
#include "MemClient.h"

void pipe()
{
	while (true)
	{
		int pid = GetCurrentProcessId();
		char szbuff[PIPE_BUFF_SIZE] = { 0 };
		sprintf(szbuff, "process id == %d\n", pid);

		if (!CPipeClient::GetInstance()->Output(szbuff))
		{
			printf("wait for svr...\n");
			Sleep(1000);
			continue;
		}
		printf(szbuff);
		Sleep(200);
	}
}
void main()
{
	CMemClient cmemclient;
	cmemclient.Open();

	int pid = GetCurrentProcessId();
	char szbuff[PIPE_BUFF_SIZE] = { 0 };
	sprintf(szbuff, "process id == %d\n", pid);

	cmemclient.WriteData(szbuff);

	return;
}