#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGRemoveLoginAutoClose.h"

WDGPlugin *g_SelfReference = NULL;

void WDGPlugin::Release()
{
	m_diffdata.clear();
	g_SelfReference = NULL;
	delete this;
}

void WDGPlugin::Free(LPVOID memory)
{
	delete memory;
	memory = NULL;
}

LPWDGPLUGININFO WDGPlugin::GetPluginInfo()
{
	static WDGPLUGININFO wpi = 
	{
		TEXT("Remove Login Auto Close"),
		TEXT("Prevents the client to close when the login is refused."),
		TEXT("[Fix]"),
		TEXT(""),
		TEXT("GreenBox"),
		1,
		0,
		{ 0x77c3dbda, 0xfd32, 0xac03, { 0x34, 0xef, 0x12, 0xac, 0xde, 0xfa, 0xce, 0xec } },
		TEXT("Recommended")
	};

	return &wpi;
}

INT32 WDGPlugin::Enabled()
{
	return 0;
}

INT32 WDGPlugin::Disabled()
{
	return 0;
}

LPCTSTR WDGPlugin::GetInputValue()
{
	return NULL;
}

DiffData *WDGPlugin::GeneratePatch()
{
	WeeDiffGenPlugin::FINDDATA sFindData = {0};
	CHAR szMsg[256];
	
	UINT32 uOffset = 0;

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = 
			"B9 '????'"					// MOV ECX, g_modeMgr
			"E8 '????'"					// CALL CModeMgr::Quit
			"C78424CC000000FFFFFFFF";	// MOV [ESP + D0h + var_4], -1
		sFindData.uDataSize = 21;
		sFindData.lpszSection = ".text";
		sFindData.chWildCard = '?';
		sFindData.uMask = WFD_PATTERN | WFD_SECTION | WFD_WILDCARD;

		uOffset = m_dgc->Match(&sFindData);
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGRemoveLoginAutoClose :: Part 1 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = 
			"\x8B\x4C\x24\x14"							// MOV ECX, [ESP+14h]
			"\xC7\x41\x0C\x03\x00\x00\x00"				// MOV [ECX+C], 3
			"\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90";	// NOPs
		sFindData.uDataSize = 21;

		m_dgc->Replace(CBAddDiffData, uOffset, &sFindData);
	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGRemoveLoginAutoClose :: Part 2 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	return &m_diffdata;
}

DiffData *WDGPlugin::GetDiffData()
{
	if(m_diffdata.size() <= 0)
	{
		return NULL;
	}

	return &m_diffdata;
}

extern "C" __declspec(dllexport) WeeDiffGenPlugin::IWDGPlugin *InitPlugin(LPVOID lpData, USHORT unWeeDiffMajorVersion, USHORT unWeeDiffMinorVersion)
{
#ifndef _DEBUG
	// Enable functions to track down memory leaks.
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	if(g_SelfReference == NULL)
	{
		g_SelfReference = new WDGPlugin(lpData);
	}

	return g_SelfReference;
}

void WDGPlugin::CBAddDiffData(WeeDiffGenPlugin::LPDIFFDATA lpDiffData)
{
	if(g_SelfReference != NULL)
	{
		g_SelfReference->m_diffdata.push_back(*lpDiffData);
	}
}