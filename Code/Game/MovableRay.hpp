#pragma once
#include "Game/Entity.hpp"

#include "Engine/Math/Segment2.hpp"

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

private:
	void ConstructArrow();
	
private:
	Segment2 m_segment;

	Rgba m_rayCastColor = Rgba::MAGENTA;
	Rgba m_segmentColor = Rgba::GRAY;
};
