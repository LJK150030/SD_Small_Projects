#include "Game/Entity.hpp"
#include "Game/Game.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Shader.hpp"



Entity::Entity(Game* the_game): m_game(the_game)
{
	m_material = g_theRenderer->CreateOrGetMaterial("white.mat");
	m_material->m_shader->SetDepth(COMPARE_LESS_EQUAL, true);
}

Entity::~Entity() = default;

Vec2 Entity::GetPosition() const
{
	return m_position;
}

float Entity::GetOrientationDegrees() const
{
	return m_orientationDegrees;
}

float Entity::GetOrientationRadians() const
{
	return m_orientationDegrees * DEGREES_TO_RADIANS_SCALE;
}

float Entity::GetScale() const
{
	return m_scale;
}

Matrix44 Entity::GetModelMatrix() const
{
	Matrix44 translation = Matrix44::MakeTranslation2D(m_position);
	Matrix44 rotation = Matrix44::MakeZRotationDegrees(m_orientationDegrees);
	Matrix44 scale = Matrix44::MakeUniformScale2D(m_scale);

	return scale * rotation * translation;
}

bool Entity::IsDead() const
{
	return m_isDead;
}
