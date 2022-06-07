#pragma once
#include "SceneNode.h"
#include "WICTextureLoader.h"
#include "DirectXFramework.h"

class TexturedCubeNode : public SceneNode
{
public:
	TexturedCubeNode(wstring name, wstring textureName) : SceneNode(name) { _textureName = textureName; }

	bool Initialise();
	void Render();
	void Shutdown() {}
private:
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> deviceContext;

	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;

	ComPtr<ID3DBlob> vertexShaderByteCode = nullptr;
	ComPtr<ID3DBlob> pixelShaderByteCode = nullptr;
	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;
	ComPtr<ID3D11InputLayout> layout;
	ComPtr<ID3D11Buffer> constantBuffer;

	ComPtr<ID3D11ShaderResourceView> texture;

	wstring _textureName;

	void BuildGeometryBuffers();
	void BuildShaders();
	void BuildVertexLayout();
	void BuildConstantBuffer();
	void BuildTexture();
};

