#include "D3DHandler.h"


D3DHandler::D3DHandler() : m_swapChain(0), m_device(0), m_deviceContext(0), m_renderTargetView(0), m_depthStencilBuffer(0), m_depthStencilState(0),
m_depthStencilView(0), m_rasterState(0), m_depthDisabledStencilState(0), m_alphaEnableBlendingState(0), m_alphaDisableBlendingState(0)
{
}

D3DHandler::~D3DHandler()
{
}

bool D3DHandler::Init(int screenWidth, int screenHeight, int vsyncEnabled, HWND hwnd, bool fullscreen, float screenDepth, float screenNear)
{
	m_vsync_enabled = vsyncEnabled;

	IDXGIFactory* factory;
	CheckSuccess(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory));

	IDXGIAdapter* adapter;
	CheckSuccess(factory->EnumAdapters(0, &adapter));

	IDXGIOutput* adapterOutput;
	CheckSuccess(adapter->EnumOutputs(0, &adapterOutput));

	unsigned int numModes;
	CheckSuccess(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr));

	auto displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList) return false;

	CheckSuccess(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList));

	unsigned int numerator = 0, denominator = 1;
	if (m_vsync_enabled) {
		for (int i = 0; i < numModes; i++)
		{
			if (displayModeList[i].Width == (unsigned int)screenWidth)
			{
				if (displayModeList[i].Height == (unsigned int)screenHeight)
				{
					numerator = displayModeList[i].RefreshRate.Numerator;
					denominator = displayModeList[i].RefreshRate.Denominator;
					break;
				}
			}
		}
	}

	DXGI_ADAPTER_DESC adapterDesc;
	CheckSuccess(adapter->GetDesc(&adapterDesc));

	//m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);
	//
	//// Convert the name of the video card to a character array and store it.
	//error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
	//if (error != 0)
	//{
	//	return false;
	//}

	delete[] displayModeList;
	adapterOutput->Release();
	factory->Release();
	adapter->Release();
	displayModeList = nullptr;
	adapterOutput = nullptr;
	adapter = nullptr;
	factory = nullptr;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = !fullscreen;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	D3D_FEATURE_LEVEL featureLevel;
	featureLevel = D3D_FEATURE_LEVEL_11_1;

	CheckSuccess(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, nullptr, &m_deviceContext));

	ID3D11Texture2D* backBufferPtr;
	CheckSuccess(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr));
	CheckSuccess(m_device->CreateRenderTargetView(backBufferPtr, nullptr, &m_renderTargetView));

	backBufferPtr->Release();
	backBufferPtr = nullptr;

	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	CheckSuccess(m_device->CreateTexture2D(&depthBufferDesc, nullptr, &m_depthStencilBuffer));

	SetupDepthStencilDesc(m_depthStencilState, true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, true, 0xFF, 0xFF, 
		D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_INCR, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS,
		D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_DECR, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS);

	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

	SetupDepthStencilDesc(m_depthDisabledStencilState, false, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, true, 0xFF, 0xFF,
		D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_INCR, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS,
		D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_DECR, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	CheckSuccess(m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView));

	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	CheckSuccess(m_device->CreateRasterizerState(&rasterDesc, &m_rasterState));
	m_deviceContext->RSSetState(m_rasterState);

	m_viewport.Width = (float)screenWidth;
	m_viewport.Height = (float)screenHeight;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
	m_deviceContext->RSSetViewports(1, &m_viewport);

	float fieldOfView = 3.141592654f / 4.0f;
	float screenAspect = (float)screenWidth / (float)screenHeight;

	m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);
	m_worldMatrix = XMMatrixIdentity();
	m_orthoMatrix = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth);

	D3D11_BLEND_DESC blendStateDescription;
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));

	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	CheckSuccess(m_device->CreateBlendState(&blendStateDescription, &m_alphaEnableBlendingState));
	blendStateDescription.RenderTarget[0].BlendEnable = FALSE;
	CheckSuccess(m_device->CreateBlendState(&blendStateDescription, &m_alphaDisableBlendingState));


	return true;
}

// Probably better suited to a macro?
void D3DHandler::CheckSuccess(HRESULT hResult)
{
	if (FAILED(hResult))
	{
		throw "Failed to Init D3D";
	}
}

void D3DHandler::SetupDepthStencilDesc(ID3D11DepthStencilState* depthStencilState, bool depthEnable, D3D11_DEPTH_WRITE_MASK depthWriteMask, D3D11_COMPARISON_FUNC depthFunc, 
	bool stencilEnable, UINT8 readMask, UINT8 writeMask, D3D11_STENCIL_OP frontFail, D3D11_STENCIL_OP frontDepthFail, D3D11_STENCIL_OP frontPass, D3D11_COMPARISON_FUNC frontFunc,
	D3D11_STENCIL_OP backFail, D3D11_STENCIL_OP backDepthFail, D3D11_STENCIL_OP backPass, D3D11_COMPARISON_FUNC backFunc)
{
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	depthStencilDesc.DepthEnable = depthEnable;
	depthStencilDesc.DepthWriteMask = depthWriteMask;
	depthStencilDesc.DepthFunc = depthFunc;
	depthStencilDesc.StencilEnable = stencilEnable;
	depthStencilDesc.StencilReadMask = readMask;
	depthStencilDesc.StencilWriteMask = writeMask;
	depthStencilDesc.FrontFace.StencilFailOp = frontFail;
	depthStencilDesc.FrontFace.StencilDepthFailOp = frontDepthFail;
	depthStencilDesc.FrontFace.StencilPassOp = frontPass;
	depthStencilDesc.FrontFace.StencilFunc = frontFunc;
	depthStencilDesc.BackFace.StencilFailOp = backFail;
	depthStencilDesc.BackFace.StencilDepthFailOp = backDepthFail;
	depthStencilDesc.BackFace.StencilPassOp = backPass;
	depthStencilDesc.BackFace.StencilFunc = backFunc;
	CheckSuccess(m_device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState));
}

void D3DHandler::Shutdown()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (m_swapChain)
	{
		m_swapChain->SetFullscreenState(false, NULL);
	}

	if (m_alphaEnableBlendingState)
	{
		m_alphaEnableBlendingState->Release();
		m_alphaEnableBlendingState = nullptr;
	}

	if (m_alphaDisableBlendingState)
	{
		m_alphaDisableBlendingState->Release();
		m_alphaDisableBlendingState = nullptr;
	}

	if (m_rasterState)
	{
		m_rasterState->Release();
		m_rasterState = nullptr;
	}

	if (m_depthStencilView)
	{
		m_depthStencilView->Release();
		m_depthStencilView = nullptr;
	}

	if (m_depthDisabledStencilState)
	{
		m_depthDisabledStencilState->Release();
		m_depthDisabledStencilState = nullptr;
	}

	if (m_depthStencilState)
	{
		m_depthStencilState->Release();
		m_depthStencilState = nullptr;
	}

	if (m_depthStencilBuffer)
	{
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = nullptr;
	}

	if (m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = nullptr;
	}

	if (m_deviceContext)
	{
		m_deviceContext->Release();
		m_deviceContext = nullptr;
	}

	if (m_device)
	{
		m_device->Release();
		m_device = nullptr;
	}

	if (m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = nullptr;
	}
}

void D3DHandler::BeginScene(float red, float green, float blue, float alpha)
{
	std::vector<float> color(4);

	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	m_deviceContext->ClearRenderTargetView(m_renderTargetView, color.data());
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void D3DHandler::EndScene()
{
	m_swapChain->Present(m_vsync_enabled, 0);
}

ID3D11Device* D3DHandler::GetDevice()
{
	return m_device;
}

ID3D11DeviceContext* D3DHandler::GetDeviceContext()
{
	return m_deviceContext;
}

void D3DHandler::GetWorldMatrix(XMMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
}

void D3DHandler::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
}

void D3DHandler::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
}

void D3DHandler::SetBackBufferRenderTarget()
{
	// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
}

void D3DHandler::ResetViewport()
{
	m_deviceContext->RSSetViewports(1, &m_viewport);
}

void D3DHandler::TurnZBufferOn()
{
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);
}

void D3DHandler::TurnZBufferOff()
{
	m_deviceContext->OMSetDepthStencilState(m_depthDisabledStencilState, 1);
}

void D3DHandler::EnableAlphaBlending()
{
	auto blendFactor = std::vector<float>(4);
	m_deviceContext->OMSetBlendState(m_alphaEnableBlendingState, blendFactor.data(), 0xffffffff);
}

void D3DHandler::DisableAlphaBlending()
{
	auto blendFactor = std::vector<float>(4);
	m_deviceContext->OMSetBlendState(m_alphaDisableBlendingState, blendFactor.data(), 0xffffffff);
}