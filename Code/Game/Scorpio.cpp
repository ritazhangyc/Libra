#include "Game/Scorpio.hpp"
#include "Game/Game.hpp"
#include "Game/Tile.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"

Scorpio::Scorpio(Map* mapInstance, Vec2 const& spawnPosition, float spawnDegrees)
	:Entity(mapInstance, spawnPosition, spawnDegrees)
{
	StartUp();
}

Scorpio::~Scorpio()
{

}

void Scorpio::StartUp()
{
	m_physicsRadius = g_gameConfigBlackboard.GetValue("scorpioPhysicsRadius", 1.0f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("scorpioCosmeticRadius", 1.0f);
	m_health = g_gameConfigBlackboard.GetValue("scorpioHealth", 1);
	m_shootTimer = g_gameConfigBlackboard.GetValue("scorpioShootCooldownSeconds", 1.0f);
	m_angularVelocity = g_gameConfigBlackboard.GetValue("scorpioTurnRate", 1.0f);

	m_entityType = EntityType::_EVIL_SCORPIO;
	m_entityFaction = EntityFaction::FACTION_EVIL;
	m_isPushedByEntities = false;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_turret.m_localOrientationDegree = g_theRNG->RollRandomFloatInRange(0.0f, 360.0f);
	m_turret.m_worldOrientationDegree = m_orientationDegrees + m_turret.m_localOrientationDegree;
	m_target = g_theGame->GetPlayer();
}

void Scorpio::Update(float deltaSecond)
{
	m_canSeePlayer = m_map->HasLineOfSight(this, m_target, g_gameConfigBlackboard.GetValue("enemyVisibleRange", 1.0f));
	m_raycastMaxPointActual = m_map->RaycastVsTiles(m_position, m_turret.m_faceDirection, g_gameConfigBlackboard.GetValue("enemyVisibleRange", 1.0f)).m_hitPoint;
	UpdateActions(deltaSecond);
}

void Scorpio::UpdateActions(float deltaSecond)
{
	if (!m_canSeePlayer)
	{
		if (m_turret.m_localOrientationDegree < -180.0f)
		{
			m_turret.m_localOrientationDegree += 360.0f;
		}
		m_turret.m_localOrientationDegree += deltaSecond * m_angularVelocity;
		m_turret.m_worldOrientationDegree = m_orientationDegrees + m_turret.m_localOrientationDegree;
	}
	else
	{
		Vec2 toPlayerDirection = (m_target->m_position - m_position);
		float targetAngle = toPlayerDirection.GetOrientationDegrees();
		m_turret.m_worldOrientationDegree = GetTurnedTowardDegrees(m_turret.m_worldOrientationDegree, targetAngle, m_angularVelocity * deltaSecond);
		m_turret.m_localOrientationDegree = m_turret.m_worldOrientationDegree - m_orientationDegrees;
		if (fabsf(GetShortestAngularDispDegrees(m_turret.m_worldOrientationDegree, targetAngle)) < 5.0f)
		{
			if (m_canShoot)
			{
				m_canShoot = false;
				m_map->SpawnNewEntity(EntityType::_EVIL_BULLET, m_position + m_turret.m_faceDirection * m_physicsRadius * 0.9f, m_turret.m_worldOrientationDegree);
				g_theGame->PlaySound(1);
			}
		}
	}

	m_turret.m_faceDirection = Vec2(1.0f, 0.0f).GetRotatedDegrees(m_turret.m_worldOrientationDegree);

	if (!m_canShoot)
	{
		if (m_shootTimer > 0.0f)
		{
			m_shootTimer -= deltaSecond;
		}
		else
		{
			m_shootTimer = 0.3f;
			m_canShoot = true;
		}
	}
}

void Scorpio::Render()
{
	RenderTurretBase();
	g_theRenderer->BindTexture(nullptr);
	DrawRay(m_position + m_turret.m_faceDirection * 0.35f, m_raycastMaxPointActual, 0.05f, Rgba8::RED);
	RenderTurretCannon();
}

void Scorpio::RenderTurretBase() const
{
	std::vector <Vertex_PCU> scorpioVerts;
	Texture* scorpioTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyTurretBase.png");
	AABB2 scorpioAABB = AABB2(Vec2(-m_cosmeticRadius, -m_cosmeticRadius), Vec2(m_cosmeticRadius, m_cosmeticRadius));

	AddVertsForAABB2D(scorpioVerts, scorpioAABB, Rgba8::WHITE);
	TransformVertexArrayXY3D((int)scorpioVerts.size(), scorpioVerts.data(), 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(scorpioTexture);
	g_theRenderer->DrawVertexArray((int)scorpioVerts.size(), scorpioVerts.data());
}

void Scorpio::RenderTurretCannon() const
{
	std::vector <Vertex_PCU> scorpioTurretVerts;
	Texture* scorpioTurretTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyCannon.png");
	AABB2 scorpioTurretAABB = AABB2(Vec2(-m_cosmeticRadius, -m_cosmeticRadius), Vec2(m_cosmeticRadius, m_cosmeticRadius));

	AddVertsForAABB2D(scorpioTurretVerts, scorpioTurretAABB, Rgba8::WHITE);
	TransformVertexArrayXY3D((int)scorpioTurretVerts.size(), scorpioTurretVerts.data(), 1.0f, m_turret.m_worldOrientationDegree, m_position);
	g_theRenderer->BindTexture(scorpioTurretTexture);
	g_theRenderer->DrawVertexArray((int)scorpioTurretVerts.size(), scorpioTurretVerts.data());
}

void Scorpio::Die()
{
	g_theGame->PlaySound(5);
	m_isDead = true;
	m_isGarbage = true;
}

void Scorpio::HitByBullet(Entity* bullet)
{
	GetDamage(bullet->m_damage);
	bullet->GetDamage(bullet->m_health);
}

void Scorpio::GetDamage(int damageValue)
{
	m_health -= damageValue;
	g_theGame->PlaySound(3);
	if (m_health <= 0)
	{
		Die();
	}
}

IntVec2 Scorpio::GetTilePosition() const
{
	int tileX = RoundDownToInt(m_position.x);
	int tileY = RoundDownToInt(m_position.y);

	return IntVec2(tileX, tileY);
}

Entity* Scorpio::GetTarget() const
{
	return m_target;
}

void Scorpio::SetTarget(Entity* target)
{
	m_target = target;
}

void Scorpio::DrawRay(Vec2 const& startPos, Vec2 const& endPos, float thickness, Rgba8 const& color)
{
	Vec2 direction = endPos - startPos;
	direction.Normalize();

	float thick_r = thickness * 0.5f;

	Vec2 upDirection = direction;
	upDirection.Rotate90Degrees();

	Vec2 startUpperPos = startPos + upDirection * thick_r;
	Vec2 startLowerPos = startPos - upDirection * thick_r;
	Vec2 endUpperPos = endPos + upDirection * thick_r;
	Vec2 endLowerPos = endPos - upDirection * thick_r;

	Vertex_PCU linePoints[6]
	{
		Vertex_PCU(startUpperPos.x, startUpperPos.y,	color.r, color.g, color.b, 255),
		Vertex_PCU(startLowerPos.x, startLowerPos.y,	color.r, color.g, color.b, 255),
		Vertex_PCU(endUpperPos.x, endUpperPos.y,		color.r, color.g, color.b, 50),

		Vertex_PCU(endUpperPos.x, endUpperPos.y,		color.r, color.g, color.b, 50),
		Vertex_PCU(startLowerPos.x, startLowerPos.y,	color.r, color.g, color.b, 255),
		Vertex_PCU(endLowerPos.x, endLowerPos.y,		color.r, color.g, color.b, 50),

	};
	g_theRenderer->DrawVertexArray(6, linePoints);
}

Vec2 Scorpio::GetTurretOrientation()
{
	return m_turret.m_faceDirection;
}
