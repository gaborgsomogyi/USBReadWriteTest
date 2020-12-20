#pragma once


#include "controls.h"


using namespace std;


class MainDialogCtrl
{
public:
	HWND m_hwnd;
	ComboBox m_guiAnimList;
	ProgressBar m_progress;
	EditBox m_actionEdit;
	EditBox m_writeErrorEdit;
	EditBox m_readErrorEdit;
	EditBox m_dataErrorEdit;
	EditBox m_progressEdit;
	Button m_guiTest;
	Button m_guiExit;
	mutable HANDLE m_testActionHandle = INVALID_HANDLE_VALUE;

	MainDialogCtrl(HWND hwnd);
	~MainDialogCtrl();

	void Command(HWND hwnd, int controlID, int command);
	void SetGUIDefaults(void);

private:

	void TestDevice(void);
};
