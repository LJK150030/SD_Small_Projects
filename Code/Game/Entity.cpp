#include "Game/Entity.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/VertexUtils.hpp"

Entity::Entity(Game* the_game)
{
	m_game = the_game;
}

Entity::~Entity()
{
}

Vec2& Entity::GetPosition() const
{
	return *m_position;
}
