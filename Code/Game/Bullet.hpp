#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"

class Game;

class Bullet : public Entity
{

public:

	Bullet(Map* mapInstance, Vec2 const& spawnPosition, float spawnDegrees);
	~Bullet();

	void					StartUp();
	virtual void			Update(float deltaSecond) override;
	virtual void			Render() override;
	virtual void			Die() override;
	virtual void			HitByBullet(Entity* bullet) override;
	virtual void			GetDamage(int damageValue) override;
	virtual IntVec2			GetTilePosition() const override;
	void					CheckBounceOffWall(float deltaSecond);
	void					BounceAgainst(Vec2 const& normal);

private:
	Vec2					m_startVelocity;
	Texture*				m_bulletTexture;
};

