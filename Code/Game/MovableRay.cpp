#include "Game/MovableRay.hpp"
#include "Game/ConvexShape.hpp"

#include "Engine/Math/MathUtils.hpp"

MovableRay::MovableRay(Game* the_game): Entity(the_game)
{
	m_segment.SetStart(Vec2(25.0f, 25.0f));
	m_segment.SetEnd(Vec2(26.0f, 25.0f));
	m_ray = Ray2::FromPoints(Vec2(25.0f, 25.0f), Vec2(26.0f, 25.0f));
	
	m_position = m_segment.GetCenter();
	m_orientationDegrees = m_segment.GetRotation();
	m_scale = 0.50f;

	m_material = g_theRenderer->CreateOrGetMaterial("white.mat");
}


MovableRay::~MovableRay()
{
	if (m_mesh)
	{
		delete m_mesh;
		m_mesh = nullptr;
	}
}


void MovableRay::Update(float delta_seconds)
{
	if(m_mesh)
	{
		delete m_mesh;
		m_mesh = nullptr;
	}

	//hand drawing the line in world space
	m_position = m_segment.GetCenter();
	m_orientationDegrees = m_segment.GetRotation();
	m_ray = Ray2::FromPoints(m_segment.m_start, m_segment.m_end);

	ConstructArrow();
	
}


void MovableRay::Render() const
{
	if (!m_isDead)
	{
		//since we are constructing the line in world space
		const Matrix44 model_matrix = Matrix44::IDENTITY;

		g_theRenderer->BindModelMatrix(model_matrix);
		g_theRenderer->BindMaterial(*m_material);
		g_theRenderer->DrawMesh(*m_mesh);

	}
}


void MovableRay::Die()
{
}


void MovableRay::Revive()
{
}


bool MovableRay::InWorldBounds() const
{
	return true;
}


void MovableRay::DrawEntity() const
{
}


bool MovableRay::DestroyEntity()
{
	return false;
}

void MovableRay::SetStart(const Vec2& pos)
{
	m_segment.m_start = pos;
}

void MovableRay::SetEnd(const Vec2& pos)
{
	m_segment.m_end = pos;
}

void MovableRay::CollideWithConvexShape(const ConvexShape2D& shape)
{
	std::vector<Segment2> segments = shape.GetConvexSegments();
	std::vector<Vec2> points = shape.GetConvexPoints();

	Matrix44 shape_local = shape.GetModelMatrix().GetInverseMatrix();
	Vec2 local_pos = shape_local.GetTransformPosition2D(m_segment.GetStart());
	Vec2 local_dir = shape_local.GetTransformVector2D(m_ray.m_dir);

	//if(m_segment.m_start)
	
	for(int plane_idx = 0; plane_idx < static_cast<int>(segments.size()); ++plane_idx)
	{
		float t_vals[2];

		Ray2 temp_array(local_pos, local_dir);

		uint num_hits = Raycast(t_vals, temp_array, segments[plane_idx]);

		if(num_hits > 0)
		{
			Vec2 local_hit = temp_array.PointAtTime(t_vals[0]);
			Matrix44 shape_world = shape.GetModelMatrix();
			Vec2 world_hit = shape_world.GetTransformPosition2D(local_hit);
			m_segment.m_end = world_hit;
		}
	}
}

void MovableRay::ConstructArrow()
{
	CPUMesh arrow_mesh;
	CpuMeshAddArrow(&arrow_mesh, m_rayCastColor, m_segment.m_start, m_segment.m_end, 0.25f);
	m_mesh = new GPUMesh(g_theRenderer);
	m_mesh->CreateFromCPUMesh<Vertex_PCU>(arrow_mesh); // we won't be updated this
}




