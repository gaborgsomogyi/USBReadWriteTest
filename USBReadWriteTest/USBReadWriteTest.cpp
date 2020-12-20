#include <windows.h>
#include <initguid.h>
#include <usbiodef.h>
#include <dbt.h>
#include "resource.h"
#include "USBReadWriteTest.h"
#include "maindialog.h"
#include "ErrorHandler.h"


#define VERSION 0.1f


HINSTANCE g_hInst = NULL;
HWND g_hMainDialog = NULL;
MainDialogCtrl* g_mainDialog = NULL;
HDEVNOTIFY g_hDeviceNotify = NULL;

INT_PTR CALLBACK OpenProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	g_hInst = hInstance;
	MSG msg;
	BOOL bQuit = FALSE;

	g_hMainDialog = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAINDIALOG), NULL, OpenProc);
	if (g_hMainDialog)
	{
		ShowWindow(g_hMainDialog, SW_SHOW);
	}
	else
	{
		MessageBox(NULL, "CreateDialog returned NULL", "Error", MB_OK | MB_ICONERROR);
		exit(-1);
	}

	g_mainDialog->SetGUIDefaults();

	while (!bQuit)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bQuit = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	DestroyWindow(g_hMainDialog);

	return 0;
}

void DoRegisterDeviceInterfaceToHwnd(GUID InterfaceClassGuid, HWND hWnd, HDEVNOTIFY* hDeviceNotify)
{
	DEV_BROADCAST_DEVICEINTERFACE NotificationFilter = { 0 };
	NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	NotificationFilter.dbcc_classguid = InterfaceClassGuid;

	*hDeviceNotify = RegisterDeviceNotification(hWnd, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);

	if (*hDeviceNotify == NULL)
		ErrorExit(TEXT("RegisterDeviceNotification"));
}

INT_PTR CALLBACK OpenProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
	{
		InitCommonControls();
		char buf[256];
		sprintf_s(buf, sizeof(buf), "USB drive read-write test by Gabor Somogyi version: %1.1f %s %s", VERSION, __DATE__, __TIME__);
		SetWindowText(hwnd, buf);
		g_mainDialog = new MainDialogCtrl(hwnd);
		DoRegisterDeviceInterfaceToHwnd(GUID_DEVINTERFACE_DISK, g_hMainDialog, &g_hDeviceNotify);
		return false;
	}
	break;

	case WM_DEVICECHANGE:
		g_mainDialog->SetGUIDefaults();
		break;

	case WM_COMMAND:
		if (g_mainDialog) g_mainDialog->Command(hwnd, LOWORD(wParam), HIWORD(wParam));
		if (LOWORD(wParam) == WM_DESTROY)
		{
			DestroyWindow(g_hMainDialog);
			PostQuitMessage(0);
			return FALSE;
		}
		return TRUE;
		break;

	default:
		return FALSE;
	}
	return TRUE;
}
