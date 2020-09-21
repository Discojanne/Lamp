#include "D3D12Core.h"
#define IID_PPV_ARGS(ppType) __uuidof(**(ppType)), IID_PPV_ARGS_Helper(ppType)
#include <comdef.h>
#include <iostream>

Direct3D12::Direct3D12()
{
    m_device = nullptr;
    m_swapChain = nullptr;

    m_commandQueue = nullptr;

    m_rtvDescriptorHeap = nullptr;

    m_commandList = nullptr;

    for (int i = 0; i < frameBufferCount; i++)
    {
        m_renderTargets[i] = nullptr;
        m_commandAllocator[i] = nullptr;
        m_fence[i] = nullptr;
    }

}

Direct3D12::~Direct3D12()
{
    Cleanup();
}

bool Direct3D12::InitD3D(HWND hwnd)
{

#ifdef _DEBUG
    //Enable the D3D12 debug layer.
    ID3D12Debug* debugController = nullptr;

    HMODULE mD3D12 = GetModuleHandle("D3D12.dll");
    if (mD3D12 == 0) {
        return false;
    }

    PFN_D3D12_GET_DEBUG_INTERFACE f = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(mD3D12, "D3D12GetDebugInterface");
    if (SUCCEEDED(f(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
    }
    debugController->Release();
#endif

	HRESULT hr;

    // -- Create the Device -- //
    IDXGIFactory4* dxgiFactory;
    hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(hr))
    {
        return false;
    }

    IDXGIAdapter1* adapter; // adapters are the graphics card (this includes the embedded graphics on the motherboard)

    int adapterIndex = 0; // we'll start looking for directx 12  compatible graphics devices starting at index 0

    bool adapterFound = false; // set this to true when a good one was found

    // find first hardware gpu that supports d3d 12
    while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // we dont want a software device
            adapterIndex++; // add this line here. Its not currently in the downloadable project
            continue;
        }

        // we want a device that is compatible with direct3d 12 (feature level 11 or higher)
        hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device6), nullptr);

        if (SUCCEEDED(hr))
        {
            adapterFound = true;
           
            break;
        }

        adapterIndex++;
    }

    if (!adapterFound)
    {
        return false;
    }

    hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device));
    if (FAILED(hr))
    {
        return false;
    }

    // -- Create the Command Queue -- //

    D3D12_COMMAND_QUEUE_DESC cqDesc = {}; // we will be using all the default values

    hr = m_device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&m_commandQueue)); // create the command queue
    if (FAILED(hr))
    {
        return false;
    }

    // -- Create the Swap Chain (double/tripple buffering) -- //

    DXGI_MODE_DESC backBufferDesc = {}; // this is to describe our display mode
    backBufferDesc.Width = 0; // 0 for automatic window size
    backBufferDesc.Height = 0; // buffer height
    backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the buffer (rgba 32 bits, 8 bits for each chanel)

    // describe our multi-sampling. We are not multi-sampling, so we set the count to 1 (we need at least one sample of course)
    DXGI_SAMPLE_DESC sampleDesc = {};
    sampleDesc.Count = 1; // multisample count (no multisampling, so we just put 1, since we still need 1 sample)

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = frameBufferCount; // number of buffers we have
    swapChainDesc.BufferDesc = backBufferDesc; // our back buffer description
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // dxgi will discard the buffer (data) after we call present
    swapChainDesc.OutputWindow = hwnd; // handle to our window
    swapChainDesc.SampleDesc = sampleDesc; // our multi-sampling description
    swapChainDesc.Windowed = true;//!FullScreen; // set to true, then if in fullscreen must call SetFullScreenState with true for full screen to get uncapped fps

    IDXGISwapChain* tempSwapChain;

    dxgiFactory->CreateSwapChain(
        m_commandQueue, // the queue will be flushed once the swap chain is created
        &swapChainDesc, // give it the swap chain description we created above
        &tempSwapChain // store the created swap chain in a temp IDXGISwapChain interface
    );

    m_swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();


    // -- Create the Back Buffers (render target views) Descriptor Heap -- //

// describe an rtv descriptor heap and create
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = frameBufferCount; // number of descriptors for this heap.
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // this heap is a render target view heap

    // This heap will not be directly referenced by the shaders (not shader visible), as this will store the output from the pipeline
    // otherwise we would set the heap's flag to D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvDescriptorHeap));
    if (FAILED(hr))
    {
        return false;
    }

    // get the size of a descriptor in this heap (this is a rtv heap, so only rtv descriptors should be stored in it.
    // descriptor sizes may vary from device to device, which is why there is no set size and we must ask the 
    // device to give us the size. we will use this size to increment a descriptor handle offset
    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // get a handle to the first descriptor in the descriptor heap. a handle is basically a pointer,
    // but we cannot literally use it like a c++ pointer.
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    // Create a RTV for each buffer (double buffering is two buffers, tripple buffering is 3).
    for (int i = 0; i < frameBufferCount; i++)
    {
        // first we get the n'th buffer in the swap chain and store it in the n'th
        // position of our ID3D12Resource array
        hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
        if (FAILED(hr))
        {
            return false;
        }

        // the we "create" a render target view which binds the swap chain buffer (ID3D12Resource[n]) to the rtv handle
        m_device->CreateRenderTargetView(m_renderTargets[i], nullptr, rtvHandle);

        // we increment the rtv handle by the rtv descriptor size we got above
        rtvHandle.Offset(1, m_rtvDescriptorSize);
    }

    // -- Create the Command Allocators -- //

    for (int i = 0; i < frameBufferCount; i++)
    {
        hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator[i]));
        if (FAILED(hr))
        {
            return false;
        }
    }

    // create the command list with the first allocator
    hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[0], NULL, IID_PPV_ARGS(&m_commandList));
    if (FAILED(hr))
    {
        return false;
    }

    // command lists are created in the recording state. our main loop will set it up for recording again so close it now
    m_commandList->Close();

    // -- Create a Fence & Fence Event -- //

// create the fences
    for (int i = 0; i < frameBufferCount; i++)
    {
        hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence[i]));
        if (FAILED(hr))
        {
            return false;
        }
        m_fenceValue[i] = 0; // set the initial fence value to 0
    }

    // create a handle to a fence event
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr)
    {
        return false;
    }

	return true;
}

void Direct3D12::Update()
{
}

bool Direct3D12::UpdatePipeline()
{

    HRESULT hr;

    // We have to wait for the gpu to finish with the command allocator before we reset it
    WaitForPreviousFrame();

    // we can only reset an allocator once the gpu is done with it
    // resetting an allocator frees the memory that the command list was stored in
    hr = m_commandAllocator[m_frameIndex]->Reset();
    if (FAILED(hr))
    {
        return false;
    }

    // reset the command list. by resetting the command list we are putting it into
  // a recording state so we can start recording commands into the command allocator.
  // the command allocator that we reference here may have multiple command lists
  // associated with it, but only one can be recording at any time. Make sure
  // that any other command lists associated to this command allocator are in
  // the closed state (not recording).
  // Here you will pass an initial pipeline state object as the second parameter,
  // but in this tutorial we are only clearing the rtv, and do not actually need
  // anything but an initial default pipeline, which is what we get by setting
  // the second parameter to NULL
    hr = m_commandList->Reset(m_commandAllocator[m_frameIndex], NULL);
    if (FAILED(hr))
    {
        return false;
    }

    // here we start recording commands into the commandList (which all the commands will be stored in the commandAllocator)

    // transition the "frameIndex" render target from the present state to the render target state so the command list draws to it starting from here
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // here we again get the handle to our current render target view so we can set it as the render target in the output merger stage of the pipeline
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);

    // set the render target for the output merger stage (the output of the pipeline)
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Clear the render target by using the ClearRenderTargetView command
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // transition the "frameIndex" render target from the render target state to the present state. If the debug layer is enabled, you will receive a
    // warning if present is called on the render target when it's not in the present state
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    hr = m_commandList->Close();
    if (FAILED(hr))
    {
        return false;
    }


    return true;
}

void Direct3D12::Render()
{

    HRESULT hr;

    if (!UpdatePipeline()) // update the pipeline by sending commands to the commandqueue
    {
        MessageBox(0, "Failed to update pipeline",
            "Error", MB_OK);
    }
    // create an array of command lists (only one command list here)
    ID3D12CommandList* ppCommandLists[] = { m_commandList };

    // execute the array of command lists
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // this command goes in at the end of our command queue. we will know when our command queue 
    // has finished because the fence value will be set to "fenceValue" from the GPU since the command
    // queue is being executed on the GPU
    hr = m_commandQueue->Signal(m_fence[m_frameIndex], m_fenceValue[m_frameIndex]);
    if (FAILED(hr))
    {
        MessageBox(0, "Failed to signal command queue",
            "Error", MB_OK);
    }

    // present the current backbuffer
    hr = m_swapChain->Present(0, 0);
    if (FAILED(hr))
    {
        MessageBox(0, "Failed to present in render()",
            "Error", MB_OK);
    }

}

void Direct3D12::Cleanup()
{

    // wait for the gpu to finish all frames
    for (int i = 0; i < frameBufferCount; ++i)
    {
        m_frameIndex = i;
        WaitForPreviousFrame();
    }

    // get swapchain out of full screen before exiting
    BOOL fs = false;
    if (m_swapChain->GetFullscreenState(&fs, NULL))
        m_swapChain->SetFullscreenState(false, NULL);

    m_device->Release();
    m_swapChain->Release();
    m_commandQueue->Release();
    m_rtvDescriptorHeap->Release();
    m_commandList->Release();

    for (int i = 0; i < frameBufferCount; ++i)
    {
        m_renderTargets[i]->Release();
        m_commandAllocator[i]->Release();
        m_fence[i]->Release();
    };

}

void Direct3D12::WaitForPreviousFrame()
{

    HRESULT hr;

    // swap the current rtv buffer index so we draw on the correct buffer
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
    // the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
    if (m_fence[m_frameIndex]->GetCompletedValue() < m_fenceValue[m_frameIndex])
    {

        // we have the fence create an event which is signaled once the fence's current value is "fenceValue"
        hr = m_fence[m_frameIndex]->SetEventOnCompletion(m_fenceValue[m_frameIndex], m_fenceEvent);
        if (FAILED(hr))
        {
            hr = m_device->GetDeviceRemovedReason();
            _com_error err2(hr);
            std::cout << "Device Status: " << err2.ErrorMessage() << std::endl;
            
            MessageBox(0, "Failed to SetEventOnCompletion in WaitForPreviousFrame()",
                "Error", MB_OK);
        }

        // We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
        // has reached "fenceValue", we know the command queue has finished executing
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    // increment fenceValue for next frame
    m_fenceValue[m_frameIndex]++;

}

HANDLE Direct3D12::getFenceEvent()
{
	return m_fenceEvent;
}
