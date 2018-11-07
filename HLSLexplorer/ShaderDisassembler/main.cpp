#include "PCH.h"
#include "App.h"

wxIMPLEMENT_APP_NO_MAIN(CMyApp);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	return wxEntry(hInstance, hPrevInstance, lpCmdLine, nShowCmd);	
}