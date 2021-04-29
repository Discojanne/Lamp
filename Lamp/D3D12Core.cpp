#include "D3D12Core.h"
#define IID_PPV_ARGS(ppType) __uuidof(**(ppType)), IID_PPV_ARGS_Helper(ppType)
#include <comdef.h>
#include <iostream>
//#include <d3dcompiler.h>
#include "CompilerClass.h"
#include <fstream>



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
    m_MSpipelineStateObject = nullptr;
    //m_rootSignatureMS = nullptr;
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

    HMODULE mD3D12 = GetModuleHandle(L"D3D12.dll");
    if (mD3D12 == 0) {
        return false;
    }

    PFN_D3D12_GET_DEBUG_INTERFACE f = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(mD3D12, "D3D12GetDebugInterface");
    if (SUCCEEDED(f(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
    }
    debugController->Release();
#endif

    m_windowHeight = height; m_windowWidth = width;

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


    if (!InitShaderLayoutGPS())
        return false;

    if (!LoadModels())
        return false;

    m_textureNameArray = { L"bride_dress_normalmap.dds", L"arms.jpg", L"shirt.jpg", L"pants.jpg", L"shoes.jpg", };

    if (!InitConstantBuffer())
        return false;
   

    if (!InitDepthTesting())
        return false;

   

    if (!LoadTextures(m_textureNameArray.at(0).c_str()))
        return false;
    

    SetViewportSR();

    m_scene->Init(m_windowWidth, m_windowHeight);
   
	return true;
}

void Direct3D12::Update(double dt)
{
    m_scene->Update(dt);

    m_cbPerObject.wvpMat = DirectX::XMMatrixTranspose(m_scene->cam.GenerateWVP());
    m_cbPerObject.normalMatrix = DirectX::XMMatrixTranspose(m_scene->cam.GenerateNormalMatrix());

    static float time = 0.0f;
    time += dt;
    //if (time > 0.03f)
    if (time > 0.04f)
    {
        m_anitmaionframe++;
        if (m_anitmaionframe > m_scene->currentAni.pose.size() - 1/*31*/)
        {
            m_anitmaionframe = 0;
        }
        time = 0;
    }
    //m_anitmaionframe = 5;

    // CPU Skinning - only do output.pos = mul(float4(input.pos, 1.0f), wvpMat); in shader
    //m_scene->testAnimationFunc(m_anitmaionframe);

    // x D
   /* for (size_t i = 0; i < m_scene->currentMesh.vertLiteVector.size(); i++)
    {
        m_scene->currentMesh.vertLiteVector[i].boneIndex[3] = animFlag;
    }

    CD3DX12_RANGE readRange(0, 0);
    void* data = nullptr;
    m_scene->currentMesh.vBufferUploadHeap->Map(0, &readRange, &data);
    int nrOfVerts = m_scene->currentMesh.vertLiteVector.size();
    memcpy(data, m_scene->currentMesh.vertLiteVector.data(), sizeof(VertLite) * nrOfVerts);
    m_scene->currentMesh.vBufferUploadHeap->Unmap(0, nullptr);*/

    

    

    for (size_t i = 0; i < m_scene->currentAni.pose[i].matr.size(); i++)
    {
        m_cbPerObject.bonePoseMatrices[i] = DirectX::XMMatrixTranspose(m_scene->currentAni.pose[m_anitmaionframe].matr[i]);
    }

    memcpy(m_cbvGPUAddress[m_frameIndex], &m_cbPerObject, sizeof(ConstantBufferPerObject));
}

bool Direct3D12::UpdatePipeline()
{

    HRESULT hr;
    
    // We have to wait for the gpu to finish with the command allocator before we reset it
    WaitForNextFrameBuffers(1 - m_swapChain->GetCurrentBackBufferIndex());

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
    //const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    const float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->ClearDepthStencilView(m_dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    m_commandList->RSSetViewports(1, &m_viewport); // set the viewports
    m_commandList->RSSetScissorRects(1, &m_scissorRect); // set the scissor rects
    //m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST); // set the primitive topology
    //m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
    //m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST); // set the primitive topology
    
    m_commandList->SetGraphicsRootSignature(m_rootSignature); // set the root signature
    
    // set the root descriptor table 0 to the constant buffer descriptor heap
    

    ///
    // set the descriptor heap
    ID3D12DescriptorHeap* descriptorHeaps[] = { m_mainDescriptorHeap };
    m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    ///

    // draw triangle
    m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBufferUploadHeaps[m_frameIndex]->GetGPUVirtualAddress());
    m_commandList->SetPipelineState(m_pipelineStateObject);

 
    
    m_commandList->SetGraphicsRootDescriptorTable(1, m_mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
    m_commandList->IASetVertexBuffers(0, 1, &m_scene->currentMesh.vertexBufferView); // set the vertex buffer (using the vertex buffer view)
    m_commandList->IASetIndexBuffer(&m_scene->currentMesh.indexBufferView);
    m_commandList->DrawIndexedInstanced(m_scene->currentMesh.face.size()*3, 1, 0, 0, 0);

    // Mesh shader
    
    m_commandList->SetPipelineState(m_MSpipelineStateObject);
    m_commandList->SetGraphicsRootSignature(m_rootSignatureMS);
    m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBufferUploadHeaps[m_frameIndex]->GetGPUVirtualAddress());
    m_commandList->SetGraphicsRootShaderResourceView(1, m_scene->currentMesh.MeshletResSB->GetGPUVirtualAddress());
    m_commandList->SetGraphicsRootShaderResourceView(2, m_scene->currentMesh.VertResSB->GetGPUVirtualAddress());
    m_commandList->SetGraphicsRootShaderResourceView(3, m_scene->currentMesh.IndexResSB->GetGPUVirtualAddress());
    m_commandList->SetGraphicsRootShaderResourceView(4, m_scene->currentMesh.UniqueResSB->GetGPUVirtualAddress());
    m_commandList->SetGraphicsRootDescriptorTable(5, m_mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
  

    m_commandList->DispatchMesh(m_scene->currentMesh.meshletVector.size(), 1, 1);

    //m_commandList->DispatchMesh(m_scene->drawThisMany, 1, 1);
    


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
        MessageBox(0, L"Failed to update pipeline",
            L"Error", MB_OK);
    }

    // create an array of command lists (only one command list here)
    ID3D12CommandList* ppCommandLists[] = { m_commandList };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // this command goes in at the end of our command queue. we will know when our command queue 
    // has finished because the fence value will be set to "fenceValue" from the GPU since the command
    // queue is being executed on the GPU
    Signal();
    

    // present the current backbuffer
    hr = m_swapChain->Present(0, 0);
    if (FAILED(hr))
    {
        MessageBox(0, L"Failed to present in render()",
            L"Error", MB_OK);
    }

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

}

void Direct3D12::Cleanup()
{
    HRESULT hr;
    Signal();

    

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

    m_scene->currentMesh.Cleanup();
    delete m_scene;

    m_rtvDescriptorHeap->Release();
    m_commandList->Release();
    m_textureBuffer->Release();
    m_mainDescriptorHeap->Release();
    m_textureBufferUploadHeap->Release();

    for (int i = 0; i < frameBufferCount; ++i)
    {
        m_renderTargets[i]->Release();
        m_commandAllocator[i]->Release();

        
        m_constantBufferUploadHeaps[i]->Release();

    };
    
    m_pipelineStateObject->Release();
    m_MSpipelineStateObject->Release();
    m_rootSignature->Release();
    m_rootSignatureMS->Release();

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
            
            MessageBox(0, L"Failed to SetEventOnCompletion in WaitForPreviousFrame()",
                L"Error", MB_OK);
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

int Direct3D12::getAnimIndex()
{
    return m_anitmaionframe;
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

    Signal();

    return true;
}

bool Direct3D12::InitConstantBuffer()
{
    HRESULT hr;

    // create the constant buffer resource heap
    // We will update the constant buffer one or more times per frame, so we will use only an upload heap
    // unlike previously we used an upload heap to upload the vertex and index data, and then copied over
    // to a default heap. If you plan to use a resource for more than a couple frames, it is usually more
    // efficient to copy to a default heap where it stays on the gpu. In this case, our constant buffer
    // will be modified and uploaded at least once per frame, so we only use an upload heap

    // first we will create a resource heap (upload heap) for each frame for the cubes constant buffers
    // As you can see, we are allocating 64KB for each resource we create. Buffer resource heaps must be
    // an alignment of 64KB. We are creating 3 resources, one for each frame. Each constant buffer is 
    // only a 4x4 matrix of floats in this tutorial. So with a float being 4 bytes, we have 
    // 16 floats in one constant buffer, and we will store 2 constant buffers in each
    // heap, one for each cube, thats only 64x2 bits, or 128 bits we are using for each
    // resource, and each resource must be at least 64KB (65536 bits)
    for (int i = 0; i < frameBufferCount; ++i)
    {
        // create resource for cube 1
        hr = m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
            D3D12_HEAP_FLAG_NONE,            
            &CD3DX12_RESOURCE_DESC::Buffer(sizeof(ConstantBufferPerObject)), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
            D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
            nullptr, // we do not have use an optimized clear value for constant buffers
            IID_PPV_ARGS(&m_constantBufferUploadHeaps[i]));
        m_constantBufferUploadHeaps[i]->SetName(L"Constant Buffer Upload Resource Heap");

        CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (so end is less than or equal to begin)

        // map the resource heap to get a gpu virtual address to the beginning of the heap
        hr = m_constantBufferUploadHeaps[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_cbvGPUAddress[i]));
        
        memcpy(m_cbvGPUAddress[i], &m_cbPerObject, sizeof(ConstantBufferPerObject)); // cube1's constant buffer data
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
    descriptorTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // this is a range of shader resource views (descriptors)
    descriptorTableRanges[0].NumDescriptors = 1; // we only have one texture right now, so the range is only 1
    descriptorTableRanges[0].BaseShaderRegister = 0; // start index of the shader registers in the range
    descriptorTableRanges[0].RegisterSpace = 0; // space 0. can usually be zero
    descriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables

    // create a descriptor table
    D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
    descriptorTable.NumDescriptorRanges = _countof(descriptorTableRanges); // we only have one range
    descriptorTable.pDescriptorRanges = &descriptorTableRanges[0]; // the pointer to the beginning of our ranges array

    // create a root descriptor, which explains where to find the data for this root parameter
    D3D12_ROOT_DESCRIPTOR rootCBVDescriptor;
    rootCBVDescriptor.RegisterSpace = 0;
    rootCBVDescriptor.ShaderRegister = 0;

    // create a root parameter and fill it out
    D3D12_ROOT_PARAMETER  rootParameters[2]; // only one parameter right now
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
    rootParameters[0].Descriptor = rootCBVDescriptor; // this is the root descriptor for this root parameter
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now

    // fill out the parameter for our descriptor table. Remember it's a good idea to sort parameters by frequency of change. Our constant
    // buffer will be changed multiple times per frame, while our descriptor table will not be changed at all (in this tutorial)
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
    rootParameters[1].DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now



    // create a static sampler
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 0;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;



    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(_countof(rootParameters), // we have 2 root parameter
        rootParameters, // a pointer to the beginning of our root parameters array
        1,
        &sampler,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
    );


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

    m_shaderCompiler = new DXILShaderCompiler();
    m_shaderCompiler->init();


    /// compile vertex shader
    IDxcBlob* vertexShader; // d3d blob for holding vertex shader bytecode

    DXILShaderCompiler::Desc desc;
    desc.source = nullptr;
    desc.sourceSize = 0;
    desc.filePath = L"Resources/Shaders/vsNew.hlsl";
    desc.entryPoint = L"VSmain";
    desc.targetProfile = L"vs_6_5";

    if (FAILED(m_shaderCompiler->compile(&desc, &vertexShader)))
    {
        MessageBox(0, L"Failed to complie vertex shader",
            L"Error", MB_OK);

        return false;
    }

    D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
    vertexShaderBytecode.BytecodeLength = vertexShader->GetBufferSize();
    vertexShaderBytecode.pShaderBytecode = vertexShader->GetBufferPointer();

    /// compile pixel shader for vs 
    IDxcBlob* pixelShaderVertex;

    desc.source = nullptr;
    desc.sourceSize = 0;
    desc.filePath = L"Resources/Shaders/psVertex.hlsl";
    desc.entryPoint = L"PSmain";
    desc.targetProfile = L"ps_6_5";

    if (FAILED(m_shaderCompiler->compile(&desc, &pixelShaderVertex)))
    {
        MessageBox(0, L"Failed to complie pixel shader",
            L"Error", MB_OK);

        return false;
    }
    // fill out shader bytecode structure for pixel shader
    D3D12_SHADER_BYTECODE pixelShaderVertexBytecode = {};
    pixelShaderVertexBytecode.BytecodeLength = pixelShaderVertex->GetBufferSize();
    pixelShaderVertexBytecode.pShaderBytecode = pixelShaderVertex->GetBufferPointer();

    /// compile mesh shader
    IDxcBlob* meshShader;

    desc.source = nullptr;
    desc.sourceSize = 0;
    desc.filePath = L"Resources/Shaders/ms.hlsl";
    desc.entryPoint = L"MSmain";
    desc.targetProfile = L"ms_6_5";

    if (FAILED(m_shaderCompiler->compile(&desc, &meshShader)))
    {
        MessageBox(0, L"Failed to complie mesh shader",
            L"Error", MB_OK);

        return false;
    }

    D3D12_SHADER_BYTECODE meshShaderBytecode = {};
    meshShaderBytecode.BytecodeLength = meshShader->GetBufferSize();
    meshShaderBytecode.pShaderBytecode = meshShader->GetBufferPointer();

    /// compile pixel shader
    IDxcBlob* pixelShader;

    desc.source = nullptr;
    desc.sourceSize = 0;
    desc.filePath = L"Resources/Shaders/psMesh.hlsl";
    desc.entryPoint = L"PSmain";
    desc.targetProfile = L"ps_6_5";

    if (FAILED(m_shaderCompiler->compile(&desc, &pixelShader)))
    {
        MessageBox(0, L"Failed to complie pixel shader",
            L"Error", MB_OK);

        return false;
    }
    // fill out shader bytecode structure for pixel shader
    D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
    pixelShaderBytecode.BytecodeLength = pixelShader->GetBufferSize();
    pixelShaderBytecode.pShaderBytecode = pixelShader->GetBufferPointer();

    /// create input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "BITANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "BONEINDEX",   0, DXGI_FORMAT_R32G32B32A32_SINT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "BONEWEIGHT",   0, DXGI_FORMAT_R32G32B32A32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "DEFORMTANG",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "DEFORMTBTAN",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "FLIPPED",   0, DXGI_FORMAT_R32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };

    /*D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "BITANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "BONEINDEX",   0, DXGI_FORMAT_R32G32B32A32_SINT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "BONEWEIGHT",   0, DXGI_FORMAT_R32G32B32A32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "DFT",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "DFB",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "FLIPPED",   0, DXGI_FORMAT_R32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };*/
  

    // fill out an input layout description structure
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};

    // we can get the number of elements in an array by "sizeof(array) / sizeof(arrayElementType)"
    inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
    inputLayoutDesc.pInputElementDescs = inputLayout;

    /// Create PSO for vs ps 
    DXGI_SAMPLE_DESC sampleDesc = {};
    sampleDesc.Count = 1; // multisample count (no multisampling, so we just put 1, since we still need 1 sample)

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
    psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
    psoDesc.pRootSignature = m_rootSignature; // the root signature that describes the input data this pso needs
    psoDesc.VS = vertexShaderBytecode; // structure describing where to find the vertex shader bytecode and how large it is
    psoDesc.PS = pixelShaderVertexBytecode; // same as VS but for pixel shader
    //psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT; // type of topology we are drawing
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
    //psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE; // type of topology we are drawing
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target
    psoDesc.SampleDesc = sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
    psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
    psoDesc.NumRenderTargets = 1; // we are only binding one render target
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // a default depth stencil state
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;

    /// Create PSO for ms ps 
    
    hr = m_device->CreateRootSignature(0, meshShader->GetBufferPointer(), meshShader->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatureMS));
    if (FAILED(hr))
        return false;
    
    D3DX12_MESH_SHADER_PIPELINE_STATE_DESC MSpsoDesc = {};
    MSpsoDesc.pRootSignature = m_rootSignatureMS;
    MSpsoDesc.MS = meshShaderBytecode;
    MSpsoDesc.PS = pixelShaderBytecode;
    MSpsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    MSpsoDesc.NumRenderTargets = 1;
    MSpsoDesc.RTVFormats[0] = m_renderTargets[0]->GetDesc().Format;
    MSpsoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    MSpsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);    // CW front; cull back
    MSpsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);         // Opaque
    MSpsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // Less-equal depth test w/ writes; no stencil
    MSpsoDesc.SampleMask = UINT_MAX;
    MSpsoDesc.SampleDesc.Count = 1;// DefaultSampleDesc();
    MSpsoDesc.SampleDesc.Quality = 0;

    auto MSpsoStream = CD3DX12_PIPELINE_MESH_STATE_STREAM(MSpsoDesc);

    D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
    streamDesc.pPipelineStateSubobjectStream = &MSpsoStream;
    streamDesc.SizeInBytes = sizeof(MSpsoStream);

    hr = m_device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_MSpipelineStateObject));
    if (FAILED(hr))
    {
        return false;
    }

    // create the pso
    hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStateObject));
    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

bool Direct3D12::LoadModels()
{

    m_scene = new Scene;
    if (!m_scene->LoadMesh(MODELFILENAME))
        return false;

    if (!m_scene->LoadAnimation(ANIMATIONFILENAME))
        return false;

    //m_scene->testAnimationFunc(6);
    m_scene->currentMesh.GenerateMeshlets();

    m_scene->CreateVertexBuffers(m_device, m_commandList);






    m_commandList->Close();
    ID3D12CommandList* ppCommandLists[] = { m_commandList };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    Signal();

    WaitForNextFrameBuffers(m_swapChain->GetCurrentBackBufferIndex());

    m_scene->currentMesh.ReleaseUploadHeaps();

    return true;
}

bool Direct3D12::InitDepthTesting()
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
        &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_windowWidth, m_windowHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthOptimizedClearValue,
        IID_PPV_ARGS(&m_depthStencilBuffer)
    );
    m_dsDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");

    m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilDesc, m_dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    return true;
}

void Direct3D12::SetViewportSR()
{
    // Fill out the Viewport
    m_viewport.TopLeftX = 0;
    m_viewport.TopLeftY = 0;
    m_viewport.Width = (float)m_windowWidth;
    m_viewport.Height = (float)m_windowHeight;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;

    // Fill out a scissor rect
    m_scissorRect.left = 0;
    m_scissorRect.top = 0;
    m_scissorRect.right = m_windowWidth;
    m_scissorRect.bottom = m_windowHeight;
}

bool Direct3D12::LoadTextures(LPCWSTR texturepath)
{
    HRESULT hr;

    hr = m_commandAllocator[m_frameIndex]->Reset();
    if (FAILED(hr))
    {
        return false;
    }

    hr = m_commandList->Reset(m_commandAllocator[m_frameIndex], NULL);
    if (FAILED(hr))
    {
        return false;
    }

    // Load the image from file
    D3D12_RESOURCE_DESC textureDesc;
    int imageBytesPerRow;
    BYTE* imageData;
    int imageSize = LoadImageDataFromFile(&imageData, textureDesc, texturepath, imageBytesPerRow);

    // make sure we have data
    if (imageSize <= 0)
        return false;

    // create a default heap where the upload heap will copy its contents into (contents being the texture)
    hr = m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &textureDesc, // the description of our texture
        D3D12_RESOURCE_STATE_COPY_DEST, // We will copy the texture from the upload heap to here, so we start it out in a copy dest state
        nullptr, // used for render targets and depth/stencil buffers
        IID_PPV_ARGS(&m_textureBuffer));
    if (FAILED(hr))
    {
        return false;
    }
    m_textureBuffer->SetName(L"Texture Buffer Resource Heap");

    UINT64 textureUploadBufferSize;
    // this function gets the size an upload buffer needs to be to upload a texture to the gpu.
    // each row must be 256 byte aligned except for the last row, which can just be the size in bytes of the row
    // eg. textureUploadBufferSize = ((((width * numBytesPerPixel) + 255) & ~255) * (height - 1)) + (width * numBytesPerPixel);
    //textureUploadBufferSize = (((imageBytesPerRow + 255) & ~255) * (textureDesc.Height - 1)) + imageBytesPerRow;
    m_device->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

    // now we create an upload heap to upload our texture to the GPU
    hr = m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize), // resource description for a buffer (storing the image data in this heap just to copy to the default heap)
        D3D12_RESOURCE_STATE_GENERIC_READ, // We will copy the contents from this heap to the default heap above
        nullptr,
        IID_PPV_ARGS(&m_textureBufferUploadHeap));
    if (FAILED(hr))
    {
        return false;
    }
    m_textureBufferUploadHeap->SetName(L"Texture Buffer Upload Resource Heap");

    ///

    // store vertex buffer in upload heap
    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = &imageData[0]; // pointer to our image data
    textureData.RowPitch = imageBytesPerRow; // size of all our triangle vertex data
    textureData.SlicePitch = imageBytesPerRow * textureDesc.Height; // also the size of our triangle vertex data

    // Now we copy the upload buffer contents to the default heap
    UpdateSubresources(m_commandList, m_textureBuffer, m_textureBufferUploadHeap, 0, 0, 1, &textureData);

    // transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel shader to get the color of pixels)
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_textureBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    // create the descriptor heap that will store our srv
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 1;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    hr = m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_mainDescriptorHeap));
    if (FAILED(hr))
    {
        return false;
    }

    // now we create a shader resource view (descriptor that points to the texture and describes it)
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    m_device->CreateShaderResourceView(m_textureBuffer, &srvDesc, m_mainDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    // Now we execute the command list to upload the initial assets (triangle data)
    m_commandList->Close();
    ID3D12CommandList* ppCommandLists[] = { m_commandList };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    Signal();

    WaitForNextFrameBuffers(m_swapChain->GetCurrentBackBufferIndex());



    // we are done with image data now that we've uploaded it to the gpu, so free it up
    delete imageData;


    return true;
}

void Direct3D12::Signal()
{
    HRESULT hr;
    m_fenceTopValue++;
    m_fenceValue[m_frameIndex] = m_fenceTopValue;
    hr = m_commandQueue->Signal(m_fence, m_fenceTopValue);
    if (FAILED(hr))
    {
        MessageBox(0, L"Failed to signal",
            L"Error", MB_OK);
    }
}

int Direct3D12::LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int& bytesPerRow)
{

    HRESULT hr;

    // we only need one instance of the imaging factory to create decoders and frames
    static IWICImagingFactory* wicFactory;

    // reset decoder, frame and converter since these will be different for each image we load
    IWICBitmapDecoder* wicDecoder = NULL;
    IWICBitmapFrameDecode* wicFrame = NULL;
    IWICFormatConverter* wicConverter = NULL;

    bool imageConverted = false;

    if (wicFactory == NULL)
    {
        // Initialize the COM library
        CoInitialize(NULL);

        // create the WIC factory
        hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&wicFactory)
        );
        if (FAILED(hr)) return 0;
    }

    // load a decoder for the image
    hr = wicFactory->CreateDecoderFromFilename(
        filename,                        // Image we want to load in
        NULL,                            // This is a vendor ID, we do not prefer a specific one so set to null
        GENERIC_READ,                    // We want to read from this file
        WICDecodeMetadataCacheOnLoad,    // We will cache the metadata right away, rather than when needed, which might be unknown
        &wicDecoder                      // the wic decoder to be created
    );
    if (FAILED(hr)) return 0;

    // get image from decoder (this will decode the "frame")
    hr = wicDecoder->GetFrame(0, &wicFrame);
    if (FAILED(hr)) return 0;

    // get wic pixel format of image
    WICPixelFormatGUID pixelFormat;
    hr = wicFrame->GetPixelFormat(&pixelFormat);
    if (FAILED(hr)) return 0;

    // get size of image
    UINT textureWidth, textureHeight;
    hr = wicFrame->GetSize(&textureWidth, &textureHeight);
    if (FAILED(hr)) return 0;

    // convert wic pixel format to dxgi pixel format
    DXGI_FORMAT dxgiFormat = GetDXGIFormatFromWICFormat(pixelFormat);

    // if the format of the image is not a supported dxgi format, try to convert it
    if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
    {
        // get a dxgi compatible wic format from the current image format
        WICPixelFormatGUID convertToPixelFormat = GetConvertToWICFormat(pixelFormat);

        // return if no dxgi compatible format was found
        if (convertToPixelFormat == GUID_WICPixelFormatDontCare) return 0;

        // set the dxgi format
        dxgiFormat = GetDXGIFormatFromWICFormat(convertToPixelFormat);

        // create the format converter
        hr = wicFactory->CreateFormatConverter(&wicConverter);
        if (FAILED(hr)) return 0;

        // make sure we can convert to the dxgi compatible format
        BOOL canConvert = FALSE;
        hr = wicConverter->CanConvert(pixelFormat, convertToPixelFormat, &canConvert);
        if (FAILED(hr) || !canConvert) return 0;

        // do the conversion (wicConverter will contain the converted image)
        hr = wicConverter->Initialize(wicFrame, convertToPixelFormat, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
        if (FAILED(hr)) return 0;

        // this is so we know to get the image data from the wicConverter (otherwise we will get from wicFrame)
        imageConverted = true;
    }

    int bitsPerPixel = GetDXGIFormatBitsPerPixel(dxgiFormat); // number of bits per pixel
    bytesPerRow = (textureWidth * bitsPerPixel) / 8; // number of bytes in each row of the image data
    int imageSize = bytesPerRow * textureHeight; // total image size in bytes

    // allocate enough memory for the raw image data, and set imageData to point to that memory
    *imageData = (BYTE*)malloc(imageSize);

    // copy (decoded) raw image data into the newly allocated memory (imageData)
    if (imageConverted)
    {
        // if image format needed to be converted, the wic converter will contain the converted image
        hr = wicConverter->CopyPixels(0, bytesPerRow, imageSize, *imageData);
        if (FAILED(hr)) return 0;
    }
    else
    {
        // no need to convert, just copy data from the wic frame
        hr = wicFrame->CopyPixels(0, bytesPerRow, imageSize, *imageData);
        if (FAILED(hr)) return 0;
    }

    // now describe the texture with the information we have obtained from the image
    resourceDescription = {};
    resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDescription.Alignment = 0; // may be 0, 4KB, 64KB, or 4MB. 0 will let runtime decide between 64KB and 4MB (4MB for multi-sampled textures)
    resourceDescription.Width = textureWidth; // width of the texture
    resourceDescription.Height = textureHeight; // height of the texture
    resourceDescription.DepthOrArraySize = 1; // if 3d image, depth of 3d image. Otherwise an array of 1D or 2D textures (we only have one image, so we set 1)
    resourceDescription.MipLevels = 1; // Number of mipmaps. We are not generating mipmaps for this texture, so we have only one level
    resourceDescription.Format = dxgiFormat; // This is the dxgi format of the image (format of the pixels)
    resourceDescription.SampleDesc.Count = 1; // This is the number of samples per pixel, we just want 1 sample
    resourceDescription.SampleDesc.Quality = 0; // The quality level of the samples. Higher is better quality, but worse performance
    resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // The arrangement of the pixels. Setting to unknown lets the driver choose the most efficient one
    resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE; // no flags

    // return the size of the image. remember to delete the image once your done with it (in this tutorial once its uploaded to the gpu)
    return imageSize;
}

DXGI_FORMAT Direct3D12::GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID)
{
    if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFloat) return DXGI_FORMAT_R32G32B32A32_FLOAT;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAHalf) return DXGI_FORMAT_R16G16B16A16_FLOAT;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBA) return DXGI_FORMAT_R16G16B16A16_UNORM;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA) return DXGI_FORMAT_R8G8B8A8_UNORM;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppBGRA) return DXGI_FORMAT_B8G8R8A8_UNORM;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR) return DXGI_FORMAT_B8G8R8X8_UNORM;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102XR) return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

    else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102) return DXGI_FORMAT_R10G10B10A2_UNORM;
    else if (wicFormatGUID == GUID_WICPixelFormat16bppBGRA5551) return DXGI_FORMAT_B5G5R5A1_UNORM;
    else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR565) return DXGI_FORMAT_B5G6R5_UNORM;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFloat) return DXGI_FORMAT_R32_FLOAT;
    else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayHalf) return DXGI_FORMAT_R16_FLOAT;
    else if (wicFormatGUID == GUID_WICPixelFormat16bppGray) return DXGI_FORMAT_R16_UNORM;
    else if (wicFormatGUID == GUID_WICPixelFormat8bppGray) return DXGI_FORMAT_R8_UNORM;
    else if (wicFormatGUID == GUID_WICPixelFormat8bppAlpha) return DXGI_FORMAT_A8_UNORM;

    else return DXGI_FORMAT_UNKNOWN;
}

WICPixelFormatGUID Direct3D12::GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID)
{
    if (wicFormatGUID == GUID_WICPixelFormatBlackWhite) return GUID_WICPixelFormat8bppGray;
    else if (wicFormatGUID == GUID_WICPixelFormat1bppIndexed) return GUID_WICPixelFormat32bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat2bppIndexed) return GUID_WICPixelFormat32bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat4bppIndexed) return GUID_WICPixelFormat32bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat8bppIndexed) return GUID_WICPixelFormat32bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat2bppGray) return GUID_WICPixelFormat8bppGray;
    else if (wicFormatGUID == GUID_WICPixelFormat4bppGray) return GUID_WICPixelFormat8bppGray;
    else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayFixedPoint) return GUID_WICPixelFormat16bppGrayHalf;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFixedPoint) return GUID_WICPixelFormat32bppGrayFloat;
    else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR555) return GUID_WICPixelFormat16bppBGRA5551;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR101010) return GUID_WICPixelFormat32bppRGBA1010102;
    else if (wicFormatGUID == GUID_WICPixelFormat24bppBGR) return GUID_WICPixelFormat32bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat24bppRGB) return GUID_WICPixelFormat32bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppPBGRA) return GUID_WICPixelFormat32bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppPRGBA) return GUID_WICPixelFormat32bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat48bppRGB) return GUID_WICPixelFormat64bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat48bppBGR) return GUID_WICPixelFormat64bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRA) return GUID_WICPixelFormat64bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBA) return GUID_WICPixelFormat64bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppPBGRA) return GUID_WICPixelFormat64bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
    else if (wicFormatGUID == GUID_WICPixelFormat48bppBGRFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
    else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
    else if (wicFormatGUID == GUID_WICPixelFormat128bppPRGBAFloat) return GUID_WICPixelFormat128bppRGBAFloat;
    else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFloat) return GUID_WICPixelFormat128bppRGBAFloat;
    else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
    else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBE) return GUID_WICPixelFormat128bppRGBAFloat;
    else if (wicFormatGUID == GUID_WICPixelFormat32bppCMYK) return GUID_WICPixelFormat32bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat64bppCMYK) return GUID_WICPixelFormat64bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat40bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;
    else if (wicFormatGUID == GUID_WICPixelFormat80bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;

//#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
//    else if (wicFormatGUID == GUID_WICPixelFormat32bppRGB) return GUID_WICPixelFormat32bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat64bppRGB) return GUID_WICPixelFormat64bppRGBA;
//    else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBAHalf) return GUID_WICPixelFormat64bppRGBAHalf;
//#endif

    else return GUID_WICPixelFormatDontCare;
}

int Direct3D12::GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat)
{
    if (dxgiFormat == DXGI_FORMAT_R32G32B32A32_FLOAT) return 128;
    else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) return 64;
    else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_UNORM) return 64;
    else if (dxgiFormat == DXGI_FORMAT_R8G8B8A8_UNORM) return 32;
    else if (dxgiFormat == DXGI_FORMAT_B8G8R8A8_UNORM) return 32;
    else if (dxgiFormat == DXGI_FORMAT_B8G8R8X8_UNORM) return 32;
    else if (dxgiFormat == DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM) return 32;

    else if (dxgiFormat == DXGI_FORMAT_R10G10B10A2_UNORM) return 32;
    else if (dxgiFormat == DXGI_FORMAT_B5G5R5A1_UNORM) return 16;
    else if (dxgiFormat == DXGI_FORMAT_B5G6R5_UNORM) return 16;
    else if (dxgiFormat == DXGI_FORMAT_R32_FLOAT) return 32;
    else if (dxgiFormat == DXGI_FORMAT_R16_FLOAT) return 16;
    else if (dxgiFormat == DXGI_FORMAT_R16_UNORM) return 16;
    else if (dxgiFormat == DXGI_FORMAT_R8_UNORM) return 8;
    else if (dxgiFormat == DXGI_FORMAT_A8_UNORM) return 8;
}
