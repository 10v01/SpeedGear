#include "stdafx.h"

void WINAPI MySleep(DWORD dwMilliseconds)
{
	DWORD myInterval = (DWORD)((double)dwMilliseconds / 2);
	return oldSleep(myInterval);
}