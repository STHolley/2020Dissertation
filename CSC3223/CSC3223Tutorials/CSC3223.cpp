#include "../../Common/Window.h"
#include "../../Common/TextureLoader.h"
#include "../../Common/Vector3.h"
#include "../../Common/Vector4.h"
#include "../../Common/MeshGeometry.h"
#include "../../Common/Maths.h"
#include <string>
#include <chrono>

#include "../../Plugins/OpenGLRendering/OGLRenderer.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"

#include "Renderer.h"
#include <iostream>
#include "Chunk.h"
#include "ChunkManager.h"

using namespace NCL;
using namespace CSC3223;

Matrix4 setCameraView(Window* w, int type, float fov) {
	if (type == 1) {
		return Matrix4::Orthographic(0.0f, 1000.0f,
			-(w->GetScreenSize().x) * 0.1f,
			(w->GetScreenSize().x) * 0.1f,
			(w->GetScreenSize().y) * 0.1f,
			-(w->GetScreenSize().y) * 0.1f);
	}
	else {
		float aspect = w->GetScreenAspect();
		return Matrix4::Perspective(0.01f, 10000.0f, aspect, fov);
	}
}

int main() {
	Window* w = Window::CreateGameWindow("CSC3223 Tutorials!", 1000, 1000, false, 0, 3);
	if (!w->HasInitialised()) {
		return -1;
	}
	Renderer* renderer = new Renderer(*w);

	w->LockMouseToWindow(true);

	float fov = 90;
	float sensitivity = 5;

	glClearColor(0.5294117647, 0.80784313725, 0.92156862745, 1);

	Vector3 absolutePosition(0, -80, 0);
	Vector2 absoluteScreenPos(0, 0);

	int renderDistance = 8;
	ChunkManager cm(16, 2);

	renderer->setTriTable(cm.triTable);
	renderer->EnableMipMapFiltering((*(OGLTexture*)cm.newTex));
	bool firstRun = true;
	glEnable(GL_CULL_FACE);
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {

		Vector2 mouseMovement = w->GetMouse()->GetRelativePosition();
		absoluteScreenPos.x += (mouseMovement.x * sensitivity);
		absoluteScreenPos.y += (mouseMovement.y * sensitivity);
		absoluteScreenPos.y = absoluteScreenPos.y > 90 ? 90 : absoluteScreenPos.y;
		absoluteScreenPos.y = absoluteScreenPos.y < -90 ? -90 : absoluteScreenPos.y;

		Matrix4 rotationX = Matrix4::Rotation(absoluteScreenPos.y, Vector3(1, 0, 0));
		Matrix4 rotationY = Matrix4::Rotation(absoluteScreenPos.x, Vector3(0, 1, 0));
		Matrix4 totalRotation = rotationX * rotationY;

		float angleX = cos(absoluteScreenPos.x * PI / 180);
		float angleZ = sin(absoluteScreenPos.x * PI / 180);

		float speed = 0.5;
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_SHIFT) || Window::GetKeyboard()->KeyHeld(KEYBOARD_SHIFT)) {
			speed = 1.5;
		}
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_W) || Window::GetKeyboard()->KeyHeld(KEYBOARD_W)) {
			absolutePosition.z += speed * angleX;
			absolutePosition.x -= speed * angleZ;
		}
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_S) || Window::GetKeyboard()->KeyHeld(KEYBOARD_S)) {
			absolutePosition.z -= speed * angleX;
			absolutePosition.x += speed * angleZ;
		}
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_A) || Window::GetKeyboard()->KeyHeld(KEYBOARD_A)) {
			absolutePosition.x += speed * angleX;
			absolutePosition.z += speed * angleZ;
		}
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_D) || Window::GetKeyboard()->KeyHeld(KEYBOARD_D)) {
			absolutePosition.x -= speed * angleX;
			absolutePosition.z -= speed * angleZ;
		}
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_SPACE) || Window::GetKeyboard()->KeyHeld(KEYBOARD_SPACE)) {
			absolutePosition.y -= speed;
		}
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_TAB) || Window::GetKeyboard()->KeyHeld(KEYBOARD_TAB)) {
			absolutePosition.y += speed;
		}
		if (Window::GetMouse()->GetWheelMovement() == 1) {
			fov--;
		}
		if (Window::GetMouse()->GetWheelMovement() == -1) {
			fov++;
		}

		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_Q)) {
			cm.setRenderDistance(cm.getRenderDistance() + 1);
		}
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_E)) {
			cm.setRenderDistance(cm.getRenderDistance() - 1);
		}

		Matrix4 translation = Matrix4::Translation(absolutePosition);

		Matrix4 camera = totalRotation * translation;
		Matrix4 camOut = translation * totalRotation;
		renderer->SetViewMatrix(camera);

		renderer->SetLightProperties(-absolutePosition, 1000.0f, Vector3(1, 1, 1));
		unsigned __int64 tS = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		cm.renderNearby(absolutePosition, renderer, false);
		unsigned __int64 tE = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		unsigned __int64 elapsed = tE - tS;
		if (firstRun) {
			std::cout << elapsed << std::endl;
			firstRun = false;
		}
		
		renderer->SetProjectionMatrix(setCameraView(w, 0, fov));

		renderer->EnableDepthBuffer(true);
		renderer->setBlendToLinear();
		renderer->EnableAlphaBlending(true);

		float time = w->GetTimer()->GetTimeDelta();

		renderer->Update(time);

		int count = 0;
		for (RenderObject* ro : renderer->getRenderObjects()) {
			count += ro->GetMesh()->GetVertexCount();
		}

		renderer->DrawString(std::to_string(cm.getRenderDistance()), Vector2(10, 10));
		renderer->DrawString(std::to_string(count), Vector2(10, 90), Vector4(1, 1, 1, 1));

		renderer->Render();

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F11)) {
			renderer->WriteDepthBuffer("Depth.png");
		}
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_NEXT)) {
			w->ShowConsole(false);
		}

		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_HOME)) {
			w->SetFullScreen(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_END)) {
			w->SetFullScreen(false);
		}
		if (Window::GetKeyboard()->KeyPressed(KEYBOARD_ESCAPE)) {
			w->DestroyGameWindow();
		}

		w->SetTitle(std::to_string(time));
	}

	delete renderer;

	Window::DestroyGameWindow();
}
