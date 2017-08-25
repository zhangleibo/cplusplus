#include <Windows.h>
#include <Stdio.h>
#include <Tchar.h>
#include <Msctf.h>
#include <Ctffunc.h>
#include <Commctrl.h>
#pragma comment(lib,"Comctl32.lib")
#include <Shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"Imm32.lib")

#include "Resource.h"

HWND		g_hPropSheetDlg;

typedef struct _IMEINFO
{
	BOOL			bTsfFrame;//是否是TSF框架的输入法
    LANGID			langid;//语言标识
	TCHAR			szDesc[256];//输入法描述

	HKL				hKL;//键盘布局句柄
	TCHAR			szKLID[9];//键盘布局标识

	CLSID			clsid;//类标识
	GUID			catid;//类别标识
	GUID			guidProfile;

	_IMEINFO		*pRight;
	_IMEINFO		*pBelow;

}IMEINFO, *PIMEINFO;

//获取键盘布局描述
BOOL GetKeyboardLayoutDescription(LPTSTR szKLID, LPTSTR lpszDescription)
{
	LSTATUS			lResult;
	BOOL			bRet;
	HKEY			hKey;
	TCHAR			szSubKey[1024];
	TCHAR			szValue[256];
	DWORD			cbValue;
	DWORD			dwType;
	
	bRet = FALSE;
	lstrcpy(szSubKey, _T("SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts\\"));
	lstrcat(szSubKey, szKLID);
	lResult = RegOpenKey(HKEY_LOCAL_MACHINE, szSubKey, &hKey);
	if(lResult == ERROR_SUCCESS)
	{
		dwType = REG_SZ;
		cbValue = sizeof(szValue);
		lResult = RegQueryValueEx(hKey, _T("Layout Display Name"), 0, &dwType, (LPBYTE)szValue, &cbValue);//先查询是否有显示名
		if(lResult == ERROR_SUCCESS)
		{
			if(SHLoadIndirectString(szValue, szValue, 512, 0) == NULL)//加载重定向字符串
			{
				lstrcpy(lpszDescription, szValue);
				bRet = TRUE;
			}
		}
		else
		{
			dwType = REG_SZ;
			cbValue = sizeof(szValue);
			lResult = RegQueryValueEx(hKey, _T("Layout Text"), 0, &dwType, (LPBYTE)szValue, &cbValue);//没有的话再查询布局文本
			if(lResult == ERROR_SUCCESS)
			{
				lstrcpy(lpszDescription, szValue);
				bRet = TRUE;
			}
		}
		RegCloseKey(hKey);
	}
		
	return bRet;
}

//是否是隐藏虚设
BOOL IsHiddenDummy(LANGID langid, LPTSTR szKLID)
{
	LSTATUS			lResult;
	BOOL			bRet;
	HKEY			hKey;
	DWORD			cbMaxValueNameLen;
	DWORD			cbMaxValueLen;
	DWORD			cbValueNameLen;
	DWORD			cbValueLen;
	LPTSTR			lpValueName;
	LPTSTR			lpValue;
	DWORD			cValue;
	DWORD			i;
	DWORD			v;

	bRet = FALSE;
	lResult = RegOpenKey(HKEY_CURRENT_USER, _T("Software\\Microsoft\\CTF\\HiddenDummyLayouts"), &hKey);
	if(lResult == ERROR_SUCCESS)
	{
		lResult = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &cValue, &cbMaxValueNameLen, &cbMaxValueLen, NULL, NULL);
		if(lResult == ERROR_SUCCESS)
		{
			lpValueName = (LPTSTR)LocalAlloc(LMEM_FIXED, cbMaxValueNameLen * sizeof(TCHAR) + sizeof(TCHAR));
			lpValue = (LPTSTR)LocalAlloc(LMEM_FIXED, cbMaxValueLen);

			for(i=0; i<cValue; i++)
			{
				cbValueNameLen = cbMaxValueNameLen * sizeof(TCHAR) + sizeof(TCHAR);
				cbValueLen = cbMaxValueLen;
				lResult = RegEnumValue(hKey, i, lpValueName, &cbValueNameLen, NULL, NULL, (LPBYTE)lpValue, &cbValueLen);
				if(lResult == ERROR_SUCCESS)
				{
					v = _tcstoul(lpValueName, NULL, 16);
					if(langid == (LANGID)LOWORD(v))
					{
						if(lstrcmpi(lpValue, szKLID) == 0)
						{
							bRet = TRUE;
							break;
						}
					}
				}
			}

			LocalFree(lpValueName);
			LocalFree(lpValue);
		}
		RegCloseKey(hKey);
	}

	return bRet;
}

//由索引获取键盘布局标识，没有使用
BOOL GetKLIDByIndex(LPTSTR lpszIndex, LPTSTR szKLID)
{
	LSTATUS			lResult;
	BOOL			bRet;
	HKEY			hKey;
	TCHAR			szValue[9];
	TCHAR			szTemp[9];
	DWORD			cbValue;
	DWORD			dwType;

	bRet = FALSE;
	lResult = RegOpenKey(HKEY_CURRENT_USER, _T("Keyboard Layout\\Preload"), &hKey);
	if(lResult == ERROR_SUCCESS)
	{
		dwType = REG_SZ;
		cbValue = sizeof(szTemp);
		lResult = RegQueryValueEx(hKey, lpszIndex, 0, &dwType, (LPBYTE)szTemp, &cbValue);
		RegCloseKey(hKey);

		if(lResult == ERROR_SUCCESS)
		{
			lResult = RegOpenKey(HKEY_CURRENT_USER, _T("Keyboard Layout\\Substitutes"), &hKey);
			if(lResult == ERROR_SUCCESS)
			{
				dwType = REG_SZ;
				cbValue = sizeof(szValue);
				lResult = RegQueryValueEx(hKey, szTemp, 0, &dwType, (LPBYTE)szValue, &cbValue);
				RegCloseKey(hKey);
				
				if(lResult == ERROR_SUCCESS)
					lstrcpy(szKLID, szValue);
				else
					lstrcpy(szKLID, szTemp);

				bRet = TRUE;
			}
		}
	}

	return bRet;
}

//插入条目到TreeView控件
HTREEITEM InsertImeTree(HWND hTree, HTREEITEM hParent, LPTSTR pszText, PIMEINFO ppRoot)
{
	TV_INSERTSTRUCT		tvInsert;

	tvInsert.hParent = hParent;
	tvInsert.hInsertAfter = TVI_LAST;
	tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
	tvInsert.item.pszText = pszText;
	tvInsert.item.cchTextMax = lstrlen(tvInsert.item.pszText);
	tvInsert.item.lParam = (LPARAM)ppRoot;
	return TreeView_InsertItem(hTree, &tvInsert);
}

//添加条目到链表
VOID AddImeItem(PIMEINFO *ppRoot, PIMEINFO pItem)
{
	PIMEINFO			pTemp;
	PIMEINFO			pTemp2;
	PIMEINFO			pRight;
	PIMEINFO			pBelow;

	if(*ppRoot)
	{
		pTemp = *ppRoot;
		while(pRight = pTemp)
		{
			if(pRight->langid == pItem->langid)
			{
				pTemp2 = pRight;
				while(pBelow = pTemp2)
				{
					if(pBelow->pBelow == NULL)
					{
						pBelow->pBelow = pItem;
						break;
					}

					pTemp2 = pBelow->pBelow;
				};			
				break;
			}
			else if(pRight->pRight == NULL)
			{
				pRight->pRight = pItem;
				break;
			}

			pTemp = pRight->pRight;
		};
	}
	else
	{
		*ppRoot = pItem;
	}
}

INT GetDefaultCbIndex(HWND hCombobox, PIMEINFO pImeInfo)
{
	TCHAR				szLang[256];
	TCHAR				szText[512];

	GetLocaleInfo(MAKELCID(pImeInfo->langid, SORT_DEFAULT), LOCALE_SLANGUAGE, szLang, 256);

	lstrcpy(szText, szLang);
	lstrcat(szText, _T(" - "));
	lstrcat(szText, pImeInfo->szDesc);
	return (INT)SendMessage(hCombobox, CB_FINDSTRING, -1, (LPARAM)szText);
}

VOID ImmGetImeList(PIMEINFO *ppRoot)
{
	HKL				ahList[32];
	TCHAR			szKLID[9];
	TCHAR			szDesc[256];
	PIMEINFO		pItem;
	int				nCount;
	int				i;
	
	nCount = GetKeyboardLayoutList(32, ahList);
	for(i=0; i<nCount; i++) 
	{
		ActivateKeyboardLayout(ahList[i], KLF_SETFORPROCESS);
		if(GetKeyboardLayoutName(szKLID))//获取键盘布局标识
		{
			if(GetKeyboardLayoutDescription(szKLID, szDesc))//获取描述信息
			{
				pItem = new IMEINFO;

				pItem->bTsfFrame = FALSE;
				pItem->langid = LOWORD(ahList[i]);
				lstrcpy(pItem->szDesc, szDesc);

				pItem->hKL = ahList[i];
				lstrcpy(pItem->szKLID, szKLID);

				pItem->clsid = CLSID();
				pItem->catid = GUID();
				pItem->guidProfile = GUID();

				pItem->pRight = NULL;
				pItem->pBelow = NULL;
				
				AddImeItem(ppRoot, pItem);
			}
		}		
	}	
}

VOID TsfGetImeList(PIMEINFO *ppRoot)
{
	HRESULT	hr;
	ITfInputProcessorProfiles *pProfiles;
	PIMEINFO pItem;

	hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, 
		NULL, 
		CLSCTX_INPROC_SERVER, 
		IID_ITfInputProcessorProfiles, 
		(LPVOID*)&pProfiles);
	if(SUCCEEDED(hr))
	{
		IEnumTfLanguageProfiles *pEnum;
		TF_LANGUAGEPROFILE langProfile;
		ULONG celt;
		LANGID *pLangId;
		ULONG ulCount;
		ULONG i;

		hr = pProfiles->GetLanguageList(&pLangId, &ulCount);
		if(SUCCEEDED(S_OK))
		{
			for(i=0; i<ulCount; i++)
			{
				hr = pProfiles->EnumLanguageProfiles(pLangId[i], &pEnum);
				if(SUCCEEDED(hr))
				{
					celt = 1;
					while(pEnum->Next(celt, &langProfile, &celt) == S_OK)
					{
						BOOL bEnable;
						BSTR bstrProfile;

						pProfiles->IsEnabledLanguageProfile(langProfile.clsid, 
							langProfile.langid, 
							langProfile.guidProfile,
							&bEnable);
						if(bEnable)
						{
							hr = pProfiles->GetLanguageProfileDescription(langProfile.clsid, 
								langProfile.langid, 
								langProfile.guidProfile,
								&bstrProfile);
							if(SUCCEEDED(S_OK))
							{
								pItem = new IMEINFO;

								pItem->bTsfFrame = TRUE;
								pItem->langid = langProfile.langid;
								lstrcpy(pItem->szDesc, bstrProfile);

								pItem->hKL = NULL;
								lstrcpy(pItem->szKLID, _T("00000000"));

								pItem->clsid = langProfile.clsid;
								pItem->catid = langProfile.catid;
								pItem->guidProfile = langProfile.guidProfile;

								pItem->pRight = NULL;
								pItem->pBelow = NULL;

								AddImeItem(ppRoot, pItem);

								SysFreeString(bstrProfile);
							}
						}
					}
					pEnum->Release();
				}
			}

			CoTaskMemFree(pLangId);
		}
		pProfiles->Release();
	}
}

VOID GetDefaultIme(PIMEINFO *ppRoot, PIMEINFO *ppDefault)
{
	PIMEINFO			pTemp;
	PIMEINFO			pTemp2;
	PIMEINFO			pRight;
	PIMEINFO			pBelow;
	HKL					hKL;
	
	//获取默认输入法
	SystemParametersInfo(SPI_GETDEFAULTINPUTLANG, 0, &hKL, 0);
	
	//查找指定的输入法结构
	pTemp = *ppRoot;
	while(pRight = pTemp)
	{
		pTemp2 = pRight;
		while(pBelow = pTemp2)
		{
			if(pBelow->hKL == hKL)
			{
				*ppDefault = pBelow;
				break;
			}

			pTemp2 = pBelow->pBelow;
		};

		pTemp = pRight->pRight;
	};

	//需要判断是否是Tsf框架的输入法
	if(HIWORD(hKL) == LOWORD(hKL))
	{
		pTemp = *ppRoot;
		while(pRight = pTemp)
		{
			pTemp2 = pRight;
			while(pBelow = pTemp2)
			{
				if(pBelow->bTsfFrame)
				{
					HRESULT	hr;
					CLSID clsid;
					GUID guidProfile;
					ITfInputProcessorProfiles *pProfiles;

					hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, 
						NULL, 
						CLSCTX_INPROC_SERVER, 
						IID_ITfInputProcessorProfiles, 
						(LPVOID*)&pProfiles);
					if(SUCCEEDED(hr))
					{
						hr = pProfiles->GetDefaultLanguageProfile(pBelow->langid,
							pBelow->catid,
							&clsid,
							&guidProfile);
						pProfiles->Release();

						if(SUCCEEDED(hr))
						{
							if(pBelow->clsid == clsid && pBelow->guidProfile == guidProfile)
							{
								*ppDefault = pBelow;
								break;
							}
						}
					}
				}

				pTemp2 = pBelow->pBelow;
			};

			pTemp = pRight->pRight;
		};
	}
}

VOID UpdateDefaultCb(HWND hCombobox, PIMEINFO *ppRoot)
{
	PIMEINFO			pTemp;
	PIMEINFO			pTemp2;
	PIMEINFO			pRight;
	PIMEINFO			pBelow;
	TCHAR				szLang[256];
	TCHAR				szText[512];
	INT					nIndex;
	
	pTemp = *ppRoot;
	while(pRight = pTemp)
	{
		GetLocaleInfo(MAKELCID(pRight->langid, SORT_DEFAULT), LOCALE_SLANGUAGE, szLang, 256);

		pTemp2 = pRight;
		while(pBelow = pTemp2)
		{	
			if(!IsHiddenDummy(pBelow->langid, pBelow->szKLID))
			{
				lstrcpy(szText, szLang);
				lstrcat(szText, _T(" - "));
				lstrcat(szText, pBelow->szDesc);

				nIndex = (INT)SendMessage(hCombobox, CB_ADDSTRING, 0, (LPARAM)szText);//添加条目
				SendMessage(hCombobox, CB_SETITEMDATA, nIndex, (LPARAM)pBelow);//设置条目值
			}

			pTemp2 = pBelow->pBelow;
		};

		pTemp = pRight->pRight;
	};
}

VOID UpdateImeTree(HWND hTree, PIMEINFO *ppRoot)
{
	PIMEINFO			pTemp;
	PIMEINFO			pTemp2;
	PIMEINFO			pRight;
	PIMEINFO			pBelow;
	TCHAR				szLang[256];
	HTREEITEM			hItem;

	pTemp = *ppRoot;
	while(pRight = pTemp)
	{
		hItem = NULL;
		GetLocaleInfo(MAKELCID(pRight->langid, SORT_DEFAULT), LOCALE_SLANGUAGE, szLang, 256);

		pTemp2 = pRight;
		while(pBelow = pTemp2)
		{
			if(!IsHiddenDummy(pBelow->langid, pBelow->szKLID))
			{
				if(hItem == NULL)
				{
					hItem = InsertImeTree(hTree, TVI_ROOT, szLang, NULL);
				}

				InsertImeTree(hTree, hItem, pBelow->szDesc, pBelow);
			}

			pTemp2 = pBelow->pBelow;
		};

		pTemp = pRight->pRight;
	};
}

VOID SetDefaultIme(PIMEINFO pImeInfo)
{
	LSTATUS			lResult;
	HKEY			hKey;
	HKEY			hSubKey;
	HKL				hKL;
	LPTSTR			lpCatid;
	LPTSTR			lpClsid;
	LPTSTR			lpProfile;
	TCHAR			szSubKey[1024];
	TCHAR			szTemp[9];
	DWORD			cbValue;

	if(pImeInfo->bTsfFrame)
	{
		wsprintf(szTemp, _T("%08x"), pImeInfo->langid);
		hKL = LoadKeyboardLayout(szTemp, KLF_NOTELLSHELL | KLF_SUBSTITUTE_OK);

		StringFromCLSID(pImeInfo->catid, &lpCatid);
		StringFromCLSID(pImeInfo->clsid, &lpClsid);
		StringFromCLSID(pImeInfo->guidProfile, &lpProfile);

		wsprintf(szSubKey, _T("Software\\Microsoft\\CTF\\Assemblies\\0x%08x"), pImeInfo->langid);
		lResult = RegCreateKey(HKEY_CURRENT_USER, szSubKey, &hKey);
		if(lResult == ERROR_SUCCESS)
		{
			lResult = RegCreateKey(hKey, lpCatid, &hSubKey);
			if(lResult == ERROR_SUCCESS)
			{
				cbValue = lstrlen(lpClsid) * sizeof(TCHAR) + sizeof(TCHAR);
				RegSetValueEx(hSubKey, _T("Default"), 0, REG_SZ, (BYTE*)lpClsid, cbValue);

				cbValue = lstrlen(lpProfile) * sizeof(TCHAR) + sizeof(TCHAR);
				RegSetValueEx(hSubKey, _T("Profile"), 0, REG_SZ, (BYTE*)lpProfile, cbValue);

				cbValue = sizeof(DWORD);
				RegSetValueEx(hSubKey, _T("KeyboardLayout"), 0, REG_DWORD, (BYTE*)&hKL, cbValue);

				RegCloseKey(hSubKey);
			}

			RegCloseKey(hKey);
		}

		CoTaskMemFree(lpCatid);
		CoTaskMemFree(lpClsid);
		CoTaskMemFree(lpProfile);
	}
	else
	{
		hKL = pImeInfo->hKL;
	}
	
	//获取默认输入法
	SystemParametersInfo(SPI_SETDEFAULTINPUTLANG, 0, &hKL, 0);
}

VOID DeleteTheseIme(PIMEINFO *ppRoot)
{
	PIMEINFO			pTemp;
	PIMEINFO			pTemp2;
	PIMEINFO			pRight;
	PIMEINFO			pBelow;

	pTemp = *ppRoot;
	while(pRight = pTemp)
	{
		pTemp2 = pRight;
		while(pBelow = pTemp2)
		{
			if(!IsHiddenDummy(pBelow->langid, pBelow->szKLID))
			{
				if(pBelow->bTsfFrame)
				{
					HRESULT	hr;
					ITfInputProcessorProfiles *pProfiles;

					hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, 
						NULL, 
						CLSCTX_INPROC_SERVER, 
						IID_ITfInputProcessorProfiles, 
						(LPVOID*)&pProfiles);
					if(SUCCEEDED(hr))
					{
						pProfiles->EnableLanguageProfile(pBelow->clsid,
							pBelow->langid,
							pBelow->guidProfile,
							FALSE);

						pProfiles->Release();
					}
				}
				else
				{
					UnloadKeyboardLayout(pBelow->hKL);
				}
			}

			pTemp2 = pBelow->pBelow;
		};

		pTemp = pRight->pRight;
	};
}

INT_PTR CALLBACK MainDlgProc(
	HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
	)
{
	static HWND				s_hDefaultCb;
	static HWND				s_hImeTree;
	static PIMEINFO			s_pImeAll;//所有的输入法
	static PIMEINFO			s_pImeDefault;//默认输入法
	static PIMEINFO			s_pImeDelete;//删除的输入法

	switch (message)
	{
	case WM_PAINT:
		{
			HDC				hDC;
			PAINTSTRUCT		ps;

			hDC = BeginPaint(hDlg, &ps);
			EndPaint(hDlg, &ps);
		}
		break;
	case WM_INITDIALOG:
		{
			HWND			hwndFocus = (HWND)wParam;
			LPARAM			lInitParam = lParam;
			INT				nIndex;
			
			s_hImeTree = GetDlgItem(hDlg, IDC_TREE_IME);
			s_hDefaultCb = GetDlgItem(hDlg, IDC_CB_DEFAULT);
			
			ImmGetImeList(&s_pImeAll);//获取Ime32框架的输入法
			TsfGetImeList(&s_pImeAll);//获取Tsf框架的输入法
			GetDefaultIme(&s_pImeAll, &s_pImeDefault);//获取默认输入法

			UpdateDefaultCb(s_hDefaultCb, &s_pImeAll);//更新组合框列表
			UpdateImeTree(s_hImeTree, &s_pImeAll);//更新树型视图控件
			
			nIndex = GetDefaultCbIndex(s_hDefaultCb, s_pImeDefault);//获取默认输入法在组合框里面的索引
			SendMessage(s_hDefaultCb, CB_SETCURSEL, nIndex, 0);//选中默认输入法
		}
		break;
	case WM_COMMAND:
		{
			int				id = (int)LOWORD(wParam);
			UINT			codeNotify = (UINT)HIWORD(wParam);
			HWND			hwndCtl = (HWND)lParam;

			switch(id)
			{
			case IDC_CB_DEFAULT:
				{
					switch(codeNotify)
					{
					case CBN_SELCHANGE:
						{
							PropSheet_Changed(g_hPropSheetDlg, hDlg);//通知属性表改变，启用应用按钮
						}
						break;
					}
				}
				break;
			case IDC_BTN_DEL:
				{
					PIMEINFO			pItem;
					PIMEINFO			pImeInfo;
					TV_ITEM				tvItem;
					INT					nIndex;

					if(tvItem.hItem = TreeView_GetSelection(s_hImeTree))
					{
						tvItem.mask = TVIF_PARAM;
						if(TreeView_GetItem(s_hImeTree, &tvItem) && tvItem.lParam)
						{
							pImeInfo = (PIMEINFO)tvItem.lParam;
							if(pImeInfo != s_pImeDefault)//不能删除默认输入法
							{
								pItem = new IMEINFO(*pImeInfo);
								pItem->pBelow = NULL;
								pItem->pRight = NULL;
								AddImeItem(&s_pImeDelete, pItem);//添加到删除链表

								TreeView_DeleteItem(s_hImeTree, tvItem.hItem);//删除树型视图

								nIndex = GetDefaultCbIndex(s_hDefaultCb, pImeInfo);
								SendMessage(s_hDefaultCb, CB_DELETESTRING, nIndex, 0);//删除组合框
								
								PropSheet_Changed(g_hPropSheetDlg, hDlg);//通知属性表改变，启用应用按钮
							}
						}
					}
				}
				break;
			case IDC_BTN_PROPERTY:
				{
					HRESULT				hr;
					TV_ITEM				tvItem;
					PIMEINFO			pImeInfo;
					ITfFnConfigure		*pConfigure;

					if(tvItem.hItem = TreeView_GetSelection(s_hImeTree))
					{
						tvItem.mask = TVIF_PARAM;
						if(TreeView_GetItem(s_hImeTree, &tvItem) && tvItem.lParam)
						{
							pImeInfo = (PIMEINFO)tvItem.lParam;
							if(pImeInfo->bTsfFrame)
							{
								hr = CoCreateInstance(pImeInfo->clsid, 
									NULL, 
									CLSCTX_INPROC_SERVER, 
									IID_ITfFnConfigure, 
									(LPVOID*)&pConfigure);
								if(SUCCEEDED(hr))
								{
									pConfigure->Show(hDlg, pImeInfo->langid, pImeInfo->guidProfile);//显示Tsf框架的输入法属性

									pConfigure->Release();
								}
							}
							else
							{
								ImmConfigureIME(pImeInfo->hKL, hDlg, IME_CONFIG_GENERAL, NULL);//显示Imm32框架的输入法属性
							}
						}
					}
				}
				break;
			}
		}
		break;
	case WM_NOTIFY:
		{
			LPNMHDR			pnmh = (LPNMHDR)lParam;			
			LPPSHNOTIFY		lppsn = (LPPSHNOTIFY)lParam;

			switch(pnmh->code)
			{
			case PSN_APPLY://确定、关闭、或应用按钮被单击
				{
					PIMEINFO			pImeInfo;
					INT					nIndex;
					
					DeleteTheseIme(&s_pImeDelete);//删除输入法

					nIndex = (INT)SendMessage(s_hDefaultCb, CB_GETCURSEL, 0, 0);//获取组合框当前选中索引
					pImeInfo = (PIMEINFO)SendMessage(s_hDefaultCb, CB_GETITEMDATA, nIndex, 0);//获取与条目关联的数据					
					SetDefaultIme(pImeInfo);//设置默认输入法

					PropSheet_UnChanged(g_hPropSheetDlg, hDlg);//取消改变，禁用应用按钮

					MessageBox(hDlg, _T("设置已经应用，本程序即将退出"), _T("提示"), MB_ICONINFORMATION);
					ExitProcess(0);

					SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
					return TRUE;
				}
				break;
			case PSN_RESET://页面注销
				{
				}
				break;
			case PSN_KILLACTIVE://页面失去焦点，或者用户单击了确定按钮。
				{					
				}
				break;
			case PSN_SETACTIVE://页面激活
				{

				}
				break;
			}
		}
		break;
	case WM_CLOSE:
		{
			EndDialog(hDlg, (INT_PTR)lParam);
		}
		break;
	}

	return FALSE;
}

INT CALLBACK PropSheetProc(
  HWND hwndDlg,
  UINT uMsg,
  LPARAM lParam
)
{
	switch(uMsg)
	{
	case PSCB_INITIALIZED://属性表初始化
		{
			g_hPropSheetDlg = hwndDlg;
		}
		break;
	case PSCB_PRECREATE://属性表创建
		{
			LPDLGTEMPLATE	pDlgTemplate = (DLGTEMPLATE *)lParam;
			
			pDlgTemplate->style |= WS_MINIMIZEBOX;
		}
		break;
	}

	return 0;	
}

//主函数
int APIENTRY _tWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine,
	int nCmdShow
	)
{
	PROPSHEETPAGE			prop = {0};//属性表页
	PROPSHEETHEADER			proh = {0};//属性表头
	
	InitCommonControls();//初始化通用控件

	CoInitialize(0);			
		
	//填充属性表页结构	
	prop.dwSize = sizeof(PROPSHEETPAGE);//结构大小
	prop.dwFlags = PSP_USETITLE;//标志
	prop.hInstance = hInstance ;//实例句柄
	prop.pszTemplate = MAKEINTRESOURCE(IDD_MAIN);//即模板ID
	prop.pszTitle = _T("常规");//属性页标题
	prop.pfnDlgProc = (DLGPROC)MainDlgProc;//控件回调例程
		
	//填充属性表头结构
	proh.dwSize = sizeof(PROPSHEETHEADER);//结构大小
	proh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE | PSH_USECALLBACK;//属性表标志
	proh.hwndParent = NULL;//父窗口句柄
	proh.hInstance = hInstance;//实例句柄
	proh.pszIcon = MAKEINTRESOURCE(IDI_MAIN);//图标ID
	proh.pszCaption = _T("输入法管理");//属性表头标题
	proh.nPages = 1;//总共属性页页数
	proh.nStartPage = 0;//启动页
	proh.ppsp = &prop;//PROPSHEETPAGE结构数组指针
	proh.pfnCallback = PropSheetProc;//回调函数
	PropertySheet(&proh);//创建属性页
	
	CoUninitialize();

	return 0;
}