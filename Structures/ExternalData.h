#pragma once
#include <DirectXMath.h>

struct ExternalData
{
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 invWorldMatrix;
};

struct SkyboxData {
	unsigned int skyboxIndex;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;
};