#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/Point.hpp"

class Camera;
class Shader;
class GPUMesh;
class Material;
class ConvexShape2D;

class Game
{

public:
	Game();
	~Game();

	void Startup();
	void Shutdown();

	void BeginFrame() const;
	void Update(double delta_seconds);
	void UpdateEntities(double delta_seconds);

	void Render() const;
	void RenderEntities() const;

	void EndFrame() const;

	bool HandleKeyPressed(unsigned char key_code);
	bool HandleKeyReleased(unsigned char key_code);
	void SetDeveloperMode(bool on_or_off);

	Vec2 GetMousePosition() const;
	bool InDeveloperMode() const;

private:
	void GarbageCollection() const;
	void InitCamera();
	void InitGameObjs();
	void UpdateNumberOfShapes();

	void MouseCollisionTest();
	
private:

	bool m_inDevMode = false;
	float m_time = 0.0f;
	int m_currentFrame = 0;

	Camera* m_gameCamera = nullptr;
	Vec3 m_camPosition = Vec3(0.0f, 0.0f, 0.0f);  
	Vec3 m_camEuler = Vec3(0.0f, 0.0f, 0.0f);

	Material* m_woodMaterial = nullptr;
	Shader* m_defaultShader = nullptr;
	GPUMesh* m_quad{};
	Matrix44 m_quadTransform = Matrix44::IDENTITY; // quad's model matrix

	Vec2 m_mousePos = Vec2::ZERO;
	Point m_mouseEntity;
	
	std::vector<ConvexShape2D*> m_convexShapes;
	int m_currentNumConvexShapes = 1;
	const int MIN_SHAPES = 1;
	const int MAX_SHAPES = 8'192;
};
