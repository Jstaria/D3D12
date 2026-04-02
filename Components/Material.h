#pragma once

#include <memory>
#include <d3d12.h>
#include <wrl/client.h>
#include <unordered_map> 

//#include "../ImGui/imgui.h"
//#include "../Helper/SimpleShader.h"
//#include "../Helper/GlobalVar.h"
#include "../Structures/PixelData.h"
#include "../MainComponents/Window.h"
#include "../Components/Transform.h"
#include "../Structures/ExternalData.h"

enum TextureID {
	ALBEDO,
	NORMAL_MAP,
	METALNESS,
	ROUGHNESS,
	EMISSION,
	PARALLAX
};

class Material
{
private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> PS;
	DirectX::XMFLOAT3 colorTint;
	DirectX::XMFLOAT3 ambientTint;
	const char* name;
	float diffusion;
	float emissive;

	DirectX::XMFLOAT2 uvOffset;
	DirectX::XMFLOAT2 uvScale;

	unsigned int materialIndex;
	unsigned int hitGroupIndex;

	std::unordered_map<TextureID, unsigned int> textures;
	//std::unordered_map<const char*, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;

public:
	Material(const char* name, std::unordered_map<TextureID, unsigned int> textures, Microsoft::WRL::ComPtr<ID3D12PipelineState> PS, DirectX::XMFLOAT3 color, float diffusion, float emissive, unsigned int hitGroupIndex);

	void SetDefaultShaderParam(ExternalData data, Transform* transform, Transform* camTransform);

	// -=| Getters |=-
	DirectX::XMFLOAT3 GetTintColor();
	DirectX::XMFLOAT3 GetAmbientColor();
	const char* GetName();
	unsigned int GetTextureID(TextureID ID);
	unsigned int GetMatIndex();
	float GetDiffusion();
	float GetEmissive();
	unsigned int GetHitGroupindex();
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipelineState();
	PixelData GetPixelData();

	// -=| Setters/Adders |=-
	void AddTextureSRV(TextureID textureID, unsigned int id);
	void SetAmbientTint(DirectX::XMFLOAT3 ambientTint);
	void SetTint(DirectX::XMFLOAT3 tint);
	void SetIndex();
	void SetUVScale(DirectX::XMFLOAT2 scale);
	void SetUVOffset(DirectX::XMFLOAT2 offset);

	// -=| Removers |=-
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> RemoveTextureSRV(const char* name);
	//Microsoft::WRL::ComPtr<ID3D11SamplerState> RemoveSampler(const char* name);

	// -=| Other |=-
	//void ImGuiDraw();
};

