#pragma once
#include "Engine/Math/Plane2.hpp"

#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"

#include <vector>

struct ConvexHull2D;
struct ConvexPolygon2D;

struct ConvexHull2D
{
public:
	std::vector<Plane2> m_planes;
	int	m_numPlanes = 0;

public:
	ConvexHull2D();
	~ConvexHull2D();
	
	ConvexHull2D(const ConvexPolygon2D& poly);
	
	void Update(const Vec2& mouse_position, bool render_this_frame);
	void DebugRender(const Matrix44& model_matrix) const;

private:
	bool m_renderFrame = false;
	
	std::vector<GPUMesh*> m_debugLineMeshList;
	std::vector<GPUMesh*> m_debugPointMeshList;
};


//--------------------------------------------------------------------

struct ConvexPolygon2D
{
public:
	std::vector<Vec2> m_points; //in counter_clockwise order
	
public:
	ConvexPolygon2D();
	~ConvexPolygon2D();

	explicit ConvexPolygon2D(const ConvexHull2D& hull);

private:
	void RandomCcwPoints(std::vector<Vec2>& out) const;
	
private:
	const float MIN_RNG_ANGLE = 10.0f;
	const float MAX_RNG_ANGLE = 170.0f;
};

//--------------------------------------------------------------------

class ConvexShape2D: public Entity
{
public:
	explicit ConvexShape2D(Game* the_game);
	~ConvexShape2D();

	void Update(float delta_seconds) override;
	void Render() const override;
	void Die() override;
	void Revive() override;
	bool InWorldBounds() const override;
	void DrawEntity() const override;
	bool DestroyEntity() override;

	bool CollisionFromPoint(const Vec2& pos);

	void AddRotationDegrees(float degrees);
	void AddScalarValue(float scale);

	bool IsPointInsideShape(const Vec2& pos) const;
	bool IsPointInsideShapeIgnorePlane(const Vec2& pos, int plane_idx) const;

	std::vector<Plane2> GetLocalConvexPlanes() const;
	std::vector<Vec2> GetLocalConvexPoints() const;
	std::vector<Segment2> GetLocalConvexSegments() const;

	std::vector<Segment2> GetWorldConvexSegments() const;


private:
	ConvexHull2D		m_hull;
	ConvexPolygon2D		m_polygon;

	Rgba m_color = Rgba(0.0980392156862745f,
		1.0000000000000000f,
		0.0980392156862745f,
		0.3200000000000000f);
	
	Rgba m_debugColor = Rgba(
		0.0784313725490196f,
		0.7098039215686275f,
		0.8000000000000000f, 
		0.2f
	);
	
	Rgba m_collideColor = Rgba(
		1.0000000000000000f, 
		0.8980392156862745f,
		0.0980392156862745f,
		0.2f
	);
	
	bool m_collideThisFrame = false;
	
	const float MIN_SIZE = 5.0f;
	const float MAX_SIZE = 15.0f;
	
	float m_minX = WORLD_BL_CORNER.x;
	float m_maxX = WORLD_TR_CORNER.x;
	float m_minY = WORLD_BL_CORNER.y;
	float m_maxY = WORLD_TR_CORNER.y;

	Vec2 m_pointLocalPos = Vec2::ZERO;
};