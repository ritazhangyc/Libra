#pragma once
#include "Game/GameCommon.hpp"

class Game;
class Map;

enum class EntityType
{
	ENTITY_TYPE_GOOD_PLAYER = 0,
	_EVIL_SCORPIO,
	_EVIL_LEO,
	_EVIL_ARIES,
	_GOOD_BULLET,
	_EVIL_BULLET,
	NUM_ENTITY_TYPES
};

enum class EntityFaction
{
	FACTION_GOOD = 0,
	FACTION_NEUTRAL,
	FACTION_EVIL,
	FACTION_NUM
};



class Entity 
{

public:
	Entity();
	Entity(Game* gameInstance, Vec2 startPosition);
	Entity(Map* mapInstance, Vec2 startPosition, float orientationDegrees);
	virtual ~Entity();

	virtual void			StartUp() = 0;
	virtual void			Update(float deltaSecond) = 0;
	virtual void			Render() = 0;
	virtual void			Die() = 0;
	virtual void			HitByBullet(Entity* bullet) = 0;

	virtual void			GetDamage(int damageValue) = 0;
	virtual IntVec2 		GetTilePosition() const = 0;

	Vec2					GetForwardNormal();
	bool					IsAlive();

	Vec2					m_position;								
	Vec2					m_velocity;								
	Vec2					m_faceDirection;						
	float					m_speed;
	float					m_orientationDegrees;
	float					m_angularVelocity;
	float					m_physicsRadius;
	float					m_cosmeticRadius;
	int						m_health;
	int						m_damage;
	bool					m_isDead;
	bool					m_isGarbage;
	Game*					m_game;
	Map*					m_map;
	EntityType				m_entityType;
	EntityFaction			m_entityFaction;
	bool					m_isPushedByEntities;
	bool					m_doesPushEntities;
	bool					m_isPushedByWalls;
	bool					m_isHitByBullets;
};

