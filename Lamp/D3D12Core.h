#pragma once
#include "d3dx12.h"
#include <dxgi1_4.h> // IDXGISwapChain3

class Direct3D12
{
public:
	Direct3D12();
	~Direct3D12();

	bool InitD3D(HWND hwnd); // initializes direct3d 12

	void Update(); // update the game logic
	bool UpdatePipeline(); // update the direct3d pipeline (update command lists)
	void Render(); // execute the command list

	void Cleanup(); // release com ojects and clean up memory
	void WaitForPreviousFrame(); // wait until gpu is finished with command list
	HANDLE getFenceEvent();

private:

	static const int frameBufferCount = 3;

	ID3D12Device6* m_device;
	IDXGISwapChain3* m_swapChain; // swapchain used to switch between render targets

	ID3D12CommandQueue* m_commandQueue; // container for command lists

	ID3D12DescriptorHeap* m_rtvDescriptorHeap; // a descriptor heap to hold resources like the render targets

	ID3D12Resource* m_renderTargets[frameBufferCount]; // number of render targets equal to buffer count

	ID3D12CommandAllocator* m_commandAllocator[frameBufferCount]; // we want enough allocators for each buffer * number of threads (we only have one thread)

	ID3D12GraphicsCommandList* m_commandList; // a command list we can record commands into, then execute them to render the frame

	ID3D12Fence* m_fence[frameBufferCount];    // an object that is locked while our command list is being executed by the gpu. We need as many 
											 //as we have allocators (more if we want to know when the gpu is finished with an asset)

	HANDLE m_fenceEvent; // a handle to an event when our fence is unlocked by the gpu

	UINT64 m_fenceValue[frameBufferCount]; // this value is incremented each frame. each fence will have its own value

	int m_frameIndex; // current rtv we are on

	int m_rtvDescriptorSize; // size of the rtv descriptor on the device (all front and back buffers will be the same size)

	

};
