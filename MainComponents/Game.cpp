#include "Game.h"
#include "Graphics.h"
#include "../Structures/Vertex.h"
#include "Input.h"
#include "../PathHelpers.h"
#include "Window.h"
#include <DirectXMath.h>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// The constructor is called after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
Game::Game()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	CreateRootSigAndPipelineState();
	CreateGeometry();
	Initialize();	
}

// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// Wait for the GPU before we shut down
	Graphics::WaitForGPU();
}

// --------------------------------------------------------
// Loads the two basic shaders, then creates the root signature
// and pipeline state object for our very basic demo.
// --------------------------------------------------------
void Game::CreateRootSigAndPipelineState()
{
	// Blobs to hold raw shader byte code used in several steps below
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderByteCode;

	// Load shaders
	{
		// Read our compiled vertex shader code into a blob
		// - Essentially just "open the file and plop its contents here"
		D3DReadFileToBlob(
			FixPath(L"VertexShader.cso").c_str(), vertexShaderByteCode.GetAddressOf());
		D3DReadFileToBlob(
			FixPath(L"PixelShader.cso").c_str(), pixelShaderByteCode.GetAddressOf());
	}

	// Input layout
	const unsigned int inputElementCount = 4;
	D3D12_INPUT_ELEMENT_DESC inputElements[inputElementCount] = {};
	{
		inputElements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3
		inputElements[0].SemanticName = "POSITION"; // Name must match semantic in shader
		inputElements[0].SemanticIndex = 0; // This is the first POSITION semantic

		inputElements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[1].Format = DXGI_FORMAT_R32G32_FLOAT; // R32 G32 = float2
		inputElements[1].SemanticName = "TEXCOORD";
		inputElements[1].SemanticIndex = 0; // This is the first TEXCOORD semantic

		inputElements[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[2].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3
		inputElements[2].SemanticName = "NORMAL";
		inputElements[2].SemanticIndex = 0; // This is the first NORMAL semantic

		inputElements[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[3].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3
		inputElements[3].SemanticName = "TANGENT";
		inputElements[3].SemanticIndex = 0; // This is the first TANGENT semantic
	}

	// Root Signature
	{
		// Setup root parameter
		// Define a table of CBV's (constant buffer views)
		D3D12_DESCRIPTOR_RANGE cbvTable = {};
		cbvTable.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvTable.NumDescriptors = 1;
		cbvTable.BaseShaderRegister = 0;
		cbvTable.RegisterSpace = 0;
		cbvTable.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_DESCRIPTOR_RANGE cbvTableP = {};
		cbvTableP.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvTableP.NumDescriptors = 1;
		cbvTableP.BaseShaderRegister = 0;
		cbvTableP.RegisterSpace = 0;
		cbvTableP.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_DESCRIPTOR_RANGE bindlessRange{};
		bindlessRange.BaseShaderRegister = 0; // Matches t0 in shader
		bindlessRange.RegisterSpace = 0; // Matches space0 in shader
		bindlessRange.NumDescriptors = -1; // All (or Graphics::MaxTextureDescriptors)
		bindlessRange.OffsetInDescriptorsFromTableStart = 0;
		bindlessRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

		// Create a single static sampler (available to all pixel shaders)
		D3D12_STATIC_SAMPLER_DESC anisoWrap = {};
		anisoWrap.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.Filter = D3D12_FILTER_ANISOTROPIC;
		anisoWrap.MaxAnisotropy = 16;
		anisoWrap.MaxLOD = D3D12_FLOAT32_MAX;
		anisoWrap.ShaderRegister = 0; // Means register(s0) in the shader
		anisoWrap.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		D3D12_STATIC_SAMPLER_DESC samplers[] = { anisoWrap };

		// Define the root parameter
		D3D12_ROOT_PARAMETER rootParams[3];
		rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[0].DescriptorTable.pDescriptorRanges = &cbvTable;

		rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[1].DescriptorTable.pDescriptorRanges = &cbvTableP;

		rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParams[2].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[2].DescriptorTable.pDescriptorRanges = &bindlessRange;

		// Describe the overall the root signature
		D3D12_ROOT_SIGNATURE_DESC rootSig = {};
		rootSig.Flags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		rootSig.NumParameters = ARRAYSIZE(rootParams);
		rootSig.pParameters = rootParams;
		rootSig.NumStaticSamplers = ARRAYSIZE(samplers);
		rootSig.pStaticSamplers = samplers;
		ID3DBlob* serializedRootSig = 0;
		ID3DBlob* errors = 0;

		D3D12SerializeRootSignature(
			&rootSig,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&serializedRootSig,
			&errors);

		// Check for errors during serialization
		if (errors != 0)
		{
			OutputDebugString((wchar_t*)errors->GetBufferPointer());
		}

		// Actually create the root sig
		Graphics::Device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(rootSignature.GetAddressOf()));
	}

	// Pipeline state
	{
		// Describe the pipeline state
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

		// -- Input assembler related ---
		psoDesc.InputLayout.NumElements = inputElementCount;
		psoDesc.InputLayout.pInputElementDescs = inputElements;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		// Overall primitive topology type (triangle, line, etc.) is set here
		// IASetPrimTop() is still used to set list/strip/adj options
		// Root sig
		psoDesc.pRootSignature = rootSignature.Get();

		// -- Shaders (VS/PS) ---
		psoDesc.VS.pShaderBytecode = vertexShaderByteCode->GetBufferPointer();
		psoDesc.VS.BytecodeLength = vertexShaderByteCode->GetBufferSize();
		psoDesc.PS.pShaderBytecode = pixelShaderByteCode->GetBufferPointer();
		psoDesc.PS.BytecodeLength = pixelShaderByteCode->GetBufferSize();

		// -- Render targets ---
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;

		// -- States ---
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		psoDesc.RasterizerState.DepthClipEnable = true;

		psoDesc.DepthStencilState.DepthEnable = true;
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

		psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		// -- Misc ---
		psoDesc.SampleMask = 0xffffffff;

		// Create the pipe state object
		Graphics::Device->CreateGraphicsPipelineState(
			&psoDesc,
			IID_PPV_ARGS(pipelineState.GetAddressOf()));
	}

	// Set up the viewport and scissor rectangle
	{
		// Set up the viewport so we render into the correct
		// portion of the render target
		viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (float)Window::Width();
		viewport.Height = (float)Window::Height();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		// Define a scissor rectangle that defines a portion of
		// the render target for clipping. This is different from
		// a viewport in that it is applied after the pixel shader.
		// 
		// We need at least one of these, but we're rendering to
		// the entire window, so it'll be the same size.
		scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = Window::Width();
		scissorRect.bottom = Window::Height();
	}
}

void Game::CreateGeometry()
{
	std::wstring AssetPath = L"../../Assets/";

	// Load textures
	unsigned int cobblestoneAlbedo = Graphics::LoadTexture(FixPath(AssetPath + L"Textures/PBR/cobblestone_albedo.png").c_str());
	unsigned int cobblestoneNormals = Graphics::LoadTexture(FixPath(AssetPath + L"Textures/PBR/cobblestone_normals.png").c_str());
	unsigned int cobblestoneRoughness = Graphics::LoadTexture(FixPath(AssetPath + L"Textures/PBR/cobblestone_roughness.png").c_str());
	unsigned int cobblestoneMetal = Graphics::LoadTexture(FixPath(AssetPath + L"Textures/PBR/cobblestone_metal.png").c_str());

	std::unordered_map<TextureID, unsigned int> cobbleMap = {
	{TextureID::ALBEDO, cobblestoneAlbedo },
	{TextureID::NORMAL_MAP, cobblestoneAlbedo },
	{TextureID::METALNESS, cobblestoneRoughness },
	{TextureID::ROUGHNESS, cobblestoneMetal },
	};

	unsigned int bronzeAlbedo = Graphics::LoadTexture(FixPath(AssetPath + L"Textures/PBR/bronze_albedo.png").c_str());
	unsigned int bronzeNormals = Graphics::LoadTexture(FixPath(AssetPath + L"Textures/PBR/bronze_normals.png").c_str());
	unsigned int bronzeRoughness = Graphics::LoadTexture(FixPath(AssetPath + L"Textures/PBR/bronze_roughness.png").c_str());
	unsigned int bronzeMetal = Graphics::LoadTexture(FixPath(AssetPath + L"Textures/PBR/bronze_metal.png").c_str());

	std::unordered_map<TextureID, unsigned int> bronzeMap = {
	{TextureID::ALBEDO, bronzeAlbedo },
	{TextureID::NORMAL_MAP, bronzeNormals },
	{TextureID::METALNESS, bronzeRoughness },
	{TextureID::ROUGHNESS, bronzeMetal },
	};

	unsigned int scratchedAlbedo = Graphics::LoadTexture(FixPath(AssetPath + L"Textures/PBR/scratched_albedo.png").c_str());
	unsigned int scratchedNormals = Graphics::LoadTexture(FixPath(AssetPath + L"Textures/PBR/scratched_normals.png").c_str());
	unsigned int scratchedRoughness = Graphics::LoadTexture(FixPath(AssetPath + L"Textures/PBR/scratched_roughness.png").c_str());
	unsigned int scratchedMetal = Graphics::LoadTexture(FixPath(AssetPath + L"Textures/PBR/scratched_metal.png").c_str());

	std::unordered_map<TextureID, unsigned int> scratchedMap = {
	{TextureID::ALBEDO, scratchedAlbedo },
	{TextureID::NORMAL_MAP, scratchedNormals },
	{TextureID::METALNESS, scratchedRoughness },
	{TextureID::ROUGHNESS, scratchedMetal },
	};

	materials.push_back(std::make_shared<Material>("cobble", cobbleMap, pipelineState, XMFLOAT4(1, 1, 1, 1)));
	materials.push_back(std::make_shared<Material>("bronze", bronzeMap, pipelineState, XMFLOAT4(1, 1, 1, 1)));
	materials.push_back(std::make_shared<Material>("scratched", scratchedMap, pipelineState, XMFLOAT4(1, 1, 1, 1)));

	drawables.push_back(std::make_shared<Mesh>("Torus", FixPath("../../Assets/Meshes/torus.obj").c_str()));
	drawables.push_back(std::make_shared<Mesh>("Cube", FixPath("../../Assets/Meshes/crate_wood.obj").c_str()));
	drawables.push_back(std::make_shared<Mesh>("Helix", FixPath("../../Assets/Meshes/helix.obj").c_str()));

}

// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::Initialize()
{
	camera = std::make_shared<FPSCamera>("MainCamera", XMFLOAT3(0, 0, -5.0f), 5.0f, .002f, 80, Window::AspectRatio(), 0.01f, 1000.0f);

	lights.push_back(std::make_shared<Light>("Directional Light", true, true, XMFLOAT3(1, 1, 1), XMFLOAT3(-1, -1, 1), 2));
	lights.push_back(std::make_shared<Light>("Directional Light", true, true, XMFLOAT3(0, 0, 2), XMFLOAT3(-1, -1, 1), 2, 10));

	gameObjs.push_back(std::make_shared<GameObject>(GameObject("Torus", drawables[0], nullptr, materials[0])));
	gameObjs.push_back(std::make_shared<GameObject>(GameObject("Cube", drawables[1], nullptr, materials[1])));
	gameObjs[1]->GetTransform()->SetPosition(-3, 0, 0);
	gameObjs.push_back(std::make_shared<GameObject>(GameObject("Helix", drawables[2], nullptr, materials[2])));
	gameObjs[2]->GetTransform()->SetPosition(3, 0, 0);
}

// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Resize the viewport and scissor rectangle
	{
		// Set up the viewport so we render into the correct
		// portion of the render target
		viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (float)Window::Width();
		viewport.Height = (float)Window::Height();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		// Define a scissor rectangle that defines a portion of
		// the render target for clipping. This is different from
		// a viewport in that it is applied after the pixel shader.
		// We need at least one of these, but we're rendering to
		// the entire window, so it'll be the same size.
		scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = Window::Width();
		scissorRect.bottom = Window::Height();
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	camera->Update(deltaTime);

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Grab the current back buffer for this frame
	Microsoft::WRL::ComPtr<ID3D12Resource> currentBackBuffer =
		Graphics::BackBuffers[Graphics::SwapChainIndex()];

	// Clearing the render target
	{
		// Transition the back buffer from present to render target
		D3D12_RESOURCE_BARRIER rb = {};

		rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		rb.Transition.pResource = currentBackBuffer.Get();
		rb.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		rb.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		Graphics::CommandList->ResourceBarrier(1, &rb);

		// Background color (Cornflower Blue in this case) for clearing
		float color[] = { 0.4f, 0.6f, 0.75f, 1.0f };

		// Clear the RTV
		Graphics::CommandList->ClearRenderTargetView(
			Graphics::RTVHandles[Graphics::SwapChainIndex()],
			color,
			0, 0); // No scissor rectangles

		// Clear the depth buffer, too
		Graphics::CommandList->ClearDepthStencilView(
			Graphics::DSVHandle,
			D3D12_CLEAR_FLAG_DEPTH,
			1.0f, // Max depth = 1.0f
			0, // Not clearing stencil, but need a value
			0, 0); // No scissor rects
	}

	// Rendering here!
	{
		// Optimization: Have less pipeline state changes by grouping draws with same state
		// Set overall pipeline state
		Graphics::CommandList->SetPipelineState(pipelineState.Get());

		// Set up other commands for rendering
		Graphics::CommandList->SetDescriptorHeaps(1, Graphics::CBVSRVDescriptorHeap.GetAddressOf());
		// Root sig (must happen before root descriptor table)
		Graphics::CommandList->SetGraphicsRootSignature(rootSignature.Get());

		// Bind a specific part of it to the "bindless range" of our root signature
		// Step 1: Grab the handle of the beginning
		D3D12_GPU_DESCRIPTOR_HANDLE handleToSRVs =
			Graphics::CBVSRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

		// Step 2: Offset to the first SRV
		unsigned int incSize = Graphics::Device->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		handleToSRVs.ptr += Graphics::MaxConstantBuffers * incSize;

		// Actually bind
		Graphics::CommandList->SetGraphicsRootDescriptorTable(2, handleToSRVs);

		Graphics::CommandList->OMSetRenderTargets(
			1, &Graphics::RTVHandles[Graphics::SwapChainIndex()], true, &Graphics::DSVHandle);

		Graphics::CommandList->RSSetViewports(1, &viewport);
		Graphics::CommandList->RSSetScissorRects(1, &scissorRect);

		Graphics::CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		XMFLOAT3 fwd = camera->GetTransform()->GetForward();
		//printf("Camera Fwd: %f %f %f\n", fwd.x, fwd.y, fwd.z);

		ExternalData camData{};
		camData.viewMatrix = camera->GetView();
		camData.projMatrix = camera->GetProjection();

		std::vector<LightStruct> lightStructs;

		for (auto& light : lights) {
			lightStructs.push_back(light->GetStruct());
		}

		for (auto& g : gameObjs)
		{
			camData.worldMatrix = g->GetTransform()->GetWorldMatrix();

			D3D12_GPU_DESCRIPTOR_HANDLE handle = Graphics::FillNextConstBufAndGetGPUDescHan((void*)(&camData), sizeof(ExternalData));
			Graphics::CommandList->SetGraphicsRootDescriptorTable(0, handle);
			
			Graphics::CommandList->SetPipelineState(g->GetMaterial()->GetPipelineState().Get());

			PixelData psData = g->GetMaterial()->GetPixelData();
			psData.cameraPosition = camera->GetTransform()->GetPosition();
			psData.lightCount = lights.size();
			memcpy(psData.lights,
				lightStructs.data(),
				sizeof(LightStruct) * 2);

			handle = Graphics::FillNextConstBufAndGetGPUDescHan((void*)(&psData), sizeof(PixelData));

			Graphics::CommandList->SetGraphicsRootDescriptorTable(1, handle);

			g->GetDrawable()->Draw();
		}
	}

	// Present
	{
		// Transition back to present
		D3D12_RESOURCE_BARRIER rb = {};

		rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		rb.Transition.pResource = currentBackBuffer.Get();
		rb.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		rb.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		Graphics::CommandList->ResourceBarrier(1, &rb);

		// Must occur BEFORE present
		Graphics::CloseAndExecuteCommandList();

		// Present the current back buffer and move to the next one
		bool vsync = Graphics::VsyncState();

		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		Graphics::AdvanceSwapChainIndex();

		Graphics::ResetAllocatorAndCommandList(Graphics::SwapChainIndex());
	}
}



