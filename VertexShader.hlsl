
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
    //float2 uv : TEXCOORD;
    //float3 normal : NORMAL;
    //float3 tangent : TANGENT;
    //float3 worldPosition : POSITION;
    //float4 shadowMapPos : SHADOW_POSITION;
};

struct ExternalData
{
    float4x4 viewMatrix;
    float4x4 projMatrix;
    float4x4 worldMatrix;
    //float4x4 invWorldMatrix;
};

cbuffer camData : register(b0)
{
    ExternalData data;
}

//cbuffer buffer : register(b0)
//{
//    float3 data;
//}

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

    float4x4 wvp = mul(data.projMatrix, mul(data.viewMatrix, data.worldMatrix));
	
    //output.worldPosition = mul(data.worldMatrix, float4(input.localPosition, 1.0f)).xyz;
    output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));
    //output.uv = input.uv;
    //output.normal = normalize(mul((float3x3) data.invWorldMatrix, input.normal));
    //output.tangent = normalize(mul((float3x3) data.invWorldMatrix, input.tangent));
    
    //matrix shadowWVP = mul(lightProjection, mul(lightView, data.worldMatrix));
    //output.shadowMapPos = mul(shadowWVP, float4(input.localPosition, 1.0f));
    
    return output;
}