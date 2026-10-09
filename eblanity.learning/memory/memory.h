#pragma once
#include <Windows.h>
#include <string>
#include <iostream>
#include <tlhelp32.h>
#include <rpcndr.h>

DWORD GetPID(std::wstring name);
HANDLE Attach(int pid);
template <typename T>
T Read(HANDLE process, uintptr_t address);
template <typename T>
bool Write(HANDLE process, uintptr_t address, const T& value);
uintptr_t GetMBaseAddr(DWORD pid, const wchar_t* mName);