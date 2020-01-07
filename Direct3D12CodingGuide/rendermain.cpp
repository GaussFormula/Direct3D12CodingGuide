#include "framework.h"

#include <dxgi.h>
#include <d3d12.h>


DWORD WINAPI RenderThreadMain(LPVOID lpThreadParameter)
{
    HWND hWnd = static_cast<HWND>(lpThreadParameter);// The handle of window

    // A pointer for DXGIFactory
    IDXGIFactory* pDXGIFactory;
    CreateDXGIFactory(IID_PPV_ARGS(&pDXGIFactory));
    ID3D12Device* pD3D12Device = NULL;
    {
        IDXGIAdapter* pIDXGIAdapter;
        // Transversal all the adapter to try 
        for (UINT i = 0U; SUCCEEDED(pDXGIFactory->EnumAdapters(i, &pIDXGIAdapter)); ++i)
        {
            if (SUCCEEDED(D3D12CreateDevice(pIDXGIAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pD3D12Device))))
            {
                pIDXGIAdapter->Release();// You can release the adapter after create device
                break;
            }
            pIDXGIAdapter->Release();
        }
    }

    ID3D12CommandQueue* pID3D12CommandQueue;
    {
        D3D12_COMMAND_QUEUE_DESC cdqc;
        // IDXGISwapChain::Present can only run in direct command queue
        cdqc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        cdqc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        cdqc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        cdqc.NodeMask = 0x1;
        pD3D12Device->CreateCommandQueue(&cdqc, IID_PPV_ARGS(&pID3D12CommandQueue));
    }

    return 0U;
}