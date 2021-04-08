#pragma once
#include "../../Common/Matrix4.h"
#include "../../Common/TextureBase.h"
#include "../../Common/ShaderBase.h"

#include <vector>
namespace NCL {
	using namespace NCL::Rendering;
	class MeshGeometry;
	namespace CSC3223 {
		using namespace Maths;

		class RenderObject
		{
		public:
			RenderObject(MeshGeometry* mesh, Matrix4 m = Matrix4());
			~RenderObject();

			MeshGeometry* GetMesh() const {
				return mesh;
			}

			void SetMesh(MeshGeometry* m);

			void  SetTransform(Matrix4 mat) {
				transform = mat;
			}

			Matrix4 GetTransform() const {
				return transform;
			}

			void SetBaseTexture(TextureBase* t) {
				texture = t;
			}

			TextureBase* GetBaseTexture() const {
				return texture;
			}

			void SetSecondTexture(TextureBase* t) {
				secondTexture = t;
			}

			TextureBase* GetSecondTexture() const {
				return secondTexture;
			}

			void SetShader(ShaderBase* s) {
				shader = s;
			}

			ShaderBase* GetShader() const {
				return shader;
			}


		protected:
			MeshGeometry* mesh;
			TextureBase* texture;
			TextureBase* secondTexture;
			ShaderBase* shader;
			Matrix4			transform;
		};
	}
}



