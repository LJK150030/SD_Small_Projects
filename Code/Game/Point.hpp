#pragma once
#include "Game/Entity.hpp"

class Point: public Entity
{
public:
	explicit Point(Game* the_game);
	~Point();

	void Update(float delta_seconds) override;
	void Render() const override;

	void Die() override;
	void Revive() override;
	bool InWorldBounds() const override;
	void DrawEntity() const override;
	
	bool DestroyEntity() override;
};
