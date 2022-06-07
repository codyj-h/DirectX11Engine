#pragma once
#include "SceneNode.h"
#include "ResourceManager.h"
#include "DirectXFramework.h"
#include "DDSTextureLoader.h"

class TerrainNode : public SceneNode
{
public:
	TerrainNode(wstring name);
	~TerrainNode();

	bool Initialise();
	void Render();
	void Shutdown(){}
		
		
private:	
	unsigned int _numberOfXPoints;
	unsigned int _numberOfZPoints;
	unsigned int _numberOfPolygons;
	unsigned int _numberOfVertices;
	unsigned int _numberOfIndices;

	BYTE r;
	BYTE g;
	BYTE b;
	BYTE a;

	//unsigned int SPACING =	10;
	//unsigned int NUMBER_OF_ROWS = 1023;
	//unsigned int NUMBER_OF_COLUMNS = 1023;
	//unsigned int WORLD_HEIGHT = 65535;


	float _terrainStartX; 
	float _terrainStartZ;
	float _terrainEndX;
	float _terrainEndZ;

	vector<VERTEX> _vertices;
	vector<UINT> _indices;
	vector<float> _heightValues;

	XMFLOAT4 _ambientLight;
	XMFLOAT4 _directionalLightVector;
	XMFLOAT4 _directionalLightColour;
	XMFLOAT4 _cameraPosition;

	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11DeviceContext> _deviceContext;

	ComPtr<ID3D11Buffer> _vertexBuffer;
	ComPtr<ID3D11Buffer> _indexBuffer;

	ComPtr<ID3DBlob> _vertexShaderByteCode = nullptr;
	ComPtr<ID3DBlob> _pixelShaderByteCode = nullptr;
	ComPtr<ID3D11VertexShader> _vertexShader;
	ComPtr<ID3D11PixelShader> _pixelShader;
	ComPtr<ID3D11InputLayout> _layout;
	ComPtr<ID3D11Buffer> _constantBuffer;

	ComPtr<ID3D11RasterizerState> _defaultRasterizerState;
	ComPtr<ID3D11RasterizerState> _wireframeRasterizerState;

	void GenerateVerticesAndIndices();
	void GenerateBuffers();
	void BuildShaders();
	void BuildVertexLayout();
	void BuildConstantBuffer();
	void BuildRendererStates();
	bool LoadHeightMap(wstring heightMapFilename);
	void LoadTerrainTextures();
	void GenerateBlendMap();
};

