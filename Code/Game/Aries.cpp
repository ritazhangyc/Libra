#include "Game/Aries.hpp"
#include "Game/Game.hpp"
#include "Game/Tile.hpp"
#include "Game/Bullet.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/TileHeatMap.hpp"
#include "Engine/Renderer/Renderer.hpp"


Aries::Aries(Map* mapInstance, Vec2 const& spawnPosition, float spawnDegrees)
	:Entity(mapInstance, spawnPosition, spawnDegrees)
{
	StartUp();
}

Aries::~Aries()
{

}

void Aries::StartUp()
{
	m_physicsRadius = g_gameConfigBlackboard.GetValue("ariesPhysicsRadius", 1.0f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("ariesCosmeticRadius", 1.0f);
	m_angularVelocity = g_gameConfigBlackboard.GetValue("ariesTurnRate", 1.0f);
	m_health = g_gameConfigBlackboard.GetValue("ariesHealth", 1);
	m_speed = g_gameConfigBlackboard.GetValue("ariesDriveSpeed", 1.0f);
	m_distanceFieldHeatMap = new TileHeatMap(m_map->GetDimensions());
	m_entityType = EntityType::_EVIL_ARIES;
	m_entityFaction = EntityFaction::FACTION_EVIL;
	m_targetOrientationDegree = g_theRNG->RollRandomFloatInRange(0.0f, 360.0f);
	m_isPushedByEntities = true;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_target = m_map->GetPlayer();

	GenerateRandomTileIndexMap();
}

void Aries::Update(float deltaSecond)
{
	m_canSeePlayer = m_map->HasLineOfSight(this, m_target, g_gameConfigBlackboard.GetValue("enemyVisibleRange", 1.0f));
	UpdateTarget(deltaSecond);
	UpdateActions(deltaSecond);
}

void Aries::UpdateTarget(float deltaSecond)
{
	UNUSED(deltaSecond);
	if (m_canSeePlayer)
	{
		GetPlayerCurrentTilePosition();
		GenerateNextWayPointPosition();
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

void Aries::UpdateActions(float deltaSecond)
{
	Vec2 targetVector;
	float targetOrientationDegrees;
	targetVector = (m_nextWaypointPosition - m_position).GetNormalized();
	targetOrientationDegrees = targetVector.GetOrientationDegrees();
	m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, targetOrientationDegrees, m_angularVelocity * deltaSecond);
	m_faceDirection = Vec2(1.0f, 0.0f).GetRotatedDegrees(m_orientationDegrees);
	m_position += m_faceDirection * m_speed * deltaSecond;
}

void Aries::Render()
{
	RenderTank();
	//RenderHeatMap();
}

void Aries::RenderTank() const
{
	std::vector <Vertex_PCU> ariesVerts;
	Texture* scorpioTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyAries.png");
	AABB2 scorpioAABB = AABB2(Vec2(-m_cosmeticRadius, -m_cosmeticRadius), Vec2(m_cosmeticRadius, m_cosmeticRadius));

	AddVertsForAABB2D(ariesVerts, scorpioAABB, Rgba8::WHITE);
	TransformVertexArrayXY3D((int)ariesVerts.size(), ariesVerts.data(), 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(scorpioTexture);
	g_theRenderer->DrawVertexArray((int)ariesVerts.size(), ariesVerts.data());
}

void Aries::RenderHeatMap() const
{
	std::vector<Vertex_PCU> heatMapVerts;
	float maxValueInHeatMap = m_distanceFieldHeatMap->GetHighestNonSpecialValue(999.0f);

	for (int tileIndex = 0; tileIndex < m_distanceFieldHeatMap->GetValuesSize(); ++tileIndex)
	{
		m_distanceFieldHeatMap->AddVertsForDebugDraw(heatMapVerts, m_map->GetTileByIndex(tileIndex).GetBounds(),
			FloatRange(0.0f, maxValueInHeatMap),
			Rgba8(0, 0, 0, 100),
			Rgba8(255, 255, 255, 100),
			999.0f);
	}

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray((int)heatMapVerts.size(), heatMapVerts.data());
}

void Aries::Die()
{
	g_theGame->PlaySound(5);

	m_isDead = true;
	m_isGarbage = true;
}

void Aries::HitByBullet(Entity* bullet)
{
	Vec2 entityToBullet = bullet->m_position - m_position;
	float angle = GetAngleDegreesBetweenVectors2D(m_faceDirection, entityToBullet);
	if (fabsf(angle) > 45.0f)
	{
		GetDamage(bullet->m_damage);
		bullet->GetDamage(bullet->m_health);
	}
	else
	{
		Bullet* b = dynamic_cast<Bullet*>(bullet);
		entityToBullet.Normalize();
		b->BounceAgainst(entityToBullet);
	}

}

void Aries::GetDamage(int damageValue)
{
	m_health -= damageValue;
	g_theGame->PlaySound(3);


	if (m_health <= 0)
	{
		Die();
	}
}

IntVec2  Aries::GetTilePosition() const
{
	int tileX = RoundDownToInt(m_position.x);
	int tileY = RoundDownToInt(m_position.y);

	return IntVec2(tileX, tileY);
}

Entity* Aries::GetTarget() const
{
	return m_target;
}

Vec2 Aries::GetLastSeenPosition() const
{
	return m_targetPosition;
}

Vec2 Aries::GetNextWayPointPosition() const
{
	return m_nextWaypointPosition;
}

void Aries::GenerateRandomTileIndexMap()
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
	m_map->PopulateDistanceField(*m_distanceFieldHeatMap, targetTile.GetTilePosition(), 999.0f, true, true);
}

void Aries::GenerateNextWayPointPosition()
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

void Aries::GetPlayerCurrentTilePosition()
{
	const Tile& playerTile = m_map->GetTileByPos(m_target->m_position);

	IntVec2 tilePos = playerTile.GetTilePosition();

	m_targetTileIndex = GetTileIndexByPos(m_map->GetDimensions().y, tilePos);

	m_targetPosition = playerTile.GetWorldPosCenter();
}

void Aries::SetTarget(Entity* target)
{
	m_target = target;
}
