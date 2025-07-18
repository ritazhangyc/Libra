#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"

class Game;
class Tile;
class TileHeatMap;

class Aries : public Entity
{

public:

	Aries(Map* mapInstance, Vec2 const& spawnPosition, float spawnDegrees);
	~Aries();

	virtual void			StartUp() override;
	virtual void			Update(float deltaSecond) override;
	void					UpdateTarget(float deltaSecond);
	void					UpdateActions(float deltaSecond);
	virtual void			Render() override;
	virtual void			RenderTank() const;
	void					RenderHeatMap() const;
	virtual void			Die() override;
	virtual void			HitByBullet(Entity* bullet) override;
	virtual void			GetDamage(int damageValue) override;
	virtual IntVec2 		GetTilePosition() const override;

	Entity*					GetTarget() const;
	Vec2					GetLastSeenPosition() const;
	Vec2					GetNextWayPointPosition() const;
	void					GenerateRandomTileIndexMap();
	void					GenerateNextWayPointPosition();
	void					GetPlayerCurrentTilePosition();
	void					SetTarget(Entity* target);

private:
	Entity* m_target;

	float					m_targetOrientationDegree;

	bool					m_canSeePlayer = false;
	bool					m_reachLastSeenPosition = true;
	Vec2					m_targetPosition;
	Vec2					m_nextWaypointPosition;
	std::vector<Vec2>		m_pathPoints;

	TileHeatMap*			m_distanceFieldHeatMap;
	int						m_targetTileIndex;
};

