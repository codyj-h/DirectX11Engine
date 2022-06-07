#include "TerrainNode.h"
#include "DirectXFramework.h"
#include <fstream>



#define NUMBER_OF_ROWS		1023
#define NUMBER_OF_COLUMNS	1023
#define SPACING				10
#define WORLD_HEIGHT        65535

struct CBUFFER
{
	XMMATRIX    CompleteTransformation;
	XMMATRIX	WorldTransformation;
	XMFLOAT4	CameraPosition;
	XMVECTOR    LightVector;
	XMFLOAT4    LightColor;
	XMFLOAT4    AmbientColor;
	XMFLOAT4    DiffuseColor;
	XMFLOAT4	SpecularColor;
	float		Shininess;
	float		Opacity;
	float       Padding[2];
};



TerrainNode::TerrainNode(wstring name) : SceneNode(name)
{
	_numberOfXPoints = NUMBER_OF_COLUMNS + 1;
	_numberOfZPoints = NUMBER_OF_ROWS + 1;
	_numberOfPolygons = NUMBER_OF_COLUMNS * NUMBER_OF_ROWS * 2;
	_numberOfVertices = _numberOfPolygons * 2;
}

TerrainNode::~TerrainNode()
{
}

bool TerrainNode::Initialise()
{
	_device = DirectXFramework::GetDXFramework()->GetDevice();
	_deviceContext = DirectXFramework::GetDXFramework()->GetDeviceContext();
	LoadHeightMap(L"Example_HeightMap.raw");
	GenerateVerticesAndIndices();
	GenerateBuffers();
	BuildShaders();
	BuildVertexLayout();
	BuildConstantBuffer();
	BuildRendererStates();
	LoadTerrainTextures();
	GenerateBlendMap();

	
	return true;
}

void TerrainNode::Render()
{
	XMMATRIX projectionTransformation = DirectXFramework::GetDXFramework()->GetProjectionTransformation();
	XMMATRIX viewTransformation = DirectXFramework::GetDXFramework()->GetViewTransformation();

	XMMATRIX completeTransformation = XMLoadFloat4x4(&_worldTransformation) * viewTransformation * projectionTransformation;

	// Draw the first cube
	CBUFFER cBuffer;
	cBuffer.CompleteTransformation = completeTransformation;
	cBuffer.WorldTransformation = XMLoadFloat4x4(&_worldTransformation);
	cBuffer.AmbientColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f);
	cBuffer.LightVector = XMVector4Normalize(XMVectorSet(1.0f, -1.0f, 1.0f, 0.0f));
	cBuffer.LightColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f);
	cBuffer.CameraPosition = XMFLOAT4(0.0f, 10.0f, -25.0f, 0.0f);

	_deviceContext->VSSetShader(_vertexShader.Get(), 0, 0);
	_deviceContext->PSSetShader(_pixelShader.Get(), 0, 0);
	_deviceContext->IASetInputLayout(_layout.Get());



	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	_deviceContext->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), &stride, &offset);
	_deviceContext->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	cBuffer.DiffuseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	cBuffer.SpecularColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	cBuffer.Shininess = 1.0f;
	cBuffer.Opacity = 1.0f;

	// Update the constant buffer 
	_deviceContext->VSSetConstantBuffers(0, 1, _constantBuffer.GetAddressOf());
	_deviceContext->PSSetConstantBuffers(0, 1, _constantBuffer.GetAddressOf());
	_deviceContext->UpdateSubresource(_constantBuffer.Get(), 0, 0, &cBuffer, 0, 0);
	_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//_deviceContext->RSSetState(_wireframeRasterizerState.Get());
	_deviceContext->DrawIndexed(_numberOfIndices, 0, 0);
}

void TerrainNode::GenerateVerticesAndIndices()
{
	
	
	_numberOfIndices = _numberOfPolygons * 3;

	float width = (float)(_numberOfXPoints * SPACING);
	float depth = (float)(_numberOfZPoints * SPACING);

	float xOffset = width * -0.5f;
	float zOffset = depth * 0.5f;

	_terrainStartX = xOffset;
	_terrainStartZ = zOffset;
	_terrainEndX = xOffset + width - 1;
	_terrainEndZ = zOffset - depth + 1;

	int hindex;
	int hindex2;

	float du = 1.0f / (_numberOfXPoints - 1);
	float dv = 1.0f / (_numberOfZPoints - 1);
	unsigned int vertexIndex = 0;
	for (int z = 0; z < NUMBER_OF_ROWS; z++)
	{
		for (int x = 0; x < NUMBER_OF_COLUMNS; x++)
		{
			hindex = (z * 1024) + x;
			hindex2 = hindex + 1024;

			// Top-left corner vertex
			VERTEX vertex;
			vertex.Position = XMFLOAT3(x * SPACING + xOffset, _heightValues[hindex] * 1024, -z * SPACING + zOffset);
			vertex.Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
			vertex.TexCoord = XMFLOAT2(0.0f, 0.0f);
			vertex.BlendMapTexCoord = XMFLOAT2(du, dv);
			_vertices.push_back(vertex);

			// Top-right corner vertex
			vertex.Position = XMFLOAT3((x + 1) * SPACING + xOffset, _heightValues[hindex + 1] * 1024, -z * SPACING + zOffset);
			vertex.Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
			vertex.TexCoord = XMFLOAT2(1.0f, 0.0f);
			vertex.BlendMapTexCoord = XMFLOAT2(du+1, dv);
			_vertices.push_back(vertex);

			// Bottom-left corner vertex
			vertex.Position = XMFLOAT3(x * SPACING + xOffset, _heightValues[hindex2] * 1024, (-z - 1) * SPACING + zOffset);
			vertex.Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
			vertex.TexCoord = XMFLOAT2(0.0f, 1.0f);
			vertex.BlendMapTexCoord = XMFLOAT2(du, dv+1);
			_vertices.push_back(vertex);

			// Bottom-right corner vertex
			vertex.Position = XMFLOAT3((x + 1) * SPACING + xOffset, _heightValues[hindex2 + 1] * 1024, (-z - 1) * SPACING + zOffset);
			vertex.Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
			vertex.TexCoord = XMFLOAT2(1.0f, 1.0f);
			vertex.BlendMapTexCoord = XMFLOAT2(du+1, dv+1);
			_vertices.push_back(vertex);

			// First triangle
			_indices.push_back(vertexIndex);
			_indices.push_back(vertexIndex + 1);
			_indices.push_back(vertexIndex + 2);
			// Second triangle
			_indices.push_back(vertexIndex + 2);
			_indices.push_back(vertexIndex + 1);
			_indices.push_back(vertexIndex + 3);
			vertexIndex = vertexIndex + 4;
		}
		
		// Calculate vertex normals
	/*	int vertexContributingCount[1024];
		for (int i = 0; i < _indices.size(); i++)
		{
			vertexContributingCount[i] = 0;
		}*/
		
	}
	int vertexNormalsIndex = 0;
	for (int x = 0; x < (int)NUMBER_OF_ROWS; x++)
	{
		for (int z = 0; z < (int)NUMBER_OF_COLUMNS; z++)
		{
			int index0 = vertexNormalsIndex;
			int index1 = vertexNormalsIndex + 1;
			int index2 = vertexNormalsIndex + 2;
			int index3 = vertexNormalsIndex + 3;

			vertexNormalsIndex += 4;
			XMVECTOR u = XMVectorSet(_vertices[index1].Position.x - _vertices[index0].Position.x,
				_vertices[index1].Position.y - _vertices[index0].Position.y,
				_vertices[index1].Position.z - _vertices[index0].Position.z,
				0.0f);

			XMVECTOR v = XMVectorSet(_vertices[index2].Position.x - _vertices[index0].Position.x,
				_vertices[index2].Position.y - _vertices[index0].Position.y,
				_vertices[index2].Position.z - _vertices[index0].Position.z,
				0.0f);

			XMVECTOR normal = XMVector3Normalize(XMVector3Cross(u, v));

			XMStoreFloat3(&_vertices[index0].Normal, XMVectorAdd(XMLoadFloat3(&_vertices[index0].Normal), normal));
			XMStoreFloat3(&_vertices[index1].Normal, XMVectorAdd(XMLoadFloat3(&_vertices[index1].Normal), normal));
			XMStoreFloat3(&_vertices[index2].Normal, XMVectorAdd(XMLoadFloat3(&_vertices[index2].Normal), normal));
			XMStoreFloat3(&_vertices[index3].Normal, XMVectorAdd(XMLoadFloat3(&_vertices[index3].Normal), normal));
		}
	}

	for (int i = 0; i < _numberOfVertices; i++)
	{
		XMVECTOR vertexnormal = XMLoadFloat3(&_vertices[i].Normal);
		XMStoreFloat3(&_vertices[i].Normal, XMVector3Normalize(vertexnormal));
	}
}


void TerrainNode::GenerateBuffers()
{
	D3D11_BUFFER_DESC vertexBufferDescriptor;
	vertexBufferDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDescriptor.ByteWidth = sizeof(VERTEX) * _numberOfVertices;
	vertexBufferDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDescriptor.CPUAccessFlags = 0;
	vertexBufferDescriptor.MiscFlags = 0;
	vertexBufferDescriptor.StructureByteStride = 0;

	// Now set up a structure that tells DirectX where to get the
	// data for the vertices from
	D3D11_SUBRESOURCE_DATA vertexInitialisationData;
	vertexInitialisationData.pSysMem = &_vertices[0];

	// and create the vertex buffer
	ThrowIfFailed(_device->CreateBuffer(&vertexBufferDescriptor, &vertexInitialisationData, _vertexBuffer.GetAddressOf()));

	D3D11_BUFFER_DESC indexBufferDescriptor;
	indexBufferDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDescriptor.ByteWidth = sizeof(UINT) * _numberOfIndices;
	indexBufferDescriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDescriptor.CPUAccessFlags = 0;
	indexBufferDescriptor.MiscFlags = 0;
	indexBufferDescriptor.StructureByteStride = 0;

	// Now set up a structure that tells DirectX where to get the
	// data for the indices from
	D3D11_SUBRESOURCE_DATA indexInitialisationData;
	indexInitialisationData.pSysMem = &_indices[0];

	// and create the index buffer
	ThrowIfFailed(_device->CreateBuffer(&indexBufferDescriptor, &indexInitialisationData, _indexBuffer.GetAddressOf()));


}

void TerrainNode::BuildShaders()
{
	DWORD shaderCompileFlags = 0;
#if defined( _DEBUG )
	shaderCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> compilationMessages = nullptr;

	//Compile vertex shader
	HRESULT hr = D3DCompileFromFile(L"TerrainShaders.hlsl",
									nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
									"VShader", "vs_5_0",
									shaderCompileFlags, 0,
									_vertexShaderByteCode.GetAddressOf(),
									compilationMessages.GetAddressOf());

	if (compilationMessages.Get() != nullptr)
	{
		// If there were any compilation messages, display them
		MessageBoxA(0, (char*)compilationMessages->GetBufferPointer(), 0, 0);
	}
	// Even if there are no compiler messages, check to make sure there were no other errors.
	ThrowIfFailed(hr);
	ThrowIfFailed(_device->CreateVertexShader(_vertexShaderByteCode->GetBufferPointer(), _vertexShaderByteCode->GetBufferSize(), NULL, _vertexShader.GetAddressOf()));

	// Compile pixel shader
	hr = D3DCompileFromFile(L"TerrainShaders.hlsl",
							nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
							"PShader", "ps_5_0",
							shaderCompileFlags, 0,
							_pixelShaderByteCode.GetAddressOf(),
							compilationMessages.GetAddressOf());

	if (compilationMessages.Get() != nullptr)
	{
		// If there were any compilation messages, display them
		MessageBoxA(0, (char*)compilationMessages->GetBufferPointer(), 0, 0);
	}
	ThrowIfFailed(hr);
	ThrowIfFailed(_device->CreatePixelShader(_pixelShaderByteCode->GetBufferPointer(), _pixelShaderByteCode->GetBufferSize(), NULL, _pixelShader.GetAddressOf()));
}

void TerrainNode::BuildVertexLayout()
{
	// Create the vertex input layout. This tells DirectX the format
	// of each of the vertices we are sending to it.

	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	ThrowIfFailed(_device->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), _vertexShaderByteCode->GetBufferPointer(), _vertexShaderByteCode->GetBufferSize(), _layout.GetAddressOf()));
}

void TerrainNode::BuildConstantBuffer()
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(CBUFFER);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ThrowIfFailed(_device->CreateBuffer(&bufferDesc, NULL, _constantBuffer.GetAddressOf()));
}

void TerrainNode::BuildRendererStates()
{
	// Set default and wireframe rasteriser states
	D3D11_RASTERIZER_DESC rasteriserDesc;
	rasteriserDesc.FillMode = D3D11_FILL_SOLID;
	rasteriserDesc.CullMode = D3D11_CULL_BACK;
	rasteriserDesc.FrontCounterClockwise = false;
	rasteriserDesc.DepthBias = 0;
	rasteriserDesc.SlopeScaledDepthBias = 0.0f;
	rasteriserDesc.DepthBiasClamp = 0.0f;
	rasteriserDesc.DepthClipEnable = true;
	rasteriserDesc.ScissorEnable = false;
	rasteriserDesc.MultisampleEnable = false;
	rasteriserDesc.AntialiasedLineEnable = false;
	ThrowIfFailed(_device->CreateRasterizerState(&rasteriserDesc, _defaultRasterizerState.GetAddressOf()));
	rasteriserDesc.FillMode = D3D11_FILL_WIREFRAME;
	ThrowIfFailed(_device->CreateRasterizerState(&rasteriserDesc, _wireframeRasterizerState.GetAddressOf()));
}

bool TerrainNode::LoadHeightMap(wstring heightMapFilename)
{
	unsigned int mapSize = _numberOfXPoints * _numberOfZPoints;
	USHORT* rawFileValues = new USHORT[mapSize];

	ifstream inputHeightMap;
	inputHeightMap.open(heightMapFilename.c_str(), std::ios_base::binary);
	if (!inputHeightMap)
	{
		return false;
	}

	inputHeightMap.read((char*)rawFileValues, mapSize * 2);
	inputHeightMap.close();

	// Normalise BYTE values to the range 0.0f - 1.0f;
	for (unsigned int i = 0; i < mapSize; i++)
	{
		_heightValues.push_back((float)rawFileValues[i] / 65536);
	}
	delete[] rawFileValues;
	return true;
}


D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

ComPtr<ID3D11ShaderResourceView> _texturesResourceView;
ComPtr<ID3D11ShaderResourceView> _blendMapResourceView;

void TerrainNode::LoadTerrainTextures()
{
	// Change the paths below as appropriate for your use
	wstring terrainTextureNames[5] = { L"grass.dds", L"darkdirt.dds", L"stone.dds", L"lightdirt.dds", L"snow.dds" };

	// Load the textures from the files
	ComPtr<ID3D11Resource> terrainTextures[5];
	for (int i = 0; i < 5; i++)
	{
		ThrowIfFailed(CreateDDSTextureFromFileEx(_device.Get(),
			_deviceContext.Get(),
			terrainTextureNames[i].c_str(),
			0,
			D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_SHADER_RESOURCE,
			0,
			0,
			false,
			terrainTextures[i].GetAddressOf(),
			nullptr
		));
	}
	// Now create the Texture2D arrary.  We assume all textures in the
	// array have the same format and dimensions

	D3D11_TEXTURE2D_DESC textureDescription;
	ComPtr<ID3D11Texture2D> textureInterface;
	terrainTextures[0].As<ID3D11Texture2D>(&textureInterface);
	textureInterface->GetDesc(&textureDescription);

	D3D11_TEXTURE2D_DESC textureArrayDescription;
	textureArrayDescription.Width = textureDescription.Width;
	textureArrayDescription.Height = textureDescription.Height;
	textureArrayDescription.MipLevels = textureDescription.MipLevels;
	textureArrayDescription.ArraySize = 5;
	textureArrayDescription.Format = textureDescription.Format;
	textureArrayDescription.SampleDesc.Count = 1;
	textureArrayDescription.SampleDesc.Quality = 0;
	textureArrayDescription.Usage = D3D11_USAGE_DEFAULT;
	textureArrayDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureArrayDescription.CPUAccessFlags = 0;
	textureArrayDescription.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> textureArray = 0;
	ThrowIfFailed(_device->CreateTexture2D(&textureArrayDescription, 0, textureArray.GetAddressOf()));

	// Copy individual texture elements into texture array.

	for (UINT i = 0; i < 5; i++)
	{
		// For each mipmap level...
		for (UINT mipLevel = 0; mipLevel < textureDescription.MipLevels; mipLevel++)
		{
			_deviceContext->CopySubresourceRegion(textureArray.Get(),
				D3D11CalcSubresource(mipLevel, i, textureDescription.MipLevels),
				NULL,
				NULL,
				NULL,
				terrainTextures[i].Get(),
				mipLevel,
				nullptr
			);
		}
	}

	// Create a resource view to the texture array.
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDescription;
	viewDescription.Format = textureArrayDescription.Format;
	viewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDescription.Texture2DArray.MostDetailedMip = 0;
	viewDescription.Texture2DArray.MipLevels = textureArrayDescription.MipLevels;
	viewDescription.Texture2DArray.FirstArraySlice = 0;
	viewDescription.Texture2DArray.ArraySize = 5;

	ThrowIfFailed(_device->CreateShaderResourceView(textureArray.Get(), &viewDescription, _texturesResourceView.GetAddressOf()));
}

void TerrainNode::GenerateBlendMap()
{
	// Note that _numberOfRows and _numberOfColumns need to be setup
	// to the number of rows and columns in your grid in order for this
	// to work.
	DWORD* blendMap = new DWORD[NUMBER_OF_ROWS * NUMBER_OF_COLUMNS];
	DWORD* blendMapPtr = blendMap;
	a = 0;
	b = 0;
	g = 0;
	r = 100;




	DWORD index = 0;
	for (DWORD i = 0; i < NUMBER_OF_COLUMNS; i++)
	{
		for (DWORD j = 0; j < NUMBER_OF_ROWS; j++)
		{
			if (_vertices[i*j].Position.y > 215)
			{
				g = 0;
				b = 100;
				r = 0;
			}
			// Calculate the appropriate blend colour for the 
			// current location in the blend map.  This has been
			// left as an exercise for you.  You need to calculate
			// appropriate values for the r, g, b and a values (each
			// between 0 and 255). The code below combines these
			// into a DWORD (32-bit value) and stores it in the blend map.

			DWORD mapValue = (a << 24) + (b << 16) + (g << 8) + r;
			*blendMapPtr++ = mapValue;
		}
	}

	D3D11_TEXTURE2D_DESC blendMapDescription;
	blendMapDescription.Width = NUMBER_OF_ROWS;
	blendMapDescription.Height = NUMBER_OF_COLUMNS;
	blendMapDescription.MipLevels = 1;
	blendMapDescription.ArraySize = 1;
	blendMapDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	blendMapDescription.SampleDesc.Count = 1;
	blendMapDescription.SampleDesc.Quality = 0;
	blendMapDescription.Usage = D3D11_USAGE_DEFAULT;
	blendMapDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	blendMapDescription.CPUAccessFlags = 0;
	blendMapDescription.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA blendMapInitialisationData;
	blendMapInitialisationData.pSysMem = blendMap;
	blendMapInitialisationData.SysMemPitch = 4 * NUMBER_OF_COLUMNS;

	ComPtr<ID3D11Texture2D> blendMapTexture;
	ThrowIfFailed(_device->CreateTexture2D(&blendMapDescription, &blendMapInitialisationData, blendMapTexture.GetAddressOf()));

	// Create a resource view to the texture array.
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDescription;
	viewDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	viewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	viewDescription.Texture2D.MostDetailedMip = 0;
	viewDescription.Texture2D.MipLevels = 1;
	_deviceContext->PSSetShaderResources(0, 1, _blendMapResourceView.GetAddressOf());
	_deviceContext->PSSetShaderResources(1, 1, _texturesResourceView.GetAddressOf());
	ThrowIfFailed(_device->CreateShaderResourceView(blendMapTexture.Get(), &viewDescription, _blendMapResourceView.GetAddressOf()));
	delete[] blendMap;


}

