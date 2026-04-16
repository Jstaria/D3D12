#pragma once
#include <memory>
#include "DirectXMath.h"
#include <wrl/client.h>
#include "d3d12.h"

#include "../MainComponents/Graphics.h"
#include "../Components/Mesh.h"
#include "../Structures/ExternalData.h"
#include "../Components/Camera.h"

#include "../Helper/OBJLoader.h"
#include "../Helper/PathHelpers.h"

class Sky
{
private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSig;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> PS;

	std::shared_ptr<Mesh> mesh;
	unsigned int skyboxIndex;

	void CreateStates();

public:
	Sky(std::shared_ptr<Mesh> mesh, unsigned int skyboxIndex);
	Sky();

	unsigned int GetIndex();

	void Draw(std::shared_ptr<Camera> cam);

};

