#include "framework.h"

#include <dxgi.h>
#include <d3d12.h>


DWORD WINAPI RenderThreadMain(LPVOID lpThreadParameter)
{
    HWND hWnd = static_cast<HWND>(lpThreadParameter);// The handle of window


    return 0U;
}