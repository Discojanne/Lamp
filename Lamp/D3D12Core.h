#pragma once
#include "d3dx12.h"
#include <dxgi1_4.h> // IDXGISwapChain3
#include "directxmath.h"

class Direct3D12
{
public:
	Direct3D12();
	~Direct3D12();

	bool InitD3D(HWND hwnd, int width, int height); // initializes direct3d 12

	void Update(); // update the game logic
	bool UpdatePipeline(); // update the direct3d pipeline (update command lists)
	void Render(); // execute the command list

	void Cleanup(); // release com ojects and clean up memory
	void WaitForNextFrameBuffers(int frameIndex); // wait until gpu is finished with command list
	HANDLE getFenceEvent();


	/*struct Vertex
	{
	private:
		float x;
		float y;
		float z;
	public:
		Vertex(float a, float b, float c) { x = a; y = b; z = c; }
	};*/

	struct Vertex {
		Vertex(float x, float y, float z, float r, float g, float b, float a) : pos(x, y, z), color(r, g, b, z) {}
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 color;
		
	};

private:

	static const int frameBufferCount = 3;

	bool InitDevice(HWND hwnd);
	bool InitCommandQueue();
	bool InitSwapChain(HWND hwnd);
	bool InitBackBuffers();
	bool InitCommandAllocators();
	bool InitFence();
	bool InitRootSignature();
	bool InitShaderLayoutGPS();
	bool InitVertexIndexBuffer();
	bool InitDepthTesting(int width, int height);
	void SetViewportSR(int width, int height);
	

	ID3D12Device6* m_device;
	ID3D12CommandQueue* m_commandQueue; // container for command lists
	IDXGISwapChain3* m_swapChain; // swapchain used to switch between render targets

	ID3D12DescriptorHeap* m_rtvDescriptorHeap; // a descriptor heap to hold resources like the render targets
	ID3D12Resource* m_renderTargets[frameBufferCount]; // number of render targets equal to buffer count

	ID3D12CommandAllocator* m_commandAllocator[frameBufferCount]; // we want enough allocators for each buffer * number of threads (we only have one thread)

	ID3D12GraphicsCommandList* m_commandList; // a command list we can record commands into, then execute them to render the frame

	ID3D12Fence1* m_fence;    // an object that is locked while our command list is being executed by the gpu. We need as many 
											 //as we have allocators (more if we want to know when the gpu is finished with an asset)

	HANDLE m_fenceEvent; // a handle to an event when our fence is unlocked by the gpu
	UINT64 m_fenceValue[frameBufferCount] = { 0 }; // this value is incremented each frame. each fence will have its own value
	UINT64 m_fenceTopValue = 0;

	int m_frameIndex; // current rtv we are on
	int m_rtvDescriptorSize; // size of the rtv descriptor on the device (all front and back buffers will be the same size)

	/// to draw geometry 

	ID3D12RootSignature* m_rootSignature; // root signature defines data shaders will access
	ID3D12PipelineState* m_pipelineStateObject; // pso containing a pipeline state

	ID3D12Resource* m_vertexBuffer; // a default buffer in GPU memory that we will load vertex data for our triangle into
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView; // a structure containing a pointer to the vertex data in gpu memory
											   // the total size of the buffer, and the size of each element (vertex)

	ID3D12Resource* m_indexBuffer; // a default buffer in GPU memory that we will load index data for our triangle into
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView; // a structure holding information about the index buffer

	D3D12_VIEWPORT m_viewport; // area that output from rasterizer will be stretched to.
	D3D12_RECT m_scissorRect; // the area to draw in. pixels outside that area will not be drawn onto
	// depth

	ID3D12Resource* m_depthStencilBuffer; // This is the memory for our depth buffer. it will also be used for a stencil buffer in a later tutorial
	ID3D12DescriptorHeap* m_dsDescriptorHeap; // This is a heap for our depth/stencil buffer descriptor

};
