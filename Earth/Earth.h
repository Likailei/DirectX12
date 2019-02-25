#pragma once

#include "Camera.h"

class Earth
{
public:
    Earth(UINT width, UINT height, std::wstring name);
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

    float m_radius;

    // Files
    Texture m_textures[1];
    Mesh m_mesh;

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

    ComPtr<ID3D12Resource> m_vertexBuffer;
    ComPtr<ID3D12Resource> m_indexBuffer;
    ComPtr<ID3D12Resource> m_textureBuffer;
    ComPtr<ID3D12Resource> m_cbvBuffers[FrameCount];
    ComPtr<ID3D12Resource> m_depthStencilBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;


    ComPtr<ID3D12Resource> vertexBufferUploadHeap;
    ComPtr<ID3D12Resource> indexBufferUploadHeap;
    ComPtr<ID3D12Resource> textureBufferUploadHeap;

    UINT m_frameIndex;
    UINT m_rtvDescSize;
    UINT m_cbuDescSize;
    UINT8* cbvGPUAddress[FrameCount];

    HANDLE m_fenceEvent;
    UINT64 m_fenceValue;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValues[FrameCount];

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void InitBundles();
    void WaitForGpu();
    void MoveToNextFrame();
    void InitMatrix();
    void GenerateMesh();
    void Subdivide(Mesh& outMesh, Mesh& inMesh);
    void LoadTextures();
};