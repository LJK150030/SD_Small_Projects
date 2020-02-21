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
	void SetEnd(float ray_t_val);

	void PreUpdate();
	bool CollideWithConvexShape(float* out, const ConvexShape2D& shape);

private:
	void ConstructArrow();
	bool CollideWithDisk(const ConvexShape2D& shape);
	
private:
	Segment2 m_raySegment;
	Ray2 m_ray;
	
	Segment2 m_debugSegment;
	
	Segment2 m_reflectingRaySegment;
	Ray2 m_reflectingRay;

	Rgba m_rayCastColor = Rgba::MAGENTA;
	Rgba m_segmentColor = Rgba::GRAY;
	Rgba m_reflectColor = Rgba::RED;

	bool m_hitThisFrame = false;

	GPUMesh* m_reflectingMesh = nullptr;
	Vec2 m_reflectingPos;
	float m_reflectingOrientationDeg;
};
