
#include "Material.h"

using namespace Microsoft::WRL;

Material::Material(const char* name, std::unordered_map<TextureID, unsigned int> textures, Microsoft::WRL::ComPtr<ID3D12PipelineState> PS, DirectX::XMFLOAT4 color)
	: name(name), PS(PS), textures(textures), colorTint(color), uvScale(1, 1), uvOffset(0, 0), ambientTint(0.05f, 0.05f, 0.05f)
{
	materialIndex = 0;
}

void Material::SetDefaultShaderParam(ExternalData data, Transform* transform, Transform* camTransform)
{

}

DirectX::XMFLOAT4 Material::GetTintColor() { return colorTint; }
DirectX::XMFLOAT3 Material::GetAmbientColor() { return ambientTint; }
const char* Material::GetName() { return name; }

unsigned int Material::GetTextureID(TextureID id) { return textures[id]; }
unsigned int Material::GetMatIndex() { return materialIndex; }

Microsoft::WRL::ComPtr<ID3D12PipelineState> Material::GetPipelineState() { return PS; }

PixelData Material::GetPixelData()
{
	PixelData pData{};
	pData.albedo = textures[ALBEDO];
	pData.normalMap = textures[NORMAL_MAP];
	pData.metalness = textures[METALNESS];
	pData.roughness = textures[ROUGHNESS];

	return pData;
}

void Material::AddTextureSRV(TextureID textureID, unsigned int id) { textures.insert({ textureID, id }); }
void Material::SetAmbientTint(DirectX::XMFLOAT3 ambient) { this->ambientTint = ambient; }
void Material::SetTint(DirectX::XMFLOAT4 tint) { this->colorTint = tint; }
void Material::SetIndex() { /*materialIndex = GlobalVar::Material::getIndexThenTick();*/ }
void Material::SetUVScale(DirectX::XMFLOAT2 scale) { this->uvScale = scale; }
void Material::SetUVOffset(DirectX::XMFLOAT2 offset) { this->uvOffset = offset; }

//void Material::ImGuiDraw()
//{
//	ImGui::DragFloat2("UV Offset", &offset.x, .1f, -1, 1);
//	ImGui::DragFloat2("UV Scale", &scale.x, .1f, 0, 5);
//	ImGui::ColorEdit4("Tint Color", &colorTint.x);
//
//	for (auto& image : textures) { ImGui::Image(reinterpret_cast<ImTextureID>(image.second.Get()), { 100,100 }); }
//}
