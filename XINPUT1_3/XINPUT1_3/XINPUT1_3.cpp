#include <windows.h>
#include "inject.h"

struct XINPUT1_3_dll { 
	HMODULE dll;
	FARPROC OrignalDllMain;
	FARPROC OrignalXInputEnable;
	FARPROC OrignalXInputGetBatteryInformation;
	FARPROC OrignalXInputGetCapabilities;
	FARPROC OrignalXInputGetDSoundAudioDeviceGuids;
	FARPROC OrignalXInputGetKeystroke;
	FARPROC OrignalXInputGetState;
	FARPROC OrignalXInputSetState;
} XINPUT1_3;

__declspec(naked) void FakeDllMain() { _asm { jmp[XINPUT1_3.OrignalDllMain] } }
__declspec(naked) void FakeXInputEnable() { _asm { jmp[XINPUT1_3.OrignalXInputEnable] } }
__declspec(naked) void FakeXInputGetBatteryInformation() { _asm { jmp[XINPUT1_3.OrignalXInputGetBatteryInformation] } }
__declspec(naked) void FakeXInputGetCapabilities() { _asm { jmp[XINPUT1_3.OrignalXInputGetCapabilities] } }
__declspec(naked) void FakeXInputGetDSoundAudioDeviceGuids() { _asm { jmp[XINPUT1_3.OrignalXInputGetDSoundAudioDeviceGuids] } }
__declspec(naked) void FakeXInputGetKeystroke() { _asm { jmp[XINPUT1_3.OrignalXInputGetKeystroke] } }
__declspec(naked) void FakeXInputGetState() { _asm { jmp[XINPUT1_3.OrignalXInputGetState] } }
__declspec(naked) void FakeXInputSetState() { _asm { jmp[XINPUT1_3.OrignalXInputSetState] } }

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	char path[MAX_PATH];
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		CopyMemory(path + GetSystemDirectory(path, MAX_PATH - 15), "\\XINPUT1_3.dll", 16);
		XINPUT1_3.dll = LoadLibrary(path);
		if (XINPUT1_3.dll == 0x0)
		{
			MessageBox(0, "Cannot load original XINPUT1_3.dll library", "Proxy", MB_ICONERROR);
			ExitProcess(0);
		}
		XINPUT1_3.OrignalDllMain = GetProcAddress(XINPUT1_3.dll, "DllMain");
		XINPUT1_3.OrignalXInputEnable = GetProcAddress(XINPUT1_3.dll, "XInputEnable");
		XINPUT1_3.OrignalXInputGetBatteryInformation = GetProcAddress(XINPUT1_3.dll, "XInputGetBatteryInformation");
		XINPUT1_3.OrignalXInputGetCapabilities = GetProcAddress(XINPUT1_3.dll, "XInputGetCapabilities");
		XINPUT1_3.OrignalXInputGetDSoundAudioDeviceGuids = GetProcAddress(XINPUT1_3.dll, "XInputGetDSoundAudioDeviceGuids");
		XINPUT1_3.OrignalXInputGetKeystroke = GetProcAddress(XINPUT1_3.dll, "XInputGetKeystroke");
		XINPUT1_3.OrignalXInputGetState = GetProcAddress(XINPUT1_3.dll, "XInputGetState");
		XINPUT1_3.OrignalXInputSetState = GetProcAddress(XINPUT1_3.dll, "XInputSetState");

		//Run actual code
		Inject(hModule, ul_reason_for_call);

		break;
	}
	case DLL_PROCESS_DETACH:
	{
		FreeLibrary(XINPUT1_3.dll);
	}
	break;
	}
	return TRUE;
}
