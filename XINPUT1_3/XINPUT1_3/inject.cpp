#include "inject.h"

LPCWSTR lpVersion = L"0.1.0";

int* DAT_016ae4f0;//LOD control
int* DAT_01766a00;
int* FUN_009ec140;
int* FUN_009b1ad0;
int* FUN_009b1a50;

int AlwaysRet1()
{
    return 1;
}

typedef void(*FUN_009ec140Pointer)(void);
typedef void(__thiscall* FUN_009b1ad0Pointer)(int param_1_00, int param_1);
typedef void(__thiscall* FUN_009b1a50Pointer)(int param_1_00, int param_1);
int proxy_FUN_00775340(void)
{
    *DAT_016ae4f0 = 1;//Disable LOD

    FUN_009ec140Pointer FUN_009ec140Ptr = (FUN_009ec140Pointer)FUN_009ec140;
    FUN_009b1ad0Pointer FUN_009b1ad0Ptr = (FUN_009b1ad0Pointer)FUN_009b1ad0;
    FUN_009b1a50Pointer FUN_009b1a50Ptr = (FUN_009b1a50Pointer)FUN_009b1a50;

    FUN_009ec140Ptr();
    FUN_009b1ad0Ptr(*DAT_01766a00, 2);
    FUN_009b1a50Ptr(*DAT_01766a00, 2);

    return 0;
}

void __fastcall HijackFunction(HANDLE hProcess, int* lpBaseAddress, int* lpProxyAddress)

{
    int jmpAsm = 0x000000E9;//E9: near jump
    int nopAsm = 0x00009090;//90: no instruction

    int relJmpAddress = (int)lpProxyAddress + (-5 - (int)lpBaseAddress);
    if ((lpBaseAddress != 0x0) && (lpProxyAddress != 0x0)) {
        WriteProcessMemory(hProcess, lpBaseAddress, &jmpAsm, 1, (SIZE_T*)0x0);
        WriteProcessMemory(hProcess, (LPVOID)((int)lpBaseAddress + 1), &relJmpAddress, 4, (SIZE_T*)0x0);
        WriteProcessMemory(hProcess, (LPVOID)((int)lpBaseAddress + 5), &nopAsm, 2, (SIZE_T*)0x0);
        std::cout << "Hijacked AI function: " << lpBaseAddress << " to " << lpProxyAddress << std::endl;
    }
    return;
}

void Inject(HINSTANCE__* hModule, unsigned long ulReasonForCall)

{
    if (ulReasonForCall == DLL_PROCESS_ATTACH) {
        //Parse cmd
        bool bLog = false;
        LPWSTR lpCmdLine = GetCommandLineW();
        int nCmdArgs = 0;
        LPWSTR* lpArgs = CommandLineToArgvW(lpCmdLine, &nCmdArgs);
        int index = 0;
        if (0 < nCmdArgs) {
            do {
                if (_wcsicmp(lpArgs[index], L"--Log") == 0) {
                    bLog = true;
                }
                index++;
            } while (index < nCmdArgs);
        }
        if (bLog == true) {
            FILE* file = (FILE*)__acrt_iob_func(1);//Associate logfile with stdout
            FILE* temp = 0x0;
            freopen_s(&temp, "wj-gif.log", "w", file);
        }
        std::cout << "Working Joe's Graphics Improvements and Fixes v"; 
        std::wcout << lpVersion << std::endl;
        std::cout << "Command Line options: " << std::endl;
        std::wcout << lpCmdLine << std::endl;
        index = 0;
        if (0 < nCmdArgs) {
            do {
                std::cout << "Arg[" << index << "]: ";
                std::wcout << lpArgs[index] << std::endl;
                index++;
            } while (index < nCmdArgs);
        }
        std::cout << "Logging: " << bLog << std::endl;

        //Get base address
        HANDLE hCurrentProcess = GetCurrentProcess();
        HMODULE hCurrentHandle = GetModuleHandleA((LPCSTR)0x0);
        std::cout << "Alien Isolation Base Address: " << hCurrentHandle << std::endl;

        //Test AI.exe
        bool bInject = true;
        int iProcessTimestamp = 0;
        ReadProcessMemory(hCurrentProcess, hCurrentHandle + 0x48d291, &iProcessTimestamp, 4, (SIZE_T*)0x0);
        if (iProcessTimestamp != 0x54e3199b) {
            MessageBoxW((HWND)0x0,
                L"Exe test failed!\n\nThis test is to ensure that Working Joe's Graphics Improvements and Fixes is injecting into the correct AI.exe. If Alien: Isolation is out-dated, please update it through Steam. If a new update to the game has been released, this mod may need to be updated.\n\nThe mod will now be disabled, Alien: Isolation will startup normally."
                , lpVersion, 0x30);
            std::cout << "TimeStamp test failed! Value: " << iProcessTimestamp << std::endl;
            std::cout << "Continuing without injecting..." << std::endl;
            bInject = false;
        }

        //Perform injection
        if (bInject)
        {
            DAT_016ae4f0 = (int*)hCurrentHandle + ((0x016ae4f0 - 0x00400000) / 4);
            DAT_01766a00 = (int*)hCurrentHandle + ((0x01766a00 - 0x00400000) / 4);
            FUN_009ec140 = (int*)hCurrentHandle + ((0x009ec140 - 0x00400000) / 4);
            FUN_009b1ad0 = (int*)hCurrentHandle + ((0x009b1ad0 - 0x00400000) / 4);
            FUN_009b1a50 = (int*)hCurrentHandle + ((0x009b1a50 - 0x00400000) / 4);

            int* iAddressToPatch = (int*)hCurrentHandle + ((0x00775340 - 0x00400000) / 4);
            HijackFunction(hCurrentProcess, iAddressToPatch, (int*)&proxy_FUN_00775340);

            iAddressToPatch = (int*)hCurrentHandle + ((0x00a19a50 - 0x00400000) / 4);
            HijackFunction(hCurrentProcess, iAddressToPatch, (int*)&AlwaysRet1);
        }
        std::cout << "Injection finished." << std::endl;
    }
    
    return;
}