
// SnDatTool.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSnDatToolApp:
// �йش����ʵ�֣������ SnDatTool.cpp
//

class CSnDatToolApp : public CWinAppEx
{
public:
	CSnDatToolApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CSnDatToolApp theApp;