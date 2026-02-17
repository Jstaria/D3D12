#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl/client.h>

#include <vector>

#include "../MainComponents/Graphics.h"
//#include "../Helper/Debug.h"
#include "../Helper/OBJLoader.h"
#include "../Structures/Vertex.h"
#include "../Structures/MeshData.h"
//#include "../ImGui/imgui.h"
#include "../Interfaces/IDrawable.h"

class Mesh : public IDrawable
{
private:
	// --- Mesh Buffers ---
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
	D3D12_INDEX_BUFFER_VIEW ibView{};

	MeshData meshData;

	// --- General ---
	bool meshToggle;
	bool wireFrameToggle;
	Vertex center;

	// --- Mesh Creation ---
	void InitializeMesh(MeshData meshData);
	void CreateMesh(MeshData& meshData);
	//MeshData LoadData(const char* filePath);
	void FindCenterOfMesh(MeshData& meshData);
	void CalculateTangents(MeshData& meshData);

public:
	// --- Constructors ---
	Mesh(const char* name, std::vector<Vertex> vertices, std::vector<unsigned int> indices);
	Mesh(MeshData meshData);
	Mesh(const char* name, const char* filePath);
	Mesh();
	~Mesh();

	// --- Mesh Getters ---
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView();
	int GetVertexCount();
	int GetIndexCount();
	const char* GetName() override;
	MeshData GetMeshData();
	Vertex GetCenter();

	bool GetToggleMesh();
	bool GetToggleWireFrame();

	// --- General ---
	//void DrawImGui(int i);
	void Draw() override;
};

