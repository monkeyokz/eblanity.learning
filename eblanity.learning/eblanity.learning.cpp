#include "include.h"
#include "overlay/overlay.h"
#include "overlay/timer/timer.h"

int main()
{
    SetConsoleTitleA("eblanity.learning | https://github.com/monkeyokz");
    std::cout << "welcome to my project, created in learning c++\n" << std::endl;

    /*int pid = GetPID(L"RustClient.exe");
    std::cout << "RustClient.exe pid -> " << pid << std::endl;

    HANDLE process = Attach(pid);
    if (process == NULL) {
        std::cout << "descriptor fail" << std::endl;
    }
    else {
        std::cout << "descriptor ezzz" << std::endl;
        CloseHandle(process);
    }

    Sleep(1000);
    uintptr_t gbase = GetMBaseAddr(pid, L"GameAssembly.dll");
    std::cout << "gbase -> 0x" << std::hex << gbase << std::dec;*/

    overlay::OverlayOnRender() = []() {
        if (overlay::OverlayIsOpen()) {
            ImGui::Begin("Overlay");
            ImGui::Text("Overlay is running");
            ImGui::End();
        }
    };

    overlay::timer::Init();
    if (!overlay::OverlayInitialize(GetModuleHandleW(nullptr))) {
        return 1;
    }

    overlay::OverlayRun();
    overlay::OverlayShutdown();
}
