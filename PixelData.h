#pragma once
#include <DirectXMath.h>
#include "Structures/LightStruct.h"

struct PixelData {
	unsigned int albedo;
	unsigned int normalMap;
	unsigned int metalness;
	unsigned int roughness;

	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;

	int lightCount;
	DirectX::XMFLOAT3 cameraPosition;

	LightStruct lights[64];