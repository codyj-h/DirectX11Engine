#include "SceneGraph.h"

bool SceneGraph::Initialise() 
{
	bool status = true;
	for (list<SceneNodePointer>::iterator listIterator = begin(children);
		listIterator != end(children) && status;
		listIterator++)
	{
		status = (*listIterator)->Initialise();
	}
	return status;
}

void SceneGraph::Update(FXMMATRIX& currentWorldTransformation)
{

	SceneNode::Update(currentWorldTransformation);

	XMMATRIX combinedWorlTransformation = XMLoadFloat4x4(&_combinedWorldTransformation);

	for (list<SceneNodePointer>::iterator listIterator = begin(children);
		listIterator != end(children);
		listIterator++)
	{
		(*listIterator)->Update(combinedWorlTransformation);
	}
}

void SceneGraph::Render()
{
	for (list<SceneNodePointer>::iterator listIterator = begin(children);
		listIterator != end(children);
		listIterator++)
	{
		(*listIterator)->Render();
	}
	
}

void SceneGraph::Shutdown()
{
	for (list<SceneNodePointer>::iterator listIterator = begin(children);
		listIterator != end(children);
		listIterator++)
	{
		(*listIterator)->Shutdown();
	}
}

void SceneGraph::Add(SceneNodePointer node)
{
	children.push_back(node);
}

void SceneGraph::Remove(SceneNodePointer node)
{
	for (list<SceneNodePointer>::iterator listIterator = begin(children);
		listIterator != end(children);
		listIterator++)
	{
		(*listIterator)-> Remove(node);
		if (*listIterator == node)
		{
			children.erase(listIterator);
		}
	}
}

SceneNodePointer SceneGraph::Find(wstring name)
{
	if (_name == name)
	{
		return shared_from_this();
	}
	SceneNodePointer returnValue = nullptr;
	for (list<SceneNodePointer>::iterator listIterator = begin(children);
		listIterator != end(children);
		listIterator++)
	{
		returnValue = (*listIterator)->Find(name);
	}
	return returnValue;

}