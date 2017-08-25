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
	BOOL			bTsfFrame;//�Ƿ���TSF��ܵ����뷨
    LANGID			langid;//���Ա�ʶ
	TCHAR			szDesc[256];//���뷨����

	HKL				hKL;//���̲��־��
	TCHAR			szKLID[9];//���̲��ֱ�ʶ

	CLSID			clsid;//���ʶ
	GUID			catid;//����ʶ
	GUID			guidProfile;

	_IMEINFO		*pRight;
	_IMEINFO		*pBelow;

}IMEINFO, *PIMEINFO;

//��ȡ���̲�������
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
		lResult = RegQueryValueEx(hKey, _T("Layout Display Name"), 0, &dwType, (LPBYTE)szValue, &cbValue);//�Ȳ�ѯ�Ƿ�����ʾ��
		if(lResult == ERROR_SUCCESS)
		{
			if(SHLoadIndirectString(szValue, szValue, 512, 0) == NULL)//�����ض����ַ���
			{
				lstrcpy(lpszDescription, szValue);
				bRet = TRUE;
			}
		}
		else
		{
			dwType = REG_SZ;
			cbValue = sizeof(szValue);
			lResult = RegQueryValueEx(hKey, _T("Layout Text"), 0, &dwType, (LPBYTE)szValue, &cbValue);//û�еĻ��ٲ�ѯ�����ı�
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

//�Ƿ�����������
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

//��������ȡ���̲��ֱ�ʶ��û��ʹ��
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

//������Ŀ��TreeView�ؼ�
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

//�����Ŀ������
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
		if(GetKeyboardLayoutName(szKLID))//��ȡ���̲��ֱ�ʶ
		{
			if(GetKeyboardLayoutDescription(szKLID, szDesc))//��ȡ������Ϣ
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
	
	//��ȡĬ�����뷨
	SystemParametersInfo(SPI_GETDEFAULTINPUTLANG, 0, &hKL, 0);
	
	//����ָ�������뷨�ṹ
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

	//��Ҫ�ж��Ƿ���Tsf��ܵ����뷨
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

				nIndex = (INT)SendMessage(hCombobox, CB_ADDSTRING, 0, (LPARAM)szText);//�����Ŀ
				SendMessage(hCombobox, CB_SETITEMDATA, nIndex, (LPARAM)pBelow);//������Ŀֵ
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
	
	//��ȡĬ�����뷨
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
	static PIMEINFO			s_pImeAll;//���е����뷨
	static PIMEINFO			s_pImeDefault;//Ĭ�����뷨
	static PIMEINFO			s_pImeDelete;//ɾ�������뷨

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
			
			ImmGetImeList(&s_pImeAll);//��ȡIme32��ܵ����뷨
			TsfGetImeList(&s_pImeAll);//��ȡTsf��ܵ����뷨
			GetDefaultIme(&s_pImeAll, &s_pImeDefault);//��ȡĬ�����뷨

			UpdateDefaultCb(s_hDefaultCb, &s_pImeAll);//������Ͽ��б�
			UpdateImeTree(s_hImeTree, &s_pImeAll);//����������ͼ�ؼ�
			
			nIndex = GetDefaultCbIndex(s_hDefaultCb, s_pImeDefault);//��ȡĬ�����뷨����Ͽ����������
			SendMessage(s_hDefaultCb, CB_SETCURSEL, nIndex, 0);//ѡ��Ĭ�����뷨
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
							PropSheet_Changed(g_hPropSheetDlg, hDlg);//֪ͨ���Ա�ı䣬����Ӧ�ð�ť
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
							if(pImeInfo != s_pImeDefault)//����ɾ��Ĭ�����뷨
							{
								pItem = new IMEINFO(*pImeInfo);
								pItem->pBelow = NULL;
								pItem->pRight = NULL;
								AddImeItem(&s_pImeDelete, pItem);//��ӵ�ɾ������

								TreeView_DeleteItem(s_hImeTree, tvItem.hItem);//ɾ��������ͼ

								nIndex = GetDefaultCbIndex(s_hDefaultCb, pImeInfo);
								SendMessage(s_hDefaultCb, CB_DELETESTRING, nIndex, 0);//ɾ����Ͽ�
								
								PropSheet_Changed(g_hPropSheetDlg, hDlg);//֪ͨ���Ա�ı䣬����Ӧ�ð�ť
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
									pConfigure->Show(hDlg, pImeInfo->langid, pImeInfo->guidProfile);//��ʾTsf��ܵ����뷨����

									pConfigure->Release();
								}
							}
							else
							{
								ImmConfigureIME(pImeInfo->hKL, hDlg, IME_CONFIG_GENERAL, NULL);//��ʾImm32��ܵ����뷨����
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
			case PSN_APPLY://ȷ�����رա���Ӧ�ð�ť������
				{
					PIMEINFO			pImeInfo;
					INT					nIndex;
					
					DeleteTheseIme(&s_pImeDelete);//ɾ�����뷨

					nIndex = (INT)SendMessage(s_hDefaultCb, CB_GETCURSEL, 0, 0);//��ȡ��Ͽ�ǰѡ������
					pImeInfo = (PIMEINFO)SendMessage(s_hDefaultCb, CB_GETITEMDATA, nIndex, 0);//��ȡ����Ŀ����������					
					SetDefaultIme(pImeInfo);//����Ĭ�����뷨

					PropSheet_UnChanged(g_hPropSheetDlg, hDlg);//ȡ���ı䣬����Ӧ�ð�ť

					MessageBox(hDlg, _T("�����Ѿ�Ӧ�ã������򼴽��˳�"), _T("��ʾ"), MB_ICONINFORMATION);
					ExitProcess(0);

					SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
					return TRUE;
				}
				break;
			case PSN_RESET://ҳ��ע��
				{
				}
				break;
			case PSN_KILLACTIVE://ҳ��ʧȥ���㣬�����û�������ȷ����ť��
				{					
				}
				break;
			case PSN_SETACTIVE://ҳ�漤��
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
	case PSCB_INITIALIZED://���Ա��ʼ��
		{
			g_hPropSheetDlg = hwndDlg;
		}
		break;
	case PSCB_PRECREATE://���Ա���
		{
			LPDLGTEMPLATE	pDlgTemplate = (DLGTEMPLATE *)lParam;
			
			pDlgTemplate->style |= WS_MINIMIZEBOX;
		}
		break;
	}

	return 0;	
}

//������
int APIENTRY _tWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine,
	int nCmdShow
	)
{
	PROPSHEETPAGE			prop = {0};//���Ա�ҳ
	PROPSHEETHEADER			proh = {0};//���Ա�ͷ
	
	InitCommonControls();//��ʼ��ͨ�ÿؼ�

	CoInitialize(0);			
		
	//������Ա�ҳ�ṹ	
	prop.dwSize = sizeof(PROPSHEETPAGE);//�ṹ��С
	prop.dwFlags = PSP_USETITLE;//��־
	prop.hInstance = hInstance ;//ʵ�����
	prop.pszTemplate = MAKEINTRESOURCE(IDD_MAIN);//��ģ��ID
	prop.pszTitle = _T("����");//����ҳ����
	prop.pfnDlgProc = (DLGPROC)MainDlgProc;//�ؼ��ص�����
		
	//������Ա�ͷ�ṹ
	proh.dwSize = sizeof(PROPSHEETHEADER);//�ṹ��С
	proh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE | PSH_USECALLBACK;//���Ա��־
	proh.hwndParent = NULL;//�����ھ��
	proh.hInstance = hInstance;//ʵ�����
	proh.pszIcon = MAKEINTRESOURCE(IDI_MAIN);//ͼ��ID
	proh.pszCaption = _T("���뷨����");//���Ա�ͷ����
	proh.nPages = 1;//�ܹ�����ҳҳ��
	proh.nStartPage = 0;//����ҳ
	proh.ppsp = &prop;//PROPSHEETPAGE�ṹ����ָ��
	proh.pfnCallback = PropSheetProc;//�ص�����
	PropertySheet(&proh);//��������ҳ
	
	CoUninitialize();

	return 0;
}