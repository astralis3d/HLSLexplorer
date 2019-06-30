#include "PCH.h"
#include "App.h"

#include <wx/xrc/xmlres.h>

extern void InitXmlResource();

bool CMyApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	// Init XRC system
	InitializeXMLResourcesSystem();


	// create the main application window
	CMyFrame *frame = new CMyFrame("HLSLexplorer 1.01");

	// and show it (the frames, unlike simple controls, are not shown when
	// created initially)
	frame->Show(true);

	// success: wxApp::OnRun() will be called which will enter the main message
	// loop and the application will run. If we returned false here, the
	// application would exit immediately.
	return true;
}

void CMyApp::InitializeXMLResourcesSystem()
{
	// For proper handling *.png images within resource files.
	wxImage::AddHandler( new wxPNGHandler );

	wxXmlResource::Get()->InitAllHandlers();
	InitXmlResource();
}
