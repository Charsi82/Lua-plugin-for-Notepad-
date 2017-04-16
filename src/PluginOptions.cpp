#include "PluginOptions.h"
#include "helpers.h"
#include "lua_main.h"

CPluginOptions::CPluginOptions()
{
	// initial values
	m_bConsoleOpenOnInit = false;
	m_bConsoleClearOnRun = false;
	m_bConsoleAutoclear = false;
	m_bShowRunTime = false;
	m_uLang0 = 0;
	m_uLang = 0;
	//for (int i = 0; i < nbFunc; i++)
	//	lang_data.push_back(new TCHAR[MAX_PATH]);
}

//CPluginOptions::~CPluginOptions()
//{
//}

UINT CPluginOptions::getOptFlags() const
{
    UINT uFlags = 0;

	if ( m_bConsoleOpenOnInit )
		 uFlags |= OPTF_CONOPENONINIT;
	if ( m_bConsoleClearOnRun )
		uFlags |= OPTF_CONCLEARONRUN;
	if ( m_bConsoleAutoclear )
		uFlags |= OPTF_CONAUTOCLEAR;
	if (m_bShowRunTime)
		uFlags |= OPTF_PRINTRUNTIME;
    return uFlags;
}

void CPluginOptions::ReadOptions()
{
	// get path to config
	int   nLen;
	TCHAR m_szDllFileName[MAX_PATH];

	nLen = (int) ::GetModuleFileName((HMODULE)execData.hNPP, szIniFilePath, MAX_PATH);
	while (nLen-- > 0)
		if ((szIniFilePath[nLen] == L'\\') || (szIniFilePath[nLen] == L'/'))
		{
			lstrcpy(m_szDllFileName, szIniFilePath + nLen + 1);
			break;
		}

	nLen = lstrlen(m_szDllFileName) - 3;
	lstrcpy(m_szDllFileName + nLen, L"ini");

	// read options
	SendNpp(NPPM_GETPLUGINSCONFIGDIR, (WPARAM)MAX_PATH, (LPARAM)szIniFilePath);
	lstrcat(szIniFilePath, L"\\");
	lstrcat(szIniFilePath, m_szDllFileName);

	m_uFlags0 = ::GetPrivateProfileInt(OptSectName, OptFlagsKey, -1, szIniFilePath);
    if ( m_uFlags0 != (UINT) -1 )
    {
		m_bConsoleOpenOnInit = !!(m_uFlags0 & OPTF_CONOPENONINIT);
		m_bConsoleClearOnRun = !!(m_uFlags0 & OPTF_CONCLEARONRUN);
		m_bConsoleAutoclear  = !!(m_uFlags0 & OPTF_CONAUTOCLEAR);
		m_bShowRunTime		 = !!(m_uFlags0 & OPTF_PRINTRUNTIME);
    }
	m_uLang0 = ::GetPrivateProfileInt(OptSectName, OptLangKey, 0, szIniFilePath);
	//if (m_uLang != m_uLang0)
	{
		m_uLang = m_uLang0;
		ApplyLang();
	}	
}

void CPluginOptions::SaveOptions()
{
	if ((getOptFlags() != m_uFlags0) || (m_uLang != m_uLang0))
	{
		TCHAR szNum[10];
		UINT  uFlags = getOptFlags();
    
		wsprintf(szNum, L"%u", uFlags);
		if (WritePrivateProfileString(OptSectName, OptFlagsKey, szNum, szIniFilePath))
			m_uFlags0 = uFlags;

		wsprintf(szNum, L"%u", m_uLang);
		if (WritePrivateProfileString(OptSectName, OptLangKey, szNum, szIniFilePath))
			m_uLang0 = m_uLang;
	}
}

void CPluginOptions::OnSwitchLang()
{
	m_uLang = (m_uLang+1)%2;
	ApplyLang();
};

void CPluginOptions::ApplyLang()
{
	TCHAR szIniFilePathL[MAX_PATH + 1];
	SendNpp(NPPM_GETNPPDIRECTORY, (WPARAM)MAX_PATH, (LPARAM)szIniFilePathL);
	lstrcat(szIniFilePathL, L"\\plugins\\lua_npp_lang.ini");

	TCHAR section_name[8];
	wsprintf(section_name, L"lang_%d", m_uLang);
	TCHAR line[8];

	HMENU _menu = GetMenu(nppData._nppHandle);

	MENUITEMINFO info;
	info.cbSize = sizeof(MENUITEMINFO);
	info.fType = MFT_STRING;
	info.fMask = MIIM_TYPE;

	TCHAR buf[MAX_PATH];
	TCHAR hot_key[MAX_PATH];

	info.dwTypeData = buf;
	for (int i = 0; i < nbFunc; i++)
	{
		if (funcItems[i]._pFunc)
		{
			int iStrSize = GetMenuString(_menu, funcItems[i]._cmdID, buf, MAX_PATH, FALSE);

			hot_key[0] = 0;
			TCHAR* j = wcschr(buf, L'\t');
			if (j) lstrcpy(hot_key, j);

			wsprintf(line, L"id_%d", i);
			GetPrivateProfileString(section_name, line, L"", buf, MAX_PATH, szIniFilePathL);
			if (hot_key[0]) lstrcat(buf, hot_key);
			SetMenuItemInfo(_menu, funcItems[i]._cmdID, FALSE, &info);
		}
	}
}

