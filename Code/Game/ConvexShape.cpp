#include "Game/GameCommon.hpp"
#include "Game/ConvexShape.hpp"
#include "Game/Game.hpp"

#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


ConvexHull2D::ConvexHull2D()
{
	m_planes = std::vector<Plane2>();
	m_numPlanes = 0;
	
	m_debugLineMeshList = std::vector<GPUMesh*>();
	m_debugPointMeshList = std::vector<GPUMesh*>();
}


ConvexHull2D::~ConvexHull2D()
{
	for (int plane_idx = 0; plane_idx < static_cast<int>(m_debugLineMeshList.size()); ++plane_idx)
	{
		if (m_debugLineMeshList[plane_idx])
		{
			delete m_debugLineMeshList[plane_idx];
			m_debugLineMeshList[plane_idx] = nullptr;
		}
	}
	
	for (int plane_idx = 0; plane_idx < static_cast<int>(m_debugPointMeshList.size()); ++plane_idx)
	{
		if (m_debugPointMeshList[plane_idx])
		{
			delete m_debugPointMeshList[plane_idx];
			m_debugPointMeshList[plane_idx] = nullptr;
		}
	}
}


ConvexHull2D::ConvexHull2D(const ConvexPolygon2D& poly)
{
	m_numPlanes = static_cast<int>(poly.m_points.size());
	m_debugLineMeshList = std::vector<GPUMesh*>();
	m_debugLineMeshList.resize(m_numPlanes);
	m_debugPointMeshList = std::vector<GPUMesh*>();
	m_debugPointMeshList.resize(m_numPlanes);

	for(int point_idx = 0; point_idx < m_numPlanes; ++point_idx)
	{
		int point_ahead = (point_idx + 1) % m_numPlanes;
		
		Vec2 p1 = poly.m_points[point_idx];
		Vec2 p2 = poly.m_points[point_ahead];
		Plane2 plane = Plane2(p1, p2);
		
		m_planes.push_back(plane);
	}
}


void ConvexHull2D::Update(const Vec2& mouse_position, bool render_this_frame)
{
	m_renderFrame = render_this_frame;

	if(m_renderFrame)
	{
		for (int plane_idx = 0; plane_idx < m_numPlanes; ++plane_idx)
		{
			//first remove lines
			if (m_debugLineMeshList[plane_idx])
			{
				delete m_debugLineMeshList[plane_idx];
				m_debugLineMeshList[plane_idx] = nullptr;
			}

			//and remove discs
			if (m_debugPointMeshList[plane_idx])
			{
				delete m_debugPointMeshList[plane_idx];
				m_debugPointMeshList[plane_idx] = nullptr;
			}


			//replace lines
			Vec2 plane_dir = m_planes[plane_idx].m_normal.GetRotated90Degrees();
			Vec2 point_1_on_plane = m_planes[plane_idx].m_normal * m_planes[plane_idx].m_signedDistance;

			Vec2 far_point_1 = plane_dir * 100.0f + point_1_on_plane;
			Vec2 far_point_2 = plane_dir * -100.0f + point_1_on_plane;

			CPUMesh line_mesh;
			CpuMeshAddLine(&line_mesh, far_point_1, far_point_2, 0.025f, Rgba::MAGENTA);
			m_debugLineMeshList[plane_idx] = new GPUMesh(g_theRenderer);
			m_debugLineMeshList[plane_idx]->CreateFromCPUMesh<Vertex_PCU>(line_mesh);

			//replace intersection point
			Vec2 closest_point = m_planes[plane_idx].ClosestPoint(mouse_position);

			CPUMesh disc_mesh;
			CpuMeshAddDisc(&disc_mesh, Rgba::YELLOW, closest_point, 0.05f);
			m_debugPointMeshList[plane_idx] = new GPUMesh(g_theRenderer);
			m_debugPointMeshList[plane_idx]->CreateFromCPUMesh<Vertex_PCU>(disc_mesh);
		}
	}
}


void ConvexHull2D::DebugRender(const Matrix44& model_matrix) const
{
	if(m_renderFrame)
	{
		g_theRenderer->BindModelMatrix(model_matrix);

		for (int plane_idx = 0; plane_idx < m_numPlanes; ++plane_idx)
		{
			if (m_debugLineMeshList[plane_idx])
			{
				g_theRenderer->DrawMesh(*m_debugLineMeshList[plane_idx]);
			}

			if (m_debugPointMeshList[plane_idx])
			{
				g_theRenderer->DrawMesh(*m_debugPointMeshList[plane_idx]);
			}
		}
	}
}

//--------------------------------------------------------------------


ConvexPolygon2D::ConvexPolygon2D()
{
	m_points = std::vector<Vec2>();
	RandomCcwPoints(m_points);
}


ConvexPolygon2D::~ConvexPolygon2D() = default;


ConvexPolygon2D::ConvexPolygon2D(const ConvexHull2D& hull)
{
	UNUSED(hull);
	ASSERT_OR_DIE(false, "ConvexPolygon2D using a hull has not been made yet")
}


void ConvexPolygon2D::RandomCcwPoints(std::vector<Vec2>& out) const
{
	out.clear();

	float rotation_degrees = 0.0f;

	while (rotation_degrees < 360.0f)
	{
		Vec2 start(1.0f, 0.0f);
		start.SetAngleDegrees(rotation_degrees);
		start.Normalize();
		out.emplace_back(start);

		const float add_rot = g_randomNumberGenerator.GetRandomFloatInRange(MIN_RNG_ANGLE, MAX_RNG_ANGLE);
		rotation_degrees += add_rot;
	}
}


//--------------------------------------------------------------------


ConvexShape2D::ConvexShape2D(Game* the_game): Entity(the_game)
{
	m_scale = g_randomNumberGenerator.GetRandomFloatInRange(MIN_SIZE, MAX_SIZE);
	m_position = Vec2(
		g_randomNumberGenerator.GetRandomFloatInRange(m_minX, m_maxX),
		g_randomNumberGenerator.GetRandomFloatInRange(m_minY, m_maxY)
	);
	
	m_hull = m_polygon;

	int triangle_set = static_cast<int>(m_polygon.m_points.size()) - 2;

	CPUMesh convex_mesh;
	for(int convex_itr = 0; convex_itr < triangle_set; ++convex_itr)
	{
		CpuMeshAddTriangle(
			&convex_mesh,
			true,
			m_polygon.m_points[0],
			m_polygon.m_points[convex_itr + 1],
			m_polygon.m_points[convex_itr + 2],
			m_color,
			convex_itr);
	}

	m_mesh = new GPUMesh(g_theRenderer);
	m_mesh->CreateFromCPUMesh<Vertex_PCU>(convex_mesh); // we won't be updated this;
}


ConvexShape2D::~ConvexShape2D()
{
	delete m_mesh;
	m_mesh = nullptr;

	if(m_debugMesh)
	{
		delete m_debugMesh;
		m_debugMesh = nullptr;
	}
}


void ConvexShape2D::Update(float delta_seconds)
{
	UNUSED(delta_seconds);

	if (m_debugMesh)
	{
		delete m_debugMesh;
		m_debugMesh = nullptr;
	}

	Rgba color = m_debugColor;

	if(m_collideThisFrame && m_game->InDeveloperMode())
	{
		color = m_collideColor;
	}
	m_hull.Update(m_pointLocalPos, m_collideThisFrame);
	m_collideThisFrame = false;
	
	CPUMesh disc_mesh;
	CpuMeshAddDisc(&disc_mesh, color, 1.0f);
	m_debugMesh = new GPUMesh(g_theRenderer);
	m_debugMesh->CreateFromCPUMesh<Vertex_PCU>(disc_mesh);
	
}


void ConvexShape2D::Render() const
{
	if (!m_isDead)
	{
		const Matrix44 model_matrix = GetModelMatrix();
		
		g_theRenderer->BindModelMatrix(model_matrix);
		g_theRenderer->BindMaterial(*m_material);
		g_theRenderer->DrawMesh(*m_mesh);
		
		if(m_game->InDeveloperMode())
		{
			g_theRenderer->BindModelMatrix(model_matrix);
			g_theRenderer->BindMaterial(*m_material);
			g_theRenderer->DrawMesh(*m_debugMesh);

			m_hull.DebugRender(model_matrix);
		}
	}
}


void ConvexShape2D::Die()
{
}


void ConvexShape2D::Revive()
{
}


bool ConvexShape2D::InWorldBounds() const
{
	return IsPointInAABB2(m_position, WORLD_BOUNDS);
}


void ConvexShape2D::DrawEntity() const
{
}


bool ConvexShape2D::DestroyEntity()
{
	return false;
}

bool ConvexShape2D::CollisionFromPoint(const Vec2& pos)
{
	m_collideThisFrame = IsPointInDisc2D(pos, m_position, m_scale);
	
	Matrix44 model_matrix = GetModelMatrix();
	model_matrix = model_matrix.GetInverseMatrix();
	m_pointLocalPos = model_matrix.GetTransformPosition2D(pos);

	if(m_collideThisFrame)
	{
		m_collideThisFrame = IsPointInsideShape(pos);
	}
	
	return 	m_collideThisFrame;
}

void ConvexShape2D::AddRotationDegrees(float degrees)
{
	m_orientationDegrees = ModFloatPositive(m_orientationDegrees + degrees, 360.0f);
}

void ConvexShape2D::AddScalarValue(float scale)
{
	m_scale = ClampFloat(m_scale + scale, MIN_SIZE, MAX_SIZE);
}

std::vector<Plane2> ConvexShape2D::GetConvexPlanes() const
{
	return m_hull.m_planes;
}

std::vector<Vec2> ConvexShape2D::GetConvexPoints() const
{
	return m_polygon.m_points;
}

std::vector<Segment2> ConvexShape2D::GetConvexSegments() const
{
	std::vector<Segment2> list;
	int num_points = static_cast<int>(m_polygon.m_points.size());

	for(int point_idx = 0; point_idx < num_points; ++point_idx)
	{
		list.push_back(Segment2(m_polygon.m_points[point_idx], m_polygon.m_points[(point_idx + 1)%num_points]));
	}

	return list;
}

bool ConvexShape2D::IsPointInsideShape(const Vec2& pos) const
{
	Matrix44 shape_local = GetModelMatrix().GetInverseMatrix();
	Vec2 local_pos = shape_local.GetTransformPosition2D(pos);

	std::vector<Plane2> plane = GetConvexPlanes();

	for(int plane_idx = 0; plane_idx < static_cast<int>(plane.size()); ++plane_idx)
	{
		Vec2 closest_point = plane[plane_idx].ClosestPoint(local_pos);
		Vec2 dir = closest_point - local_pos;
		float compare = DotProduct(plane[plane_idx].m_normal, dir);

		if (compare < 0.0f)
		{
			return false;
		}
	}

	return true;
}

bool ConvexShape2D::IsPointInsideShapeIgnorePlane(const Vec2& pos, int plane_idx) const
{
	Matrix44 shape_local = GetModelMatrix().GetInverseMatrix();
	Vec2 local_pos = shape_local.GetTransformPosition2D(pos);

	std::vector<Plane2> plane = GetConvexPlanes();

	for (int idx = 0; idx < static_cast<int>(plane.size()); ++idx)
	{
		if(idx == plane_idx)
		{
			continue;
		}
		
		Vec2 closest_point = plane[idx].ClosestPoint(local_pos);
		Vec2 dir = closest_point - local_pos;
		float compare = DotProduct(plane[idx].m_normal, dir);

		if (compare < 0.0f)
		{
			return false;
		}
	}

	return true;
}
