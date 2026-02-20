#pragma once
#include <DirectXMath.h>
#include "LightStruct.h"

struct PixelData {
	unsigned int albedo;
	unsigned int normalMap;
	unsigned int metalness;
	unsigned int roughness;
	unsigned int emission;
	unsigned int parallax;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;
	DirectX::XMFLOAT3 cameraPosition;
	int lightCount;
	LightStruct lights[64];
};