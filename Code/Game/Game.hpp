#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Renderer/RenderContext.hpp"

class Camera;
class Shader;
class GPUMesh;
class Material;

class Game
{
public:
	bool m_inDevMode;


public:
	Game();
	~Game();
	void Startup();
	void Update(double delta_seconds);
	void UpdateImGUI() const;
	void Render() const;
	void Shutdown();
	bool HandleKeyPressed(unsigned char key_code);
	bool HandleKeyReleased(unsigned char key_code);
	void SetDeveloperMode(bool on_or_off);
	void GarbageCollection() const;
	void MoveCamera(const Vec3& local_movement);

private:
	
	Camera* m_gameCamera = nullptr;

	Material* m_normalsMaterial = nullptr;
	Material* m_tangentsMaterial = nullptr;
	Material* m_bitangentsMaterial = nullptr;
	Material* m_couchMaterial = nullptr;
	Material* m_woodMaterial = nullptr;
	Material* m_pebblesMaterial = nullptr;
 	Shader* m_defaultShader = nullptr;
 	Shader* m_worldNormalsShader = nullptr;
	int m_testshaders = 0; //0 is color, 1 is normal, 2 is tangent, 3 is bitangents

	float m_time;
	float m_spawnNewPointEvery = 0.125f;
	float m_cameraTime = 0.0f;
	int m_currentFrame = 0;
	float m_mouseMovementScale = 0.05f;
	float m_cameraMovementScale = 64.0f;
	float m_deltaMovement = 1.0f;

	Vec3 m_camPosition		= Vec3(0.0f, 0.0f, -10.0f); // we'll start 10 back; 
	Vec3 m_camEuler			= Vec3(0.0f, 0.0f, 0.0f); 
	float m_camFOVDegrees	= 90.0f;

	GPUMesh* m_cube; 
	Matrix44 m_cubeTransform = Matrix44::IDENTITY; // cube's mode l matrix

	GPUMesh* m_quad; 
	Matrix44 m_quadTransform = Matrix44::IDENTITY; // cube's model matrix

	GPUMesh* m_sphere;
	Matrix44 m_sphereTransform = Matrix44::IDENTITY; // cube's model matrix
	
	LightT m_pointLights1;
	LightT m_pointLights2;
	LightT m_pointLights3;
	LightT m_pointLights4;
	Vec3 m_pl1Position = Vec3::ZERO;
	Vec3 m_pl2Position = Vec3::ZERO;
	Vec3 m_pl3Position = Vec3::ZERO;
	Vec3 m_pl4Position = Vec3::ZERO;
};
