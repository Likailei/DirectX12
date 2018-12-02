#pragma once

#include "Camera.h"
#include "Cube.h"

const UINT NUM_TEXTURE = 539;

class Game
{
public:
	Game(UINT width, UINT height, std::wstring name);
	HWND m_hwnd;
	Camera m_camera;
	Timer m_timer;

	UINT GetWidth() const { return m_width; }
	UINT GetHeight() const { return m_height; }
	const WCHAR* GetTitle() const { return m_title.c_str(); }

	void OnKeyUp(UINT8 key);
	void OnKeyDown(UINT8 key);

	void OnInit();
	void OnUpdate();
	void OnRender();
	void OnDestroy();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
	void OnMWheelRotate(WPARAM btnState);

	void LoadFiles();
	void LoadTextures();

private:
	UINT m_width;
	UINT m_height;
	float m_aspectRatio;
	std::wstring m_title;

	static const UINT FrameCount = 2;
	UINT m_frameCounter = 0;
	double m_intermediateTime = 0.0f;
	// Camera.
	float mTheta = 1.5f*XM_PI;
	float mPhi = XM_PIDIV4;
	float mRadius = 5.0f;
	POINT m_mousePos;

	// Files
	std::vector<std::string> m_pngFileName;
	std::vector<Texture> m_textures;

	struct Vertex {
		Vertex() {}
		Vertex(float x, float y, float z, float u, float v, float nx, float ny, float nz, UINT fi) :
			position(x, y, z), uv(u, v), normal(nx, ny, nz), faceIndex(fi) {}

		XMFLOAT3 position;
		XMFLOAT2 uv;
		XMFLOAT3 normal;
		UINT faceIndex;
	};

	struct Mesh {
		std::vector<Vertex> v;
		std::vector<UINT> i;
	};

	struct cbObject {
		XMFLOAT4X4 wvpMat;
		XMFLOAT4 ambientColor;
		float ambientIntensity;
		XMFLOAT3 direction;
		float padding[40];
	};

	std::vector<Cube> cubes;
	cbObject constantObject;

	bool m_useWarpDevice = false;
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_cbvSrvHeap;
	ComPtr<ID3D12DescriptorHeap> m_samplerHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;

	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_indexBuffer;
	ComPtr<ID3D12Resource> m_textureBuffers[NUM_TEXTURE];
	ComPtr<ID3D12Resource> m_cbvBuffers[FrameCount];
	ComPtr<ID3D12Resource> m_depthStencilBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

	UINT m_frameIndex;
	UINT m_rtvDescSize;
	UINT m_cbuDescSize;
	UINT8* cbvGPUAddress[FrameCount];

	HANDLE m_fenceEvent;
	UINT64 m_fenceValue;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValues[FrameCount];

	// Matrix.
	XMVECTOR cubePos;
	XMVECTOR cameraPos;
	XMVECTOR cameraTar;
	XMVECTOR cameraUp;
	XMFLOAT4X4 projMat, viewMat, rotMat, worldMat;

	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void WaitForGpu();
	void MoveToNextFrame();
	void InitMatrix();
};