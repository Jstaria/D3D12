#include "Sky.h"
#include <d3dcompiler.h>

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

void Sky::CreateStates()
{
	D3D12_DESCRIPTOR_RANGE cbvTable = {};
	cbvTable.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	cbvTable.NumDescriptors = 1;
	cbvTable.BaseShaderRegister = 1;
	cbvTable.RegisterSpace = 0;
	cbvTable.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// Define the root parameter
	D3D12_ROOT_PARAMETER rootParams[1];
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParams[0].DescriptorTable.pDescriptorRanges = &cbvTable;

	D3D12_STATIC_SAMPLER_DESC anisoWrap = {};
	anisoWrap.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	anisoWrap.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	anisoWrap.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	anisoWrap.Filter = D3D12_FILTER_ANISOTROPIC;
	anisoWrap.MaxAnisotropy = 16;
	anisoWrap.MaxLOD = D3D12_FLOAT32_MAX;
	anisoWrap.ShaderRegister = 0;  
	anisoWrap.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC samplers[] = { anisoWrap };

	D3D12_ROOT_SIGNATURE_DESC rootSig{};

	rootSig.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
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

	if (errors != 0)
	{
		OutputDebugString((wchar_t*)errors->GetBufferPointer());
	}

	Graphics::Device->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(this->rootSig.GetAddressOf()));

	// PSO
	Microsoft::WRL::ComPtr<ID3DBlob> vsByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob> psByteCode;
	D3DReadFileToBlob(FixPath(L"SkyVS.cso").c_str(), vsByteCode.GetAddressOf());
	D3DReadFileToBlob(FixPath(L"SkyPS.cso").c_str(), psByteCode.GetAddressOf());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	psoDesc.pRootSignature = this->rootSig.Get();

	psoDesc.VS.pShaderBytecode = vsByteCode->GetBufferPointer();
	psoDesc.VS.BytecodeLength = vsByteCode->GetBufferSize();
	psoDesc.PS.pShaderBytecode = psByteCode->GetBufferPointer();
	psoDesc.PS.BytecodeLength = psByteCode->GetBufferSize();

	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;

	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT; 
	psoDesc.RasterizerState.DepthClipEnable = true;
	psoDesc.DepthStencilState.DepthEnable = true;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

	psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoDesc.SampleMask = 0xffffffff;

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

	psoDesc.InputLayout = { inputElements, inputElementCount };

	Graphics::Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(PS.GetAddressOf()));
}

Sky::Sky(std::shared_ptr<Mesh> mesh, unsigned int skyboxIndex)
	: mesh(mesh), skyboxIndex(skyboxIndex)
{
	CreateStates();
}

Sky::Sky()
{
}

unsigned int Sky::GetIndex() { return skyboxIndex; }

void Sky::Draw(std::shared_ptr<Camera> cam)
{
	// Set pipeline stuff (assuming we're using the heap from Game)
	Graphics::CommandList->SetPipelineState(PS.Get());
	Graphics::CommandList->SetGraphicsRootSignature(rootSig.Get());
	//Graphics::CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	SkyboxData skyData{};
	skyData.skyboxIndex = skyboxIndex;

	{
		skyData.viewMatrix = cam->GetView();
		skyData.projMatrix = cam->GetProjection();

		D3D12_GPU_DESCRIPTOR_HANDLE handle = Graphics::FillNextConstBufAndGetGPUDescHan((void*)(&skyData), sizeof(SkyboxData));
		
		Graphics::CommandList->SetGraphicsRootDescriptorTable(0, handle);
	}

	mesh->Draw();
}

