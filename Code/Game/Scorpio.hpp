#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/Player.hpp"

class Game;
struct Turret;

class Scorpio : public Entity
{

public:

	Scorpio(Map* mapInstance, Vec2 const& spawnPosition, float spawnDegrees);
	~Scorpio();

	virtual void			StartUp() override;
	virtual void			Update(float deltaSecond) override;
	void					UpdateActions(float deltaSecond);

	virtual void			Render() override;
	void					RenderTurretBase() const;
	void					RenderTurretCannon() const;

	virtual void			Die() override;
	virtual void			HitByBullet(Entity* bullet) override;
	virtual void			GetDamage(int damageValue) override;
	virtual IntVec2			GetTilePosition() const override;

	Entity*					GetTarget() const;
	void					SetTarget(Entity* target);
	void					DrawRay(Vec2 const& startPos, Vec2 const& endPos, float thickness, Rgba8 const& color);
	Vec2					GetTurretOrientation();
	Turret					m_turret;

private:
	Entity*					m_target;
	Vec2					m_raycastMaxPointActual;
	float					m_shootTimer;
	bool					m_canShoot = true;
	bool					m_canSeePlayer = false;
};

