
#include "tool_dir_name/project.h"
#include "tool_dir_name/config.hpp"

#include <filesystem>
#include <Windows.h>
#include <tlhelp32.h>
#include <iostream>

#include <vector>

bool StopProcesses(const std::vector<std::string>& inexePaths) 
{
    bool result = false;
    std::vector<std::string> exePaths = inexePaths;
    for (auto exePath : exePaths) 
    {
        std::string inputFileName = std::filesystem::path(exePath).filename().string();
        // 将exePath转换为宽字符格式
        std::replace(exePath.begin(), exePath.end(), '/', '\\');
        wchar_t exePathWide[MAX_PATH];
        wchar_t exeName[MAX_PATH];
        // 将窄字符格式的文件名 inputFileName 转换为宽字符格式，并保存在 exeName 中。CP_UTF8 表示使用 UTF-8 编码。
        int numChars = MultiByteToWideChar(CP_UTF8, 0, inputFileName.c_str(), -1, exeName, MAX_PATH);
        if (numChars == 0) {
            std::cout << "Error: failed to convert exeName to wide char format, error code = " << GetLastError() << std::endl;
            return false;
        }
        //  将窄字符格式的路径 exePath 转换为宽字符格式，保存在 exePathWide 中。
        int numChars_2 = MultiByteToWideChar(CP_UTF8, 0, exePath.c_str(), -1, exePathWide, MAX_PATH);
        if (numChars_2 == 0) {
            std::cout << "Error: failed to convert exePath to wide char format, error code = " << GetLastError() << std::endl;
            return false;
        }

        // 创建进程快照句柄
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            std::cout << "Error: failed to create process snapshot handle, error code = " << GetLastError() << std::endl;
            return false;
        }

        // 初始化进程入口结构
        PROCESSENTRY32W processEntry;  // 定义一个用于遍历进程的结构体
        processEntry.dwSize = sizeof(PROCESSENTRY32W);  // 设置结构体的大小
        // 遍历流程列表
        BOOL bFound = Process32FirstW(hSnapshot, &processEntry);  // 使用 Process32FirstW 函数获取第一个进程的信息。
        while (bFound) {
            // 获取进程的完整路径
            if (wcscmp(processEntry.szExeFile, exeName) == 0)  // 区分大小写的比较
            {
                TCHAR fullPath[MAX_PATH];
                HANDLE tempprocessHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processEntry.th32ProcessID);
                if (tempprocessHandle != NULL) {
                    DWORD size = MAX_PATH;
                    if (QueryFullProcessImageName(tempprocessHandle, 0, fullPath, &size)) 
                    {
                        //std::cout << "Matched process: " << processEntry.szExeFile << " (PID: " << processEntry.th32ProcessID << ") Location: " << fullPath << std::endl;

                        // 比较完整路径与exePath
                        if (wcscmp(fullPath, exePathWide) == 0)
                        {
                            // 打开进程句柄
                            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processEntry.th32ProcessID);
                            if (hProcess != NULL) {
                                // 终止进程
                                if (TerminateProcess(hProcess, 0)) {
                                    std::cout << "Success: stopped process " << exePath << std::endl;
                                    result = true;
                                }
                                else 
                                {
                                    std::cout << "Error: stopped process " << exePath << std::endl;
                                    std::cout << "Error: failed to terminate process, error code = " << GetLastError() << std::endl;
                                }
                                // 关闭过程手柄
                                CloseHandle(hProcess);
                            }
                            else {
                                std::cout << "Error: failed to open process handle, error code = " << GetLastError() << std::endl;
                            }
                        }
                    }
                    CloseHandle(tempprocessHandle);
                }
                break;
            }
            bFound = Process32NextW(hSnapshot, &processEntry);
        }
        CloseHandle(hSnapshot);

        if (!result) 
        {
            std::cout << "Error: process " << exePath << " not found" << std::endl;
        }
    }

    return result;
}



int main()
{

	Config config;
#ifndef NDEBUG
	std::string configPath = "../../../../config/config.json";
#else
	std::string configPath = "./config.json";
#endif
    if (!config.read_config(configPath))
    {
        std::cout << "ERROR : Failed to read config file " << configPath << std::endl;
        return 1;
    }

    // 停止指定进程
    bool is_kill = StopProcesses(config.vec_dir);
    if (is_kill)
    {
        std::cout << "Process termination completed." << std::endl;
    }


	return 0;
}






//bool StopProcess(std::string exePath) {
//    bool result = false;
//
//    std::string inputFileName = std::filesystem::path(exePath).filename().string();
//    // 将exePath转换为宽字符格式
//    std::replace(exePath.begin(), exePath.end(), '/', '\\');
//  
//    wchar_t exePathWide[MAX_PATH];
//    wchar_t exeName[MAX_PATH];
//    int numChars = MultiByteToWideChar(CP_UTF8, 0, inputFileName.c_str(), -1, exeName, MAX_PATH);
//    if (numChars == 0) {
//        std::cout << "Error: failed to convert exeName to wide char format, error code = " << GetLastError() << std::endl;
//        return false;
//    }
//    int numChars_2 = MultiByteToWideChar(CP_UTF8, 0, exePath.c_str(), -1, exePathWide, MAX_PATH);
//    if (numChars_2 == 0) {
//        std::cout << "Error: failed to convert exePath to wide char format, error code = " << GetLastError() << std::endl;
//        return false;
//    }
//    // 替换斜杠为反斜杠
//
//    // 创建进程快照句柄
//    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
//    if (hSnapshot == INVALID_HANDLE_VALUE) {
//        std::cout << "Error: failed to create process snapshot handle, error code = " << GetLastError() << std::endl;
//        return false;
//    }
//    // 初始化进程入口结构
//    PROCESSENTRY32W processEntry;
//    processEntry.dwSize = sizeof(PROCESSENTRY32W);
//    // 遍历流程列表
//    BOOL bFound = Process32FirstW(hSnapshot, &processEntry);
//    while (bFound)
//    {
//        // 获取进程的完整路径
//        if (wcscmp(processEntry.szExeFile, exeName) == 0)  // 区分大小写的比较
//        {
//            TCHAR fullPath[MAX_PATH];
//            HANDLE tempprocessHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processEntry.th32ProcessID);
//            if (tempprocessHandle != NULL)
//            {
//                DWORD size = MAX_PATH;
//                if (QueryFullProcessImageName(tempprocessHandle, 0, fullPath, &size)) 
//                {
//                    std::wcout << "Matched process: " << processEntry.szExeFile << " (PID: " << processEntry.th32ProcessID << ") Location: " << fullPath << std::endl;
//
//                    // 比较完整路径与exePath
//                    int is = wcscmp(fullPath, exePathWide);
//                    if (is == 0)
//                    {
//                        // 打开进程句柄
//                        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processEntry.th32ProcessID);
//                        if (hProcess != NULL)
//                        {
//                            // 终止进程
//                            if (TerminateProcess(hProcess, 0))
//                            {
//                                std::cout << "Success: stopped process " << exePath << std::endl;
//                                result = true;
//                            }
//                            else
//                            {
//                                std::cout << "Error: failed to terminate process, error code = " << GetLastError() << std::endl;
//                            }
//                            // 关闭过程手柄
//                            CloseHandle(hProcess);
//                        }
//                        else {
//                            std::cout << "Error: failed to open process handle, error code = " << GetLastError() << std::endl;
//                        }
//                    }
//                }
//                CloseHandle(tempprocessHandle);
//            }
//            break;
//        }
//        bFound = Process32NextW(hSnapshot, &processEntry);
//    }
//    CloseHandle(hSnapshot);
//
//    if (!result) {
//        std::cout << "Error: process " << exePath << " not found" << std::endl;
//    }
//    return result;
//}








