#include "memory.h"

DWORD GetPID(std::wstring name) {
	HANDLE tool = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32W entry{};
	entry.dwSize = sizeof(entry);
	if (Process32FirstW(tool, &entry)) {
		do {
			if (name == entry.szExeFile) {
				CloseHandle(tool); return entry.th32ProcessID;
			}
		} while (Process32NextW(tool, &entry));
	}
}

HANDLE Attach(int pid) {
	auto process = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_READ, 0, pid);
	return process; // return descriptor
}

template <typename T>
T Read(HANDLE process, uintptr_t address) {
	T value{};
	ReadProcessMemory(process, reinterpret_cast<LPCVOID>(address), &value, sizeof(T), nullptr);
	return value;
} // convenient wrapper for read

template <typename T>
bool Write(HANDLE process, uintptr_t address, const T& value) {
	return WriteProcessMemory(process, reinterpret_cast<LPVOID>(address), &value, sizeof(T), nullptr) != 0;
} // convenient wrapper for write

uintptr_t GetMBaseAddr(DWORD pid, const wchar_t* mName) {
	uintptr_t mBase = 0;
	HANDLE tool = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
	MODULEENTRY32W mentry{};
	mentry.dwSize = sizeof(mentry);
	if (Module32FirstW(tool, &mentry)) {
		do {
			if (_wcsicmp(mentry.szModule, mName) == 0) {
				mBase = reinterpret_cast<uintptr_t>(mentry.modBaseAddr); // read mbase
				break;
			}
		} while (Module32NextW(tool, &mentry));
	}

	CloseHandle(tool);
	return mBase; // return mbase
}