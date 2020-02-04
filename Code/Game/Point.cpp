#include "Engine/Math/MathUtils.hpp"

#include "Game/Point.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


Point::Point(Game* the_game): Entity(the_game)
{
	m_scale = 0.1f;

	CPUMesh disc_mesh;
	CpuMeshAddDisc(&disc_mesh, Rgba::CYAN, 1.0f);
	m_mesh = new GPUMesh(g_theRenderer);
	m_mesh->CreateFromCPUMesh<Vertex_PCU>(disc_mesh); // we won't be updated this;
}


Point::~Point()
{
	delete m_mesh;
	m_mesh = nullptr;
}


void Point::Update(float delta_seconds)
{
	UNUSED(delta_seconds)

	if(!m_isDead)
	{
		m_position = m_game->GetMousePosition();
	}
}


void Point::Render() const
{
	if (!m_isDead && InWorldBounds())
	{
		const Matrix44 model_matrix = GetModelMatrix();
		g_theRenderer->BindModelMatrix(model_matrix);
		g_theRenderer->BindMaterial(*m_material);
		g_theRenderer->DrawMesh(*m_mesh);
	}
}


void Point::Die()
{
	m_isDead = true;
}

void Point::Revive()
{
	m_isDead = true;
}


bool Point::InWorldBounds() const
{
	return IsPointInAABB2(m_position, WORLD_BOUNDS);
}


void Point::DrawEntity() const
{
}

bool Point::DestroyEntity()
{
	return false;
}
