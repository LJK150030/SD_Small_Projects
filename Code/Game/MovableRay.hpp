#pragma once
#include "Game/Entity.hpp"

#include "Engine/Math/Segment2.hpp"
#include "Engine/Math/Ray2.hpp"

class ConvexShape2D;

class MovableRay: public Entity
{
public:
	explicit MovableRay(Game* the_game);
	~MovableRay();

	void Update(float delta_seconds) override;
	void Render() const override;
	void Die() override;
	void Revive() override;
	bool InWorldBounds() const override;
	void DrawEntity() const override;
	bool DestroyEntity() override;

	void SetStart(const Vec2& pos);
	void SetEnd(const Vec2& pos);

	void CollideWithConvexShape(const ConvexShape2D& shape);

private:
	void ConstructArrow();
	
private:
	Segment2 m_segment;
	Ray2 m_ray;

	Rgba m_rayCastColor = Rgba::MAGENTA;
	Rgba m_segmentColor = Rgba::GRAY;
};
