#include <Windows.h>
#include <MMSystem.h>
#pragma comment(lib, "Winmm.lib")

typedef int (WINAPI *TypeMsgBoxA)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
TypeMsgBoxA oldMsgBoxA = NULL;
int WINAPI MyMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCation, UINT uType);

typedef int (WINAPI *TypeMsgBoxW)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
TypeMsgBoxW oldMsgBoxW = NULL;
int WINAPI MyMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCation, UINT uType);

//SetTimer
typedef UINT_PTR(WINAPI *SETTIMER)(__in_opt HWND hWnd, __in UINT_PTR nIDEvent, __in UINT uElapse, __in_opt TIMERPROC lpTimerFunc);
SETTIMER oldSetTimer = NULL;
UINT_PTR WINAPI MySetTimer(__in_opt HWND hWnd, __in UINT_PTR nIDEvent, __in UINT uElapse, __in_opt TIMERPROC lpTimerFunc);

//GetMessageTime
typedef LONG(WINAPI *GETMESSAGETIME)(VOID);
GETMESSAGETIME oldGetMessageTime = NULL;
LONG WINAPI MyGetMessageTime(VOID);

//Sleep
typedef void (WINAPI *TypeSleep)(DWORD dwMilliseconds);
TypeSleep oldSleep = NULL;
void WINAPI MySleep(DWORD dwMilliseconds);

//GetTickCount
typedef DWORD(WINAPI *GETTICKCOUNT)(void);
GETTICKCOUNT oldGetTickCount = NULL;
DWORD WINAPI MyGetTickCount(void);

//QueryPerformanceCounter
typedef BOOL(WINAPI *QUERYPERFORMANCECOUNTER)(__out LARGE_INTEGER *lpPerformanceCount);
QUERYPERFORMANCECOUNTER oldQueryPerformanceCounter = NULL;
BOOL WINAPI MyQueryPerformanceCounter(__out LARGE_INTEGER *lpPerformanceCount);

//timeSetEvent
typedef MMRESULT(WINAPI *TIMESETEVENT)(__in UINT uDelay, __in UINT uResolution, __in LPTIMECALLBACK fptc, __in DWORD_PTR dwUser, __in UINT fuEvent);
TIMESETEVENT oldTimeSetEvent = NULL;
MMRESULT WINAPI MyTimeSetEvent(__in UINT uDelay, __in UINT uResolution, __in LPTIMECALLBACK fptc, __in DWORD_PTR dwUser, __in UINT fuEvent);

//timeGetTime
typedef DWORD(WINAPI *TIMEGETTIME)(void);
TIMEGETTIME oldTimeGetTime = NULL;
DWORD WINAPI MyTimeGetTime(void);


