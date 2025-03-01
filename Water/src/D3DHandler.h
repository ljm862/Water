#pragma once
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <directxmath.h>
#include <vector>
#include <memory>
#include <stdexcept>
using namespace DirectX;
class D3DHandler
{
public:
	D3DHandler();
	~D3DHandler();

	bool Init(int screenWidth, int screenHeight, int vsyncEnabled, HWND hwnd, bool fullscreen, float screenDepth, float screenNear);

	void Shutdown();

	void BeginScene(float r, float g, float b, float a);
	void EndScene();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	void GetProjectionMatrix(XMMATRIX&);
	void GetWorldMatrix(XMMATRIX&);
	void GetOrthoMatrix(XMMATRIX&);

	void SetBackBufferRenderTarget();
	void ResetViewport();

	void TurnZBufferOn();
	void TurnZBufferOff();

	void EnableAlphaBlending();
	void DisableAlphaBlending();

private:
	void CheckSuccess(HRESULT hResult);
	void SetupDepthStencilDesc(ID3D11DepthStencilState* depthStencilState, bool depthEnable, D3D11_DEPTH_WRITE_MASK depthWriteMask, D3D11_COMPARISON_FUNC depthFunc, bool stencilEnable, UINT8 readMask, UINT8 writeMask,
		D3D11_STENCIL_OP frontFail, D3D11_STENCIL_OP frontDepthFail, D3D11_STENCIL_OP frontPass, D3D11_COMPARISON_FUNC frontFunc, 
		D3D11_STENCIL_OP backFail, D3D11_STENCIL_OP backDepthFail, D3D11_STENCIL_OP backPass, D3D11_COMPARISON_FUNC backFunc);

private:
	int m_vsync_enabled;
	XMMATRIX m_projectionMatrix;
	XMMATRIX m_worldMatrix;
	XMMATRIX m_orthoMatrix;

	IDXGISwapChain* m_swapChain;
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11RasterizerState* m_rasterState;

	D3D11_VIEWPORT m_viewport;
	ID3D11DepthStencilState* m_depthDisabledStencilState;
	ID3D11BlendState* m_alphaEnableBlendingState;
	ID3D11BlendState* m_alphaDisableBlendingState;
};

