#pragma once


#ifndef __AFXWIN_H__
	#error "Include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"


class CGDEApp : public CWinApp
{
public:
    // Construction

    CGDEApp();

public:
    // Overrides

    virtual BOOL InitInstance();

private:
    // Implementation

	DECLARE_MESSAGE_MAP()

    // Attributes

};


extern CGDEApp theApp;
