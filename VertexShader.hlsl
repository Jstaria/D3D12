
struct VertexShaderInput
{
    float3 localPosition : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 worldPosition : POSITION;
   // float4 shadowMapPos : SHADOW_POSITION;
};

cbuffer camData : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float4x4 world;
    float4x4 worldInverseTranspose;
}


// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main(VertexShaderInput input)
{
	// Set up output struct
    VertexToPixel output;

	// Calc screen position
    matrix wvp = mul(projection, mul(view, world));
    output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));

	// Make sure the lighting vectors are in world space
    output.normal = normalize(mul((float3x3) worldInverseTranspose, input.normal));
    output.tangent = normalize(mul((float3x3) world, input.tangent));

	// Calc vertex world pos
    output.worldPosition = mul(world, float4(input.localPosition, 1.0f)).xyz;

	// Pass through the uv
    output.uv = input.uv;

    return output;
}