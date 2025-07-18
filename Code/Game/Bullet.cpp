#include "Game/Bullet.hpp"
#include "Game/Map.hpp"
#include "Game/Tile.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"


Bullet::Bullet(Map* mapInstance, Vec2 const& spawnPosition, float spawnDegrees)
	:Entity(mapInstance, spawnPosition, spawnDegrees)
{

}

Bullet::~Bullet()
{

}

void Bullet::StartUp()
{
	m_physicsRadius = g_gameConfigBlackboard.GetValue("bulletPhysicsRadius", 1.0f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("bulletCosmeticWidth", 1.0f);
	m_speed = g_gameConfigBlackboard.GetValue("bulletSpeed", 1.0f);

	m_doesPushEntities = false;
	m_isPushedByEntities = false;
	m_isPushedByWalls = false;
	m_isHitByBullets = false;

	m_faceDirection = Vec2(1.0f, 0.0f).GetRotatedDegrees(m_orientationDegrees);

	if (m_entityType == EntityType::_GOOD_BULLET)
	{
		m_health = 3;
		m_damage = g_gameConfigBlackboard.GetValue("bulletGoodDamage", 1);
		m_bulletTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/FriendlyBolt.png");
		m_entityFaction = EntityFaction::FACTION_GOOD;
	}
	else if (m_entityType == EntityType::_EVIL_BULLET)
	{
		m_health = 1;
		m_damage = g_gameConfigBlackboard.GetValue("bulletBadDamage", 1);
		m_bulletTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyBolt.png");
		m_entityFaction = EntityFaction::FACTION_EVIL;
	}
}

void Bullet::Update(float deltaSecond)
{
	CheckBounceOffWall(deltaSecond);
	m_orientationDegrees += m_angularVelocity * deltaSecond;
	m_velocity = m_speed * m_faceDirection;
	m_position += m_velocity * deltaSecond;
}

void Bullet::CheckBounceOffWall(float deltaSecond)
{
	Vec2 lastPosition = m_position;
	Vec2 currentPosition = (m_speed * m_faceDirection * deltaSecond) + lastPosition;
	if (m_map->IsPointInSolid(currentPosition))
	{
		Tile lastTile = m_map->GetTileByPos(lastPosition);
		Tile currentTile = m_map->GetTileByPos(currentPosition);
		Vec2 normalImpact = lastTile.GetWorldPosMin() - currentTile.GetWorldPosMin();
		normalImpact.Normalize();
		GetDamage(1);
		BounceAgainst(normalImpact);
	}
}

void Bullet::BounceAgainst(Vec2 const& normal)
{
	m_faceDirection.Reflect(normal);

	g_theGame->PlaySound(6);

	float m_randomDegrees = g_theRNG->RollRandomFloatInRange(-8.0f, 8.0f);
	m_faceDirection.RotateDegrees(m_randomDegrees);
	m_orientationDegrees = m_faceDirection.GetOrientationDegrees();
}

void Bullet::Render()
{
	std::vector <Vertex_PCU> bulletVerts;
	AABB2 bulletAABB = AABB2(Vec2(-g_gameConfigBlackboard.GetValue("bulletCosmeticWidth", 1.0f) * 0.5f, -g_gameConfigBlackboard.GetValue("bulletCosmeticHeight", 1.0f) * 0.5f), Vec2(g_gameConfigBlackboard.GetValue("bulletCosmeticWidth", 1.0f) * 0.5f, g_gameConfigBlackboard.GetValue("bulletCosmeticHeight", 1.0f) * 0.5f));

	AddVertsForAABB2D(bulletVerts, bulletAABB, Rgba8::WHITE);
	TransformVertexArrayXY3D((int)bulletVerts.size(), bulletVerts.data(), 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(m_bulletTexture);
	g_theRenderer->DrawVertexArray((int)bulletVerts.size(), bulletVerts.data());
}

void Bullet::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}

void Bullet::HitByBullet(Entity* bullet)
{
	UNUSED(bullet);
}

void Bullet::GetDamage(int damageValue)
{
	m_health -= damageValue;

	if (m_health <= 0)
	{
		Die();
	}
}

IntVec2 Bullet::GetTilePosition() const
{
	int tileX = RoundDownToInt(m_position.x);
	int tileY = RoundDownToInt(m_position.y);

	return IntVec2(tileX, tileY);
}


