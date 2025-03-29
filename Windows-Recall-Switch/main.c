#include <stdio.h>
#include <shlwapi.h>
#include <windows.h>
#include "main.h"

// Unresolved External Symbol __imp__StrStrIA@8
#pragma comment(lib, "shlwapi.lib")

// Define Program Title
#define TITLE L"Windows-Recall-Switch v1.0.0.0"

// Define Program Version
#define VERSION "v1.0.0.0"

// Define Buffer Size
#define BUF1 4096
#define BUF2 4096
#define BUF3 4096
#define BUF4 4096
#define BUF5 4096

// Clear Screen Function
void ClearScreen() {
    HANDLE hStdOut;
    COORD coordScreen = { 0, 0 };    // Home For The Cursor
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    // Get The Number Of Character Cells In The Current Buffer
    if (!GetConsoleScreenBufferInfo(hStdOut, &csbi))
    {
        return;
    }
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    // Fill The Entire Screen With Blanks
    if (!FillConsoleOutputCharacter(hStdOut,         // Handle To Console Screen Buffer
        (TCHAR)' ',      // Character To Write To The Buffer
        dwConSize,       // Number Of Cells To Write
        coordScreen,     // Coordinates Of First Cell
        &cCharsWritten)) // Receive Number Of Characters Written
    {
        return;
    }
    // Get The Current Text Attribute
    if (!GetConsoleScreenBufferInfo(hStdOut, &csbi))
    {
        return;
    }
    // Set The Buffer's Attributes Accordingly
    if (!FillConsoleOutputAttribute(hStdOut,          // Handle To Console Screen Buffer
        csbi.wAttributes, // Character Attributes To Use
        dwConSize,        // Number Of Cells To Set Attribute
        coordScreen,      // Coordinates Of First Cell
        &cCharsWritten))  // Receive Number Of Characters Written
    {
        return;
    }
    // Put The Cursor At Its Home Coordinates
    SetConsoleCursorPosition(hStdOut, coordScreen);
}

// Wrong Choice Function
void wrong_choice() {
    ClearScreen();
    puts("Wrong Choice! Try Again.");
}

// Press Enter Function
void press_enter() {
    printf("Press Enter To Continue ... ");
    (void)getchar();
}

// Get Windows Recall Status Function
void GetWindowsRecallStatus() {
    STARTUPINFO si;
    SECURITY_ATTRIBUTES sa;
    PROCESS_INFORMATION pi;
    HANDLE g_hChildStd_OUT_Rd = NULL;
    HANDLE g_hChildStd_OUT_Wr = NULL;
    DWORD dwRead;
    char chBuf[BUF4];
    char outbuf[BUF5] = { 0 };
    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    // Create A Pipe For The Child Process's STDOUT
    if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &sa, 0))
    {
        puts("Error: CreatePipe Failed!");
        printf("Error Code: %d\n", GetLastError());
        return;
    }
    // Ensure The Read Handle To The Pipe For STDOUT Is Not Inherited
    if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
    {
        puts("Error: SetHandleInformation Failed!");
        printf("Error Code: %d\n", GetLastError());
        return;
    }
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = g_hChildStd_OUT_Wr;
    si.hStdOutput = g_hChildStd_OUT_Wr;
    si.dwFlags |= STARTF_USESTDHANDLES;
    ZeroMemory(&pi, sizeof(pi));
    // Start The Child Process
    if (!CreateProcess(L"C:\\Windows\\System32\\dism.exe", L"C:\\Windows\\System32\\dism.exe /online /get-featureinfo /featurename:recall", NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        if (!CreateProcess(L"C:\\Windows\\Sysnative\\dism.exe", L"C:\\Windows\\Sysnative\\dism.exe /online /get-featureinfo /featurename:recall", NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
        {
            puts("Error: CreateProcess Failed!");
            printf("Error Code: %d\n", GetLastError());
            return;
        }
    }
    // Close Child Process's STDOUT Write Handle
    CloseHandle(g_hChildStd_OUT_Wr);
    // Read STDOUT From Child Process
    while (1) {
        if (!ReadFile(g_hChildStd_OUT_Rd, chBuf, sizeof(chBuf), &dwRead, NULL))
        {
            break;
        }
        if (dwRead > 0) {
            chBuf[dwRead - 1] = '\0';
            strcat_s(outbuf, sizeof(outbuf), chBuf);
        }
    }
    // Check Windows Recall Status
    if (StrStrIA(outbuf, "State : Enabled") != NULL) {
        puts("State : Enabled");
    }
    else if (StrStrIA(outbuf, "State : Disabled") != NULL) {
        puts("State : Disabled");
    }
    else {
        puts("Unknown Windows Recall Status");
    }
    // Close Child Process's STDOUT Read Handle
    CloseHandle(g_hChildStd_OUT_Rd);
    // Wait Until Child Process Exits
    WaitForSingleObject(pi.hProcess, INFINITE);
    // Close Process And Thread Handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

// Get True Windows Version Function
BOOL GetTrueWindowsVersion(OSVERSIONINFOEX* pOSversion) {
    // Function Pointer To Driver Function
    NTSTATUS(WINAPI * pRtlGetVersion)(
        PRTL_OSVERSIONINFOW lpVersionInformation) = NULL;
    // Load The System-DLL
    HINSTANCE hNTdllDll = LoadLibrary(L"ntdll.dll");
    // Successfully Loaded?
    if (hNTdllDll != NULL)
    {
        // Get The Function Pointer To RtlGetVersion
        pRtlGetVersion = (NTSTATUS(WINAPI*)(PRTL_OSVERSIONINFOW))
            GetProcAddress(hNTdllDll, "RtlGetVersion");
        // If Successful Then Read The Function
        if (pRtlGetVersion != NULL)
        {
            pRtlGetVersion((PRTL_OSVERSIONINFOW)pOSversion);
        }
        // Free The Library
        FreeLibrary(hNTdllDll);
    }
    // Always True
    return (TRUE);
}

// Get Windows Version Function
void GetWindowsVersion() {
    OSVERSIONINFOEXW osInfo;
    osInfo.dwOSVersionInfoSize = sizeof(osInfo);
    GetTrueWindowsVersion(&osInfo);
    printf("Windows Version: %d.%d.%d\n", osInfo.dwMajorVersion, osInfo.dwMinorVersion, osInfo.dwBuildNumber);
}

// Enable Windows Recall Function
void enable() {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    ClearScreen();
    puts("Enabling Windows Recall...");
    puts("");
    Sleep(3000);
    // Start The Child Process
    if (!CreateProcess(L"C:\\Windows\\System32\\dism.exe", L"C:\\Windows\\System32\\dism.exe /online /enable-feature /featurename:recall", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        if (!CreateProcess(L"C:\\Windows\\Sysnative\\dism.exe", L"C:\\Windows\\Sysnative\\dism.exe /online /enable-feature /featurename:recall", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            puts("Error: CreateProcess Failed!");
            printf("Error Code: %d\n", GetLastError());
            return;
        }
    }
    // Wait Until Child Process Exits
    WaitForSingleObject(pi.hProcess, INFINITE);
    // Close Process And Thread Handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    puts("");
    puts("Windows Recall Status:");
    puts("-------------------------------");
    GetWindowsRecallStatus();
    puts("-------------------------------");
    puts("");
    reboot();
}

// Disable Windows Recall Function
void disable() {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    ClearScreen();
    puts("Disabling Windows Recall...");
    puts("");
    Sleep(3000);
    // Start The Child Process
    if (!CreateProcess(L"C:\\Windows\\System32\\dism.exe", L"C:\\Windows\\System32\\dism.exe /online /disable-feature /featurename:recall", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        if (!CreateProcess(L"C:\\Windows\\Sysnative\\dism.exe", L"C:\\Windows\\Sysnative\\dism.exe /online /disable-feature /featurename:recall", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            puts("Error: CreateProcess Failed!");
            printf("Error Code: %d\n", GetLastError());
            return;
        }
    }
    // Wait Until Child Process Exits
    WaitForSingleObject(pi.hProcess, INFINITE);
    // Close Process And Thread Handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    puts("");
    puts("Windows Recall Status:");
    puts("-------------------------------");
    GetWindowsRecallStatus();
    puts("-------------------------------");
    puts("");
    reboot();
}

// Reboot Function
void reboot() {
    char choice[BUF3];
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    puts("Operation Complete, Reboot Is Required To Apply Changes.");
    puts("");
    printf("Do You Want To Reboot Now? [Y/n]: ");
    fgets(choice, sizeof(choice), stdin);
    if (_stricmp(choice, "y\n") == 0) {
        ClearScreen();
        puts("Reboot Initiated, Rebooting In 10 Seconds.");
        puts("");
        Sleep(3000);
        // Start The Child Process
        if (!CreateProcess(L"C:\\Windows\\System32\\shutdown.exe", L"C:\\Windows\\System32\\shutdown.exe /r /t 10 /soft", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            if (!CreateProcess(L"C:\\Windows\\SysWOW64\\shutdown.exe", L"C:\\Windows\\SysWOW64\\shutdown.exe /r /t 10 /soft", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
            {
                if (!CreateProcess(L"C:\\Windows\\SysArm32\\shutdown.exe", L"C:\\Windows\\SysArm32\\shutdown.exe /r /t 10 /soft", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
                {
                    puts("Error: CreateProcess Failed!");
                    printf("Error Code: %d\n", GetLastError());
                    return;
                }
            }
        }
        // Wait Until Child Process Exits
        WaitForSingleObject(pi.hProcess, INFINITE);
        // Close Process And Thread Handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        puts("Rebooting In Progress...");
        puts("");
        puts("Press Enter To Exit Windows-Recall-Switch.");
        puts("");
        press_enter();
        exit(0);
    }
    else {
        ClearScreen();
        puts("You Chose Not To Reboot Now, Reboot Later To Apply Changes.");
        puts("");
        Sleep(3000);
        puts("Going Back To The Main Menu...");
        puts("");
        puts("Press Enter To Go Back To The Main Menu.");
        puts("");
        press_enter();
        main();
    }
}

// Show Info Function
void info() {
    char choice[BUF2];
    while (1) {
        ClearScreen();
        puts("==========================================================");
        puts("");
        printf(" Windows-Recall-Switch %s\n", VERSION);
        puts("");
        puts(" Made By RC Chuah-(RaynerSec)");
        puts("");
        puts(" https://github.com/rc-chuah/Windows-Recall-Switch");
        puts(" https://github.com/RaynerSec/Windows-Recall-Switch");
        puts("----------------------------------------------------------");
        puts("                 -- Licensed GNU GPL v3.0 --              ");
        puts("");
        puts(" A Program Made For Easily Disabling And Enabling");
        puts(" Windows Recall For Privacy And Security Reasons Without");
        puts(" Uninstalling Windows Recall Features In Windows.");
        puts(" Feel Free To Contribute On The Github Page.");
        puts("");
        puts("  1 - Main Menu");
        puts("  2 - Exit Menu");
        puts("");
        puts("==========================================================");
        puts("");
        printf("Select An Option And Then Press ENTER: ");
        fgets(choice, sizeof(choice), stdin);
        if (strcmp(choice, "1\n") == 0) {
            main();
        }
        else if (strcmp(choice, "2\n") == 0) {
            exit(0);
        }
        else {
            wrong_choice();
            press_enter();
        }
    }
}

// Main Function
int main() {
    char choice[BUF1];
    SetConsoleTitle(TITLE);
    while (1) {
        ClearScreen();
        puts(".......................................................................................................................");
        puts("  __          ___           _                          _____                _ _        _____         _ _       _");
        puts("  \\ \\        / (_)         | |                        |  __ \\              | | |      / ____|       (_) |     | |");
        puts("   \\ \\  /\\  / / _ _ __   __| | _____      _____ ______| |__) |___  ___ __ _| | |_____| (_____      ___| |_ ___| |__");
        puts("    \\ \\/  \\/ / | | '_ \\ / _` |/ _ \\ \\ /\\ / / __|______|  _  // _ \\/ __/ _` | | |______\\___ \\ \\ /\\ / / | __/ __| '_ \\");
        puts("     \\  /\\  /  | | | | | (_| | (_) \\ V  V /\\__ \\      | | \\ \\  __/ (_| (_| | | |      ____) \\ V  V /| | || (__| | | |");
        puts("      \\/  \\/   |_|_| |_|\\__,_|\\___/ \\_/\\_/ |___/      |_|  \\_\\___|\\___\\__,_|_|_|     |_____/ \\_/\\_/ |_|\\__\\___|_| |_|");
        puts(".......................................................................................................................");
        puts("");
        printf("Windows-Recall-Switch %s\n", VERSION);
        puts("");
        GetWindowsVersion();
        puts("");
        puts("Windows Recall Status:");
        puts("-------------------------------");
        GetWindowsRecallStatus();
        puts("-------------------------------");
        puts("");
        puts("  1 - Enable Windows Recall");
        puts("  2 - Disable Windows Recall");
        puts("  3 - Info");
        puts("  4 - Exit Menu");
        puts("");
        printf("Select An Option And Then Press ENTER: ");
        fgets(choice, sizeof(choice), stdin);
        if (strcmp(choice, "1\n") == 0) {
            enable();
        }
        else if (strcmp(choice, "2\n") == 0) {
            disable();
        }
        else if (strcmp(choice, "3\n") == 0) {
            info();
        }
        else if (strcmp(choice, "4\n") == 0) {
            exit(0);
        }
        else {
            wrong_choice();
            press_enter();
        }
    }
    return 0;
}