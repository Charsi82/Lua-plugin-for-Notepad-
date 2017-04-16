#ifndef _plugin_options_h_
#define _plugin_options_h_
//---------------------------------------------------------------------------
#include "windows.h"
#include <vector>

#define OptSectName L"Options"
#define OptFlagsKey L"Flags"
#define OptLangKey  L"Language"

class CPluginOptions
{
    public:
        CPluginOptions();
//        ~CPluginOptions();

		bool m_bConsoleOpenOnInit; //открывать при запуске
		bool m_bConsoleClearOnRun;
		bool m_bConsoleAutoclear;
		bool m_bShowRunTime;

        void ReadOptions();
        void SaveOptions();

		void OnSwitchLang();
		void ApplyLang();
protected:
		
        UINT getOptFlags() const;
		TCHAR szIniFilePath[MAX_PATH];

        enum eOptConsts {		   
			   OPTF_CONOPENONINIT = 0x0001,
			   OPTF_CONCLEARONRUN = 0x0002,
			   OPTF_CONAUTOCLEAR  = 0x0004,
			   OPTF_PRINTRUNTIME  = 0x0008
        };
private:
        UINT  m_uFlags;
        UINT  m_uFlags0;
		UINT  m_uLang;
		UINT  m_uLang0;
};

//---------------------------------------------------------------------------
#endif
