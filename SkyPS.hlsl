
cbuffer ExternalData : register(b1)
{
    float4x4 viewMatrix;
    float4x4 projMatrix;
    unsigned int skyboxIndex;
}

struct VertexToPixelSky
{
    float4 position : SV_POSITION;
    float3 sampleDir : DIRECTION;
};

SamplerState SkySampler : register(s0);

float4 main(VertexToPixelSky input) : SV_TARGET
{
    TextureCube CubeMap = ResourceDescriptorHeap[skyboxIndex];
    return CubeMap.Sample(SkySampler, input.sampleDir);
}