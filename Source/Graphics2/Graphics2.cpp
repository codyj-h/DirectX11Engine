#include "Graphics2.h"

Graphics2 app;

void Graphics2::CreateSceneGraph()
{
	GetCamera()->SetCameraPosition(0.0f, 512.0f, -500.0f);
	SceneGraphPointer sceneGraph = GetSceneGraph();
	
	

	shared_ptr<TerrainNode> node = make_shared<TerrainNode>(L"Terrain");	
	sceneGraph->Add(node);

	shared_ptr<MeshNode> plane = make_shared<MeshNode>(L"Plane1", L"Plane\\Bonanza.3DS");
	plane->SetWorldTransform(XMMatrixTranslation(500, 500, 0));
	sceneGraph->Add(plane);


	
}

void Graphics2::UpdateSceneGraph()
{
	SceneGraphPointer sceneGraph = GetSceneGraph();
	//W	
	if (GetAsyncKeyState(0x57) < 0)
	{
		GetCamera()->SetForwardBack(10);
	}
	////A
	//short GetAsyncKeyState(0x41);
	if (GetAsyncKeyState(0x41) < 0)
	{
		GetCamera()->SetLeftRight(-10);
	}
	////S
	//short GetAsyncKeyState(0x53);
	if (GetAsyncKeyState(0x53) < 0)
	{
		GetCamera()->SetForwardBack(-10);
	}
	////D
	//short GetAsyncKeyState(0x44);
	if (GetAsyncKeyState(0x44) < 0)
	{
		GetCamera()->SetLeftRight(10);
	}
	if (GetAsyncKeyState(0x28) < 0)
	{
		GetCamera()->SetPitch(2);
	}
	if (GetAsyncKeyState(0x26) < 0)
	{
		GetCamera()->SetPitch(-2);
	}

	if (GetAsyncKeyState(0x25) < 0)
	{
		GetCamera()->SetYaw(-2);
	}
	if (GetAsyncKeyState(0x27) < 0)
	{
		GetCamera()->SetYaw(2);
	}
	////SPC
	//short GetAsyncKeyState(0x20);
	
	////CTRL
	//short GetAsyncKeyState(0xA2);
	
	// This is where you make any changes to the local world transformations to nodes
	// in the scene graph
}
