#include "Renderer.h"
#include "Chunk.h"
#include "FastNoise.h"
#include "ChunkManager.h"

using namespace NCL;
using namespace CSC3223;

ChunkManager::ChunkManager(int cs, int rd)
{
	chunkSize = cs;
	RENDER_DISTANCE = rd;
}

int ChunkManager::mapPosition(int x, int y, int z) {
	return x + (y * (chunkSize + 1)) + (z * (chunkSize + 1) * (chunkSize + 1));
}

int ChunkManager::mapPosition(int x, int z) {
	return x + (z * (chunkSize + 1));
}

Chunk* ChunkManager::generateChunk(int xPos, int yPos, int zPos) {
	Chunk* c = new Chunk(xPos, yPos, zPos, chunkSize);
	float floorVal = 0.4;
	bool* noise = new bool[(chunkSize + 1) * (chunkSize + 1) * (chunkSize + 1)];
	for (int x = 0; x <= chunkSize; x++) {
		for (int y = 0; y <= chunkSize; y++) {
			for (int z = 0; z <= chunkSize; z++) {
				//heightmap goes from ocean floor to peaks
				float valHT = (0.5 + (1.0 + fn.GetSimplexFractal(xPos * chunkSize + x, zPos * chunkSize + z)) / 4) * (chunkSize * maxChunkHeight); //half max to max
				float valHB = (1.0 + fn.GetSimplexFractal(xPos * chunkSize + x, zPos * chunkSize + z)) * (chunkSize / 4);
				bool valP = false;
				if ((yPos * chunkSize + y) <= valHT) {
					float test = (1.0 + fn.GetSimplexFractal(xPos * chunkSize + x, yPos * chunkSize + y, zPos * chunkSize + z)) / 2.0; //Values bound between -1 and 1
					if (test > floorVal) {
						valP = true;
					}
					else {
						valP = false;
					}
					if (yPos * chunkSize + y <= valHB) {//Floor of the world
						valP = true;
					}
				}
				noise[mapPosition(x, y, z)] = valP;
			}
		}
	}
	c->setNoise(noise);
	return c;
}

void ChunkManager::generateNearby(Vector3 camera, bool useGPU) {
	camera /= chunkSize;
	camera = -camera;
	Vector3 change(RENDER_DISTANCE, RENDER_DISTANCE, RENDER_DISTANCE);
	Vector3 start = camera - change;
	Vector3 end = camera + change;
	for (int x = start.x; x < end.x; x++) {
		for (int y = start.y; y < end.y; y++) {
			if (y < 0 || y >= maxChunkHeight) continue;
			for (int z = start.z; z < end.z; z++) {
				Vector3 p(x, y, z);
				bool found = false;
				for (Chunk* check : generated) {
					if (check->getPosition() == p) {
						found = true;
					}
				}
				if (!found) {
					if (useGPU) {
						generated.emplace_back(new Chunk(x, y, z, chunkSize));
					}
					else {
						generated.emplace_back(generateChunk(x, y, z));
					}

				}
			}
		}
	}
}

void ChunkManager::renderNearby(Vector3 camera, Renderer* renderer, bool useGPU) {
	//camera = Vector3(0, camera.y, 0);
	generateNearby(camera, useGPU);
	renderer->RemoveAllRenderObjects();
	camera /= chunkSize;
	camera = -camera;
	Vector3 change = Vector3(RENDER_DISTANCE, RENDER_DISTANCE, RENDER_DISTANCE);
	Vector3 start = camera - change;
	Vector3 end = camera + change;
	for (int x = round(start.x); x < round(end.x); x++) {
		for (int y = round(start.y); y < round(end.y); y++) {
			for (int z = round(start.z); z < round(end.z); z++) {

				Vector3 p(x, y, z);
				bool found = false;
				int loc = 0;
				for (Chunk* load : generated) {
					if (load->getPosition() == p) {
						found = true;
						break;
					}
					else {
						loc++;
					}
				}
				if (found) {
					Chunk* curr = generated.at(loc);
					if (CtoRO[curr] != nullptr) {
						RenderObject* ro = CtoRO[curr];
						renderer->AddRenderObject(ro);
					}
					else {
						if (useGPU == true) {
							RenderObject* ro = curr->renderChunkGPU();
							ro->SetShader(lightingShaderGeom);
							ro->SetBaseTexture(newTex);
							renderer->AddRenderObject(ro);
							CtoRO[curr] = ro;
						}
						else {
							RenderObject* ro = curr->renderChunkCPU(triTable);
							ro->SetShader(lightingShader);
							ro->SetBaseTexture(newTex);
							renderer->AddRenderObject(ro);
							CtoRO[curr] = ro;
						}
					}
				}
			}
		}
	}
}