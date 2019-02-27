#include "Earth.h"
                            
Earth::Earth(UINT width, UINT height, std::wstring name) :
    m_width(width),
    m_height(height),
    m_aspectRatio(1.0f),
    m_camera(Camera(m_hwnd, static_cast<float>(width) / static_cast<float>(height))),
    m_title(name),
    //m_speed(8.0f, -6.0f, 6.0f),
    m_speed(50.0f, -15.0f, 30.0f),
    m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_rtvDescSize(0),
    m_cbuDescSize(0),
    m_frameIndex(0),
    m_radius(5.0f)
{
    m_timer.Tick();
    srand(time(NULL));

    cubemapFaceCenters[FRONT] = XMFLOAT3(0.0f, 0.0f, -m_radius);
    cubemapFaceCenters[BACK] = XMFLOAT3(0.0f, 0.0f, m_radius);
    cubemapFaceCenters[LEFT] = XMFLOAT3(-m_radius, 0.0f, 0.0f);
    cubemapFaceCenters[RIGHT] = XMFLOAT3(m_radius, 0.0f, 0.0f);
    cubemapFaceCenters[TOP] = XMFLOAT3(0.0f, m_radius, 0.0f);
    cubemapFaceCenters[BOTTOM] = XMFLOAT3(0.0f, -m_radius, 0.0f);

    for (int i = 0; i < 6; ++i) {
        XMStoreFloat3(&cubemapFaceNormals[i], XMVector3Normalize(XMLoadFloat3(&cubemapFaceCenters[i])));
    }

    constantObject.ambientColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    constantObject.ambientIntensity = 0.0f;
    constantObject.direction = XMFLOAT3(0.3f, -0.6f, -0.4f);
}

void Earth::OnInit() {
    
    LoadPipeline();
    LoadAssets();
}

void Earth::LoadPipeline() {
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    ThrowIfFailed(D3D12CreateDevice(
        nullptr,
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&m_device)));

    // Command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

    // Swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        m_hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    ));

    // This sample does not support fullscreen transitions.
    ThrowIfFailed(factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain.As(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Descriptor heaps.
    {
        // RTV
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        // SRV CBV heaps.
        D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc = {};
        cbvSrvHeapDesc.NumDescriptors = TextureCnt + FrameCount;
        cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_PPV_ARGS(&m_cbvSrvHeap)));

        // Sampler heaps.
        D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
        samplerHeapDesc.NumDescriptors = 1;
        samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_samplerHeap)));

        // Depth stencil heap.
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

        m_rtvDescSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_cbuDescSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_mainCommandAllocator)));
        // Create a RTV and a command allocator for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescSize);

            ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_frameCommandAllocators[n])));
            ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(&m_frameBundleAllocators[n])));
        }
    }
}

void Earth::LoadAssets() {

    // Root signature.
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, TextureCnt, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        CD3DX12_ROOT_PARAMETER1 rootParameters[2];
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_VERTEX);
        

        // create static sampler
        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;

        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));

        ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    // Pipeline state.
    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;
        ComPtr<ID3DBlob> errorMessages;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif
        HRESULT h;
        h = D3DCompileFromFile(L"VS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", compileFlags, 0, &vertexShader, &errorMessages);
        if (errorMessages != nullptr)
        {
            OutputDebugStringA((char*)errorMessages->GetBufferPointer());
        }

        h = D3DCompileFromFile(L"PS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", compileFlags | D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES, 0, &pixelShader, &errorMessages);
        if (errorMessages != nullptr)
        {
            OutputDebugStringA((char*)errorMessages->GetBufferPointer());
        }

        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            { "FACE", 0, DXGI_FORMAT_R8_UINT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // PSO.
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, sizeof(inputElementDescs) / sizeof(D3D12_INPUT_ELEMENT_DESC) };
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.SampleDesc.Count = 1;
        //psoDesc.SampleDesc.Quality = DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN;

        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
    }

    // CommandList.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_mainCommandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));
    
    for (UINT8 i = 0; i < FrameCount; i++) {
        ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, m_frameBundleAllocators[i].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_bundles[i])));
    }
    
    // Vertex buffer and index buffer.
    { 
        GenerateMesh();
        UINT vSize = m_mesh.v.size() * sizeof(Vertex);
        UINT iSize = m_mesh.i.size() * sizeof(UINT);

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(vSize),
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_vertexBuffer)));

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(vSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&vertexBufferUploadHeap)));

        D3D12_SUBRESOURCE_DATA vertexData = {};
        vertexData.pData = (UINT8*)(m_mesh.v.data());
        vertexData.RowPitch = vSize;
        vertexData.SlicePitch = vertexData.RowPitch;

        UpdateSubresources<1>(m_commandList.Get(), m_vertexBuffer.Get(), vertexBufferUploadHeap.Get(), 0, 0, 1, &vertexData);
        m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = vSize;

        // Index buffer.
        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(iSize),
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_indexBuffer)));

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(iSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&indexBufferUploadHeap)));

        D3D12_SUBRESOURCE_DATA indexData = {};
        indexData.pData = (UINT8*)(m_mesh.i.data());
        indexData.RowPitch = iSize;
        indexData.SlicePitch = indexData.RowPitch;

        UpdateSubresources<1>(m_commandList.Get(), m_indexBuffer.Get(), indexBufferUploadHeap.Get(), 0, 0, 1, &indexData);
        m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

        // Initialize the vertex buffer view.
        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
        m_indexBufferView.SizeInBytes = iSize;
    }

    // Texture buffer.
    {
        LoadTextures();

        // Describe and create a Texture2D.
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.MipLevels = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        textureDesc.Width = m_textures[0].width;
        textureDesc.Height = m_textures[0].height;

        for (int i = 0; i < TextureCnt; ++i) {
            ThrowIfFailed(m_device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE,
                &textureDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr,
                IID_PPV_ARGS(&m_textureBuffers[i])));
            
            UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_textureBuffers[i].Get(), 0, 1);

            // Copy data to the intermediate upload heap and then schedule a copy 
            // from the upload heap to the Texture2D.
            // Create the GPU upload buffer.
            ThrowIfFailed(m_device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&textureBufferUploadHeaps[i])));

            D3D12_SUBRESOURCE_DATA textureData = {};
            textureData.pData = m_textures[i].data.data();
            textureData.RowPitch = m_textures[i].width * 4;
            textureData.SlicePitch = textureData.RowPitch * m_textures[i].height;

            UpdateSubresources(m_commandList.Get(), m_textureBuffers[i].Get(), textureBufferUploadHeaps[i].Get(), 0, 0, 1, &textureData);
            m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_textureBuffers[i].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
        }
        
        // Describe and create a SRV for the texture.
        CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart());
        for (int i = 0; i < TextureCnt; ++i) {
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            m_device->CreateShaderResourceView(m_textureBuffers[i].Get(), &srvDesc, srvHandle);

            srvHandle.Offset(m_cbuDescSize);
        }
    }

    // Constant buffer view.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart(), TextureCnt, m_cbuDescSize);    // Move past the SRVs.

        UINT64 bufferSize = 1024 * 64;
        for (UINT8 i = 0; i < FrameCount; i++) {
            ThrowIfFailed(m_device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_cbvBuffers[i])));

            CD3DX12_RANGE readRange(0, 0);
            ThrowIfFailed(m_cbvBuffers[i]->Map(0, &readRange, reinterpret_cast<void**>(&cbvGPUAddress[i])));
            
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
            cbvDesc.BufferLocation = m_cbvBuffers[i]->GetGPUVirtualAddress();
            cbvDesc.SizeInBytes = sizeof(cbObject);
            m_device->CreateConstantBufferView(&cbvDesc, cbvHandle);

            cbvHandle.Offset(1, m_cbuDescSize);
        }
    }
    // Depth stencil view buffer.
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
        depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

        D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
        depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
        depthOptimizedClearValue.DepthStencil.Stencil = 0;

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depthOptimizedClearValue,
            IID_PPV_ARGS(&m_depthStencilBuffer)
        ));

        m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    }

    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(m_device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValues[m_frameIndex]++;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        WaitForGpu();
    }

    InitBundles();
    InitMatrix();
}

void Earth::InitBundles() {
    // Record bundle.
    ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvHeap.Get() };
    CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart());
    CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle(m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart(), TextureCnt + m_frameIndex, m_cbuDescSize);

    for (UINT8 i = 0; i < FrameCount; i++) {
        m_bundles[i]->SetGraphicsRootSignature(m_rootSignature.Get());
        m_bundles[i]->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
        m_bundles[i]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_bundles[i]->SetGraphicsRootDescriptorTable(0, srvHandle);
        m_bundles[i]->SetGraphicsRootDescriptorTable(1, cbvHandle);  
        m_bundles[i]->IASetVertexBuffers(0, 1, &m_vertexBufferView);
        m_bundles[i]->IASetIndexBuffer(&m_indexBufferView);

        m_bundles[i]->DrawIndexedInstanced(m_mesh.i.size(), 1, 0, 0, 0);
 
        ThrowIfFailed(m_bundles[i]->Close());
    }
}

void Earth::PopulateCommandList()
{
    ThrowIfFailed(m_frameCommandAllocators[m_frameIndex]->Reset());
    ThrowIfFailed(m_commandList->Reset(m_frameCommandAllocators[m_frameIndex].Get(), m_pipelineState.Get()));

    ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvHeap.Get() };
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);
    
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    const float clearColor[] = { 0.776f, 0.886f, 1.0f, 0.5f };
    //const float clearColor[] = { 0.1f, 0.1f, 0.1f, 0.5f };
    //const float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    m_commandList->ExecuteBundle(m_bundles[m_frameIndex].Get());

    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());
}

void Earth::WaitForGpu()
{
    // Schedule a Signal command in the queue.
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

    // Wait until the fence has been processed.
    ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

    // Increment the fence value for the current frame.
    m_fenceValues[m_frameIndex]++;
}

void Earth::MoveToNextFrame()
{
    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

    // Update the frame index.
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    }

    // Set the fence value for the next frame.
    m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}

void Earth::InitMatrix()
{
    /*XMMATRIX tmpMat = XMMatrixPerspectiveFovLH(45.0f*(3.14f / 180.0f), (float)m_width / (float)m_height, 0.1f, 1000.0f);
    XMStoreFloat4x4(&projMat, tmpMat);

    cameraPos = XMVectorSet(2.0f, 1.0f, -3.0f, 0.0f);
    cameraTar = XMVectorZero();
    cameraUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    tmpMat = XMMatrixLookAtLH(cameraPos, cameraTar, cameraUp);
    XMStoreFloat4x4(&viewMat, tmpMat);

    BlockPos = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    tmpMat = XMMatrixTranslationFromVector(BlockPos);
    XMStoreFloat4x4(&worldMat, tmpMat);

    XMStoreFloat4x4(&rotMat, XMMatrixIdentity());*/
}

void Earth::GenerateMesh()
{
    // Golden ratio
    const float phi = 1.618033f;
    // Edge length
    float a = m_radius / 0.951056f;
    // Half of rectangle's length
    float l = a * phi / 2.0f;
    // Half of rectangle's width
    float w = a / 2.0f;

    Mesh m;
    m.v = {
        Vertex(0.0f , l    , w   ,  0.0f, 0.0f, TOP ),
        Vertex(0.0f , l    , -w  ,  0.0f, 0.0f, TOP ),
        Vertex(-l   , w    , 0.0f,  0.0f, 0.0f, BACK ),
        Vertex(-w   , 0.0f , l   ,  0.0f, 0.0f, RIGHT ),
        Vertex(w    , 0.0f , l   ,  0.0f, 0.0f, RIGHT ),
        Vertex(l    , w    , 0.0f,  0.0f, 0.0f, FRONT ),
        Vertex(w    , 0.0f , -l  ,  0.0f, 0.0f, LEFT ),
        Vertex(-w   , 0.0f , -l  ,  0.0f, 0.0f, LEFT ),
        Vertex(-l   , -w   , 0.0f,  0.0f, 0.0f, BACK ),
        Vertex(0.0f , -l   , w   ,  0.0f, 0.0f, BOTTOM ),
        Vertex(l    , -w   , 0.0f,  0.0f, 0.0f, FRONT ),
        Vertex(0.0f , -l   , -w  ,  0.0f, 0.0f, BOTTOM ),
    };

    m.i = {
            0, 1, 2,  0, 2, 3,  0, 3, 4,  0, 4, 5, 0, 5, 1,
            1, 5, 6,  1, 6, 7,  2, 1, 7,  2, 7, 8, 2, 8, 3,
            3, 8, 9,  4, 3, 9,  4, 9, 10, 5, 4, 10, 5, 10, 6,
            11, 7, 6, 11, 8, 7, 11, 9, 8, 11, 10, 9, 11, 6, 10,
    };

    Mesh m1, m2, m3, m4;
    Subdivide(m1, m);
    Subdivide(m2, m1);
    Subdivide(m3, m2);
    Subdivide(m4, m3);
    Subdivide(m_mesh, m4);
}

void Earth::Subdivide(Mesh& outMesh, Mesh& inMesh)
{
    int offset = 0;

    //            A
    //           / \
    //          /   \
    //         Q     O
    //        /       \
    //       /         \
    //      C-----P-----B
    for (int n = 0; n < inMesh.i.size(); n += 3) {
        Vertex va, vb, vc;
        va = inMesh.v[inMesh.i[n]];
        vb = inMesh.v[inMesh.i[n + 1]];
        vc = inMesh.v[inMesh.i[n + 2]];

        XMVECTOR A = XMLoadFloat3(&va.position);
        XMVECTOR B = XMLoadFloat3(&vb.position);
        XMVECTOR C = XMLoadFloat3(&vc.position);

        // Mid point.
        XMVECTOR O = (A + B) / 2;
        XMVECTOR P = (B + C) / 2;
        XMVECTOR Q = (A + C) / 2;

        // Porject to circumscribed sphere.
        XMVECTOR nO = XMVector3Normalize(O);
        XMVECTOR nP = XMVector3Normalize(P);
        XMVECTOR nQ = XMVector3Normalize(Q);

        O = nO * m_radius;
        P = nP * m_radius;
        Q = nQ * m_radius;

        Vertex vo{ O, nO };
        Vertex vp{ P, nP };
        Vertex vq{ Q, nQ };

        /*vo.face = CalculateUV(vo.uv, O);
        vp.face = CalculateUV(vp.uv, P);
        vq.face = CalculateUV(vq.uv, Q);*/

        CalculateUVProj(vo.uv, O);
        CalculateUVProj(vp.uv, P);
        CalculateUVProj(vq.uv, Q);

        outMesh.v.push_back(va); outMesh.v.push_back(vb); outMesh.v.push_back(vc);
        outMesh.v.push_back(vq); outMesh.v.push_back(vo); outMesh.v.push_back(vp);

        outMesh.i.push_back(0 + offset); outMesh.i.push_back(4 + offset); outMesh.i.push_back(3 + offset);
        outMesh.i.push_back(4 + offset); outMesh.i.push_back(1 + offset); outMesh.i.push_back(5 + offset);
        outMesh.i.push_back(3 + offset); outMesh.i.push_back(4 + offset); outMesh.i.push_back(5 + offset);
        outMesh.i.push_back(3 + offset); outMesh.i.push_back(5 + offset); outMesh.i.push_back(2 + offset);

        offset += 6;
    }
}

void Earth::LoadTextures()
{
    std::string textureFileName[] = { "./assets/neg_z.png", "./assets/pos_z.png", "./assets/neg_x.png", "./assets/pos_x.png", "./assets/pos_y.png", "./assets/neg_y.png" };
   /* for (int i = 0; i < TextureCnt; ++i) {
        lodepng::decode(m_textures[i].data, m_textures[i].width, m_textures[i].height, textureFileName[i]);
    }*/
    lodepng::decode(m_textures[0].data, m_textures[0].width, m_textures[0].height, "./assets/1.png");
}

Face Earth::CalculateUVCubemap(XMFLOAT2& uv, XMVECTOR& l)
{
    XMFLOAT3 fL, aL;
    XMStoreFloat3(&aL, XMVectorAbs(l));
    XMStoreFloat3(&fL, l);
    Face f;

    if (aL.x > aL.y && aL.x > aL.z) {
        f = aL.x > 0.0f ? RIGHT : LEFT;
    }
    else if (aL.y > aL.x && aL.y > aL.z) {
        f = aL.y > 0.0f ? TOP : BOTTOM;
    }
    else if (aL.z > aL.x && aL.z > aL.y) {
        f = aL.z > 0.0f ? BACK : FRONT;
    }
    //else if (aL.x == aL.y && aL.x == aL.z) {
    //    f = aL.x > 0.0f ? RIGHT : LEFT;
    //}

    XMVECTOR c = XMLoadFloat3(&cubemapFaceCenters[f]);
    XMVECTOR n = XMLoadFloat3(&cubemapFaceNormals[f]);
    XMVECTOR j = XMVector3Dot(c - l, n);
    XMVECTOR nl = XMVector3Normalize(l);
    XMVECTOR k = XMVector3Dot(nl, n);

    XMVECTOR p = (XMVectorGetX(j) / XMVectorGetX(k)) * nl + l;
    p = XMVector3Normalize(p);

    float w = 1.0f;
    switch (f)
    {
    case FRONT:
    {
        float u = (w / 2.0f + XMVectorGetX(p)) / w;
        float v = (w / 2.0f - XMVectorGetY(p)) / w;
        uv =  XMFLOAT2(u, v);
        break;
    }
    case BACK:
    {
        float u = (w / 2.0f - XMVectorGetX(p)) / w;
        float v = (w / 2.0f - XMVectorGetY(p)) / w;
        uv = XMFLOAT2(u, v);
        break;
    }
    case LEFT:
    {
        float u = (w / 2.0f - XMVectorGetZ(p)) / w;
        float v = (w / 2.0f - XMVectorGetY(p)) / w;
        uv = XMFLOAT2(u, v);
        break;
    }
    case RIGHT:
    {
        float u = (w / 2.0f + XMVectorGetZ(p)) / w;
        float v = (w / 2.0f - XMVectorGetY(p)) / w;
        uv = XMFLOAT2(u, v);
        break;
    }
    case TOP:
    {
        float u = (w / 2.0f + XMVectorGetX(p)) / w;
        float v = (w / 2.0f - XMVectorGetZ(p)) / w;
        uv = XMFLOAT2(u, v);
        break;
    }
    case BOTTOM:
    {
        float u = (w / 2.0f + XMVectorGetX(p)) / w;
        float v = (w / 2.0f + XMVectorGetZ(p)) / w;
        uv = XMFLOAT2(u, v);
        break;
    }
    default:
        break;
    }

    return f;
}

void Earth::CalculateUVProj(XMFLOAT2 & uv, XMVECTOR & l)
{
    l = XMVector3Normalize(l);

    // 点所在的水平圆周半径
    float theta = XMVectorGetX(XMVector3AngleBetweenVectors(l, XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f)));
    float r = sin(theta) * m_radius;
    float length = 2.0f * r * PI;

    // 点所在过圆心的垂直面与-z轴夹角
    XMVECTOR projX_l = XMVectorSet(XMVectorGetX(l), 0.0f, XMVectorGetZ(l), XMVectorGetW(l));
    float phi = XMVectorGetX(XMVector3AngleBetweenVectors(projX_l, XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f)));
    // 判断在+x/-x轴部分
    phi = XMVectorGetX(l) < 0.0f ? 2 * PI - phi : phi;

    uv.y = 1.0f / 2.0f - XMVectorGetY(l);
    uv.x = phi / (2.0f * PI);
}

void Earth::OnKeyboardInput(double& dt) {
    if (GetAsyncKeyState('W') & 0x8000)
        m_camera.Walk(m_speed.forward*dt);

    if (GetAsyncKeyState('S') & 0x8000)
        m_camera.Walk(m_speed.back*dt);

    if (GetAsyncKeyState('A') & 0x8000)
        m_camera.Strafe(-m_speed.strafe*dt);

    if (GetAsyncKeyState('D') & 0x8000)
        m_camera.Strafe(m_speed.strafe*dt);

    m_camera.UpdateViewMatrix();

    wchar_t debugStr[100];
    int cnt = swprintf(debugStr, 100, L"%f %f %f\n", m_camera.GetPosition3f().x, m_camera.GetPosition3f().y, m_camera.GetPosition3f().z);
    WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), debugStr, cnt, NULL, NULL);
}

void Earth::OnMouseDown(WPARAM btnState, int x, int y)
{
    m_lastMousePosition.x = (float)x;
    m_lastMousePosition.y = (float)y;

    SetCapture(m_hwnd);
}

void Earth::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void Earth::OnMouseMove(WPARAM btnState, int x, int y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f*static_cast<float>(x - m_lastMousePosition.x));
        float dy = XMConvertToRadians(0.25f*static_cast<float>(y - m_lastMousePosition.y));

        m_camera.Pitch(dy);
        m_camera.RotateY(dx);
    }

    m_lastMousePosition.x = x;
    m_lastMousePosition.y = y;
}

void Earth::OnMWheelRotate(WPARAM btnState)
{
    //m_camera.OnMouseWheelRotate(btnState);
}

void Earth::OnUpdate() {
    m_frameCounter++;
    double dt = m_timer.Tick();

    OnKeyboardInput(dt);
    
    //TODO: 判断移动后脚下方块是否是固体

    if (m_frameCounter == 300) {
        double t = m_timer.GetElapseTime();
        double delta = t - m_intermediateTime;

        wchar_t fps[64];
        swprintf_s(fps, L"%f fps", 300 / delta);
        std::wstring windowText = m_title + L": " + fps;
        SetWindowText(m_hwnd, windowText.c_str());

        m_frameCounter = 0;
        m_intermediateTime = t;
    }
    //XMStoreFloat4(&constantObject.ambientColor, XMVectorSet(0.0f, 1.0f, 1.0f, 1.0f));
    //constantObject.ambientIntensity = 0.2f;

    XMMATRIX view = m_camera.GetView();
    XMMATRIX proj = m_camera.GetProj();
    XMMATRIX viewProj = XMMatrixMultiply(view, proj);

    XMFLOAT3 p = XMFLOAT3(0.0f, 0.0f, 0.0f);
    XMMATRIX world = XMMatrixTranslationFromVector(XMLoadFloat3(&p));
    XMMATRIX tempWVPMat = XMMatrixMultiply(world, viewProj);
        
    XMStoreFloat4x4(&constantObject.wvpMat, XMMatrixTranspose(tempWVPMat));
    memcpy(cbvGPUAddress[m_frameIndex], &constantObject, sizeof(constantObject));
}

void Earth::OnRender() {
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(0, 0));

    MoveToNextFrame();
}

void Earth::OnDestroy() {
    WaitForGpu();
    CloseHandle(m_fenceEvent);
}
