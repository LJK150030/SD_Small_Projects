#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Renderer/RenderContext.hpp"

class Camera;
class Shader;
class GPUMesh;
class Material;

class Game
{

public:
	Game();
	~Game();

	void Startup();
	void Shutdown();

	void BeginFrame() const;
	void Update(double delta_seconds);

	void Render() const;

	void EndFrame() const;

	bool HandleKeyPressed(unsigned char key_code);
	bool HandleKeyReleased(unsigned char key_code);
	void SetDeveloperMode(bool on_or_off);

private:
	void GarbageCollection() const;
	void InitCamera();
	void InitGameObjs();

private:

	bool m_inDevMode = false;
	float m_time = 0.0f;
	int m_currentFrame = 0;

	Camera* m_gameCamera = nullptr;
	Vec3 m_camPosition = Vec3(0.0f, 0.0f, 0.0f);  
	Vec3 m_camEuler = Vec3(0.0f, 0.0f, 0.0f);
	float m_camFOVDegrees = 90.0f;

	Material* m_woodMaterial = nullptr;
	Shader* m_defaultShader = nullptr;
	GPUMesh* m_quad;
	Matrix44 m_quadTransform = Matrix44::IDENTITY; // quad's model matrix

	
	Vec2 m_mousePos = Vec2::ZERO;
};
