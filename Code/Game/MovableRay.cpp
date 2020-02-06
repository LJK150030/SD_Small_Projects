#include "Game/MovableRay.hpp"
#include "Engine/Math/MathUtils.hpp"

MovableRay::MovableRay(Game* the_game): Entity(the_game)
{
	m_segment.SetStart(Vec2(25.0f, 25.0f));
	m_segment.SetEnd(Vec2(26.0f, 25.0f));
	
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

void MovableRay::ConstructArrow()
{
	const float thickness = 0.25f;
	const Vec2 dir =			(m_segment.m_end - m_segment.m_start).GetNormalized();
	const Vec2 orthogonal =		dir.GetRotated90Degrees();

	const Vec2 top_left =		m_segment.m_start + thickness * orthogonal;
	const Vec2 bottom_left =	m_segment.m_start + -thickness * orthogonal;
	const Vec2 top_right =		m_segment.m_end - dir + thickness * orthogonal;
	const Vec2 bottom_right =	m_segment.m_end - dir -thickness * orthogonal;

 	const Vec2 arrow_bottom =	bottom_right - dir + -2.0f * thickness * orthogonal;
 	const Vec2 arrow_mid =		m_segment.m_end;
 	const Vec2 arrow_top =		top_right - dir + 2.0f * thickness * orthogonal;
	
	CPUMesh arrow_mesh;
	CpuMeshAddTriangle(&arrow_mesh, false, top_left, bottom_left, bottom_right, m_rayCastColor, 0);
	CpuMeshAddTriangle(&arrow_mesh, true, top_left, bottom_right, top_right, m_rayCastColor, 1);
	CpuMeshAddTriangle(&arrow_mesh, true, arrow_bottom, arrow_mid, arrow_top, m_rayCastColor, 2);

	m_mesh = new GPUMesh(g_theRenderer);
	m_mesh->CreateFromCPUMesh<Vertex_PCU>(arrow_mesh); // we won't be updated this
}


