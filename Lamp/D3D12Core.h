#pragma once
#include <initguid.h> // https://github.com/microsoft/DirectX-Graphics-Samples/issues/567
#include "d3dx12.h"
#include <dxgi1_4.h> // IDXGISwapChain3
#include <wincodec.h> // WICPixelFormatGUID

#include "Scene.h" // Contains math

class DXILShaderCompiler;

#define testtexturename L"Resources/textures/nightmare3.png"

//#define MODELFILENAME "cube"
#define MODELFILENAME "bride_dress"
//#define ANIMATIONFILENAME "ca2"
#define ANIMATIONFILENAME "man_walk"

class Direct3D12
{
public:
	Direct3D12();
	~Direct3D12();
	
	bool InitD3D(HWND hwnd, int width, int height); // initializes direct3d 12

	void Update(double dt); // update the game logic
	bool UpdatePipeline(); // update the direct3d pipeline (update command lists)
	void Render(); // execute the command list

	void Cleanup(); // release com ojects and clean up memory
	void WaitForNextFrameBuffers(int frameIndex); // wait until gpu is finished with command list
	HANDLE getFenceEvent();

	struct ConstantBufferPerObject {

		DirectX::XMMATRIX wvpMat;
		DirectX::XMMATRIX bonePoseMatrices[32];
	};

	// used for debug in main in the window
	int getAnimIndex();

private:

	static const int frameBufferCount = 3;

	bool InitDevice(HWND hwnd);
	bool InitCommandQueue();
	bool InitSwapChain(HWND hwnd);
	bool InitBackBuffers();
	bool InitCommandAllocators();
	bool InitFence();
	bool InitConstantBuffer();
	bool InitRootSignature();
	bool InitShaderLayoutGPS();
	bool LoadModels();
	bool InitDepthTesting();
	void SetViewportSR();
	bool LoadTextures(LPCWSTR texturepath);
	void Signal();

	int m_windowWidth = 0;
	int m_windowHeight = 0;

	ID3D12Device6* m_device;
	ID3D12CommandQueue* m_commandQueue; // container for command lists
	IDXGISwapChain3* m_swapChain; // swapchain used to switch between render targets

	ID3D12DescriptorHeap* m_rtvDescriptorHeap; // a descriptor heap to hold resources like the render targets
	ID3D12Resource* m_renderTargets[frameBufferCount]; // number of render targets equal to buffer count

	ID3D12CommandAllocator* m_commandAllocator[frameBufferCount]; // we want enough allocators for each buffer * number of threads (we only have one thread)

	ID3D12GraphicsCommandList6* m_commandList; // a command list we can record commands into, then execute them to render the frame

	// fence
	ID3D12Fence1* m_fence;    // an object that is locked while our command list is being executed by the gpu. We need as many 
	HANDLE m_fenceEvent; // a handle to an event when our fence is unlocked by the gpu
	UINT64 m_fenceValue[frameBufferCount] = { 0 }; // this value is incremented each frame. each fence will have its own value
	UINT64 m_fenceTopValue = 0;

	int m_frameIndex; // current rtv we are on
	int m_rtvDescriptorSize; // size of the rtv descriptor on the device (all front and back buffers will be the same size)

	D3D12_VIEWPORT m_viewport; // area that output from rasterizer will be stretched to.
	D3D12_RECT m_scissorRect; // the area to draw in. pixels outside that area will not be drawn onto

	// depth
	ID3D12Resource* m_depthStencilBuffer; // This is the memory for our depth buffer. it will also be used for a stencil buffer in a later tutorial
	ID3D12DescriptorHeap* m_dsDescriptorHeap; // This is a heap for our depth/stencil buffer descriptor



	/// God class stuff 

	// Root signature and pipeline states
	ID3D12RootSignature* m_rootSignature;			// root signature defines data shaders will access
	ID3D12RootSignature* m_rootSignatureMS; 
	ID3D12PipelineState* m_pipelineStateObject;		// pso containing a pipeline state
	ID3D12PipelineState* m_MSpipelineStateObject;
	DXILShaderCompiler* m_shaderCompiler;

	// Constant buffer
	ID3D12Resource* m_constantBufferUploadHeaps[frameBufferCount]; // this is the memory on the gpu where constant buffers for each frame will be placed
	UINT8* m_cbvGPUAddress[frameBufferCount]; // this is a pointer to each of the constant buffer resource heaps

	ConstantBufferPerObject m_cbPerObject; // this is the constant buffer data we will send to the gpu 


	Scene* m_scene;
	//to debug
	int m_anitmaionframe = 0;

	
	/// Texture stuff ¤ 

	ID3D12Resource* m_textureBuffer; // the resource heap containing our texture

	int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int& bytesPerRow);

	DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
	WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
	int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);

	ID3D12DescriptorHeap* m_mainDescriptorHeap;
	ID3D12Resource* m_textureBufferUploadHeap;

	std::vector<std::wstring> m_textureNameArray;
	
	///

	

};
