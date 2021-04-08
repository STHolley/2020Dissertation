#include "Renderer.h"
#include "../../Common/TextureWriter.h"
#include "../../Common/Maths.h"
#include "../../Common/Matrix3.h"
using namespace NCL;
using namespace Rendering;
using namespace CSC3223;


Renderer::Renderer(Window& w) : OGLRenderer(w)
{
	defaultShader = new OGLShader("rasterisationVert.glsl", "rasterisationFrag.glsl");
	projMatrix = Matrix4::Orthographic(-1.0f, 1.0f, (float)currentWidth, 0.0f, 0.0f, (float)currentHeight);


}

Renderer::~Renderer()
{
	delete defaultShader;
}

void Renderer::RenderFrame() {
	OGLShader* activeShader = nullptr;

	int modelLocation = 0;

	for (const RenderObject* object : renderObjects) {
		OGLShader* objectShader = (OGLShader*)object->GetShader();
		if (!object->GetMesh()) {
			continue;
		}
		if (objectShader == nullptr) {
			objectShader = defaultShader;
		}
		if (objectShader != activeShader) {
			activeShader = objectShader;


			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


			BindShader(activeShader);
			int projLocation = glGetUniformLocation(activeShader->GetProgramID(), "projMatrix");
			int viewLocation = glGetUniformLocation(activeShader->GetProgramID(), "viewMatrix");
			modelLocation = glGetUniformLocation(activeShader->GetProgramID(), "modelMatrix");
			ApplyLightToShader(activeLight, activeShader);
			glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
			glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);
			int timeLocation = glGetUniformLocation(activeShader->GetProgramID(), "time");

			if (timeLocation >= 0.0) {
				float totalTime = (float)frameTimer.GetTotalTimeMSec() * 1000;
				glUniform1f(timeLocation, totalTime);
			}
		}

		Matrix4 mat = object->GetTransform();
		glUniformMatrix4fv(modelLocation, 1, false, (float*)&mat);

		BindTextureToShader(object->GetBaseTexture(), "mainTex", 0);
		BindTextureToShader(object->GetBaseTexture(), "triMap", 1);
		BindTextureToShader(object->GetSecondTexture(), "triMap", 1);
		BindMesh(object->GetMesh());
		DrawBoundMesh();
	}
}

void NCL::CSC3223::Renderer::WriteDepthBuffer(const string& filepath) const {
	float* data = new float[currentWidth * currentHeight];
	glReadPixels(0, 0, currentWidth, currentHeight, GL_DEPTH_COMPONENT, GL_FLOAT, data);

	char* pixels = new char[currentWidth * currentHeight * 3]; //3 for RGB
	char* pixelPointer = pixels;

	for (int y = 0; y < currentHeight; ++y) {
		for (int x = 0; x < currentWidth; ++x) {
			float depthValue = data[(y * currentWidth) + x];

			float mult = 1.0f / 0.333f;

			float redAmount = Maths::Clamp(depthValue, 0.0f, 0.333f) * mult;
			float greenAmount = (Maths::Clamp(depthValue, 0.333f, 0.666f) - 0.333f) * mult;
			float blueAmount = (Maths::Clamp(depthValue, 0.666f, 1.0f) - 0.666f) * mult;

			unsigned char redByte = (char)(redAmount * 255);
			unsigned char greenByte = (char)(greenAmount * 255);
			unsigned char blueByte = (char)(blueAmount * 255);

			*pixelPointer++ = (char)(redAmount * 255);
			*pixelPointer++ = (char)(greenAmount * 255);
			*pixelPointer++ = (char)(blueAmount * 255);
		}
	}
	TextureWriter::WritePNG(filepath, pixels, currentWidth, currentHeight, 3);

	delete pixels;
	delete data;
}

void Renderer::OnWindowResize(int w, int h) {
	OGLRenderer::OnWindowResize(w, h);
	projMatrix = Matrix4::Orthographic(-1.0f, 1.0f, (float)currentWidth, 0.0f, 0.0f, (float)currentHeight);
}

void Renderer::EnableBilinearFiltering(OGLTexture& t) {
	GLuint id = t.GetObjectID();
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::EnableMipMapFiltering(OGLTexture& t) {
	GLuint id = t.GetObjectID();
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::EnableTextureRepeating(OGLTexture& t, bool uRepeat, bool vRepeat) {
	GLuint id = t.GetObjectID();

	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uRepeat ? GL_REPEAT : GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, vRepeat ? GL_REPEAT : GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::EnableAlphaBlending(bool state) {
	if (state) {
		glEnable(GL_BLEND);
	}
	else {
		glDisable(GL_BLEND);
	}
}

void Renderer::setBlendToLinear() {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
void Renderer::setBlendToAdditive() {
	glBlendFunc(GL_ONE, GL_ONE);
}
void Renderer::setBlendToInvert() {
	glBlendFunc(GL_ONE_MINUS_SRC1_COLOR, GL_ONE_MINUS_DST_COLOR);
}

void Renderer::SetLightProperties(Vector3 pos, float radius, Vector3 colour)
{
	activeLight.position = pos;
	activeLight.radius = radius;
	activeLight.colour = colour;
}

void NCL::CSC3223::Renderer::setTriTable(int inp[256][16])
{
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 16; j++) {
			tri.table[i][j] = inp[i][j];
		}
	}
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(tri_table), tri.table, GL_STATIC_COPY);
}

void Renderer::ApplyLightToShader(const Light& l, const OGLShader* s)
{
	glUniform3fv(glGetUniformLocation(s->GetProgramID(), "lightColour"), 1, (float*)&l.colour);
	glUniform3fv(glGetUniformLocation(s->GetProgramID(), "lightPos"), 1, (float*)&l.position);
	glUniform1f(glGetUniformLocation(s->GetProgramID(), "lightRadius"), l.radius);

	Matrix3 rotation = Matrix3(viewMatrix);
	Vector3 invCamPos = viewMatrix.GetPositionVector();

	Vector3 camPos = rotation * -invCamPos;

	glUniform3fv(glGetUniformLocation(s->GetProgramID(), "cameraPos"), 1, (float*)&camPos);
}
