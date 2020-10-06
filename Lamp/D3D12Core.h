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

	// this is the structure of our constant buffer.
	struct ConstantBuffer {
		DirectX::XMFLOAT4 colorMultiplier;
	};

	struct ConstantBufferPerObject {
		DirectX::XMFLOAT4X4 wvpMat;
	};

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
	bool InitVertexIndexBuffer();
	bool InitDepthTesting(int width, int height);
	void SetViewportSR(int width, int height);
	void BuildCamMatrices(int width, int height);
	

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

	/// Camera matrices etc
	
	// Constant buffers must be 256-byte aligned which has to do with constant reads on the GPU.
	// We are only able to read at 256 byte intervals from the start of a resource heap, so we will
	// make sure that we add padding between the two constant buffers in the heap (one for cube1 and one for cube2)
	// Another way to do this would be to add a float array in the constant buffer structure for padding. In this case
	// we would need to add a float padding[50]; after the wvpMat variable. This would align our structure to 256 bytes (4 bytes per float)
	// The reason i didn't go with this way, was because there would actually be wasted cpu cycles when memcpy our constant
	// buffer data to the gpu virtual address. currently we memcpy the size of our structure, which is 16 bytes here, but if we
	// were to add the padding array, we would memcpy 64 bytes if we memcpy the size of our structure, which is 50 wasted bytes
	// being copied.
	int m_ConstantBufferPerObjectAlignedSize = (sizeof(ConstantBufferPerObject) + 255) & ~255;

	ConstantBufferPerObject m_cbPerObject; // this is the constant buffer data we will send to the gpu 
											// (which will be placed in the resource we created above)

	ID3D12Resource* m_constantBufferUploadHeaps[frameBufferCount]; // this is the memory on the gpu where constant buffers for each frame will be placed

	UINT8* m_cbvGPUAddress[frameBufferCount]; // this is a pointer to each of the constant buffer resource heaps

	DirectX::XMFLOAT4X4 m_cameraProjMat; // this will store our projection matrix
	DirectX::XMFLOAT4X4 m_cameraViewMat; // this will store our view matrix

	DirectX::XMFLOAT4 m_cameraPosition; // this is our cameras position vector
	DirectX::XMFLOAT4 m_cameraTarget; // a vector describing the point in space our camera is looking at
	DirectX::XMFLOAT4 m_cameraUp; // the worlds up vector

	DirectX::XMFLOAT4X4 m_cube1WorldMat; // our first cubes world matrix (transformation matrix)
	DirectX::XMFLOAT4X4 m_cube1RotMat; // this will keep track of our rotation for the first cube
	DirectX::XMFLOAT4 m_cube1Position; // our first cubes position in space

	DirectX::XMFLOAT4X4 m_cube2WorldMat; // our first cubes world matrix (transformation matrix)
	DirectX::XMFLOAT4X4 m_cube2RotMat; // this will keep track of our rotation for the second cube
	DirectX::XMFLOAT4 m_cube2PositionOffset; // our second cube will rotate around the first cube, so this is the position offset from the first cube

	int m_numCubeIndices; // the number of indices to draw the cube

};
