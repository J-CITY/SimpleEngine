#pragma once
#include <Windows.h>
#include <system_error>

namespace IKIGAI::RENDER {
// DXGI's presentation thread can send synchronous messages to the SDL HWND.
// Service these while waiting, without removing SDL's queued input events.
inline void WaitWithWindowMessages(HANDLE event) {
    MSG message{};
    for (;;) {
        PeekMessageW(&message,nullptr,0,0,PM_NOREMOVE);
        const auto result=MsgWaitForMultipleObjectsEx(1,&event,INFINITE,QS_SENDMESSAGE,MWMO_INPUTAVAILABLE);
        if(result==WAIT_OBJECT_0)return;
        if(result==WAIT_FAILED)throw std::system_error(GetLastError(),std::system_category(),"DXGI message wait");
    }
}

}
