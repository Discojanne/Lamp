//#include "pch.h"
#include "CompilerClass.h"
//#include "../../../Sail/utils/Utils.h"
#include <atlstr.h>
#include <string>

DXILShaderCompiler::DXILShaderCompiler()
	: m_linker(nullptr)
	, m_library(nullptr)
	, m_includeHandler(nullptr)
	, m_compiler(nullptr) {

}

DXILShaderCompiler::~DXILShaderCompiler() {

	if (m_compiler)
	{
		m_compiler->Release();
	}
	if (m_library)
	{
		m_library->Release();
	}
	if (m_includeHandler)
	{
		m_includeHandler->Release();
	}
	if (m_linker)
	{
		m_linker->Release();
	}
}

HRESULT DXILShaderCompiler::init() {

	HMODULE dll = LoadLibraryA("dxcompiler.dll");
	if (!dll) { 
		MessageBox(0, L"dxcompiler.dll is missing", L"Error", 0);
		return E_FAIL;
	}

	DxcCreateInstanceProc pfnDxcCreateInstance = DxcCreateInstanceProc(GetProcAddress(dll, "DxcCreateInstance"));

	HRESULT hr = E_FAIL;

	if (SUCCEEDED(hr = pfnDxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler)))) {
		if (SUCCEEDED(hr = pfnDxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_library)))) {
			if (SUCCEEDED(m_library->CreateIncludeHandler(&m_includeHandler))) {
				if (SUCCEEDED(hr = pfnDxcCreateInstance(CLSID_DxcLinker, IID_PPV_ARGS(&m_linker)))) {

				}
			}
		}
	}

	return hr;
}

HRESULT DXILShaderCompiler::compile(DXILShaderCompiler::Desc* desc, IDxcBlob** ppResult) {
	HRESULT hr = E_FAIL;

	if (desc) {
		IDxcBlobEncoding* source = nullptr;
		if (desc->source) {
			// Compile from source allocated on stack
			hr = m_library->CreateBlobWithEncodingFromPinned(desc->source, desc->sourceSize, CP_UTF8, &source);
		}
		else {
			// Compile from file path
			hr = m_library->CreateBlobFromFile(desc->filePath, nullptr, &source);
		}

		if (SUCCEEDED(hr)) {
			IDxcOperationResult* pResult = nullptr;
			if (SUCCEEDED(hr = m_compiler->Compile(
				source,									// program text
				desc->filePath,							// file name, mostly for error messages
				desc->entryPoint,						// entry point function
				desc->targetProfile,					// target profile
				desc->compileArguments.data(),          // compilation arguments
				(UINT)desc->compileArguments.size(),	// number of compilation arguments
				desc->defines.data(),					// define arguments
				(UINT)desc->defines.size(),				// number of define arguments
				m_includeHandler,						// handler for #include directives
				&pResult))) {
				HRESULT hrCompile = E_FAIL;
				if (SUCCEEDED(hr = pResult->GetStatus(&hrCompile))) {
					if (SUCCEEDED(hrCompile)) {
						if (ppResult) {
							pResult->GetResult(ppResult);
							hr = S_OK;
						}
						else {
							hr = E_FAIL;
						}
					}
					else {
						IDxcBlobEncoding* pPrintBlob = nullptr;
						if (SUCCEEDED(pResult->GetErrorBuffer(&pPrintBlob))) {
							// We can use the library to get our preferred encoding.
							IDxcBlobEncoding* pPrintBlob16 = nullptr;
							m_library->GetBlobAsUtf16(pPrintBlob, &pPrintBlob16);

							OutputDebugStringW((LPCWSTR)pPrintBlob16->GetBufferPointer());
							MessageBox(0, (LPCWSTR)pPrintBlob16->GetBufferPointer(), L"", 0);

							if (pPrintBlob)
							{
								pPrintBlob->Release();
							}
							if (pPrintBlob16)
							{
								pPrintBlob16->Release();
							}

						}
					}
					if (pResult)
						pResult->Release();
				}
			}
		}
	}

	if (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) || hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
		std::string errMsg("Missing shader: " + std::string(CW2A(desc->filePath)) + "\n");
		MessageBoxA(0, errMsg.c_str(), "DXILShaderCompiler Error", 0);
		OutputDebugStringA(errMsg.c_str());

	}

	return hr;
}
