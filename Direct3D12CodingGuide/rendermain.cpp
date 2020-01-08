#include "framework.h"

#include <dxgi.h>
#include <d3d12.h>


DWORD WINAPI RenderThreadMain(LPVOID lpThreadParameter)
{
    HWND hWnd = static_cast<HWND>(lpThreadParameter);// The handle of rendering window

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

    IDXGISwapChain* pIDXGISwapChain;
    {
        DXGI_SWAP_CHAIN_DESC scdc;
        scdc.BufferDesc.Width = scdc.BufferDesc.Height = 0U;
        scdc.BufferDesc.RefreshRate.Numerator = 60U;
        scdc.BufferDesc.RefreshRate.Denominator = 1U;
        scdc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scdc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        scdc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        scdc.SampleDesc.Count = 1U;
        scdc.SampleDesc.Quality = 0U;
        scdc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scdc.BufferCount = 2;
        scdc.OutputWindow = hWnd;
        scdc.Windowed = TRUE;
        scdc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        scdc.Flags = 0U;
        pDXGIFactory->CreateSwapChain(pID3D12CommandQueue,&scdc,&pIDXGISwapChain);
    }
    pDXGIFactory->Release();
    pIDXGISwapChain->Present(0, 0);
    return 0U;
}