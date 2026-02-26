#pragma once

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

#define MAX_LIGHTS 64

#include "DirectXMath.h"

struct LightStruct
{
    int Type = 0; 
    DirectX::XMFLOAT3 Direction = DirectX::XMFLOAT3(0,0,0); 

    float Range = 0; 
    DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(0, 0, 0);

    float Intensity = 0;
    DirectX::XMFLOAT3 Color = DirectX::XMFLOAT3(0, 0, 0);

    float SpotFallOff = 0; 
    DirectX::XMFLOAT3 Padding = DirectX::XMFLOAT3(0, 0, 0);
};