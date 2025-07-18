#include "Game/Leo.hpp"
#include "Game/Game.hpp"
#include "Game/Tile.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/TileHeatMap.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

Leo::Leo(Map* mapInstance, Vec2 const& spawnPosition, float spawnDegrees)
	:Entity(mapInstance, spawnPosition, spawnDegrees)
{

	StartUp();
}

Leo::~Leo()
{

}

void Leo::StartUp()
{
	m_physicsRadius = g_gameConfigBlackboard.GetValue("leoPhysicsRadius", 1.0f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("leoCosmeticRadius", 1.0f);
	m_angularVelocity = g_gameConfigBlackboard.GetValue("leoTurnRate", 1.0f);
	m_speed = g_gameConfigBlackboard.GetValue("leoDriveSpeed", 1.0f);
	m_health = g_gameConfigBlackboard.GetValue("leoHealth", 1);
	m_distanceFieldHeatMap = new TileHeatMap(m_map->GetDimensions());
	m_shootTimer = 1.0f;
	m_entityType = EntityType::_EVIL_LEO;
	m_entityFaction = EntityFaction::FACTION_EVIL;
	m_targetOrientationDegree = g_theRNG->RollRandomFloatInRange(0.0f, 360.0f);
	m_isPushedByEntities = true;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_target = g_theGame->GetPlayer();

	GenerateRandomTileIndexMap();

}

void Leo::Update(float deltaSecond)
{
	m_canSeePlayer = m_map->HasLineOfSight(this, m_target, g_gameConfigBlackboard.GetValue("enemyVisibleRange", 1.0f));
	UpdateTarget(deltaSecond);
	UpdateActions(deltaSecond);
}

void Leo::UpdateTarget(float deltaSecond)
{
	UNUSED(deltaSecond);
	if (m_canSeePlayer)
	{
		GetPlayerCurrentTilePosition();
		GenerateNextWayPointPosition();

		float anglePlayerLeo = GetShortestAngularDispDegrees(m_orientationDegrees, (m_targetPosition - m_position).GetOrientationDegrees());
		if (fabsf(anglePlayerLeo) <= 5.0f)
		{
			if (m_canShoot)
			{
				m_canShoot = false;
				m_map->SpawnNewEntity(EntityType::_EVIL_BULLET, m_position + m_faceDirection * m_physicsRadius * 0.9f, m_orientationDegrees);
				g_theGame->PlaySound(1);
			}
		}
	}
	else
	{
		if (!m_reachLastSeenPosition && !m_target->m_isDead)
		{
			if (IsPointInsideDisc2D(m_targetPosition, m_position, m_physicsRadius))
			{
				m_reachLastSeenPosition = true;
			}
		}
		else
		{
			if (IsPointInsideDisc2D(m_targetPosition, m_position, m_physicsRadius))
			{
				GenerateRandomTileIndexMap();
			}
			else
			{
				GenerateNextWayPointPosition();
			}
		}
	}


}

void Leo::UpdateActions(float deltaSecond)
{
	Vec2 targetVector;
	float targetOrientationDegrees;
	targetVector = (m_nextWaypointPosition - m_position).GetNormalized();
	targetOrientationDegrees = targetVector.GetOrientationDegrees();
	m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, targetOrientationDegrees, m_angularVelocity * deltaSecond);
	m_faceDirection = Vec2(1.0f, 0.0f).GetRotatedDegrees(m_orientationDegrees);
	m_position += m_faceDirection * m_speed * deltaSecond;



	if (!m_canShoot)
	{
		if (m_shootTimer > 0.0f)
		{
			m_shootTimer -= deltaSecond;
		}
		else
		{
			m_shootTimer = 1.0f;
			m_canShoot = true;
		}
	}
}

void Leo::Render()
{
	RenderTankBase();
}

void Leo::RenderTankBase()
{
	std::vector <Vertex_PCU> leoVerts;
	Texture* scorpioTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyTank4.png");
	AABB2 scorpioAABB = AABB2(Vec2(-m_cosmeticRadius, -m_cosmeticRadius), Vec2(m_cosmeticRadius, m_cosmeticRadius));

	AddVertsForAABB2D(leoVerts, scorpioAABB, Rgba8::WHITE);
	TransformVertexArrayXY3D((int)leoVerts.size(), leoVerts.data(), 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(scorpioTexture);
	g_theRenderer->DrawVertexArray((int)leoVerts.size(), leoVerts.data());
}


void Leo::Die()
{
	g_theGame->PlaySound(5);
	m_isDead = true;
	m_isGarbage = true;
}

void Leo::HitByBullet(Entity* bullet)
{
	GetDamage(bullet->m_damage);
	bullet->GetDamage(bullet->m_health);
}

void Leo::GetDamage(int damageValue)
{
	m_health -= damageValue;
	g_theGame->PlaySound(3);
	if (m_health <= 0)
	{
		Die();
	}
}

IntVec2 Leo::GetTilePosition() const
{
	int tileX = RoundDownToInt(m_position.x);
	int tileY = RoundDownToInt(m_position.y);

	return IntVec2(tileX, tileY);
}

Entity* Leo::GetTarget() const
{
	return m_target;
}

Vec2 Leo::GetNextWayPointPosition() const
{
	return m_nextWaypointPosition;
}

void Leo::GenerateRandomTileIndexMap()
{
	int mapSize = m_map->GetTilesSize();
	int targetTileIndex;
	while (true)
	{
		targetTileIndex = g_theRNG->RollRandomIntLessThan(mapSize);

		if (!IsIntValueInsideVector(targetTileIndex, m_map->GetAllScorpioAndSolidTilesIndex()))
		{
			break;
		}
	}

	m_targetTileIndex = targetTileIndex;
	const Tile& tile = m_map->GetTileByIndex(m_targetTileIndex);
	m_targetPosition = tile.GetWorldPosCenter();
	const Tile& targetTile = m_map->GetTileByIndex(m_targetTileIndex);
	m_map->PopulateDistanceField(*m_distanceFieldHeatMap, targetTile.GetTilePosition());
}

void Leo::GenerateNextWayPointPosition()
{
	if (!m_map->RaycastVsTiles(m_position, m_targetPosition, true).m_hitAnything)
	{
		m_nextWaypointPosition = m_targetPosition;
		return;
	}
	else if (m_pathPoints.size() >= 2 && !m_map->RaycastVsTiles(m_position, m_pathPoints[m_pathPoints.size() - 2], true).m_hitAnything)
	{
		m_pathPoints.pop_back();
		m_nextWaypointPosition = m_pathPoints[m_pathPoints.size() - 1];
		return;
	}
	if (IsPointInsideDisc2D(m_nextWaypointPosition, m_position, m_physicsRadius))
	{
		if (m_pathPoints.size() == 0)
		{
			GenerateRandomTileIndexMap();
		}
		else
		{
			m_pathPoints.pop_back();
			return;
		}
	}
}

void Leo::GetPlayerCurrentTilePosition()
{
	const Tile& playerTile = m_map->GetTileByPos(m_target->m_position);

	IntVec2 tilePos = playerTile.GetTilePosition();

	m_targetTileIndex = GetTileIndexByPos(m_map->GetDimensions().y, tilePos);

	m_targetPosition = playerTile.GetWorldPosCenter();
}

Vec2 Leo::GetLastSeenPosition() const
{
	return m_targetPosition;
}

void Leo::SetTarget(Entity* target)
{
	m_target = target;
}

