#include "D3D12Core.h"
#define IID_PPV_ARGS(ppType) __uuidof(**(ppType)), IID_PPV_ARGS_Helper(ppType)
#include <comdef.h>
#include <iostream>
#include <d3dcompiler.h>

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
        m_fence = nullptr;
    }

    m_fenceEvent = 0;
    m_frameIndex = 0;
    m_rtvDescriptorSize = 0;
}

Direct3D12::~Direct3D12()
{
    Cleanup();
}

bool Direct3D12::InitD3D(HWND hwnd, int width, int height)
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

    if (!InitDevice(hwnd))
        return false;

    if (!InitCommandQueue())
        return false;
    
    if (!InitSwapChain(hwnd))
        return false;
    
    if (!InitBackBuffers())
        return false;
    
    if (!InitCommandAllocators())
        return false;

    if (!InitFence())
        return false;
    
    if (!InitRootSignature())
        return false;

    if (!InitConstantBuffer())
        return false;

    if (!InitShaderLayoutGPS())
        return false;

    if (!InitVertexIndexBuffer())
        return false;

    if (!InitDepthTesting(width, height))
        return false;

    SetViewportSR(width, height);

	return true;
}

void Direct3D12::Update()
{

    // update app logic, such as moving the camera or figuring out what objects are in view
    static float rIncrement = 0.00002f;
    static float gIncrement = 0.00006f;
    static float bIncrement = 0.00009f;

    m_cbColorMultiplierData.colorMultiplier.x += rIncrement;
    m_cbColorMultiplierData.colorMultiplier.y += gIncrement;
    m_cbColorMultiplierData.colorMultiplier.z += bIncrement;

    if (m_cbColorMultiplierData.colorMultiplier.x >= 1.0 || m_cbColorMultiplierData.colorMultiplier.x <= 0.0)
    {
        m_cbColorMultiplierData.colorMultiplier.x = m_cbColorMultiplierData.colorMultiplier.x >= 1.0 ? 1.0 : 0.0;
        rIncrement = -rIncrement;
    }
    if (m_cbColorMultiplierData.colorMultiplier.y >= 1.0 || m_cbColorMultiplierData.colorMultiplier.y <= 0.0)
    {
        m_cbColorMultiplierData.colorMultiplier.y = m_cbColorMultiplierData.colorMultiplier.y >= 1.0 ? 1.0 : 0.0;
        gIncrement = -gIncrement;
    }
    if (m_cbColorMultiplierData.colorMultiplier.z >= 1.0 || m_cbColorMultiplierData.colorMultiplier.z <= 0.0)
    {
        m_cbColorMultiplierData.colorMultiplier.z = m_cbColorMultiplierData.colorMultiplier.z >= 1.0 ? 1.0 : 0.0;
        bIncrement = -bIncrement;
    }


    //m_constantBufferUploadHeap[m_frameIndex]->Map(,);


    CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
    m_constantBufferUploadHeap[m_frameIndex]->Map(0, &readRange, reinterpret_cast<void**>(&m_cbColorMultiplierGPUAddress[m_frameIndex]));

    // copy our ConstantBuffer instance to the mapped constant buffer resource
    memcpy(m_cbColorMultiplierGPUAddress[m_frameIndex], &m_cbColorMultiplierData, sizeof(m_cbColorMultiplierData));

    CD3DX12_RANGE writeRange(0, sizeof(ConstantBuffer));

    m_constantBufferUploadHeap[m_frameIndex]->Unmap(0, &writeRange);
}

bool Direct3D12::UpdatePipeline()
{

    HRESULT hr;

    // We have to wait for the gpu to finish with the command allocator before we reset it
    WaitForNextFrameBuffers(m_swapChain->GetCurrentBackBufferIndex());

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

    // get a handle to the depth/stencil buffer
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    // set the render target for the output merger stage (the output of the pipeline)
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // Clear the render target by using the ClearRenderTargetView command
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->ClearDepthStencilView(m_dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    m_commandList->SetGraphicsRootSignature(m_rootSignature); // set the root signature

    // set constant buffer descriptor heap
    ID3D12DescriptorHeap* descriptorHeaps[] = { m_mainDescriptorHeap[m_frameIndex] };
    m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    // set the root descriptor table 0 to the constant buffer descriptor heap
    m_commandList->SetGraphicsRootDescriptorTable(0, m_mainDescriptorHeap[m_frameIndex]->GetGPUDescriptorHandleForHeapStart());

    // draw triangle
    m_commandList->SetPipelineState(m_pipelineStateObject);
    
    m_commandList->RSSetViewports(1, &m_viewport); // set the viewports
    m_commandList->RSSetScissorRects(1, &m_scissorRect); // set the scissor rects
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView); // set the vertex buffer (using the vertex buffer view)
    m_commandList->IASetIndexBuffer(&m_indexBufferView);
    //m_commandList->DrawInstanced(4, 1, 0, 0); // finally draw 3 vertices (draw the triangle)
    m_commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
    //m_commandList->DrawIndexedInstanced(6, 1, 0, 4, 0);

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
    m_fenceTopValue++;
    hr = m_commandQueue->Signal(m_fence, m_fenceTopValue);
    if (FAILED(hr))
    {
        MessageBox(0, "Failed to signal command queue",
            "Error", MB_OK);
    }
    m_fenceValue[m_frameIndex] = m_fenceTopValue;

    // present the current backbuffer
    hr = m_swapChain->Present(0, 0);
    if (FAILED(hr))
    {
        MessageBox(0, "Failed to present in render()",
            "Error", MB_OK);
    }

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

}

void Direct3D12::Cleanup()
{
    HRESULT hr;
    m_fenceTopValue++;
    hr = m_commandQueue->Signal(m_fence, m_fenceTopValue);
    if (FAILED(hr))
    {
        MessageBox(0, "Failed to signal command queue",
            "Error", MB_OK);
    }
    m_fenceValue[m_frameIndex] = m_fenceTopValue;

    // wait for the gpu to finish all frames
    for (int i = 0; i < frameBufferCount; ++i)
    {
        WaitForNextFrameBuffers(i);
    }

    // close the fence event
    CloseHandle(m_fenceEvent);

    // get swapchain out of full screen before exiting
    BOOL fs = false;
    if (SUCCEEDED(m_swapChain->GetFullscreenState(&fs, NULL)))
        m_swapChain->SetFullscreenState(false, NULL);


    m_rtvDescriptorHeap->Release();
    m_commandList->Release();
    

    for (int i = 0; i < frameBufferCount; ++i)
    {
        m_renderTargets[i]->Release();
        m_commandAllocator[i]->Release();

        m_mainDescriptorHeap[i]->Release();
        m_constantBufferUploadHeap[i]->Release();
        
    };

    m_pipelineStateObject->Release();
    m_rootSignature->Release();
    m_vertexBuffer->Release();
    m_indexBuffer->Release();

    m_depthStencilBuffer->Release();
    m_dsDescriptorHeap->Release();

    m_fence->Release();
    
    m_swapChain->Release();
    m_device->Release();
    m_commandQueue->Release();
}

void Direct3D12::WaitForNextFrameBuffers(int frameIndex)
{

    HRESULT hr;

    // if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
    // the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
    UINT64 tempFenceVlaue = m_fence->GetCompletedValue();
    if (tempFenceVlaue < m_fenceValue[m_frameIndex])
    {

        // we have the fence create an event which is signaled once the fence's current value is "fenceValue"
        hr = m_fence->SetEventOnCompletion(m_fenceValue[m_frameIndex], m_fenceEvent);
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

    

}

HANDLE Direct3D12::getFenceEvent()
{
	return m_fenceEvent;
}

bool Direct3D12::InitDevice(HWND hwnd)
{
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

    dxgiFactory->Release();

    return true;
}

bool Direct3D12::InitCommandQueue()
{
    
    HRESULT hr;
    
    // -- Create the Command Queue -- //

    D3D12_COMMAND_QUEUE_DESC cqDesc = {}; // we will be using all the default values

    hr = m_device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&m_commandQueue)); // create the command queue
    if (FAILED(hr))
    {
        return false;
    }


    return true;
}

bool Direct3D12::InitSwapChain(HWND hwnd)
{
    HRESULT hr;

    // -- Create the Swap Chain (double/tripple buffering) -- //

    IDXGIFactory4* dxgiFactory;
    hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(hr))
    {
        return false;
    }

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

    dxgiFactory->Release();

    return true;
}

bool Direct3D12::InitBackBuffers()
{
    HRESULT hr;

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

    return true;
}

bool Direct3D12::InitCommandAllocators()
{
    HRESULT hr;

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

    return true;
}

bool Direct3D12::InitFence()
{
    HRESULT hr;

    // -- Create a Fence & Fence Event -- //

    // create the fences

    hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    if (FAILED(hr))
    {
        return false;
    }
    for (int i = 0; i < frameBufferCount; i++)
    {

        m_fenceValue[i] = 0; // set the initial fence value to 0
    }

    // create a handle to a fence event
    m_fenceEvent = CreateEvent(0, false, false, 0);
    if (m_fenceEvent == 0)
    {
        return false;
    }

    hr = m_commandQueue->Signal(m_fence, ++m_fenceTopValue);
    if (FAILED(hr))
    {
        MessageBox(0, "Failed to signal command queue",
            "Error", MB_OK);
    }
    m_fenceValue[m_frameIndex] = m_fenceTopValue;

    return true;
}

bool Direct3D12::InitConstantBuffer()
{
    HRESULT hr;


    for (int i = 0; i < frameBufferCount; ++i)
    {
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = 1;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        hr = m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_mainDescriptorHeap[i]));
        if (FAILED(hr))
        {
            return false;
        }
    }

    // create the constant buffer resource heap
    // We will update the constant buffer one or more times per frame, so we will use only an upload heap
    // unlike previously we used an upload heap to upload the vertex and index data, and then copied over
    // to a default heap. If you plan to use a resource for more than a couple frames, it is usually more
    // efficient to copy to a default heap where it stays on the gpu. In this case, our constant buffer
    // will be modified and uploaded at least once per frame, so we only use an upload heap

    // create a resource heap, descriptor heap, and pointer to cbv for each frame
    for (int i = 0; i < frameBufferCount; ++i)
    {
        hr = m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
            D3D12_HEAP_FLAG_NONE, // no flags
            &CD3DX12_RESOURCE_DESC::Buffer(1024 * 64), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
            D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
            nullptr, // we do not have use an optimized clear value for constant buffers
            IID_PPV_ARGS(&m_constantBufferUploadHeap[i])
        );
        m_constantBufferUploadHeap[i]->SetName(L"Constant Buffer Upload Resource Heap");

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = m_constantBufferUploadHeap[i]->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = (sizeof(ConstantBuffer) + 255) & ~255;    // CB size is required to be 256-byte aligned.
        m_device->CreateConstantBufferView(&cbvDesc, m_mainDescriptorHeap[i]->GetCPUDescriptorHandleForHeapStart());

        ZeroMemory(&m_cbColorMultiplierData, sizeof(m_cbColorMultiplierData));
    }

    return true;
}

bool Direct3D12::InitRootSignature()
{
    HRESULT hr;

    // create root signature

    // create a descriptor range (descriptor table) and fill it out
    // this is a range of descriptors inside a descriptor heap
    D3D12_DESCRIPTOR_RANGE  descriptorTableRanges[1]; // only one range right now
    descriptorTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; // this is a range of constant buffer views (descriptors)
    descriptorTableRanges[0].NumDescriptors = 1; // we only have one constant buffer, so the range is only 1
    descriptorTableRanges[0].BaseShaderRegister = 0; // start index of the shader registers in the range
    descriptorTableRanges[0].RegisterSpace = 0; // space 0. can usually be zero
    descriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables

    // create a descriptor table
    D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
    descriptorTable.NumDescriptorRanges = _countof(descriptorTableRanges); // we only have one range
    descriptorTable.pDescriptorRanges = &descriptorTableRanges[0]; // the pointer to the beginning of our ranges array

    // create a root parameter and fill it out
    D3D12_ROOT_PARAMETER  rootParameters[1]; // only one parameter right now
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
    rootParameters[0].DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // our pixel shader will be the only shader accessing this parameter for now

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(_countof(rootParameters), // we have 1 root parameter
        rootParameters, // a pointer to the beginning of our root parameters array
        0,
        nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);

    ID3DBlob* signature;
    hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
    if (FAILED(hr))
    {
        return false;
    }

    hr = m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

bool Direct3D12::InitShaderLayoutGPS()
{
    HRESULT hr;
    // create vertex and pixel shaders

    // when debugging, we can compile the shader files at runtime.
    // but for release versions, we can compile the hlsl shaders
    // with fxc.exe to create .cso files, which contain the shader
    // bytecode. We can load the .cso files at runtime to get the
    // shader bytecode, which of course is faster than compiling
    // them at runtime

    // compile vertex shader
    ID3DBlob* vertexShader; // d3d blob for holding vertex shader bytecode
    ID3DBlob* errorBuff; // a buffer holding the error data if any

    hr = D3DCompileFromFile(L"Resources/Shaders/VertexShadertest.hlsl",
        nullptr,
        nullptr,
        "VSmain",
        "vs_5_1",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &vertexShader,
        &errorBuff);
    if (FAILED(hr))
    {
        OutputDebugStringA((char*)errorBuff->GetBufferPointer());
        return false;
    }

    // fill out a shader bytecode structure, which is basically just a pointer
    // to the shader bytecode and the size of the shader bytecode
    D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
    vertexShaderBytecode.BytecodeLength = vertexShader->GetBufferSize();
    vertexShaderBytecode.pShaderBytecode = vertexShader->GetBufferPointer();

    // compile pixel shader
    ID3DBlob* pixelShader;
    hr = D3DCompileFromFile(L"Resources/Shaders/PixelShadertest.hlsl",
        nullptr,
        nullptr,
        "PSmain",
        "ps_5_1",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &pixelShader,
        &errorBuff);
    if (FAILED(hr))
    {
        OutputDebugStringA((char*)errorBuff->GetBufferPointer());
        return false;
    }

    // fill out shader bytecode structure for pixel shader
    D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
    pixelShaderBytecode.BytecodeLength = pixelShader->GetBufferSize();
    pixelShaderBytecode.pShaderBytecode = pixelShader->GetBufferPointer();

    // create input layout

   // The input layout is used by the Input Assembler so that it knows
   // how to read the vertex data bound to it.

    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // fill out an input layout description structure
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};

    // we can get the number of elements in an array by "sizeof(array) / sizeof(arrayElementType)"
    inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
    inputLayoutDesc.pInputElementDescs = inputLayout;

    // create a pipeline state object (PSO)

    // In a real application, you will have many pso's. for each different shader
    // or different combinations of shaders, different blend states or different rasterizer states,
    // different topology types (point, line, triangle, patch), or a different number
    // of render targets you will need a pso

    // VS is the only required shader for a pso. You might be wondering when a case would be where
    // you only set the VS. It's possible that you have a pso that only outputs data with the stream
    // output, and not on a render target, which means you would not need anything after the stream
    // output.

    DXGI_SAMPLE_DESC sampleDesc = {};
    sampleDesc.Count = 1; // multisample count (no multisampling, so we just put 1, since we still need 1 sample)

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
    psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
    psoDesc.pRootSignature = m_rootSignature; // the root signature that describes the input data this pso needs
    psoDesc.VS = vertexShaderBytecode; // structure describing where to find the vertex shader bytecode and how large it is
    psoDesc.PS = pixelShaderBytecode; // same as VS but for pixel shader
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target
    psoDesc.SampleDesc = sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
    psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
    psoDesc.NumRenderTargets = 1; // we are only binding one render target
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // a default depth stencil state
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    // create the pso
    hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStateObject));
    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

bool Direct3D12::InitVertexIndexBuffer()
{
    HRESULT hr;

    // Create vertex buffer

    //Vertex vList[] = {
    //    // first quad (closer to camera, blue)
    //    { -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f },
    //    {  0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f },
    //    { -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f },
    //    {  0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f },

    //    // second quad (further from camera, green)
    //    { -0.75f,  0.75f,  0.7f, 0.0f, 1.0f, 0.0f, 1.0f },
    //    {   0.0f,  0.0f, 0.7f, 0.0f, 1.0f, 0.0f, 1.0f },
    //    { -0.75f,  0.0f, 0.7f, 0.0f, 1.0f, 0.0f, 1.0f },
    //    {   0.0f,  0.75f,  0.7f, 0.0f, 1.0f, 0.0f, 1.0f }
    //};

    // a quad
    Vertex vList[] = {
        // first quad (closer to camera, blue)
        { -0.5f,  0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f },
        {  0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 1.0f, 1.0f },
        { -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f },
        {  0.5f,  0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f }
    };

    int vBufferSize = sizeof(vList);

    // create default heap
    // default heap is memory on the GPU. Only the GPU has access to this memory
    // To get data into this heap, we will have to upload the data using
    // an upload heap
    m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
                                        // from the upload heap to this heap
        nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
        IID_PPV_ARGS(&m_vertexBuffer));

    // Create index buffer

// a quad (2 triangles)
    DWORD iList[] = {
        0, 1, 2, // first triangle
        0, 3, 1 // second triangle
    };

    int iBufferSize = sizeof(iList);

    // create default heap to hold index buffer
    m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(iBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
        nullptr, // optimized clear value must be null for this type of resource
        IID_PPV_ARGS(&m_indexBuffer));

    // we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
    m_vertexBuffer->SetName(L"Index Buffer Resource Heap");

    // create upload heap to upload index buffer
    ID3D12Resource* iBufferUploadHeap;
    m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        IID_PPV_ARGS(&iBufferUploadHeap));
    iBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

    // store vertex buffer in upload heap
    D3D12_SUBRESOURCE_DATA indexData = {};
    indexData.pData = reinterpret_cast<BYTE*>(iList); // pointer to our index array
    indexData.RowPitch = iBufferSize; // size of all our index buffer
    indexData.SlicePitch = iBufferSize; // also the size of our index buffer

    // we are now creating a command with the command list to copy the data from
    // the upload heap to the default heap
    if (!iBufferUploadHeap)
    {
        return false;
    }
    else
    {
        UpdateSubresources(m_commandList, m_indexBuffer, iBufferUploadHeap, 0, 0, 1, &indexData);
    }


    // transition the vertex buffer data from copy destination state to vertex buffer state
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

    // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
    m_indexBufferView.SizeInBytes = iBufferSize;


    // we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
    m_vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

    // create upload heap
    // upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
    // We will upload the vertex buffer using this heap to the default heap
    ID3D12Resource* vBufferUploadHeap = nullptr;
    m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        IID_PPV_ARGS(&vBufferUploadHeap));
    vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

    // store vertex buffer in upload heap
    D3D12_SUBRESOURCE_DATA vertexData = {};
    vertexData.pData = reinterpret_cast<BYTE*>(vList); // pointer to our vertex array
    vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
    vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data

    // we are now creating a command with the command list to copy the data from
    // the upload heap to the default heap
    if (!vBufferUploadHeap)
    {
        return false;
    }
    else
    {
        UpdateSubresources(m_commandList, m_vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);
    }

    // transition the vertex buffer data from copy destination state to vertex buffer state
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

    // Now we execute the command list to upload the initial assets (triangle data)
    m_commandList->Close();
    ID3D12CommandList* ppCommandLists[] = { m_commandList };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
    m_fenceValue[m_frameIndex]++;
    hr = m_commandQueue->Signal(m_fence, m_fenceValue[m_frameIndex]);
    if (FAILED(hr))
    {
        return false;
    }

    WaitForNextFrameBuffers(m_swapChain->GetCurrentBackBufferIndex());

    iBufferUploadHeap->Release();
    vBufferUploadHeap->Release();


    // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
    m_vertexBufferView.SizeInBytes = vBufferSize;

    

    return true;
}

bool Direct3D12::InitDepthTesting(int width, int height)
{
    HRESULT hr;

    // create a depth stencil descriptor heap so we can get a pointer to the depth stencil buffer
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsDescriptorHeap));
    if (FAILED(hr))
    {
        return false;
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthOptimizedClearValue,
        IID_PPV_ARGS(&m_depthStencilBuffer)
    );
    m_dsDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");

    m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilDesc, m_dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    return true;
}

void Direct3D12::SetViewportSR(int width, int height)
{
    // Fill out the Viewport
    m_viewport.TopLeftX = 0;
    m_viewport.TopLeftY = 0;
    m_viewport.Width = (float)width;
    m_viewport.Height = (float)height;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;

    // Fill out a scissor rect
    m_scissorRect.left = 0;
    m_scissorRect.top = 0;
    m_scissorRect.right = width;
    m_scissorRect.bottom = height;
}
