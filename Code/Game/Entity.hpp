#pragma once

struct Rgba;
struct Vec2;
struct Vertex_PCU;
class Game;

class Entity
{
public:
	explicit Entity(Game* the_game);
	virtual ~Entity();
	virtual void Update(float delta_seconds) = 0;
	virtual void Render() const = 0;
	virtual void Die() = 0;
	virtual void ApplySpeed(float speed) = 0;
	virtual void InBounds() = 0;
	virtual void DrawEntity() const = 0;
	virtual bool DestroyEntity() = 0;
	Vec2& GetPosition() const;

protected:
	Game* m_game = nullptr;
	Vec2* m_position = nullptr; // the Entity's 2D (x,y) Cartesian origin/center location, in world space 
	float m_physicsRadius = 0.0f; // the Entity's (inner, conservative) disc-radius for all physics purposes 
	float m_cosmeticRadius = 0.0f; // the Entity's (outer, liberal) disc-radius that fully encompasses it 
	float m_orientationDegrees = 0.0f; // the Entity's forward-facing direction, as an angle in degrees
	Vec2* m_velocity = nullptr; // the Entity's linear 2D (x,y) velocity, in world units per second
	float m_angularVelocity = 0.0f; // the Entity's angular velocity (spin rate), in degrees per second
	bool m_isDead = false; // whether the Entity should [not] participate in game logic
	bool m_isGarbage = false; // whether the Entity should be deleted at the end of Game::Update()
	const Rgba* m_physicsColor = nullptr;
	const Rgba* m_cosmeticColor = nullptr;
};
