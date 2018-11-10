#ifndef __APP_H__
#define __APP_H__

#pragma once

#include <wx/wx.h>
#include "MainFrame.h"

class CMyApp : public wxApp
{
public:
	virtual bool OnInit()	wxOVERRIDE;

private:
	void InitializeXMLResourcesSystem();
};



#endif