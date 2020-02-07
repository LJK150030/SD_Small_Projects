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


void MovableRay::SetEnd(const float ray_t_val)
{
	m_segment.m_end = m_ray.PointAtTime(ray_t_val);
}


void MovableRay::PreUpdate()
{
	m_position = m_segment.GetCenter();
	m_orientationDegrees = m_segment.GetRotation();
	m_ray = Ray2::FromPoints(m_segment.m_start, m_segment.m_end);
	m_hitThisFrame = false;
}



bool MovableRay::CollideWithConvexShape(float* out, const ConvexShape2D& shape)
{
	std::vector<Plane2> planes = shape.GetConvexPlanes();

	//early out inside
	if(shape.IsPointInsideShape(m_ray.m_pos))
	{
		out[0] = 0.01f;
		return true;
	}

	const bool mid_check = CollideWithDisk(shape);
	
	if(mid_check)
	{
 		float largest_t_val = 0.0f;
 		int plane_intersection_idx = -1;
 
 		//check all planes if they intersect.
  		for(int plane_idx = 0; plane_idx < static_cast<int>(planes.size()); ++plane_idx)
  		{
  			float t_vals[2];
  			 
  			//world ray cast, to local ray cast
  			Matrix44 shape_local_space = shape.GetModelMatrix().GetInverseMatrix();
  			Vec2 local_pos = shape_local_space.GetTransformPosition2D(m_ray.m_pos);
  			Vec2 local_dir = shape_local_space.GetTransformVector2D(m_ray.m_dir);
  			
  			Ray2 local_ray_cast(local_pos, local_dir);
  
  			uint num_hits = Raycast(t_vals, local_ray_cast, planes[plane_idx]);
  
  			if(num_hits > 0)
  			{
  				if(t_vals[0] > largest_t_val)
  				{
  					largest_t_val = t_vals[0];
  					plane_intersection_idx = plane_idx;
  				}
  			}
  		}
 
 		//check if the point is in the hull, without the plan that it intersected.
 		if(plane_intersection_idx >= 0)
 		{
 			Vec2 contact_point = m_ray.PointAtTime(largest_t_val);
 			if(shape.IsPointInsideShapeIgnorePlane(contact_point, plane_intersection_idx))
 			{
				const float segment_end_t_value = m_segment.GetLength();

 				out[0] = ClampFloat(largest_t_val, 0.0f, segment_end_t_value);
				return true;
 			}
 		}

	}

	return false;
}


bool MovableRay::CollideWithDisk(const ConvexShape2D& shape)
{
	float t_vals[2];

	uint num_hits = Raycast(t_vals, m_ray, shape.GetPosition(), shape.GetScale());

	if(num_hits > 0)
	{
		return true;
	}

	return false;
}


void MovableRay::ConstructArrow()
{
	CPUMesh arrow_mesh;
	CpuMeshAddArrow(&arrow_mesh, m_rayCastColor, m_segment.m_start, m_segment.m_end, 0.25f);
	m_mesh = new GPUMesh(g_theRenderer);
	m_mesh->CreateFromCPUMesh<Vertex_PCU>(arrow_mesh); // we won't be updated this
}




