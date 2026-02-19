#include "Lighting.hlsli"

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
//struct VertexToPixel
//{
//    float4 screenPosition : SV_POSITION;
//    float2 uv : TEXCOORD;
//    float3 normal : NORMAL;
//    float3 tangent : TANGENT;
//    float3 worldPosition : POSITION;
//    float4 shadowMapPos : SHADOW_POSITION;
//};

cbuffer PixelData : register(b0)
{
    uint albedo;
    uint normalMap;
    uint metalness;
    uint roughness;
    uint emission;
    uint parallax;
    float2 uvScale;
    float2 uvOffset;
    float3 cameraPosition;
    //int lightCount;
    
};

SamplerState BasicSampler : register(s0);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    Texture2D AlbedoTexture = ResourceDescriptorHeap[albedo];
    //Texture2D NormalMap = ResourceDescriptorHeap[normalMap];
    //Texture2D RoughnessMap = ResourceDescriptorHeap[roughness];
    //Texture2D MetalMap = ResourceDescriptorHeap[metalness];
    
    //float shadowAmount = 0; //ShadowAmount(input);

    //input.normal = normalize(input.normal);
    //input.tangent = normalize(input.tangent);
    //input.uv = (input.uv * uvScale) + uvOffset;

    //float3 totalLight = float3(1, 1, 1);
	
    float3 color = AlbedoTexture.Sample(BasicSampler, input.uv).xyz /** iTint.xyz*/;
    //color = pow(color, 2.2f);
    
    //float3 ambientTerm = /*iAmbientColor * */color;
	
    //float3 normalFromMap = normalize(NormalMap.Sample(BasicSampler, input.uv) * 2 - 1).xyz;
    
    //// calc matrix
    //float3 N = input.normal;
    //float3 T = normalize(input.tangent - N * dot(N, input.tangent));
    //float3 B = cross(T, N);
    
    //float3x3 TBN = float3x3(T, B, N);
    //input.normal = mul(normalFromMap, TBN);
    
    //totalLight += ambientTerm;
    
    //for (int i = 0; i < MAX_LIGHTS; i++)
    //{
    //    if (lights[i].Intensity < 0)
    //        continue;
        
    //    switch (lights[i].Type)
    //    {
    //        case LIGHT_TYPE_DIRECTIONAL:
    //            totalLight += DirectionalLight(lights[i], color, BasicSampler, RoughnessMap, input, cameraPosition) * (i == 0 ? shadowAmount : 1);
    //            break;
            
    //        case LIGHT_TYPE_POINT:
    //            totalLight += PointLight(lights[i], color, BasicSampler, RoughnessMap, input, cameraPosition);
    //            break;
            
    //        case LIGHT_TYPE_SPOT:
    //            totalLight += SpotLight(lights[i], color, BasicSampler, RoughnessMap, input, cameraPosition);
    //            break;
    //    }
    //}
	
    //float3 gammaAdjustedColor = pow(totalLight, 1.0f / 2.2f);
    
    return float4(color, 1);
}