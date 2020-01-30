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
	virtual void InBounds() = 0;
	virtual void DrawEntity() const = 0;
	virtual bool DestroyEntity() = 0;

	Vec2& GetPosition() const;

protected:
	Game* m_game = nullptr;
	Vec2* m_position = nullptr;			// the Entity's 2D (x,y) Cartesian origin/center location, in world space 
	float m_orientationDegrees = 0.0f;	// the Entity's forward-facing direction, as an angle in degrees
	bool m_isDead = false;				// whether the Entity should [not] participate in game logic
	bool m_isGarbage = false;			// whether the Entity should be deleted at the end of Game::Update()
};
