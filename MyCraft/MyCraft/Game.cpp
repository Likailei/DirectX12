#include "Game.h"
UINT indexTexture = 0;
double lastTime = 0.0f;

const UINT8 MAPWIDTH_X = 30;
const UINT8 MAPWIDTH_Z = 30;
const UINT8 MAPWIDTH_Y = 2;
const UINT  CUBECOUNT = MAPWIDTH_X * MAPWIDTH_Y * MAPWIDTH_Z;

Game::Game(UINT width, UINT height, std::wstring name) :
	m_width(width),
	m_height(height),
	m_aspectRatio(1.0f),
	m_camera(Camera(m_hwnd, static_cast<float>(width) / static_cast<float>(height), XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f))),
	m_title(name),
	m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
	m_rtvDescSize(0),
	m_cbuDescSize(0),
	m_frameIndex(0)
{
	m_timer.Tick();

	constantObject.ambientColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	constantObject.ambientIntensity = 0.0f;
	constantObject.direction = XMFLOAT3(0.3f, -0.6f, -0.4f);

	LoadFiles();
	LoadTextures();

	float startX = (MAPWIDTH_X - 1) / 2.0f;
	float startZ = (MAPWIDTH_Z - 1) / 2.0f;
	float endY = -static_cast<float>(MAPWIDTH_Y);
	for (float Y = 0; Y > endY; Y -= 1.0f) {
		for (float Z = -startZ; Z <= startZ; Z += 1.0f) {
			for (float X = -startX; X <= startX; X += 1.0f) {
				Cube c(0);
				c.cubePosition = XMVectorSet(X, Y, Z, 1.0f);
				cubes.push_back(c);
			}
		}
	}
}

void Game::OnInit() {
	LoadPipeline();
	LoadAssets();
}

void Game::LoadPipeline() {
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
		m_commandQueue.Get(),		// Swap chain needs the queue so that it can force a flush on it.
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
		cbvSrvHeapDesc.NumDescriptors = NUM_TEXTURE + FrameCount * CUBECOUNT;
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

void Game::LoadAssets() {
	ComPtr<ID3D12Resource> vertexBufferUploadHeap;
	ComPtr<ID3D12Resource> indexBufferUploadHeap;
	ComPtr<ID3D12Resource> textureBufferUploadHeap[NUM_TEXTURE];

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
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, NUM_TEXTURE, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		CD3DX12_ROOT_PARAMETER1 rootParameters[3];
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_VERTEX);
		rootParameters[2].InitAsConstants(6, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);

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

		/*CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		*/
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
		h = D3DCompileFromFile(L"VS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "Main", "vs_5_1", compileFlags, 0, &vertexShader, nullptr);
		if (errorMessages != nullptr)
		{
			OutputDebugStringA((char*)errorMessages->GetBufferPointer());
		}

		h = D3DCompileFromFile(L"PS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "Main", "ps_5_1", compileFlags | D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES, 0, &pixelShader, &errorMessages);
		if (errorMessages != nullptr)
		{
			OutputDebugStringA((char*)errorMessages->GetBufferPointer());
		}

		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{ "FACEINDEX", 0, DXGI_FORMAT_R32_UINT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
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

		Vertex vList[] =
		{
			//front face
			{ -0.5f,  0.5f, -0.5f, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f, 0 },
			{  0.5f, -0.5f, -0.5f, 1.0f, 1.0f,  0.0f,  0.0f, -1.0f, 0 },
			{ -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  0.0f,  0.0f, -1.0f, 0 },
			{  0.5f,  0.5f, -0.5f, 1.0f, 0.0f,  0.0f,  0.0f, -1.0f, 0 },

			//right side face
			{  0.5f,  0.5f, -0.5f, 0.0f, 0.0f,  1.0f,  0.0f,  0.0f, 1 },
			{  0.5f, -0.5f,  0.5f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f, 1 },
			{  0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  1.0f,  0.0f,  0.0f, 1 },
			{  0.5f,  0.5f,  0.5f, 1.0f, 0.0f,  1.0f,  0.0f,  0.0f, 1 },

			//left side face
			{ -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, -1.0f,  0.0f,  0.0f, 2 },
			{ -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, -1.0f,  0.0f,  0.0f, 2 },
			{ -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f, 2 },
			{ -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, -1.0f,  0.0f,  0.0f, 2 },

			//back face
			{  0.5f,  0.5f,  0.5f, 0.0f, 0.0f,  0.0f,  0.0f,  1.0f, 3 },
			{ -0.5f, -0.5f,  0.5f, 1.0f, 1.0f,  0.0f,  0.0f,  1.0f, 3 },
			{  0.5f, -0.5f,  0.5f, 0.0f, 1.0f,  0.0f,  0.0f,  1.0f, 3 },
			{ -0.5f,  0.5f,  0.5f, 1.0f, 0.0f,  0.0f,  0.0f,  1.0f, 3 },
			//top face
			{ -0.5f,  0.5f,  0.5f, 0.0f, 0.0f,  0.0f,  1.0f,  0.0f, 4 },
			{  0.5f,  0.5f, -0.5f, 1.0f, 1.0f,  0.0f,  1.0f,  0.0f, 4 },
			{ -0.5f,  0.5f, -0.5f, 0.0f, 1.0f,  0.0f,  1.0f,  0.0f, 4 },
			{  0.5f,  0.5f,  0.5f, 1.0f, 0.0f,  0.0f,  1.0f,  0.0f, 4 },

			//bottom face
			{ -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,  0.0f, -1.0f,  0.0f, 5 },
			{  0.5f, -0.5f,  0.5f, 1.0f, 1.0f,  0.0f, -1.0f,  0.0f, 5 },
			{ -0.5f, -0.5f,  0.5f, 0.0f, 1.0f,  0.0f, -1.0f,  0.0f, 5 },
			{  0.5f, -0.5f, -0.5f, 1.0f, 0.0f,  0.0f, -1.0f,  0.0f, 5 },

		};

		DWORD iList[] = {
			//front face
			0, 1, 2, // first triangle
			0, 3, 1, // second triangle

			//left face
			4, 5, 6, // first triangle
			4, 7, 5, // second triangle

			//right face
			8, 9, 10, // first triangle
			8, 11, 9, // second triangle

			//back face
			12, 13, 14, // first triangle
			12, 15, 13, // second triangle

			//top face
			16, 17, 18, // first triangle
			16, 19, 17, // second triangle

			//bottom face
			20, 21, 22, // first triangle
			20, 23, 21, // second triangle
		};

		UINT vSize = sizeof(vList);
		UINT iSize = sizeof(iList);

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
		vertexData.pData = reinterpret_cast<UINT8*>(vList);
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
		indexData.pData = reinterpret_cast<UINT8*>(iList);
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
		// Describe and create a Texture2D.
		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.MipLevels = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		UINT64 uploadBufferSize = 0;
		std::vector<UINT64> defaultBufferSize;
		for (int i = 0; i < NUM_TEXTURE; i++) {
			textureDesc.Width = m_textures[i].width;
			textureDesc.Height = m_textures[i].height;

			ThrowIfFailed(m_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&textureDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&m_textureBuffers[i])));

			UINT64 s = GetRequiredIntermediateSize(m_textureBuffers[i].Get(), 0, 1);

			defaultBufferSize.push_back(s);
			uploadBufferSize += s;
		}


		// Copy data to the intermediate upload heap and then schedule a copy 
		// from the upload heap to the Texture2D.
		for (int i = 0; i < NUM_TEXTURE; i++) {
			// Create the GPU upload buffer.
			ThrowIfFailed(m_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&textureBufferUploadHeap[i])));

			D3D12_SUBRESOURCE_DATA textureData = {};
			textureData.pData = m_textures[i].data.data();
			textureData.RowPitch = m_textures[i].width * 4;
			textureData.SlicePitch = textureData.RowPitch * m_textures[i].height;

			UpdateSubresources(m_commandList.Get(), m_textureBuffers[i].Get(), textureBufferUploadHeap[i].Get(), 0, 0, 1, &textureData);
			m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_textureBuffers[i].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		}


		// Describe and create a SRV for the texture.
		CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart());
		for (int i = 0; i < NUM_TEXTURE; i++) {
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
		CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart(), NUM_TEXTURE, m_cbuDescSize);	// Move past the SRVs.

		UINT cbCountInBufferStep = 1024 * 64 / sizeof(cbObject);
		UINT bufferCount = (CUBECOUNT + cbCountInBufferStep - 1) / cbCountInBufferStep;
		UINT64 bufferSize = 1024 * 64 * bufferCount;

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

			UINT64 offset = 0;
			for (UINT n = 0; n < CUBECOUNT; n++) {
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				cbvDesc.BufferLocation = m_cbvBuffers[i]->GetGPUVirtualAddress() + offset;
				cbvDesc.SizeInBytes = sizeof(cbObject);
				m_device->CreateConstantBufferView(&cbvDesc, cbvHandle);

				offset += cbvDesc.SizeInBytes;
				cbvHandle.Offset(1, m_cbuDescSize);
			}
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

void Game::InitBundles() {
	// Record bundle.
	ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvHeap.Get() };
	CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart());
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle(m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart(), NUM_TEXTURE + m_frameIndex * CUBECOUNT, m_cbuDescSize);

	for (UINT8 i = 0; i < FrameCount; i++) {
		m_bundles[i]->SetGraphicsRootSignature(m_rootSignature.Get());
		m_bundles[i]->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		m_bundles[i]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_bundles[i]->SetGraphicsRootDescriptorTable(0, srvHandle);
		m_bundles[i]->SetGraphicsRootDescriptorTable(1, cbvHandle);
		m_bundles[i]->IASetVertexBuffers(0, 1, &m_vertexBufferView);
		m_bundles[i]->IASetIndexBuffer(&m_indexBufferView);

		for (UINT n = 0; n < cubes.size(); n++) {
			m_bundles[i]->SetGraphicsRootDescriptorTable(1, cbvHandle);
			m_bundles[i]->SetGraphicsRoot32BitConstants(2, 6, cubes[n].faceIndex, 0);
			m_bundles[i]->DrawIndexedInstanced(36, 1, 0, 0, 0);

			cbvHandle.Offset(1, m_cbuDescSize);
		}
		ThrowIfFailed(m_bundles[i]->Close());
	}
}

void Game::PopulateCommandList()
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

void Game::WaitForGpu()
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

	// Wait until the fence has been processed.
	ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	m_fenceValues[m_frameIndex]++;
}

void Game::MoveToNextFrame()
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

void Game::InitMatrix()
{
	/*XMMATRIX tmpMat = XMMatrixPerspectiveFovLH(45.0f*(3.14f / 180.0f), (float)m_width / (float)m_height, 0.1f, 1000.0f);
	XMStoreFloat4x4(&projMat, tmpMat);

	cameraPos = XMVectorSet(2.0f, 1.0f, -3.0f, 0.0f);
	cameraTar = XMVectorZero();
	cameraUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	tmpMat = XMMatrixLookAtLH(cameraPos, cameraTar, cameraUp);
	XMStoreFloat4x4(&viewMat, tmpMat);

	cubePos = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	tmpMat = XMMatrixTranslationFromVector(cubePos);
	XMStoreFloat4x4(&worldMat, tmpMat);

	XMStoreFloat4x4(&rotMat, XMMatrixIdentity());*/
}

void Game::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_camera.OnMouseDown(btnState, x, y);
}

void Game::OnMouseUp(WPARAM btnState, int x, int y)
{
	m_camera.OnMouseUp(btnState, x, y);
}

void Game::OnMouseMove(WPARAM btnState, int x, int y)
{
	m_camera.OnMouseMove(btnState, x, y);
}

void Game::OnMWheelRotate(WPARAM btnState)
{
	m_camera.OnMouseWheelRotate(btnState);
}

void Game::LoadFiles() {
	std::ifstream f;
	f.open("originalName.txt");

	std::string line;
	while (std::getline(f, line)) {
		m_pngFileName.push_back(line);
	}

	f.close();
}

void Game::LoadTextures()
{
	for (int i = 0; i < m_pngFileName.size(); i++) {
		Texture t;
		lodepng::decode(t.data, t.width, t.height, m_pngFileName[i]);
		t.name = m_pngFileName[i];
		t.ID = i;

		//if(t.data.size() == 16384)
		m_textures.push_back(t);
	}
}

void Game::OnUpdate() {
	m_frameCounter++;
	m_timer.Tick();

	if (m_frameCounter == 500) {
		double t = m_timer.GetElapseTime();
		double delta = t - m_intermediateTime;

		wchar_t fps[64];
		swprintf_s(fps, L"%f fps", 500 / delta);
		std::wstring windowText = m_title + L": " + fps;
		SetWindowText(m_hwnd, windowText.c_str());

		m_frameCounter = 0;
		m_intermediateTime = t;
	}
	//XMStoreFloat4(&constantObject.ambientColor, XMVectorSet(0.0f, 1.0f, 1.0f, 1.0f));
	//constantObject.ambientIntensity = 0.2f;

	for (UINT n = 0; n < cubes.size(); n++) {
		XMStoreFloat4x4(&constantObject.wvpMat, m_camera.GetObjTransWVPMat(cubes[n].cubePosition));
		memcpy(cbvGPUAddress[m_frameIndex] + sizeof(constantObject) * n, &constantObject, sizeof(constantObject));
	}
}

void Game::OnRender() {
	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(m_swapChain->Present(1, 0));

	MoveToNextFrame();
}

void Game::OnDestroy() {
	WaitForGpu();

	CloseHandle(m_fenceEvent);
}

void Game::OnKeyUp(UINT8 key) {
	m_camera.OnKeyUp(key);
}

void Game::OnKeyDown(UINT8 key) {
	m_camera.OnKeyDown(key);
	switch (key) {
	case 'N':
		indexTexture++;
		break;
	case 'L':
		indexTexture--;
		break;
	}
}
