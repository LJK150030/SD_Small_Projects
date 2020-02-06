#include "Game/GameCommon.hpp"
#include "Game/ConvexShape.hpp"
#include "Game/Game.hpp"

#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


ConvexShape2D::ConvexShape2D(Game* the_game): Entity(the_game)
{
	m_scale = g_randomNumberGenerator.GetRandomFloatInRange(m_minSize, m_maxSize);;
	m_position = Vec2(
		g_randomNumberGenerator.GetRandomFloatInRange(m_minX, m_maxX),
		g_randomNumberGenerator.GetRandomFloatInRange(m_minY, m_maxY));
	
	RandomCcwPoints(m_polygon.m_points);

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

	if(m_collideThisFrame)
	{
		color = m_collideColor;
	}
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
	
	return 	m_collideThisFrame;
}

void ConvexShape2D::RandomCcwPoints(std::vector<Vec2>& out) const
{
	out.clear();

	float rotation_degrees = 0.0f;

	while(rotation_degrees < 360.0f)
	{
		Vec2 start(1.0f, 0.0f);
		start.SetAngleDegrees(rotation_degrees);
		start.Normalize();
		out.emplace_back(start);

		const float add_rot = g_randomNumberGenerator.GetRandomFloatInRange(m_minAngle, m_maxAngle);
		rotation_degrees += add_rot;
	}
}
