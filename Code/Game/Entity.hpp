#pragma once
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Renderer/GPUMesh.hpp"

struct	Rgba;
struct	Vec2;
struct	Vertex_PCU;
class	Game;

class Entity
{
public:
	explicit Entity(Game* the_game);
	virtual ~Entity();

	virtual void Update(float delta_seconds) = 0;
	virtual void Render() const = 0;

	virtual void Die() = 0;
	virtual void Revive() = 0;
	virtual bool InWorldBounds() const = 0;
	virtual void DrawEntity() const = 0;
	virtual bool DestroyEntity() = 0;

	Vec2		GetPosition() const;
	float		GetOrientationDegrees() const;
	float		GetOrientationRadians() const;
	float		GetScale() const;
	Matrix44	GetModelMatrix() const;
	bool		IsDead() const;

protected:
	Game* m_game = nullptr;
	
	Vec2 m_position = Vec2::ZERO;			// the Entity's 2D (x,y) Cartesian origin/center location, in world space 
	float m_orientationDegrees = 0.0f;	// the Entity's forward-facing direction, as an angle in degrees
	float m_scale = 0.0f;
	
	bool m_isDead = false;				// whether the Entity should [not] participate in game logic
	bool m_isGarbage = false;			// whether the Entity should be deleted at the end of Game::Update()

	GPUMesh* m_mesh = nullptr;
	Material* m_material = nullptr;

	GPUMesh* m_debugMesh = nullptr;
};
