#pragma once
#include "DirectXFramework.h"
#include "TexturedCubeNode.h"
#include "MeshNode.h"
#include "TerrainNode.h"
#include "Camera.h"

class Graphics2 : public DirectXFramework
{
public:
	void CreateSceneGraph();
	void UpdateSceneGraph();

};

