#pragma once
#include "../../Plugins/OpenGLRendering/OGLRenderer.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"

#include "RenderObject.h"
#include "../../Common/Vector2.h"
#include "Renderer.h"

namespace NCL {
	namespace CSC3223 {

		class Renderer : public OGLRenderer
		{
		public:
			Renderer(Window& w);
			~Renderer();

			struct Light {
				Vector3 position;
				float radius;
				Vector3 colour;
			};

			struct tri_table {
				int table[256][16];
			};

			void AddRenderObject(RenderObject* ro) {
				renderObjects.emplace_back(ro);
			}

			void DeleteAllRenderObjects() {
				for (const RenderObject* object : renderObjects) {
					delete object;
				}
				renderObjects.clear();
			}

			vector<RenderObject*> getRenderObjects() {
				return renderObjects;
			}

			void DeleteLastRenderObject() {
				if (renderObjects.size() > 0) {
					delete renderObjects.back();
					renderObjects.pop_back();
				}
			}

			void RemoveRenderObject(RenderObject* c) {
				for (int i = 0; i < renderObjects.size(); i++) {
					if (renderObjects[i] == c) {
						renderObjects.erase(renderObjects.begin() + i);
						break;
					}
				}
			}

			void RemoveAllRenderObjects() {
				renderObjects.clear();
			}

			void SetProjectionMatrix(const Matrix4& m) {
				projMatrix = m;
			}

			void SetViewMatrix(const Matrix4& m) {
				viewMatrix = m;
			}

			void EnableDepthBuffer(bool state) {
				if (state) {
					glEnable(GL_DEPTH_TEST);
				}
				else {
					glDisable(GL_DEPTH_TEST);
				}
			};

			void WriteDepthBuffer(const string& filepath) const;

			void EnableBilinearFiltering(OGLTexture& t);
			void DisableBilinearFiltering(OGLTexture& t);
			void EnableMipMapFiltering(OGLTexture& t);
			void DisableMipMapFiltering(OGLTexture& t);
			void EnableTextureRepeating(OGLTexture& t, bool uRepeat, bool vRepeat);

			void EnableAlphaBlending(bool state);

			void setBlendToLinear();
			void setBlendToAdditive();
			void setBlendToInvert();

			void SetLightProperties(Vector3 pos, float radius, Vector3 colour);

			void setTriTable(int inp[256][16]);

		protected:
			void ApplyLightToShader(const Light& l, const OGLShader* s);

			GLuint ssbo;
			GLuint binding = 0;

			Light activeLight;
			tri_table tri;
			GameTimer frameTimer;

			void RenderNode(RenderObject* root);

			void OnWindowResize(int w, int h)	override;

			void RenderFrame()	override;
			OGLShader* defaultShader;

			Matrix4		projMatrix;
			Matrix4		viewMatrix;

			vector<RenderObject*> renderObjects;

		};
	}
}

