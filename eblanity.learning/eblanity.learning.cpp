#include "include.h"

int main()
{
    SetConsoleTitleA("eblanity.learning | https://github.com/monkeyokz"); // set console name
    //SetWindowDisplayAffinity
    std::cout << "welcome to my project, created in learning c++\n" << std::endl;

    int pid = GetPID(L"RustClient.exe");
    std::cout << "RustClient.exe pid -> " << pid << std::endl; // pid output

    Attach(pid);
    if (Attach == NULL) {
        std::cout << "descriptor fail" << std::endl;
    }
    else {
        std::cout << "descriptor ezzz" << std::endl;
    }

    Sleep(1000);
    uintptr_t gbase = GetMBaseAddr(pid, L"GameAssembly.dll");
    std::cout << "gbase -> 0x" << std::hex << gbase << std::dec;

    std::cin.get();
}