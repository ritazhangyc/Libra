#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"

class Game;
class TileHeatMap;

class Leo : public Entity
{

public:

	Leo(Map* mapInstance, Vec2 const& spawnPosition, float spawnDegrees);
	~Leo();

	virtual void			StartUp() override;

	virtual void			Update(float deltaSecond) override;
	void					UpdateTarget(float deltaSecond);
	void					UpdateActions(float deltaSecond);
	virtual void			Render() override;
	void					RenderTankBase();

	virtual void			Die() override;
	virtual void			HitByBullet(Entity* bullet) override;
	virtual void			GetDamage(int damageValue) override;
	virtual IntVec2 		GetTilePosition() const override;

	Entity*					GetTarget() const;
	Vec2					GetNextWayPointPosition() const;
	void					GenerateRandomTileIndexMap();
	void					GenerateNextWayPointPosition();
	void					GetPlayerCurrentTilePosition();
	Vec2					GetLastSeenPosition() const;
	void					SetTarget(Entity* target);

private:
	Entity*					m_target;
	float					m_targetOrientationDegree;
	float					m_timer = 0.0f;
	float					m_shootTimer;
	bool					m_canShoot = true;
	bool					m_canSeePlayer = false;
	bool					m_reachLastSeenPosition = true;
	Vec2					m_targetPosition;
	Vec2					m_nextWaypointPosition;
	std::vector<Vec2>		m_pathPoints;

	TileHeatMap*			m_distanceFieldHeatMap;
	int						m_targetTileIndex;

};

