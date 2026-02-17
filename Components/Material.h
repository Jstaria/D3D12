#pragma once

#include <memory>
#include <d3d11.h>
#include <wrl/client.h>
#include <unordered_map> 

//#include "../ImGui/imgui.h"
//#include "../Helper/SimpleShader.h"
//#include "../Helper/GlobalVar.h"
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
	std::shared_ptr<ID3D12PipelineState> PS;
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT3 ambientTint;
	const char* name;

	DirectX::XMFLOAT2 uvOffset;
	DirectX::XMFLOAT2 uvScale;

	unsigned int materialIndex;

	std::unordered_map<TextureID, unsigned int> textures;
	//std::unordered_map<const char*, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;

public:
	Material(const char* name, std::unordered_map<TextureID, unsigned int> textures, DirectX::XMFLOAT4 color);

	void SetDefaultShaderParam(ExternalData data, Transform* transform, Transform* camTransform);

	// -=| Getters |=-
	DirectX::XMFLOAT4 GetTintColor();
	DirectX::XMFLOAT3 GetAmbientColor();
	const char* GetName();
	unsigned int GetTextureID(TextureID ID);
	unsigned int GetMatIndex();

	// -=| Setters/Adders |=-
	void AddTextureSRV(TextureID textureID, unsigned int id);
	void SetAmbientTint(DirectX::XMFLOAT3 ambientTint);
	void SetTint(DirectX::XMFLOAT4 tint);
	void SetIndex();
	void SetUVScale(DirectX::XMFLOAT2 scale);
	void SetUVOffset(DirectX::XMFLOAT2 offset);

	// -=| Removers |=-
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> RemoveTextureSRV(const char* name);
	//Microsoft::WRL::ComPtr<ID3D11SamplerState> RemoveSampler(const char* name);

	// -=| Other |=-
	//void ImGuiDraw();
};

