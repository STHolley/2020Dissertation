#include "RenderObject.h"
#include "../../Common/MeshGeometry.h"
#include <OGLRenderer.cpp>

using namespace NCL;
using namespace CSC3223;

RenderObject::RenderObject(MeshGeometry* inMesh, Matrix4 m)
{
	mesh = inMesh;
	transform = m;
	texture = nullptr;
	shader = nullptr;
}

void RenderObject::SetMesh(MeshGeometry* inMesh) {
	mesh = inMesh;
}

RenderObject::~RenderObject()
{
}