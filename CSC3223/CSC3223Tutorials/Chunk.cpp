#include "Chunk.h"
#include "../../Common/TextureLoader.h"
#include "../../Common/Vector3.h"
#include "../../Common/Vector4.h"
#include "../../Common/MeshGeometry.h"
#include "../../Common/Maths.h"
#include <math.h>
#include <cmath>
#include <map>

#include "../../Plugins/OpenGLRendering/OGLRenderer.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "Renderer.h"

using namespace NCL;
using namespace CSC3223;



Chunk::Chunk()
{
	worldX = 0;
	worldY = 0;
	worldZ = 0;
	chunkSize = 16;
}

Chunk::Chunk(int xPos, int yPos, int zPos, int size)
{
	chunkSize = size;
	worldX = xPos;
	worldY = yPos;
	worldZ = zPos;
}

Chunk::~Chunk()
{
	delete noise;
}

int Chunk::mapPosition(int x, int y, int z) {
	int position = x + (y * (chunkSize + 1)) + (z * (chunkSize + 1) * (chunkSize + 1));
	return position;
}

int Chunk::mapPosition(int x, int z) {
	int position = x + (z * (chunkSize + 1));
	return position;
}

RenderObject* Chunk::renderChunkGPU() {
	Vector4 white(0.8, 0.8, 0.8, 1);
	Vector4 green(11.0 / 255.0, 102.0 / 255.0, 35.0 / 255.0, 1);
	Vector4 blue(0, 119.0 / 255.0, 190.0 / 255.0, 1);
	Vector4 grey(0.5, 0.5, 0.5, 1);
	int whiteThresh = 120;
	int greenThresh = 56;
	int blueThresh = 16;
	vector<Vector4> col;
	vector<Vector3> pts;
	for (int x = 0; x <= chunkSize - 1; x++) {
		int globalX = worldX * chunkSize + x;
		for (int y = 0; y <= chunkSize - 1; y++) {
			int globalY = worldY * chunkSize + y;
			for (int z = 0; z <= chunkSize - 1; z++) {
				int globalZ = worldZ * chunkSize + z;

				pts.emplace_back(Vector3(globalX, globalY, globalZ));
				if (globalY >= whiteThresh) {
					col.emplace_back(white);
				}
				else if (globalY >= greenThresh) {
					col.emplace_back(green);
				}
				else if (globalY >= blueThresh) {
					col.emplace_back(blue);
				}
				else {
					col.emplace_back(grey);
				}
			}
		}
	}
	OGLMesh* points = new OGLMesh();
	points->SetVertexPositions(pts);
	points->SetVertexColours(col);
	points->SetPrimitiveType(GeometryPrimitive::Points);
	points->UploadToGPU();
	RenderObject* rObj = new RenderObject(points);
	return rObj;
}

RenderObject* Chunk::renderChunkCPU(int triTable[256][16]) {
	Vector4 white(0.8, 0.8, 0.8, 1);
	Vector4 green(11.0 / 255.0, 102.0 / 255.0, 35.0 / 255.0, 1);
	Vector4 blue(0, 119.0 / 255.0, 190.0 / 255.0, 1);
	Vector4 grey(0.5, 0.5, 0.5, 1);
	int whiteThresh = 120;
	int greenThresh = 56;
	int blueThresh = 16;
	vector<Vector4> col;
	vector<Vector3> pts;
	vector<Vector3> norm;
	vector<Vector2> tex;
	for (int x = 0; x <= chunkSize - 1; x++) {
		int globalX = worldX * chunkSize + x;
		for (int y = 0; y <= chunkSize - 1; y++) {
			int globalY = worldY * chunkSize + y;
			for (int z = 0; z <= chunkSize - 1; z++) {
				int globalZ = worldZ * chunkSize + z;


				bool* cubes = new bool[8]{
					noise[mapPosition(x,y,z)],noise[mapPosition(x + 1,y,z)], noise[mapPosition(x + 1,y,z + 1)], noise[mapPosition(x,y,z + 1)],
					noise[mapPosition(x,y + 1,z)], noise[mapPosition(x + 1,y + 1,z)], noise[mapPosition(x + 1,y + 1,z + 1)], noise[mapPosition(x,y + 1,z + 1)]
				};

				int cubeIndex = 0;
				int shifter = 0;

				for (int i = 0; i < 8; i++) {
					if (cubes[i]) {
						cubeIndex |= (1 << shifter);
					}
					shifter++;
				}
				delete[] cubes;
				//float interp = (1 / (max(cubes[x], cubes[x+1]) / min(cubes[x], cubes[x+1])));
				for (int i = 0; i < 16; i++) { //For all results in the tri table
					int val = triTable[cubeIndex][i];
					if (val != -1) { //An active position
						switch (val) { //Get the "Position" and turn it into a real 3D coord
						case(0):
							pts.emplace_back(Vector3(globalX + 0.5, globalY, globalZ));
							break;
						case(1):
							pts.emplace_back(Vector3(globalX + 1, globalY, globalZ + 0.5));
							break;
						case(2):
							pts.emplace_back(Vector3(globalX + 0.5, globalY, globalZ + 1));
							break;
						case(3):
							pts.emplace_back(Vector3(globalX, globalY, globalZ + 0.5));
							break;
						case(4):
							pts.emplace_back(Vector3(globalX + 0.5, globalY + 1, globalZ));
							break;
						case(5):
							pts.emplace_back(Vector3(globalX + 1, globalY + 1, globalZ + 0.5));
							break;
						case(6):
							pts.emplace_back(Vector3(globalX + 0.5, globalY + 1, globalZ + 1));
							break;
						case(7):
							pts.emplace_back(Vector3(globalX, globalY + 1, globalZ + 0.5));
							break;
						case(8):
							pts.emplace_back(Vector3(globalX, globalY + 0.5, globalZ));
							break;
						case(9):
							pts.emplace_back(Vector3(globalX + 1, globalY + 0.5, globalZ));
							break;
						case(10):
							pts.emplace_back(Vector3(globalX + 1, globalY + 0.5, globalZ + 1));
							break;
						case(11):
							pts.emplace_back(Vector3(globalX, globalY + 0.5, globalZ + 1));
							break;
						}
						if (globalY >= whiteThresh) {
							col.emplace_back(white);
						}
						else if (globalY >= greenThresh) {
							col.emplace_back(green);
						}
						else if (globalY >= blueThresh) {
							col.emplace_back(blue);
						}
						else {
							col.emplace_back(grey);
						}
					}
				}
			}
		}
	}
	for (int i = 0; i < pts.size(); i += 3) {
		Vector3 a = pts[i];
		Vector3 b = pts[i + 1];
		Vector3 c = pts[i + 2];
		Vector3 normal = Vector3::Cross(b - a, c - a);
		normal.Normalise();
		norm.emplace_back(normal);
		norm.emplace_back(normal);
		norm.emplace_back(normal);
		tex.emplace_back(Vector2(0, 0));
		tex.emplace_back(Vector2(1, 0));
		tex.emplace_back(Vector2(0.5, 1));
	}
	OGLMesh* points = new OGLMesh();
	points->SetVertexPositions(pts);
	points->SetVertexColours(col);
	points->SetVertexNormals(norm);
	points->SetVertexTextureCoords(tex);
	points->SetPrimitiveType(GeometryPrimitive::Triangles);
	points->UploadToGPU();
	RenderObject* rObj = new RenderObject(points);
	return rObj;
}