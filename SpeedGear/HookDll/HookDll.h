// HookDll.h : HookDll DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CHookDllApp
// �йش���ʵ�ֵ���Ϣ������� HookDll.cpp
//

class CHookDllApp : public CWinApp
{
public:
	CHookDllApp();

// ��д
public:
	virtual BOOL InitInstance();
	int ExitInstance();

	DECLARE_MESSAGE_MAP()
};

