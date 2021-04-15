#include "stdafx.h"
#include "D3D12Core.h"

// Handle to the window
HWND hwnd = NULL;

LPCTSTR WindowName = L"Window name";
LPCTSTR WindowTitle = L"Lamp";

Direct3D12* D3D12RendererPointer;

// width and height of the window
int Width = 800;
int Height = 600;

// is window full screen?
bool FullScreen = false;

struct Timer
{
	double timerFrequency = 0.0;
	long long lastFrameTime = 0;
	long long lastSecond = 0;
	double frameDelta = 0;
	int fps = 0;
	float fpsSum = 0;
	int iterator = 0;
	float fpstracker = 0;

	Timer()
	{
		LARGE_INTEGER li;
		QueryPerformanceFrequency(&li);

		// seconds
		timerFrequency = double(li.QuadPart);

		// milliseconds
		//timerFrequency = double(li.QuadPart) / 1000.0;

		// microseconds
		//timerFrequency = double(li.QuadPart) / 1000000.0;

		QueryPerformanceCounter(&li);
		lastFrameTime = li.QuadPart;
	}

	// Call this once per frame
	double GetFrameDelta()
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		frameDelta = double(li.QuadPart - lastFrameTime) / timerFrequency;
		if (frameDelta > 0) {
			fpstracker += frameDelta;
			iterator++;
			fpsSum += 1 / frameDelta;
			if (fpstracker > 0.5f)
			{
				fps = fpsSum / iterator;
				fpstracker = 0;
				iterator = 0;
				fpsSum = 0;
			}
		}
		lastFrameTime = li.QuadPart;
		return frameDelta;
	}
};

Timer timer;

bool InitializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen);
void gameloop();
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {

	RECT rc = { 0, 0, Width, Height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	// create the window
	if (!InitializeWindow(hInstance, nShowCmd, 
		rc.right - rc.left, rc.bottom - rc.top, FullScreen))
	{
		MessageBox(0, L"Window Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}
	
	// initialize direct3d
	D3D12RendererPointer = new Direct3D12;
	if (!D3D12RendererPointer->InitD3D(hwnd, Width, Height))
	{
		MessageBox(0, L"Failed to initialize direct3d 12 :D",
			L"Error", MB_OK);
		D3D12RendererPointer->Cleanup();
		return 1;
	}

	// start the main loop
	gameloop();


	delete D3D12RendererPointer;


	return 0;
}

bool InitializeWindow(HINSTANCE hInstance,int ShowWnd,int width, int height,bool fullscreen) {

	if (fullscreen) {

		HMONITOR hmon = MonitorFromWindow(hwnd,
			MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hmon, &mi);

		width = mi.rcMonitor.right - mi.rcMonitor.left;
		height = mi.rcMonitor.bottom - mi.rcMonitor.top;
	}

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WindowName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {

		MessageBoxA(NULL, "Error registering class",
			"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	hwnd = CreateWindowEx(NULL,
		WindowName,
		WindowTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!hwnd) {

		MessageBoxW(NULL, L"Error creating window",
			L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	if (fullscreen) {
		SetWindowLong(hwnd, GWL_STYLE, 0);
	}

	ShowWindow(hwnd, ShowWnd);
	UpdateWindow(hwnd);


	return true;
}

void gameloop() {
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			// run game code
			double dt = timer.GetFrameDelta();
			D3D12RendererPointer->Update(dt);
			D3D12RendererPointer->Render();


			static float a = 0.0f;
			a += dt;
			if (a > 0.05f)
			{
				std::string text = std::to_string(timer.fps) + ", AnimFrame: " + std::to_string(D3D12RendererPointer->getAnimIndex());
				SetWindowTextA(hwnd, text.c_str());
				a = 0.0f;
			}
			
		}
	}

}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	
	switch (msg)
	{

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			if (MessageBox(0, L"Are you sure you want to exit?",
				L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
				DestroyWindow(hwnd);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}