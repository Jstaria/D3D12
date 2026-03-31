#define PI 3.1415f
#define RPP 25
#define RECDEP 10

// === Structs ===

// Layout of data in the vertex buffer
struct Vertex
{
    float3 localPosition;
    float2 uv;
    float3 normal;
    float3 tangent;
};


// Payload for rays (data that is "sent along" with each ray during raytrace)
// Note: This should be as small as possible, and must match our C++ size definition
struct RayPayload
{
    float3 color;
    uint RecursionDepth;
    uint RayPerPixelIndex;
};
struct SceneData
{
    matrix InverseViewProjection;
    float3 CameraPosition;
    float pad;
};
struct EntityData
{
    float4 Color;
    uint VertexBufferDescriptorIndex;
    uint IndexBufferDescriptorIndex;
    float diffusion;
    float pad[3];
    
};
// Note: We'll be using the built-in BuiltInTriangleIntersectionAttributes struct
// for triangle attributes, so no need to define our own.  It contains a single float2.



// === Constant buffers ===

// === Resources ===

cbuffer DrawData : register(b0)
{
    uint SceneDataConstantBufferIndex;
    uint EntityDataDescriptorIndex;
    uint SceneTLASDescriptorIndex;
    uint OutputUAVDescriptorIndex;
};


// === Helpers ===

// Barycentric interpolation of data from the triangle's vertices
Vertex InterpolateVertices(uint triangleIndex, float2 barycentrics)
{
	// Get the data for this entity
    StructuredBuffer<EntityData> ed =
        ResourceDescriptorHeap[EntityDataDescriptorIndex];
    EntityData thisEntity = ed[InstanceIndex()];
    
    // Get the geometry buffers
    StructuredBuffer<uint> IndexBuffer = ResourceDescriptorHeap[thisEntity.IndexBufferDescriptorIndex];
    StructuredBuffer<Vertex> VertexBuffer = ResourceDescriptorHeap[thisEntity.VertexBufferDescriptorIndex];
	
	// Grab the 3 indices for this triangle
    uint firstIndex = triangleIndex * 3;
    uint indices[3];
    indices[0] = IndexBuffer[firstIndex + 0];
    indices[1] = IndexBuffer[firstIndex + 1];
    indices[2] = IndexBuffer[firstIndex + 2];

	// Grab the 3 corresponding vertices
    Vertex verts[3];
    verts[0] = VertexBuffer[indices[0]];
    verts[1] = VertexBuffer[indices[1]];
    verts[2] = VertexBuffer[indices[2]];
	
	// Calculate the barycentric data for vertex interpolation
    float3 barycentricData = float3(
		1.0f - barycentrics.x - barycentrics.y,
		barycentrics.x,
		barycentrics.y);
	
	// Loop through the barycentric data and interpolate
    Vertex finalVert = (Vertex) 0;
    for (uint i = 0; i < 3; i++)
    {
        finalVert.localPosition += verts[i].localPosition * barycentricData[i];
        finalVert.uv += verts[i].uv * barycentricData[i];
        finalVert.normal += verts[i].normal * barycentricData[i];
        finalVert.tangent += verts[i].tangent * barycentricData[i];
    }
    return finalVert;
}

float rand(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

// Not really “random”, but quick to implement
float2 rand2(float2 uv)
{
    return float2(
rand(uv),
rand(uv.yx));
}

float3 RandomVector(float u0, float u1)
{
    float a = u0 * 2 - 1;
    float b = sqrt(1 - a * a);
    float phi = 2.0f * PI * u1;
    float x = b * cos(phi);
    float y = b * sin(phi);
    float z = a;
    return float3(x, y, z);
}

float3 RandomCosineWeightedHemisphere(float u0, float u1, float3 unitNormal)
{
    float a = u0 * 2 - 1;
    float b = sqrt(1 - a * a);
    float phi = 2.0f * PI * u1;
    float x = unitNormal.x + b * cos(phi);
    float y = unitNormal.y + b * sin(phi);
    float z = unitNormal.z + a;
    return float3(x, y, z);
}


// Calculates an origin and direction from the camera for specific pixel indices
RayDesc CalcRayFromCamera(float2 rayIndices, float3 camPos, float4x4 invVP)
{
	// Offset to the middle of the pixel
    float2 pixel = rayIndices + 0.5f;
    float2 screenPos = pixel / DispatchRaysDimensions().xy * 2.0f - 1.0f;
    screenPos.y = -screenPos.y;

	// Unproject the coords
    float4 worldPos = mul(invVP, float4(screenPos, 0, 1));
    worldPos.xyz /= worldPos.w;

	// Set up the ray
    RayDesc ray;
    ray.Origin = camPos.xyz;
    ray.Direction = normalize(worldPos.xyz - ray.Origin);
    ray.TMin = 0.01f;
    ray.TMax = 1000.0f;
    return ray;
}


// === Shaders ===

// Ray generation shader - Launched once for each ray we want to generate
// (which is generally once per pixel of our output texture)
[shader("raygeneration")]
void RayGen()
{
    // Grab the constant buffer
    ConstantBuffer<SceneData> cb =
        ResourceDescriptorHeap[SceneDataConstantBufferIndex];
    
	// Get the ray indices
    uint2 rayIndices = DispatchRaysIndex().xy;

	// Calculate the ray from the camera through a particular
	// pixel of the output buffer using this shader's indices


	// Set up the payload for the ray
	// This initializes the struct to all zeros
    //RayPayload payload = (RayPayload) 0;

    RaytracingAccelerationStructure SceneTLAS = ResourceDescriptorHeap[SceneTLASDescriptorIndex];
	// Perform the ray trace for this ray
    

    float3 totalColor = float3(0, 0, 0);
    int raysPerPixel = RPP;
    
    for (int r = 0; r < raysPerPixel; r++)
    {
        float2 adjustedIndices = (float2) rayIndices;
        float ray01 = (float) r / raysPerPixel;
        adjustedIndices += rand2(rayIndices.xy * ray01);
        
        RayDesc ray = CalcRayFromCamera(adjustedIndices, cb.CameraPosition, cb.InverseViewProjection);
        
        RayPayload payload;
        payload.color = float3(1, 1, 1);
        payload.RecursionDepth = 0;
        payload.RayPerPixelIndex = r;
        
        TraceRay(
		SceneTLAS,
		RAY_FLAG_NONE,
		0xFF, 0, 0, 0,
        ray,
		payload);
        
        totalColor += payload.color;
    }
    
    // Average results
    float3 avg = totalColor / raysPerPixel;
    
	// Set the final color of the buffer
    // Set the final color of the buffer (gamma corrected)
    RWTexture2D<float4> OutputColor = ResourceDescriptorHeap[OutputUAVDescriptorIndex];
    OutputColor[rayIndices] = float4(pow(avg, 1.0f / 2.2f), 1);
}


// Miss shader - What happens if the ray doesn't hit anything?
[shader("miss")]
void Miss(inout RayPayload payload)
{
	// Hemispheric gradient
    float3 upColor = float3(0.3f, 0.5f, 0.95f);
    float3 downColor = float3(1, 1, 1);

	// Interpolate based on the direction of the ray
    float interpolation = dot(normalize(WorldRayDirection()), float3(0, 1, 0)) * 0.5f + 0.5f;
    float3 skyColor = lerp(downColor, upColor, interpolation);
	
	// Alter the payload color by the sky color
    payload.color *= skyColor;
}


// Closest hit shader - Runs the first time a ray hits anything
[shader("closesthit")]
void ClosestHit(inout RayPayload payload, BuiltInTriangleIntersectionAttributes hitAttributes)
{
	//// Get the interpolated vertex data
 //   Vertex interpolatedVert = InterpolateVertices(
	//	PrimitiveIndex(),
	//	hitAttributes.barycentrics);

	//// Use the resulting data to set the final color
	//// Note: Here is where we would do actual shading!
 //   payload.color = interpolatedVert.normal;
    
    if (payload.RecursionDepth == RECDEP)
    {
        payload.color = float3(0, 0, 0);
        return;
    }
    
    // Get the data for this entity
    StructuredBuffer<EntityData> entityDataBuffer =
        ResourceDescriptorHeap[EntityDataDescriptorIndex];
    
    RaytracingAccelerationStructure SceneTLAS = ResourceDescriptorHeap[SceneTLASDescriptorIndex];
    EntityData thisEntity = entityDataBuffer[InstanceIndex()];
    payload.color *= thisEntity.Color.rgb;
    
    Vertex hit = InterpolateVertices(PrimitiveIndex(), hitAttributes.barycentrics);
    float3 normal_WS = normalize(mul(hit.normal, (float3x3) ObjectToWorld4x3()));
    
    float2 pixelUV = (float2) DispatchRaysIndex().xy / DispatchRaysDimensions().xy;
    float2 rng = rand2(pixelUV * (payload.RecursionDepth + 1) +
                payload.RayPerPixelIndex +
                RayTCurrent());
    
    float3 randomBounce = RandomCosineWeightedHemisphere(rand(rng), rand(rng.yx), normal_WS);
    float3 refl = reflect(WorldRayDirection(), normal_WS);
    
    RayDesc ray;
    ray.Origin = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    ray.Direction = normalize(lerp(refl, randomBounce, thisEntity.diffusion));
    ray.TMin = 0.0001f;
    ray.TMax = 1000.0f;
    
    payload.RecursionDepth++;
    TraceRay(
		SceneTLAS,
		RAY_FLAG_NONE,
		0xFF, 0, 0, 0, ray,
		payload);
}