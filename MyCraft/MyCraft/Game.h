#pragma once

#include "Camera.h"
#include "Region.h"

const UINT NUM_TEXTURE = 539;
const float PLAYER_HEIGHT = 1.8f;

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

    void OnInit();
    void OnUpdate();
    void OnRender();
    void OnDestroy();

    void OnKeyboardInput(double& dt);
    void OnMouseDown(WPARAM btnState, int x, int y);
    void OnMouseUp(WPARAM btnState, int x, int y);
    void OnMouseMove(WPARAM btnState, int x, int y);
    void OnMWheelRotate(WPARAM btnState);

    void LoadFiles();
    void LoadTextures();

private:
    struct cbObject {
        XMFLOAT4X4 wvpMat;
        XMFLOAT4 ambientColor;
        float ambientIntensity;
        XMFLOAT3 direction;
        float padding[40];
    };

    struct Speed {
        Speed() {}
        Speed(float f, float b, float s) : forward(f), back(b), strafe(s) {}

        float forward;
        float back;
        float strafe;
    };

    struct MousePosition {
        float x;
        float y;
    };

    UINT m_width;
    UINT m_height;
    float m_aspectRatio;
    std::wstring m_title;

    Speed m_speed;
    MousePosition m_lastMousePosition;

    static const UINT FrameCount = 3;
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

    std::vector<Block> m_blocks;
    Region m_region;
    std::vector<Chunk> m_chunks;
    std::vector<Vertex> blockVertices;
    std::vector<UINT> blockIndices;
    std::vector<XMFLOAT3> blockPositions;
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
    ComPtr<ID3D12CommandAllocator> m_mainCommandAllocator;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> m_frameCommandAllocators[FrameCount];
    ComPtr<ID3D12CommandAllocator> m_frameBundleAllocators[FrameCount];
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    ComPtr<ID3D12GraphicsCommandList> m_bundles[FrameCount];
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipelineState;

    ComPtr<ID3D12Resource> m_vertexBuffers[CHUNKCNT];
    ComPtr<ID3D12Resource> m_indexBuffers[CHUNKCNT];
    ComPtr<ID3D12Resource> m_textureBuffers[NUM_TEXTURE];
    ComPtr<ID3D12Resource> m_cbvBuffers[FrameCount];
    ComPtr<ID3D12Resource> m_depthStencilBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferViews[CHUNKCNT];
    D3D12_INDEX_BUFFER_VIEW m_indexBufferViews[CHUNKCNT];


    ComPtr<ID3D12Resource> vertexBufferUploadHeaps[CHUNKCNT];
    ComPtr<ID3D12Resource> indexBufferUploadHeaps[CHUNKCNT];
    ComPtr<ID3D12Resource> textureBufferUploadHeap[NUM_TEXTURE];

    UINT m_frameIndex;
    UINT m_rtvDescSize;
    UINT m_cbuDescSize;
    UINT8* cbvGPUAddress[FrameCount];

    HANDLE m_fenceEvent;
    UINT64 m_fenceValue;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValues[FrameCount];

    // Matrix.
    XMVECTOR BlockPos;
    XMVECTOR cameraPos;
    XMVECTOR cameraTar;
    XMVECTOR cameraUp;
    XMFLOAT4X4 projMat, viewMat, rotMat, worldMat;

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void InitBundles();
    void WaitForGpu();
    void MoveToNextFrame();
    void InitMatrix();
    void InitRegion(XMFLOAT3* playerPos);
};