// HookDll.cpp : ���� DLL �ĳ�ʼ�����̡�
//

#include "stdafx.h"
#include "HookDll.h"
#include <Windows.h>
#include <psapi.h>
#include "Detours/include/detours.h"
#include "hooked_def.h"
#include <tlhelp32.h>
#include "seqList.h"

#if defined _M_X64
#pragma comment(lib, "Detours/lib.X64/detours.lib")
#elif defined _M_IX86
#pragma comment(lib, "Detours/lib.X86/detours.lib")
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*
	 ȫ�ֹ������
*/
#pragma data_seg("Share")
HWND g_hWnd = NULL ;			// �����ھ��
float multiSpeed = 1;
BOOL do_hook = FALSE;
#pragma data_seg()
#pragma comment(linker,"/section:Share,rws")


float pre_multiSpeed = 1;
BOOL first_GetMessageTimed = FALSE;
BOOL first_timeGetTimed = FALSE;
BOOL first_GetTickCounted = FALSE;
BOOL first_QueryPerformanceCountered = FALSE;
seqList <DWORD> injected_list;
int cannot_exit = 0;
HINSTANCE g_hInstance = NULL;
HANDLE test_hProcess = NULL;
HANDLE hProcess = NULL;				//	��ǰ����
BOOL bIsInJected = FALSE;			//	�Ƿ���ע����
TCHAR* msgToMain = new TCHAR[200];	//	���������������Ϣ

/* 
	��������ԭ������
*/
bool AdjustProcessTokenPrivilege();
void HookOn();			//	��ʼHOOK
void HookOff();			//	�ر�HOOK
void GetAddress();			//	ע��
void WINAPI setSpeed(double newSpeed);
BOOL WINAPI StartHook(DWORD dwProcID);	// ���ع���
BOOL WINAPI StopHook(DWORD dwProcID);				// ж�ع���

DWORD First_GetMessageTime = 0;
DWORD Pre_GetMessageTime = 0;
DWORD New_First_GetMessageTime = 0;
DWORD New_Pre_GetMessageTime = 0;
BOOL Has_Pre_GetMessageTime = FALSE;
DWORD First_timeGetTime = 0;
DWORD Pre_timeGetTime = 0;
DWORD New_First_timeGetTime = 0;
DWORD New_Pre_timeGetTime = 0;
BOOL Has_Pre_timeGetTime = FALSE;
DWORD First_GetTickCount = 0;
DWORD Pre_GetTickCount = 0;
DWORD New_First_GetTickCount = 0;
DWORD New_Pre_GetTickCount = 0;
BOOL Has_Pre_GetTickCount = FALSE;
LARGE_INTEGER First_QueryPerformanceCounter;
LARGE_INTEGER Pre_QueryPerformanceCounter;
LARGE_INTEGER New_First_QueryPerformanceCounter;
LARGE_INTEGER New_Pre_QueryPerformanceCounter;
BOOL Has_Pre_QueryPerformanceCounter = FALSE;

//
//TODO: ����� DLL ����� MFC DLL �Ƕ�̬���ӵģ�
//		��Ӵ� DLL �������κε���
//		MFC �ĺ������뽫 AFX_MANAGE_STATE ����ӵ�
//		�ú�������ǰ�档
//
//		����:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// �˴�Ϊ��ͨ������
//		}
//
//		�˺������κ� MFC ����
//		������ÿ��������ʮ����Ҫ������ζ��
//		��������Ϊ�����еĵ�һ�����
//		���֣������������ж������������
//		������Ϊ���ǵĹ��캯���������� MFC
//		DLL ���á�
//
//		�й�������ϸ��Ϣ��
//		����� MFC ����˵�� 33 �� 58��
//

// CHookDllApp

BEGIN_MESSAGE_MAP(CHookDllApp, CWinApp)
END_MESSAGE_MAP()


// CHookDllApp ����

CHookDllApp::CHookDllApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CHookDllApp ����

CHookDllApp theApp;


// CHookDllApp ��ʼ��
/*
	dll������ڣ����������dllʱ����ִ��InitInstance()  
*/
BOOL CHookDllApp::InitInstance()
{
	CWinApp::InitInstance();
	AdjustProcessTokenPrivilege();
	g_hInstance = AfxGetInstanceHandle();//	��ȡ��ǰDLLʵ�����
	
	DWORD dwPid = ::GetCurrentProcessId();
	hProcess = ::OpenProcess(PROCESS_ALL_ACCESS,0,dwPid);
/*	
	TCHAR* procName = new TCHAR[MAX_PATH];
	GetModuleFileName(NULL,procName,MAX_PATH);  // �������ȡ�ý���������ô�Ϳ����ж��Ƿ��ǻ��ܽ���������
	CString info;
	info.Format(_T("����id = %d , ������ %s"),dwPid,procName);
	AfxMessageBox(info);
*/	
	if (hProcess == NULL)
	{
		CString str;
		str.Format(_T("OpenProcess fail�� and error code = %d"),GetLastError());
		AfxMessageBox(str);
		return FALSE;
	}
	GetAddress();
	if(do_hook)
		HookOn();
	do_hook = TRUE;
	return TRUE;
}
int CHookDllApp::ExitInstance()
{
	HookOff();
	while (cannot_exit > 0) { }
	return CWinApp::ExitInstance();
}

void GetAddress()
{
	if (TRUE == bIsInJected)
	{
		return;
	}
	bIsInJected = TRUE;	// ��ֻ֤����һ��

	oldMsgBoxA = (TypeMsgBoxA) ::GetProcAddress(GetModuleHandle(TEXT("User32.dll")), "MessageBoxA");
	oldMsgBoxW = (TypeMsgBoxW) ::GetProcAddress(GetModuleHandle(TEXT("User32.dll")), "MessageBoxW");
	oldSetTimer = (SETTIMER)GetProcAddress(GetModuleHandle(TEXT("User32.dll")), "SetTimer");
	oldGetMessageTime = (GETMESSAGETIME)GetProcAddress(GetModuleHandle(TEXT("User32.dll")), "GetMessageTime");
	oldSleep = (TypeSleep)::GetProcAddress(GetModuleHandle(TEXT("Kernel32.dll")), "Sleep");
	oldGetTickCount = (GETTICKCOUNT)GetProcAddress(GetModuleHandle(TEXT("Kernel32.dll")), "GetTickCount");
	oldQueryPerformanceCounter = (QUERYPERFORMANCECOUNTER)GetProcAddress(GetModuleHandle(TEXT("Kernel32.dll")), "QueryPerformanceCounter");
	oldTimeSetEvent = (TIMESETEVENT)GetProcAddress(GetModuleHandle(TEXT("Winmm.dll")), "timeSetEvent");
	oldTimeGetTime = (TIMEGETTIME)GetProcAddress(GetModuleHandle(TEXT("Winmm.dll")), "timeGetTime");
}

/*
	����API��ַ�滻ԭAPI��ַ
*/
void HookOn()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)oldMsgBoxA, MyMessageBoxA);
	DetourAttach(&(PVOID&)oldMsgBoxW, MyMessageBoxW);
	DetourAttach(&(PVOID&)oldSetTimer, MySetTimer);
	DetourAttach(&(PVOID&)oldGetMessageTime, MyGetMessageTime);
	//DetourAttach(&(PVOID&)oldSleep, MySleep);
	DetourAttach(&(PVOID&)oldGetTickCount, MyGetTickCount);
	DetourAttach(&(PVOID&)oldQueryPerformanceCounter, MyQueryPerformanceCounter);
	DetourAttach(&(PVOID&)oldTimeSetEvent, MyTimeSetEvent);
	DetourAttach(&(PVOID&)oldTimeGetTime, MyTimeGetTime);
	DetourTransactionCommit();

}

/*
	�ָ�ԭAPI��ַ
*/
void HookOff()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)oldMsgBoxA, MyMessageBoxA);
	DetourDetach(&(PVOID&)oldMsgBoxW, MyMessageBoxW);
	DetourDetach(&(PVOID&)oldSetTimer, MySetTimer);
	DetourDetach(&(PVOID&)oldGetMessageTime, MyGetMessageTime);
	//DetourDetach(&(PVOID&)oldSleep, MySleep);
	DetourDetach(&(PVOID&)oldGetTickCount, MyGetTickCount);
	DetourDetach(&(PVOID&)oldQueryPerformanceCounter, MyQueryPerformanceCounter);
	DetourDetach(&(PVOID&)oldTimeSetEvent, MyTimeSetEvent);
	DetourDetach(&(PVOID&)oldTimeGetTime, MyTimeGetTime);
	DetourTransactionCommit();
}

DWORD FindProc(LPCSTR lpName)
{
	DWORD aProcId[1024], dwProcCnt, dwModCnt;
	char szPath[MAX_PATH];
	HMODULE hMod;

	//ö�ٳ����н���ID
	if (!EnumProcesses(aProcId, sizeof(aProcId), &dwProcCnt))
	{
		//cout << "EnumProcesses error: " << GetLastError() << endl;
		return 0;
	}

	//�������н���
	for (DWORD i = 0; i < dwProcCnt; ++i)
	{
		//�򿪽��̣����û��Ȩ�޴�������
		HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, aProcId[i]);
		if (NULL != hProc)
		{
			//�򿪽��̵ĵ�1��Module��������������Ƿ���Ŀ�����
			if (EnumProcessModules(hProc, &hMod, sizeof(hMod), &dwModCnt))
			{
				GetModuleBaseNameA(hProc, hMod, szPath, MAX_PATH);
				if (0 == _stricmp(szPath, lpName))
				{
					CloseHandle(hProc);
					return aProcId[i];
				}
			}
			CloseHandle(hProc);
		}
	}
	return 0;
}

BOOL WINAPI StartHook(DWORD dwProcID)
{
	TCHAR szDLLFolder[MAX_PATH + 1] = { 0 };
	GetModuleFileName(AfxGetApp()->m_hInstance, szDLLFolder, MAX_PATH);

	g_hInstance = AfxGetInstanceHandle();
	AdjustProcessTokenPrivilege();
	
	if (injected_list.search(dwProcID) >= 0)
	{
		MessageBoxW(NULL, _T("��������Ѿ���ע�����"), _T("��ע��"), 0);
		return FALSE;
	}

	test_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcID);
	if (test_hProcess == NULL)
	{
		MessageBoxW(NULL, _T("�޷��򿪽���"), _T("�򿪽��̴���"), 0);
		return FALSE;
	}
	PTHREAD_START_ROUTINE pfnStartAddr = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
	if(pfnStartAddr == NULL)
	{
		CloseHandle(test_hProcess);
		return FALSE;
	}

	PVOID pRemoteThread = VirtualAllocEx(test_hProcess, NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (pRemoteThread == NULL)
	{
		CloseHandle(test_hProcess);
		return FALSE;
	}

	BOOL is_succeed = WriteProcessMemory(test_hProcess, pRemoteThread, szDLLFolder, MAX_PATH+1, 0);
	if (!is_succeed)
	{
		CloseHandle(test_hProcess);
		return FALSE;
	}

	HANDLE test_hThread = CreateRemoteThread(test_hProcess, NULL, 0, pfnStartAddr, pRemoteThread, 0, NULL);
	VirtualFreeEx(test_hProcess, pRemoteThread, 0x1000, MEM_RELEASE);

	if (test_hThread == NULL)
	{
		CloseHandle(test_hProcess);
		return FALSE;
	}

	injected_list.append(dwProcID);

	return TRUE;
}

BOOL WINAPI StopHook(DWORD dwProcID)
{
	BOOL bMore = FALSE, bFound = FALSE;
	HANDLE hSnapshot;
	MODULEENTRY32 me = { sizeof(me) };
	TCHAR szDLLFolder[MAX_PATH + 1] = { 0 };
	GetModuleFileName(AfxGetApp()->m_hInstance, szDLLFolder, MAX_PATH);
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcID);

	bMore = Module32First(hSnapshot, &me);
	for (; bMore; bMore = Module32Next(hSnapshot, &me))
	{
		if (!_tcsicmp((LPCTSTR)me.szModule, szDLLFolder) || !_tcsicmp((LPCTSTR)me.szExePath, szDLLFolder))
		{
			bFound = TRUE;
			break;
		}
	}
	
	test_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcID);
	if (test_hProcess == NULL)
	{
		MessageBoxW(NULL, _T("�޷��򿪽���"), _T("�򿪽��̴���"), 0);
		return FALSE;
	}

	PTHREAD_START_ROUTINE pfnStopAddr = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "FreeLibrary");
	if (pfnStopAddr == NULL)
	{
		CloseHandle(test_hProcess);
		return FALSE;
	}

	HANDLE test_hThread = CreateRemoteThread(test_hProcess, NULL, 0, pfnStopAddr, me.modBaseAddr, 0, NULL);
	if (test_hThread == NULL)
	{
		CloseHandle(test_hProcess);
		return FALSE;
	}

	int temp_loc = injected_list.search(dwProcID);
	if(temp_loc >= 0)injected_list.remove(temp_loc);

	return TRUE;
}

int WINAPI MyMessageBoxA(HWND hWnd,LPCSTR lpText,LPCSTR lpCation,UINT uType)
{
	int nRet = 0;

	oldMsgBoxA(hWnd,"FAQ, leather MessageBoxA",lpCation,uType);
	nRet = oldMsgBoxA(hWnd,lpText,lpCation,uType);

	return nRet;
}

int WINAPI MyMessageBoxW(HWND hWnd,LPCWSTR lpText,LPCWSTR lpCation,UINT uType)
{
	int nRet = 0;

	oldMsgBoxW(hWnd,_T("FAQ, leather MessageBoxW"),lpCation,uType);
	nRet = oldMsgBoxW(hWnd,lpText,lpCation,uType);

	return nRet;
}

UINT_PTR WINAPI MySetTimer(__in_opt HWND hWnd, __in UINT_PTR nIDEvent, __in UINT uElapse, __in_opt TIMERPROC lpTimerFunc)
{
	UINT uMyElapse = (UINT)((double)uElapse / multiSpeed);

	return oldSetTimer(hWnd, nIDEvent, uMyElapse, lpTimerFunc);
}

LONG WINAPI MyGetMessageTime(VOID)
{
	if (multiSpeed != pre_multiSpeed)
	{
		first_GetMessageTimed = FALSE;
		first_timeGetTimed = FALSE;
		first_GetTickCounted = FALSE;
		first_QueryPerformanceCountered = FALSE;
		pre_multiSpeed = multiSpeed;
	}
	if (first_GetMessageTimed == FALSE)
	{
		if (Has_Pre_GetMessageTime == FALSE)
		{
			First_GetMessageTime = oldGetMessageTime();
			New_First_GetMessageTime = First_GetMessageTime;
			New_Pre_GetMessageTime = First_GetMessageTime;
			Has_Pre_GetMessageTime = TRUE;
		}
		else
		{
			First_GetMessageTime = Pre_GetMessageTime;
			New_First_GetMessageTime = New_Pre_GetMessageTime;
		}
		first_GetMessageTimed = TRUE;
	}
	Pre_GetMessageTime = oldGetMessageTime();
	New_Pre_GetMessageTime = LONG(double(Pre_GetMessageTime - First_GetMessageTime) * multiSpeed) + New_First_GetMessageTime;
	return New_Pre_GetMessageTime;
}

void WINAPI MySleep(DWORD dwMilliseconds)
{
	cannot_exit++;
	DWORD myInterval = (DWORD)((double)dwMilliseconds / multiSpeed);
	oldSleep(myInterval);
	cannot_exit--;
	return;
}

DWORD WINAPI MyGetTickCount(void)
{
	if (multiSpeed != pre_multiSpeed)
	{
		first_GetMessageTimed = FALSE;
		first_timeGetTimed = FALSE;
		first_GetTickCounted = FALSE;
		first_QueryPerformanceCountered = FALSE;
		pre_multiSpeed = multiSpeed;
	}
	if (first_GetTickCounted == FALSE)
	{
		if (Has_Pre_GetTickCount == FALSE)
		{
			First_GetTickCount = oldGetTickCount();
			New_First_GetTickCount = First_GetTickCount;
			New_Pre_GetTickCount = First_GetTickCount;
			Has_Pre_GetTickCount = TRUE;
		}
		else
		{
			First_GetTickCount = Pre_GetTickCount;
			New_First_GetTickCount = New_Pre_GetTickCount;
		}
		first_GetTickCounted = TRUE;
	}
	Pre_GetTickCount = oldGetTickCount();
	New_Pre_GetTickCount = DWORD(double(Pre_GetTickCount - First_GetTickCount) * multiSpeed) + New_First_GetTickCount;
	return New_Pre_GetTickCount;
}

BOOL WINAPI MyQueryPerformanceCounter(__out LARGE_INTEGER *lpPerformanceCount)
{
	if (multiSpeed != pre_multiSpeed)
	{
		first_GetMessageTimed = FALSE;
		first_timeGetTimed = FALSE;
		first_GetTickCounted = FALSE;
		first_QueryPerformanceCountered = FALSE;
		pre_multiSpeed = multiSpeed;
	}
	if (first_QueryPerformanceCountered == FALSE)
	{
		if (Has_Pre_QueryPerformanceCounter == FALSE)
		{
			oldQueryPerformanceCounter(&First_QueryPerformanceCounter);
			New_First_QueryPerformanceCounter = First_QueryPerformanceCounter;
			New_Pre_QueryPerformanceCounter = First_QueryPerformanceCounter;
			Has_Pre_QueryPerformanceCounter = TRUE;
		}
		else
		{
			First_QueryPerformanceCounter = Pre_QueryPerformanceCounter;
			New_First_QueryPerformanceCounter = New_Pre_QueryPerformanceCounter;
		}
		first_QueryPerformanceCountered = TRUE;
	}
	BOOL bSucceed = oldQueryPerformanceCounter(&Pre_QueryPerformanceCounter);
	*lpPerformanceCount = Pre_QueryPerformanceCounter;
	if (bSucceed == TRUE)
	{
		lpPerformanceCount->QuadPart = LONGLONG(double(lpPerformanceCount->QuadPart - First_QueryPerformanceCounter.QuadPart) * multiSpeed) + New_First_QueryPerformanceCounter.QuadPart;
		New_Pre_QueryPerformanceCounter.QuadPart = lpPerformanceCount->QuadPart;
	}
	return bSucceed;
}

MMRESULT WINAPI MyTimeSetEvent(__in UINT uDelay, __in UINT uResolution, __in LPTIMECALLBACK fptc, __in DWORD_PTR dwUser, __in UINT fuEvent)
{
	UINT uMyDelay = (UINT)((double)uDelay / multiSpeed);
	return oldTimeSetEvent(uMyDelay, uResolution, fptc, dwUser, fuEvent);
}

DWORD WINAPI MyTimeGetTime(void)
{
	if (multiSpeed != pre_multiSpeed)
	{
		first_GetMessageTimed = FALSE;
		first_timeGetTimed = FALSE;
		first_GetTickCounted = FALSE;
		first_QueryPerformanceCountered = FALSE;
		pre_multiSpeed = multiSpeed;
	}
	if (first_timeGetTimed == FALSE)
	{
		if (Has_Pre_timeGetTime == FALSE)
		{
			First_timeGetTime = oldTimeGetTime();
			New_First_timeGetTime = First_timeGetTime;
			New_Pre_timeGetTime = First_timeGetTime;
			Has_Pre_timeGetTime = TRUE;
		}
		else
		{
			First_timeGetTime = Pre_timeGetTime;
			New_First_timeGetTime = New_Pre_timeGetTime;
		}
		first_timeGetTimed = TRUE;
	}
	Pre_timeGetTime = oldTimeGetTime();
	New_Pre_timeGetTime = DWORD(double(Pre_timeGetTime - First_timeGetTime) * multiSpeed) + First_timeGetTime;
	return New_Pre_timeGetTime;
}

bool AdjustProcessTokenPrivilege()
{
	LUID luidTmp;
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{

		return false;
	}

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luidTmp))
	{
		CloseHandle(hToken);
		return FALSE;
	}

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = luidTmp;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
	{

		CloseHandle(hToken);

		return FALSE;
    }
    return true;
}

void WINAPI setSpeed(double newSpeed)
{
	multiSpeed = newSpeed;
	return ;
}
