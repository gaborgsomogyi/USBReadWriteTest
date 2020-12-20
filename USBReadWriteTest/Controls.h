#pragma once


#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "resource.h"


class SimpleControl
{
public:
	SimpleControl(HWND hwndParent, int id, BOOL initialState = TRUE) :
		_hWndParent(hwndParent),
		_hWnd(GetDlgItem(hwndParent, id)),
		_id(id)
	{
		if (!initialState) SetEnable(FALSE);
	}

	void SetShow(BOOL s) { if (s) ::ShowWindow(_hWnd, SW_SHOW); else ::ShowWindow(_hWnd, SW_HIDE); }
	BOOL IsVisible() { return(::IsWindowVisible(_hWnd)); }
	void SetFocus() { ::SetFocus(_hWnd); }
	void SetEnable(BOOL e) { ::EnableWindow(_hWnd, e); }
	HWND HwndParent() const { return _hWndParent; }
	HWND Hwnd() const { return _hWnd; }
	int id() const { return _id; }

protected:
	int _id;
	HWND _hWndParent;
	HWND _hWnd;
};

class Group : public SimpleControl
{
public:
	Group(HWND hwndParent, int id, BOOL initialState = TRUE) :
		SimpleControl(hwndParent, id, initialState) {}

	void SetName(char const* newName) { SendMessage(_hWnd, WM_SETTEXT, 0, (LPARAM)newName); }
};

class Button : public SimpleControl
{
public:
	Button(HWND hwndParent, int id, BOOL initialState = TRUE) :
		SimpleControl(hwndParent, id, initialState) {}

	void SetName(char const* newName) { SendMessage(_hWnd, WM_SETTEXT, 0, (LPARAM)newName); }
};

class Slider : public SimpleControl
{
public:
	Slider(HWND hwndParent, int id, BOOL initialState = TRUE) :
		SimpleControl(hwndParent, id, initialState) {}

	LRESULT GetPos(void) { return SendMessage(_hWnd, TBM_GETPOS, 0, 0); }
	void SetPos(int pos, bool refresh = true) { SendMessage(_hWnd, TBM_SETPOS, (WPARAM)refresh, (LPARAM)pos); }
	void SetRange(int min, int max) { SendMessage(_hWnd, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(min, max)); }
	LRESULT GetRangeMin() { return SendMessage(_hWnd, TBM_GETRANGEMIN, (WPARAM)0, (LPARAM)0); }
	LRESULT GetRangeMax() { return SendMessage(_hWnd, TBM_GETRANGEMAX, (WPARAM)0, (LPARAM)0); }
};

class TreeView : public SimpleControl
{
public:
	char tmpbuf[MAX_PATH] = { 0 };

	TreeView(HWND hwndParent, int id, BOOL initialState = TRUE) :
		SimpleControl(hwndParent, id, initialState) {}

	bool GetCheck(HTREEITEM hti)
	{
		TVITEM item;
		item.hItem = hti;
		item.stateMask = TVIF_STATE;
		SendMessage(_hWnd, TVM_GETITEM, (WPARAM)0, (LPARAM)&item);
		if (item.state & INDEXTOSTATEIMAGEMASK(2))
			return true;
		else
			return false;
	}

	void SetCheck(HTREEITEM hti, bool check)
	{
		TVITEM item;
		item.hItem = hti;
		item.mask = TVIF_STATE;
		item.stateMask = TVIS_STATEIMAGEMASK;
		if (check)
			item.state = INDEXTOSTATEIMAGEMASK(2);
		else
			item.state = INDEXTOSTATEIMAGEMASK(1);
		SendMessage(_hWnd, TVM_SETITEM, (WPARAM)0, (LPARAM)&item);
	}

	void SetCheckTree(HTREEITEM item, bool check)
	{
		HTREEITEM child;
		do
		{
			SetCheck(item, check);
			if (child = GetChild(item))
				SetCheckTree(child, check);
		} while (item = GetNext(item));
	}

	HTREEITEM GetChild(HTREEITEM item)
	{
		return (HTREEITEM)SendMessage(_hWnd, TVM_GETNEXTITEM, (WPARAM)TVGN_CHILD, (LPARAM)item);
	}

	HTREEITEM GetNext(HTREEITEM item)
	{
		return (HTREEITEM)SendMessage(_hWnd, TVM_GETNEXTITEM, (WPARAM)TVGN_NEXT, (LPARAM)item);
	}

	HTREEITEM GetPrev(HTREEITEM item)
	{
		return (HTREEITEM)SendMessage(_hWnd, TVM_GETNEXTITEM, (WPARAM)TVGN_PREVIOUS, (LPARAM)item);
	}

	HTREEITEM GetRoot(void)
	{
		return (HTREEITEM)SendMessage(_hWnd, TVM_GETNEXTITEM, (WPARAM)TVGN_ROOT, (LPARAM)0);
	}
	HTREEITEM GetParent(HTREEITEM item)
	{
		return(HTREEITEM)SendMessage(_hWnd, TVM_GETNEXTITEM, (WPARAM)TVGN_PARENT, (LPARAM)item);
	}

	char* GetItemName(HTREEITEM item)
	{
		TVITEM tvi;
		tvi.hItem = item;
		tvi.mask = TVIF_TEXT;
		tvi.pszText = tmpbuf;
		tvi.cchTextMax = MAX_PATH;
		SendMessage(_hWnd, TVM_GETITEM, (WPARAM)0, (LPARAM)&tvi);
		return tmpbuf;
	}

	void* GetUserData(HTREEITEM item)
	{
		TVITEM tvi;
		tvi.hItem = item;
		tvi.mask = TVIF_PARAM;
		SendMessage(_hWnd, TVM_GETITEM, (WPARAM)0, (LPARAM)&tvi);
		return (void*)tvi.lParam;
	}

	void SetItemName(HTREEITEM item, char* name)
	{
		TVITEM tvi;
		tvi.hItem = item;
		tvi.mask = TVIF_TEXT;
		tvi.pszText = name;
		tvi.cchTextMax = (int)strlen(name);
		SendMessage(_hWnd, TVM_SETITEM, (WPARAM)0, (LPARAM)&tvi);
	}

	void SetItemImage(HTREEITEM item, int im)
	{
		TVITEM tvi;
		tvi.hItem = item;
		tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.iImage = im;
		tvi.iSelectedImage = im;
		SendMessage(_hWnd, TVM_SETITEM, (WPARAM)0, (LPARAM)&tvi);
	}

	void DeleteItem(HTREEITEM item)
	{
		SendMessage(_hWnd, TVM_DELETEITEM, (WPARAM)0, (LPARAM)item);
	}

	void ClearAll(void)
	{
		HTREEITEM root = GetRoot();
		if (root) SendMessage(_hWnd, TVM_DELETEITEM, (WPARAM)0, (LPARAM)root);
	}

	void SetExpand(HTREEITEM item, bool expand)
	{
		if (expand) SendMessage(_hWnd, TVM_EXPAND, TVE_EXPAND, (LPARAM)item);
		else SendMessage(_hWnd, TVM_EXPAND, TVE_COLLAPSE, (LPARAM)item);
	}

	void Sort(HTREEITEM item)
	{
		SendMessage(_hWnd, TVM_SORTCHILDREN, 0, (LPARAM)item);
	}

	HTREEITEM Add(HTREEITEM parent, char* text, LPARAM userdata = 0)
	{
		TV_INSERTSTRUCT tvinsert;
		tvinsert.hParent = parent;
		tvinsert.hInsertAfter = TVI_LAST;
		tvinsert.item.pszText = text;
		tvinsert.item.lParam = userdata;
		tvinsert.item.mask = TVIF_TEXT | TVIF_PARAM;
		return (HTREEITEM)SendMessage(_hWnd, TVM_INSERTITEM, 0, (LPARAM)&tvinsert);
	}
};

class ListBox : public SimpleControl
{
public:
	LRESULT c_line = 0;
	char linebuf[MAX_PATH] = { 0 };

	ListBox(HWND hwndParent, int id, BOOL initialState = TRUE) :
		SimpleControl(hwndParent, id, initialState)
	{
		c_line = -1;
	}

	LRESULT GetSel(void)
	{
		c_line = SendMessage(_hWnd, LB_GETCURSEL, 0, (LPARAM)0);
		return c_line;
	}

	void InitStorage(int maxnum, int memsize) { SendMessage(_hWnd, LB_INITSTORAGE, (WPARAM)maxnum, (LPARAM)memsize); }
	void Clear(void) { SendMessage(_hWnd, LB_RESETCONTENT, 0, (LPARAM)0); }

	void SetSel(void)
	{
		if (c_line >= 0)
			SendMessage(_hWnd, LB_SETCURSEL, (WPARAM)c_line, (LPARAM)0);
	}

	void SetSel(WPARAM sel)
	{
		c_line = sel;
		SendMessage(_hWnd, LB_SETCURSEL, (WPARAM)c_line, (LPARAM)0);
	}

	LRESULT GetCount(void) { return SendMessage(_hWnd, LB_GETCOUNT, (WPARAM)0, (LPARAM)0); }

	char* GetLine(LPARAM linepos)
	{
		memset(linebuf, 0, sizeof(linebuf));
		SendMessage(_hWnd, LB_GETTEXT, (WPARAM)linepos, (LPARAM)linebuf);
		return linebuf;
	}

	char* GetCurrLine(void)
	{
		memset(linebuf, 0, sizeof(linebuf));
		GetSel();
		if (c_line < 0) return NULL;
		SendMessage(_hWnd, LB_GETTEXT, (WPARAM)c_line, (LPARAM)linebuf);
		return linebuf;
	}

	void AddLine(char const* str)
	{
		SendMessage(_hWnd, LB_ADDSTRING, 0, (LPARAM)str);
		GetSel();
	}

	void SetLine(char const* str, int i) { SendMessage(_hWnd, LB_SETITEMDATA, (WPARAM)i, (LPARAM)str); }

	void Save(char* name)
	{
		FILE* f = NULL;
		if (!fopen_s(&f, name, "w"))
		{
			for (int i = 0; i < GetCount(); i++)
			{
				fputs(GetLine(i), f);
				fputc('\n', f);
			}
			fclose(f);
		}
	}

	void Load(char* name)
	{
		FILE* f = NULL;
		char buf[MAX_PATH + 256];
		memset(buf, 0, sizeof(buf));
		Clear();
		if (!fopen_s(&f, name, "r"))
		{
			while (fgets(buf, MAX_PATH + 254, f))
			{
				buf[strlen(buf) - 1] = '\0';
				AddLine(buf);
			}
			fclose(f);
		}
	}

	void AddLine(char const* str, WPARAM pos)
	{
		if (pos < 0)
			SendMessage(_hWnd, LB_INSERTSTRING, 0, (LPARAM)str);
		else
			SendMessage(_hWnd, LB_INSERTSTRING, (WPARAM)pos, (LPARAM)str);
	}

	void RemoveLine(void)
	{
		GetSel();
		if (c_line >= 0)
			SendMessage(_hWnd, LB_DELETESTRING, c_line, (LPARAM)0);
	}

	void MoveUp(void)
	{
		GetSel();
		if (!c_line) return;
		GetLine(c_line);
		RemoveLine();
		AddLine(linebuf, c_line - 1);
		SetSel(c_line - 1);
	}

	void MoveDown(void)
	{
		GetSel();
		LRESULT count = GetCount();
		if (c_line == count - 1) return;
		GetLine(c_line);
		RemoveLine();
		AddLine(linebuf, c_line + 1);
		SetSel(c_line + 1);
	}

	void Invalidate(void)
	{
		SendMessage(_hWndParent, WM_COMMAND, MAKEWPARAM(_id, LBN_SELCHANGE), (LPARAM)_hWnd);
	}
};

class ComboBox : public SimpleControl
{
public:
	LRESULT c_line = 0;
	char linebuf[MAX_PATH] = { 0 };

	ComboBox(HWND hwndParent, int id, BOOL initialState = TRUE) :
		SimpleControl(hwndParent, id, initialState)
	{
		c_line = -1;
	}

	LRESULT GetSel(void)
	{
		c_line = SendMessage(_hWnd, CB_GETCURSEL, 0, (LPARAM)0);
		return c_line;
	}

	void Clear(void) { SendMessage(_hWnd, CB_RESETCONTENT, 0, (LPARAM)0); }

	void SetSel(void)
	{
		if (c_line >= 0)
			SendMessage(_hWnd, CB_SETCURSEL, (WPARAM)c_line, (LPARAM)0);
	}

	void SetSel(int sel)
	{
		c_line = sel;
		SendMessage(_hWnd, CB_SETCURSEL, (WPARAM)c_line, (LPARAM)0);
	}

	LRESULT GetCount(void) { return SendMessage(_hWnd, CB_GETCOUNT, (WPARAM)0, (LPARAM)0); }

	char* GetCurrLine(void)
	{
		memset(linebuf, 0, sizeof(linebuf));
		GetSel();
		if (c_line < 0) return NULL;
		SendMessage(_hWnd, CB_GETLBTEXT, (WPARAM)c_line, (LPARAM)linebuf);
		return linebuf;
	}

	void AddLine(char const* str)
	{
		SendMessage(_hWnd, CB_ADDSTRING, 0, (LPARAM)str);
		GetSel();
	}

	void SetLine(char const* str, int i) { SendMessage(_hWnd, CB_SETITEMDATA, (WPARAM)i, (LPARAM)str); }

	void AddLine(char const* str, int pos)
	{
		if (pos < 0)
			SendMessage(_hWnd, CB_INSERTSTRING, 0, (LPARAM)str);
		else
			SendMessage(_hWnd, CB_INSERTSTRING, (WPARAM)pos, (LPARAM)str);
	}

	void RemoveLine(void)
	{
		GetSel();
		if (c_line >= 0)
			SendMessage(_hWnd, CB_DELETESTRING, c_line, (LPARAM)0);
	}

	void Invalidate(void)
	{
		SendMessage(_hWndParent, WM_COMMAND, MAKEWPARAM(_id, CBN_SELCHANGE), (LPARAM)_hWnd);
	}
};

class CheckBox : public Button
{
public:
	CheckBox(HWND hwndParent, int id, BOOL initialState = TRUE) :
		Button(hwndParent, id, initialState) {}

	bool IsChecked() { return(SendMessage(_hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED); }
	void SetCheck(BOOL check) { if (check) SendMessage(_hWnd, BM_SETCHECK, (WPARAM)BST_CHECKED, 0); else SendMessage(_hWnd, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0); }
};

class RadioButton : public Button
{
public:
	RadioButton(HWND hwndParent, int id, BOOL initialState = TRUE) :
		Button(hwndParent, id, initialState) {}

	BOOL IsSelected() { return(SendMessage(_hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED); }
	void Select() { SendMessage(_hWnd, BM_SETCHECK, (WPARAM)BST_CHECKED, 0); }
	void DeSelect() { SendMessage(_hWnd, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0); }
};

class EditBox : public SimpleControl
{
public:
	char text[MAX_PATH] = { 0 };

	EditBox(HWND hwndParent, int id, BOOL initialState = TRUE) :
		SimpleControl(hwndParent, id, initialState) {}

	void SetString(const char* buf) { SendMessage(_hWnd, WM_SETTEXT, 0, (LPARAM)buf); }

	void SetString(float f, bool clamp = false)
	{
		if (clamp) sprintf_s(text, MAX_PATH, "%5.2f\0", f);
		else sprintf_s(text, MAX_PATH, "%f\0", f);
		SendMessage(_hWnd, WM_SETTEXT, 0, (LPARAM)text);
	}

	void SetString(int i)
	{
		sprintf_s(text, MAX_PATH, "%ld", i);
		SendMessage(_hWnd, WM_SETTEXT, 0, (LPARAM)text);
	}

	void SetString(unsigned int i)
	{
		sprintf_s(text, MAX_PATH, "%ld", i);
		SendMessage(_hWnd, WM_SETTEXT, 0, (LPARAM)text);
	}

	void SetReadonly(BOOL readonly) { SendMessage(_hWnd, EM_SETREADONLY, (WPARAM)readonly, 0); }

	static BOOL IsChanged(int code) { return code == EN_CHANGE; }
	int GetLength() { return (int)(SendMessage(_hWnd, WM_GETTEXTLENGTH, 0, 0)); }

	char* GetString(void)
	{
		SendMessage(_hWnd, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)text);
		return text;
	}

	float GetFloat(void)
	{
		float out = 0.f;
		SendMessage(_hWnd, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)text);
		if (strlen(text) > 0)
		{
			sscanf_s(text, "%f", &out);
		}
		return out;
	}

	int GetInt(void)
	{
		int out = 0;
		SendMessage(_hWnd, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)text);
		if (strlen(text) > 0)
		{
			sscanf_s(text, "%d", &out);
		}
		return out;
	}

	void Select() { SendMessage(_hWnd, EM_SETSEL, 0, -1); }

	void Invalidate(void)
	{
		SendMessage(_hWndParent, WM_COMMAND, MAKEWPARAM(_id, EN_CHANGE), (LPARAM)_hWnd);
	}
};

class ScrollBar : public SimpleControl
{
public:
	ScrollBar(HWND hwndParent, int id, BOOL initialState = TRUE) :
		SimpleControl(hwndParent, id, initialState) {}

	void SetRange(int min, int max)
	{
		SetScrollRange(_hWnd, SB_CTL, min, max, TRUE);
	}

	void SetPos(int pos)
	{
		SetScrollPos(_hWnd, SB_CTL, pos, TRUE);
	}

	void SetPage(int pos)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_PAGE;
		SetScrollInfo(_hWnd, SB_CTL, &si, TRUE);
	}

	int GetPos(int pos)
	{
		return GetScrollPos(_hWnd, SB_CTL);
	}
};

class ProgressBar : public SimpleControl
{
public:
	ProgressBar(HWND hwndParent, int id, BOOL initialState = TRUE) :
		SimpleControl(hwndParent, id, initialState) {}

	LRESULT GetPos(void) { return SendMessage(_hWnd, PBM_GETPOS, 0, 0); }
	void SetPos(int pos) { SendMessage(_hWnd, PBM_SETPOS, (WPARAM)pos, (LPARAM)0); }
	void SetRange(int min, int max) { SendMessage(_hWnd, PBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(min, max)); }
};

class Static : public SimpleControl
{
public:
	Static(HWND hwndParent, int id, BOOL initialState = TRUE) :
		SimpleControl(hwndParent, id, initialState) {}
};
