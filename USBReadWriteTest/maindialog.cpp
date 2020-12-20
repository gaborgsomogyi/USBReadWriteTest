#include <windows.h>
#include <initguid.h>
#include <usbiodef.h>
#include <setupapi.h>
#include "controls.h"
#include "maindialog.h"
#include "ErrorHandler.h"
#include "USBReadWriteTest.h"


using namespace std;


#pragma comment (lib, "setupapi.lib")


MainDialogCtrl::MainDialogCtrl(HWND hwnd) :
	m_hwnd(hwnd),
	m_guiAnimList(hwnd, IDC_DEVICE_COMBO),
	m_progress(hwnd, IDC_PROGRESS),
	m_actionEdit(hwnd, IDC_ACTION_EDIT),
	m_progressEdit(hwnd, IDC_PROGRESS_EDIT),
	m_guiTest(hwnd, IDC_BUT_TEST),
	m_guiExit(hwnd, IDC_BUT_EXIT)
{
}

MainDialogCtrl::~MainDialogCtrl(void)
{
}

void MainDialogCtrl::Command(HWND hwnd, int controlID, int command)
{
	switch (command)
	{
	case BN_CLICKED:
		switch (controlID)
		{
		case IDC_BUT_TEST:
			TestDevice();
			break;

		case IDC_BUT_EXIT:
			DestroyWindow(hwnd);
			PostQuitMessage(0);
			break;
		}
		break;
	}
}

void MainDialogCtrl::SetGUIDefaults(void)
{
	if (m_testActionHandle != INVALID_HANDLE_VALUE)
		return;

	m_guiAnimList.Clear();
	m_progress.SetRange(0, 100);
	m_progress.SetPos(0);
	m_actionEdit.SetEnable(false);
	m_progressEdit.SetEnable(false);

	GUID diskClassDeviceInterfaceGuid = GUID_DEVINTERFACE_DISK;
	HDEVINFO hDevInfo = SetupDiGetClassDevs(&diskClassDeviceInterfaceGuid, NULL, 0, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	if (hDevInfo != INVALID_HANDLE_VALUE)
	{
		SP_DEVICE_INTERFACE_DATA devIntfData = { 0 };
		devIntfData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		DWORD dwMemberIdx = 0;
		while (SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &diskClassDeviceInterfaceGuid, dwMemberIdx, &devIntfData))
		{
			SP_DEVINFO_DATA devData = { 0 };
			devData.cbSize = sizeof(SP_DEVINFO_DATA);
			DWORD dwSize = 0;
			SetupDiGetDeviceInterfaceDetail(hDevInfo, &devIntfData, NULL, 0, &dwSize, NULL);

			HANDLE hHeap = GetProcessHeap();
			PSP_DEVICE_INTERFACE_DETAIL_DATA devIntfDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize);
			devIntfDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			if (SetupDiGetDeviceInterfaceDetail(hDevInfo, &devIntfData, devIntfDetailData, dwSize, &dwSize, &devData))
			{
				HANDLE disk = CreateFile(devIntfDetailData->DevicePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (disk == INVALID_HANDLE_VALUE)
					ErrorExit(TEXT("CreateFile"));
				
				DISK_GEOMETRY driveInfo = { 0 };
				DWORD dwResult = 0;
				if (!DeviceIoControl(disk, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &driveInfo, sizeof(driveInfo), &dwResult, NULL))
					ErrorExit(TEXT("IOCTL_DISK_GET_DRIVE_GEOMETRY"));

				if (driveInfo.MediaType == RemovableMedia)
					m_guiAnimList.AddLine(devIntfDetailData->DevicePath);

				CloseHandle(disk);
			}
			HeapFree(hHeap, 0, devIntfDetailData);

			dwMemberIdx += 1;
		}

		SetupDiDestroyDeviceInfoList(hDevInfo);
	}

	if (m_guiAnimList.GetCount() != 0)
		m_guiAnimList.SetSel(0);
}

DWORD WINAPI testActionThread(LPVOID lpParameter)
{
	MainDialogCtrl* mainDialogCtrl = (MainDialogCtrl*)lpParameter;
	int progress = 0;
	int lprogress = -1;
	char buf[256];
	ULONGLONG unableToWriteSectors = 0;
	ULONGLONG unableToReadSectors = 0;
	ULONGLONG badSectors = 0;

	HANDLE disk = CreateFile(mainDialogCtrl->m_guiAnimList.GetCurrLine(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (disk == INVALID_HANDLE_VALUE)
		ErrorExit(TEXT("CreateFile"));

	DISK_GEOMETRY driveInfo;
	DWORD dwResult;
	if (!DeviceIoControl(disk, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &driveInfo, sizeof(driveInfo), &dwResult, NULL))
		ErrorExit(TEXT("IOCTL_DISK_GET_DRIVE_GEOMETRY"));

	ULONGLONG lastSector = driveInfo.Cylinders.QuadPart * driveInfo.TracksPerCylinder * driveInfo.SectorsPerTrack;
	ULONGLONG totalSectors = lastSector + 1;
	ULONGLONG totalBytes = totalSectors * driveInfo.BytesPerSector;

	HANDLE hHeap = GetProcessHeap();
	unsigned char* sectorData = (unsigned char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, driveInfo.BytesPerSector);
	for (DWORD i = 0; i < driveInfo.BytesPerSector; ++i)
		sectorData[i] = rand() & 0xFF;

	progress = 0;
	lprogress = -1;
	mainDialogCtrl->m_actionEdit.SetString("Writing");
	for (ULONGLONG i = 0; i < totalSectors; ++i)
	{
		progress = i * 100 / totalSectors;
		if (progress != lprogress)
		{
			mainDialogCtrl->m_progress.SetPos(progress);
			lprogress = progress;
		}

		if (i % 0x8F == 0)
		{
			sprintf_s(buf, sizeof(buf), "%lld/%lld", i, totalSectors);
			mainDialogCtrl->m_progressEdit.SetString(buf);
		}
		DWORD dwBytesWritten = 0;
		bool writeSuccess = WriteFile(disk, sectorData, driveInfo.BytesPerSector, &dwBytesWritten, NULL);
		if (!writeSuccess || dwBytesWritten != driveInfo.BytesPerSector)
			unableToWriteSectors += 1;
	}

	progress = 0;
	lprogress = -1;
	unsigned char* readSectorData = (unsigned char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, driveInfo.BytesPerSector);
	mainDialogCtrl->m_actionEdit.SetString("Reading");
	for (ULONGLONG i = 0; i < totalSectors; ++i)
	{
		progress = i * 100 / totalSectors;
		if (progress != lprogress)
		{
			mainDialogCtrl->m_progress.SetPos(progress);
			lprogress = progress;
		}

		if (i % 0x8F == 0)
		{
			sprintf_s(buf, sizeof(buf), "%lld/%lld", i, totalSectors);
			mainDialogCtrl->m_progressEdit.SetString(buf);
		}
		DWORD dwBytesRead = 0;
		bool readSuccess = ReadFile(disk, sectorData, driveInfo.BytesPerSector, &dwBytesRead, NULL);
		if (!readSuccess || dwBytesRead != driveInfo.BytesPerSector)
			unableToReadSectors += 1;
		if (readSuccess && memcmp(readSectorData, sectorData, driveInfo.BytesPerSector) != 0)
			badSectors += 1;
	}

	HeapFree(hHeap, 0, readSectorData);
	HeapFree(hHeap, 0, sectorData);
	CloseHandle(disk);

	sprintf_s(buf, sizeof(buf), "Device tested with the following statistics: \nUnable to write sectors: %lld\nUnable to read sectors: %lld\nBad sectors: %lld", unableToWriteSectors, unableToReadSectors, badSectors);
	MessageBox(NULL, buf, TEXT("Info"), MB_OK);

	mainDialogCtrl->m_guiAnimList.SetEnable(true);
	mainDialogCtrl->m_guiTest.SetEnable(true);
	mainDialogCtrl->m_guiExit.SetEnable(true);

	mainDialogCtrl->m_testActionHandle = INVALID_HANDLE_VALUE;

	return 0;
}

void MainDialogCtrl::TestDevice(void)
{
	if (m_guiAnimList.GetSel() == -1)
	{
		MessageBox(NULL, "Please select a device", TEXT("Error"), MB_OK | MB_ICONERROR);
		return;
	}

	m_guiAnimList.SetEnable(false);
	m_guiTest.SetEnable(false);
	m_guiExit.SetEnable(false);

	m_testActionHandle = CreateThread(0, 0, testActionThread, this, 0, NULL);
}
