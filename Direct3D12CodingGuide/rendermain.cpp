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

    ID3D12DescriptorHeap* pRTVHeap;
    {
        D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc =
        {
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV,1,D3D12_DESCRIPTOR_HEAP_FLAG_NONE,0x1
        };
        pD3D12Device->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&pRTVHeap));
    }

    ID3D12Resource* pFrameBuffer;
    pIDXGISwapChain->GetBuffer(0, IID_PPV_ARGS(&pFrameBuffer));
    pD3D12Device->CreateRenderTargetView(pFrameBuffer, NULL, pRTVHeap->GetCPUDescriptorHandleForHeapStart());

    ID3D12CommandAllocator* pDirectCommandAllocator;
    pD3D12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pDirectCommandAllocator));

    ID3D12GraphicsCommandList* pDirectCommandList;

    pD3D12Device->CreateCommandList(0x1, D3D12_COMMAND_LIST_TYPE_DIRECT, pDirectCommandAllocator, NULL,
        IID_PPV_ARGS(&pDirectCommandList));

    D3D12_RESOURCE_BARRIER CommonToRenderTarget =
    {
        D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,D3D12_RESOURCE_BARRIER_FLAG_NONE,{
        pFrameBuffer,0,D3D12_RESOURCE_STATE_COMMON,D3D12_RESOURCE_STATE_RENDER_TARGET
}
    };
    pDirectCommandList->ResourceBarrier(1, &CommonToRenderTarget);

    float rgbaColor[4] = { 1.0f,0.0f,1.0f,1.0f };
    pDirectCommandList->ClearRenderTargetView(pRTVHeap->GetCPUDescriptorHandleForHeapStart(), rgbaColor, 0, NULL);

    D3D12_RESOURCE_BARRIER RenderTargetToCommon =
    {
        D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,D3D12_RESOURCE_BARRIER_FLAG_NONE,{
        pFrameBuffer,0,D3D12_RESOURCE_STATE_RENDER_TARGET,D3D12_RESOURCE_STATE_COMMON
}
    };
    pDirectCommandList->ResourceBarrier(1, &RenderTargetToCommon);
    pDirectCommandList->Close();
    pID3D12CommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&pDirectCommandList));
    pIDXGISwapChain->Present(0, 0);
    return 0U;
}