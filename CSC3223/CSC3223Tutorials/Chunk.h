#pragma once
#include "../../Common/TextureLoader.h"
#include "../../Common/Vector3.h"
#include "../../Common/Vector4.h"
#include "../../Common/MeshGeometry.h"
#include "../../Common/Maths.h"
#include <map>

#include "../../Plugins/OpenGLRendering/OGLRenderer.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "Renderer.h"

namespace NCL {
	namespace CSC3223 {
		class Chunk
		{
		public:
			Chunk();
			Chunk(int x, int y, int z, int size);
			~Chunk();

			RenderObject* renderChunkCPU(int triTable[256][16]);
			RenderObject* renderChunkGPU();

			Vector3 getPosition() {
				return Vector3(worldX, worldY, worldZ);
			}

			int mapPosition(int x, int y, int z);
			int mapPosition(int x, int z);

			void setNoise(bool* n) {
				noise = n;
			};

			bool* getNoise() {
				return noise;
			}

		private:
			int worldX, worldY, worldZ; //Positions
			int chunkSize = 16;
			bool* noise;
			float floorVal = 0.4;
		};
	};
}