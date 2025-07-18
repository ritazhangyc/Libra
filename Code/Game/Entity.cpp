#include "Game/Entity.hpp"

Entity::Entity()
{
}

Entity::Entity(Game* gameInstance, Vec2 startPosition)
	:m_game(gameInstance)
	, m_position(startPosition)
{
}

Entity::Entity(Map* mapInstance, Vec2 startPosition, float orientationDegrees)
	:m_map(mapInstance)
	,m_position(startPosition)
	,m_orientationDegrees(orientationDegrees)
{

}

Entity::~Entity()
{

}

Vec2 Entity::GetForwardNormal() 
{
	m_faceDirection = Vec2(1.f, 0.f);
	m_faceDirection.RotateDegrees(m_orientationDegrees);
	return  m_faceDirection;
}

bool Entity::IsAlive()
{
	return !m_isDead;
}

