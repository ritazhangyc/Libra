#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/TileHeatMap.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"
//#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/Player.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Scorpio.hpp"
#include "Game/Leo.hpp"
#include "Game/Aries.hpp"
#include "Game/Bullet.hpp"
#include <map>

std::map<std::string, MapDefinition> MapDefinition::s_mapDefinitions;

Map::Map()
{
}

Map::Map(MapDefinition const& mapDefition)
{
	m_mapDefinition = mapDefition;
	m_dimensions = m_mapDefinition.m_dimensions;
	m_exitPosition = Vec2((float)m_mapDefinition.m_dimensions.x - 1.5f, (float)m_mapDefinition.m_dimensions.y - 1.5f);
}

Map::~Map()
{
}

void Map::StartUp()
{
	SpawnFixedValues();
	SpawnAccessibleMap();
	SpawnEntities();
}

void Map::SpawnFixedValues()
{
	m_currentMode = MapRenderMode::MAP_MODE_NORMAL;
	int width = m_mapDefinition.m_dimensions.x;
	int height = m_mapDefinition.m_dimensions.y;
	m_tiles.reserve(width * height);
	m_dijkstraHeatMap = new TileHeatMap(IntVec2(width, height));
}

void Map::SpawnAccessibleMap()
{
	SpawnMaps();
	SpawnWorm();
	SpawnStartExitArea();
	int totalRespawnTimes = 1;
	while (!PopulateDistanceField(*m_dijkstraHeatMap, IntVec2(1, 1)))
	{

		SpawnMaps();
		SpawnWorm();
		SpawnStartExitArea();
		totalRespawnTimes += 1;
	}
}

void Map::SpawnMaps()
{
	m_tiles.clear();

	size_t width = m_mapDefinition.m_dimensions.x;
	size_t height = m_mapDefinition.m_dimensions.y;

	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			IntVec2 tilePosition = IntVec2(i, j);
			if (i == 0 || j == 0 || i == m_mapDefinition.m_dimensions.x - 1 || j == m_mapDefinition.m_dimensions.y - 1)
			{
				Tile tile = Tile(tilePosition, m_mapDefinition.m_edgeType);
				m_tiles.push_back(tile);
				continue;
			}
			else
			{
				Tile tile = Tile(tilePosition, m_mapDefinition.m_fillType);
				m_tiles.push_back(tile);
			}
		}
	}
 	m_mapDefinition.m_exitPosition = m_tiles[m_tiles.size() - 1].GetWorldPosCenter() - Vec2(1.f, 1.f);
 
// 	m_tiles[2 * height + 4].SetTileType(TileType::TILE_TYPE_STONE);
// 	m_tiles[3 * height + 4].SetTileType(TileType::TILE_TYPE_STONE);
// 	m_tiles[4 * height + 2].SetTileType(TileType::TILE_TYPE_STONE);
// 	m_tiles[4 * height + 3].SetTileType(TileType::TILE_TYPE_STONE);
// 	m_tiles[4 * height + 4].SetTileType(TileType::TILE_TYPE_STONE);
// 
// 	m_tiles[(m_dimensions.x - 5) * height - 3].SetTileType(TileType::TILE_TYPE_STONE);
// 	m_tiles[(m_dimensions.x - 5) * height - 4].SetTileType(TileType::TILE_TYPE_STONE);
// 	m_tiles[(m_dimensions.x - 5) * height - 5].SetTileType(TileType::TILE_TYPE_STONE);
// 	m_tiles[(m_dimensions.x - 5) * height - 6].SetTileType(TileType::TILE_TYPE_STONE);
// 	m_tiles[(m_dimensions.x - 4) * height - 6].SetTileType(TileType::TILE_TYPE_STONE);
// 	m_tiles[(m_dimensions.x - 3) * height - 6].SetTileType(TileType::TILE_TYPE_STONE);
// 	m_tiles[(m_dimensions.x - 2) * height - 6].SetTileType(TileType::TILE_TYPE_STONE);
// 
// 	m_tiles[(m_dimensions.x - 1) * height - 2].SetTileType(TileType::TILE_TYPE_EXIT);
// 	m_tiles[1 * height + 1].SetTileType(TileType::TILE_TYPE_START);
// 
// 	m_exitPosition = m_tiles[m_tiles.size() - 1].GetWorldPosCenter() - Vec2(1.f, 1.f);

}

void Map::SpawnWorm()
{
	for (int i = 0; i < m_mapDefinition.m_wormTypes.size(); ++i)
	{
		TileDefinition& tileDef = m_mapDefinition.m_wormTypes[i];

		for (int wormIndex = 0; wormIndex < m_mapDefinition.m_wormCounts[i]; ++wormIndex)
		{
			IntVec2 startPosition = GenerateTilePositionInsideDimensions();
			int startIndex = GetTileIndexByPos(GetDimensions().y, startPosition);
			m_tiles[startIndex].SetTileDef(tileDef);

			for (int stepIndex = 0; stepIndex < m_mapDefinition.m_wormMaxLength[i]; ++stepIndex)
			{
				int direction = g_theRNG->RollRandomIntLessThan(4);
				IntVec2 temporaryPos = startPosition;

				switch (direction)
				{
				case 0:
					startPosition += IntVec2(0, 1);
					break;
				case 1:
					startPosition += IntVec2(0, -1);
					break;
				case 2:
					startPosition += IntVec2(1, 0);
					break;
				case 3:
					startPosition += IntVec2(-1, 0);
					break;
				}

				if (startPosition.x == 0 ||
					startPosition.x == m_mapDefinition.m_dimensions.x - 1 ||
					startPosition.y == 0 ||
					startPosition.y == m_mapDefinition.m_dimensions.y - 1)
				{
					startPosition = temporaryPos;
					continue;
				}

				startIndex = GetTileIndexByPos(GetDimensions().y, startPosition);
				m_tiles[startIndex].SetTileDef(tileDef);
			}
		}
	}

}

void Map::SpawnStartExitArea()
{
	size_t height = m_mapDefinition.m_dimensions.y;

	m_tiles[2 * height + 4].SetTileDef(m_mapDefinition.m_startBunkerTileType);
	m_tiles[3 * height + 4].SetTileDef(m_mapDefinition.m_startBunkerTileType);
	m_tiles[4 * height + 2].SetTileDef(m_mapDefinition.m_startBunkerTileType);
	m_tiles[4 * height + 3].SetTileDef(m_mapDefinition.m_startBunkerTileType);
	m_tiles[4 * height + 4].SetTileDef(m_mapDefinition.m_startBunkerTileType);

	m_tiles[1 * height + 1].SetTileDef(m_mapDefinition.m_startFloorTileType);
	m_tiles[1 * height + 2].SetTileDef(m_mapDefinition.m_startFloorTileType);
	m_tiles[1 * height + 3].SetTileDef(m_mapDefinition.m_startFloorTileType);
	m_tiles[1 * height + 4].SetTileDef(m_mapDefinition.m_startFloorTileType);
	m_tiles[1 * height + 5].SetTileDef(m_mapDefinition.m_startFloorTileType);
	m_tiles[2 * height + 1].SetTileDef(m_mapDefinition.m_startFloorTileType);
	m_tiles[2 * height + 2].SetTileDef(m_mapDefinition.m_startFloorTileType);
	m_tiles[2 * height + 3].SetTileDef(m_mapDefinition.m_startFloorTileType);
	m_tiles[2 * height + 5].SetTileDef(m_mapDefinition.m_startFloorTileType);
	m_tiles[3 * height + 1].SetTileDef(m_mapDefinition.m_startFloorTileType);
	m_tiles[3 * height + 2].SetTileDef(m_mapDefinition.m_startFloorTileType);
	m_tiles[3 * height + 3].SetTileDef(m_mapDefinition.m_startFloorTileType);
	m_tiles[3 * height + 5].SetTileDef(m_mapDefinition.m_startFloorTileType);
	m_tiles[4 * height + 1].SetTileDef(m_mapDefinition.m_startFloorTileType);
	m_tiles[4 * height + 5].SetTileDef(m_mapDefinition.m_startFloorTileType);
	m_tiles[5 * height + 1].SetTileDef(m_mapDefinition.m_startFloorTileType);
	m_tiles[5 * height + 2].SetTileDef(m_mapDefinition.m_startFloorTileType);
	m_tiles[5 * height + 3].SetTileDef(m_mapDefinition.m_startFloorTileType);
	m_tiles[5 * height + 4].SetTileDef(m_mapDefinition.m_startFloorTileType);
	m_tiles[5 * height + 5].SetTileDef(m_mapDefinition.m_startFloorTileType);

	m_tiles[(m_mapDefinition.m_dimensions.x - 5) * height - 3].SetTileDef(m_mapDefinition.m_endBunkerTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 5) * height - 4].SetTileDef(m_mapDefinition.m_endBunkerTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 5) * height - 5].SetTileDef(m_mapDefinition.m_endBunkerTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 5) * height - 6].SetTileDef(m_mapDefinition.m_endBunkerTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 4) * height - 6].SetTileDef(m_mapDefinition.m_endBunkerTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 3) * height - 6].SetTileDef(m_mapDefinition.m_endBunkerTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 2) * height - 6].SetTileDef(m_mapDefinition.m_endBunkerTileType);


	m_tiles[(m_mapDefinition.m_dimensions.x - 1) * height - 7].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 1) * height - 6].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 1) * height - 5].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 1) * height - 4].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 1) * height - 3].SetTileDef(m_mapDefinition.m_endFloorTileType);

	m_tiles[(m_mapDefinition.m_dimensions.x - 2) * height - 7].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 2) * height - 5].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 2) * height - 4].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 2) * height - 3].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 2) * height - 2].SetTileDef(m_mapDefinition.m_endFloorTileType);

	m_tiles[(m_mapDefinition.m_dimensions.x - 3) * height - 7].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 3) * height - 5].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 3) * height - 4].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 3) * height - 3].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 3) * height - 2].SetTileDef(m_mapDefinition.m_endFloorTileType);

	m_tiles[(m_mapDefinition.m_dimensions.x - 4) * height - 7].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 4) * height - 5].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 4) * height - 4].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 4) * height - 3].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 4) * height - 2].SetTileDef(m_mapDefinition.m_endFloorTileType);

	m_tiles[(m_mapDefinition.m_dimensions.x - 5) * height - 7].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 5) * height - 2].SetTileDef(m_mapDefinition.m_endFloorTileType);

	m_tiles[(m_mapDefinition.m_dimensions.x - 6) * height - 7].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 6) * height - 6].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 6) * height - 5].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 6) * height - 4].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 6) * height - 3].SetTileDef(m_mapDefinition.m_endFloorTileType);
	m_tiles[(m_mapDefinition.m_dimensions.x - 6) * height - 2].SetTileDef(m_mapDefinition.m_endFloorTileType);


	m_tiles[(m_mapDefinition.m_dimensions.x - 1) * height - 2].SetTileDef(TileDefinition::s_definitions.find("MapExit")->second);
	m_tiles[1 * height + 1].SetTileDef(TileDefinition::s_definitions.find("MapEntry")->second);
}

void Map::SpawnEntities()
{
	m_allEntities.reserve(100);
	for (int i = 0; i < m_mapDefinition.m_scorpioNum; i++)
	{
		SpawnNewEntity(EntityType::_EVIL_SCORPIO, GeneratePositionNotInSolid(), 0.0f);
	}
	for (int i = 0; i < m_mapDefinition.m_leoNum; i++)
	{
		SpawnNewEntity(EntityType::_EVIL_LEO, GeneratePositionNotInSolid(), 0.0f);
	}
	for (int i = 0; i < m_mapDefinition.m_ariesNum; i++)
	{
		SpawnNewEntity(EntityType::_EVIL_ARIES, GeneratePositionNotInSolid(), 0.0f);
	}
}

void Map::SpawnNewEntity(EntityType type, Vec2 const& position, float orientationDegrees)
{
	switch (type)
	{
	case  EntityType::_EVIL_SCORPIO:
	{
		Scorpio* scorpio = new Scorpio(this, position, orientationDegrees);
		AddEntityInMap(*scorpio);
		break;
	}

	case  EntityType::_EVIL_LEO:
	{
		Leo* leo = new Leo(this, position, orientationDegrees);
		AddEntityInMap(*leo);
		break;
	}

	case  EntityType::_EVIL_ARIES:
	{
		Aries* aries = new Aries(this, position, orientationDegrees);
		AddEntityInMap(*aries);
		break;
	}

	case  EntityType::_GOOD_BULLET:
	{
		Bullet* bullet = new Bullet(this, position, orientationDegrees);
		bullet->m_entityType = EntityType::_GOOD_BULLET;
		bullet->StartUp();
		AddEntityInMap(*bullet);
		break;
	}

	case  EntityType::_EVIL_BULLET:
	{
		Bullet* bullet = new Bullet(this, position, orientationDegrees);
		bullet->m_entityType = EntityType::_EVIL_BULLET;
		bullet->StartUp();
		AddEntityInMap(*bullet);
		break;
	}
	}
}

void Map::Update(float deltaSeconds)
{
	UpdateAllEntities(deltaSeconds);
	CheckEntityCollisionWithWall();
	CheckEntityCollisionWithEntity();
	CheckBulletCollisionWithEntities();
	CheckPlayerPositionOverlapwithExit();
	ClearAllGarbage();
}

void Map::UpdateAllEntities(float deltaSeconds)
{
	for (int entityIndex = 0; entityIndex < m_allEntities.size(); entityIndex++)
	{
		if (m_allEntities[entityIndex] != nullptr)
		{
			m_allEntities[entityIndex]->Update(deltaSeconds);
		}
	}
}

void Map::CheckEntityWithCertainTile(Entity* entity, int tileIndex)
{
// 	if (TileDefinition::s_definitions[(int)m_tiles[tileIndex].GetTileType()]->m_isSolid)
// 	{
// 		PushDiscOutOfAABB2D(entity->m_position, entity->m_physicsRadius, m_tiles[tileIndex].GetBounds());
// 	}
	if (tileIndex >= m_tiles.size())
	{
		return;
	}
	Vec2 nearsetPointOnAABB = GetNearestPointOnAABB2D(entity->m_position, m_tiles[tileIndex].GetBounds());

	if (m_tiles[tileIndex].GetTileDef()->m_isSolid && IsPointInsideDisc2D(nearsetPointOnAABB, entity->m_position, entity->m_physicsRadius))
	{
		PushDiscOutOfAABB2D(entity->m_position, entity->m_physicsRadius, m_tiles[tileIndex].GetBounds());
	}
}

void Map::CheckBulletCollisionWithEntities()
{
	for (int i = 0; i < m_entityListsByType[(int)EntityType::_GOOD_BULLET].size(); i++)
	{
		Entity* goodBullet = m_entityListsByType[(int)EntityType::_GOOD_BULLET][i];

		for (int j = 0; j < m_factionEntities[(int)EntityFaction::FACTION_EVIL].size(); j++)
		{
			Entity* evilEntity = m_factionEntities[(int)EntityFaction::FACTION_EVIL][j];

			if (goodBullet != nullptr && evilEntity != nullptr && evilEntity->m_isHitByBullets && !evilEntity->m_isDead)
			{
				float goodBulletRadius = goodBullet->m_physicsRadius;
				float evilEntityRadius = evilEntity->m_physicsRadius;

				float distanceBullet = GetDistance2D(goodBullet->m_position, evilEntity->m_position);

				if (distanceBullet < goodBulletRadius + evilEntityRadius)
				{
					evilEntity->HitByBullet(goodBullet);
				}
			}
		}
	}

	for (int i = 0; i < m_entityListsByType[(int)EntityType::_EVIL_BULLET].size(); i++)
	{
		Entity* badBullet = m_entityListsByType[(int)EntityType::_EVIL_BULLET][i];

		for (int j = 0; j < m_factionEntities[(int)EntityFaction::FACTION_GOOD].size(); j++)
		{
			Entity* goodEntity = m_factionEntities[(int)EntityFaction::FACTION_GOOD][j];

			if (badBullet != nullptr && goodEntity != nullptr && goodEntity->m_isHitByBullets && !goodEntity->m_isDead)
			{
				float goodBulletRadius = badBullet->m_physicsRadius;
				float evilEntityRadius = goodEntity->m_physicsRadius;

				float distanceBullet = GetDistance2D(badBullet->m_position, goodEntity->m_position);

				if (distanceBullet < goodBulletRadius + evilEntityRadius)
				{
					goodEntity->HitByBullet(badBullet);
				}
			}
		}
	}
}

void Map::CheckPlayerPositionOverlapwithExit()
{
	if (IsPointInsideDisc2D(m_exitPosition,
		m_entityListsByType[(int)EntityType::ENTITY_TYPE_GOOD_PLAYER][0]->m_position,
		m_entityListsByType[(int)EntityType::ENTITY_TYPE_GOOD_PLAYER][0]->m_physicsRadius))
	{
		g_theGame->PutPlayerIntoNextMap();
	}
}

void Map::CheckEntityCollisionWithWall()
{
	for (int i = 0; i < m_allEntities.size(); i++)
	{
		if (m_allEntities[i] == nullptr || m_allEntities[i]->m_isPushedByWalls == false)
		{
			continue;
		}

		if (m_allEntities[i]->m_entityType == EntityType::ENTITY_TYPE_GOOD_PLAYER)
		{
			if (g_theGame->GetNoClipMode())
			{
				break;
			}

			Entity* entity = m_allEntities[i];
			IntVec2 tilePos = entity->GetTilePosition();

			int indexInVector = m_dimensions.y * tilePos.x + (tilePos.y);

			CheckEntityWithCertainTile(entity, indexInVector - 1);
			CheckEntityWithCertainTile(entity, indexInVector + 1);
			CheckEntityWithCertainTile(entity, indexInVector - m_dimensions.y);
			CheckEntityWithCertainTile(entity, indexInVector + m_dimensions.y);
			CheckEntityWithCertainTile(entity, indexInVector - m_dimensions.y - 1);
			CheckEntityWithCertainTile(entity, indexInVector - m_dimensions.y + 1);
			CheckEntityWithCertainTile(entity, indexInVector + m_dimensions.y - 1);
			CheckEntityWithCertainTile(entity, indexInVector + m_dimensions.y + 1);
		}
		if (m_allEntities[i]->m_entityType == EntityType::_EVIL_LEO)
		{
			Entity* entity = m_allEntities[i];
			IntVec2 tilePos = entity->GetTilePosition();

			int indexInVector = m_dimensions.y * tilePos.x + (tilePos.y);

			CheckEntityWithCertainTile(entity, indexInVector);
			CheckEntityWithCertainTile(entity, indexInVector - 1);
			CheckEntityWithCertainTile(entity, indexInVector + 1);
			CheckEntityWithCertainTile(entity, indexInVector - m_dimensions.y);
			CheckEntityWithCertainTile(entity, indexInVector + m_dimensions.y);
			CheckEntityWithCertainTile(entity, indexInVector - m_dimensions.y - 1);
			CheckEntityWithCertainTile(entity, indexInVector - m_dimensions.y + 1);
			CheckEntityWithCertainTile(entity, indexInVector + m_dimensions.y - 1);
			CheckEntityWithCertainTile(entity, indexInVector + m_dimensions.y + 1);
		}
		if (m_allEntities[i]->m_entityType == EntityType::_EVIL_SCORPIO)
		{
			Entity* entity = m_allEntities[i];
			IntVec2 tilePos = entity->GetTilePosition();

			int indexInVector = m_dimensions.y * tilePos.x + (tilePos.y);

			CheckEntityWithCertainTile(entity, indexInVector);
			CheckEntityWithCertainTile(entity, indexInVector - 1);
			CheckEntityWithCertainTile(entity, indexInVector + 1);
			CheckEntityWithCertainTile(entity, indexInVector - m_dimensions.y);
			CheckEntityWithCertainTile(entity, indexInVector + m_dimensions.y);
			CheckEntityWithCertainTile(entity, indexInVector - m_dimensions.y - 1);
			CheckEntityWithCertainTile(entity, indexInVector - m_dimensions.y + 1);
			CheckEntityWithCertainTile(entity, indexInVector + m_dimensions.y - 1);
			CheckEntityWithCertainTile(entity, indexInVector + m_dimensions.y + 1);
		}
		if (m_allEntities[i]->m_entityType == EntityType::_EVIL_ARIES)
		{
			Entity* entity = m_allEntities[i];
			IntVec2 tilePos = entity->GetTilePosition();

			int indexInVector = m_dimensions.y * tilePos.x + (tilePos.y);

			CheckEntityWithCertainTile(entity, indexInVector);
			CheckEntityWithCertainTile(entity, indexInVector - 1);
			CheckEntityWithCertainTile(entity, indexInVector + 1);
			CheckEntityWithCertainTile(entity, indexInVector - m_dimensions.y);
			CheckEntityWithCertainTile(entity, indexInVector + m_dimensions.y);
			CheckEntityWithCertainTile(entity, indexInVector - m_dimensions.y - 1);
			CheckEntityWithCertainTile(entity, indexInVector - m_dimensions.y + 1);
			CheckEntityWithCertainTile(entity, indexInVector + m_dimensions.y - 1);
			CheckEntityWithCertainTile(entity, indexInVector + m_dimensions.y + 1);
		}
	}
}

void Map::CheckEntityCollisionWithEntity()
{
	for (int i = 0; i < m_allEntities.size(); i++)
	{
		Entity* A = m_allEntities[i];
		if (A == nullptr)  continue;

		for (int j = i + 1; j < m_allEntities.size(); j++)
		{
			Entity* B = m_allEntities[j];
			if (B == nullptr)  continue;

			float distanceAB = GetDistance2D(A->m_position, B->m_position);
			if (distanceAB > A->m_physicsRadius + B->m_physicsRadius)  continue;

			if (!A->m_doesPushEntities || !B->m_doesPushEntities)
			{
				continue;
			}
			else if (A->m_isPushedByEntities && !B->m_isPushedByEntities)
			{
				PushDiscOutOfDisc2D(A->m_position, A->m_physicsRadius, B->m_position, B->m_physicsRadius);
			}
			else if (!A->m_isPushedByEntities && B->m_isPushedByEntities)
			{
				PushDiscOutOfDisc2D(B->m_position, B->m_physicsRadius, A->m_position, A->m_physicsRadius);
			}
			else if (A->m_isPushedByEntities && B->m_isPushedByEntities)
			{
				PushDiscsOutOfEachOther2D(A->m_position, A->m_physicsRadius, B->m_position, B->m_physicsRadius);
			}

		}
	}
}

void Map::ClearAllGarbage()
{
	for (int entityIndex = 0; entityIndex < m_allEntities.size(); entityIndex++)
	{
		Entity* entity = m_allEntities[entityIndex];

		if (entity != nullptr)
		{
			if (entity->m_isGarbage)
			{
				RemoveEntityFromMap(*m_allEntities[entityIndex]);
				delete entity;
			}
		}
	}
}


void Map::Render() const
{
	switch (m_currentMode)
	{
	case MapRenderMode::MAP_MODE_NORMAL:
		RenderTiles();
		RenderEntities();
		break;
	case MapRenderMode::MAP_MODE_HEATMAP_NOTILES:
		RenderDistanceFieldMap();
		RenderEntities();
		break;
	case MapRenderMode::MAP_MODE_NUM:
		break;
	default:
		break;
	}
}

void Map::RenderTiles() const
{

	Vec2 uvAtMins, uvAtMaxs;
	std::vector<Vertex_PCU> tileVerts;
	tileVerts.reserve(m_tiles.size() * 6);
	SpriteSheet* spriteSheet = g_theGame->GetTileSpriteSheet();

	for (int tileIndex = 0; tileIndex < m_tiles.size(); tileIndex++)
	{
		IntVec2 tileSpritePos = m_tiles[tileIndex].GetTileDef()->m_spriteCoords;

		uvAtMins = spriteSheet->GetSpriteUVsByIntPos(tileSpritePos).m_mins;
		uvAtMaxs = spriteSheet->GetSpriteUVsByIntPos(tileSpritePos).m_maxs;
		AddVertsForAABB2D(tileVerts, m_tiles[tileIndex].GetBounds(), m_tiles[tileIndex].GetColor(), uvAtMins, uvAtMaxs);
	}

	g_theRenderer->BindTexture(&spriteSheet->GetTexture());

	g_theRenderer->DrawVertexArray((int)tileVerts.size(), tileVerts.data());
}

void Map::RenderEntities() const
{
	for (size_t i = 0; i < m_allEntities.size(); ++i)
	{
		if (m_allEntities[i] != nullptr)
		{
			m_allEntities[i]->Render();
		}
	}
}


void Map::RenderDistanceFieldMap() const
{
	std::vector<Vertex_PCU> heatMapVerts;
	float maxValueInHeatMap = m_dijkstraHeatMap->GetHighestNonSpecialValue(999.0f);

	for (int tileIndex = 0; tileIndex < m_tiles.size(); ++tileIndex)
	{
		m_dijkstraHeatMap->AddVertsForDebugDraw(heatMapVerts, m_tiles[tileIndex].GetBounds(),
			FloatRange(0.0f, maxValueInHeatMap),
			Rgba8(0, 0, 0, 100),
			Rgba8(255, 255, 255, 100),
			999.0f);
	}

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray((int)heatMapVerts.size(), heatMapVerts.data());
}

void Map::RenderDeveloperMode() const
{
	if (!m_entityListsByType[(int)EntityType::ENTITY_TYPE_GOOD_PLAYER][0]->m_isDead
		&& !m_entityListsByType[(int)EntityType::ENTITY_TYPE_GOOD_PLAYER][0]->m_isGarbage)
	{
		Player* player = dynamic_cast<Player*>(m_entityListsByType[(int)EntityType::ENTITY_TYPE_GOOD_PLAYER][0]);
		Vec2 position = player->m_position;
		Vec2 forward = player->GetForwardNormal();
		Vec2 orientationAbsolute = player->GetTurret()->m_faceDirection;
		Vec2 rotate90target = forward.GetRotated90Degrees();
		Vec2 velocity = player->m_velocity;
		float cosmeticRadius = player->m_cosmeticRadius;
		float physicsRadius = player->m_physicsRadius;
		float target = player->GetTargetDegree();
		Vec2 orientationGoal = Vec2(1.0f, 0.0f).GetRotatedDegrees(target);
		float turretTarget = player->GetTurret()->m_orientationGoal;
		Vec2 turretGoal = Vec2(1.0f, 0.0f).GetRotatedDegrees(turretTarget);

		DebugDrawRing(position, cosmeticRadius, 0.05f, Rgba8::MAGENTA);
		DebugDrawRing(position, physicsRadius, 0.05f, Rgba8::CYAN);
		DebugDrawLine(position, position + rotate90target * cosmeticRadius, 0.05f, Rgba8::GREEN);
		DebugDrawLine(position, position + velocity, 0.05f, Rgba8::YELLOW);
		DebugDrawLine(position, position + forward * cosmeticRadius, 0.05f, Rgba8::RED);
		DebugDrawLine(position + orientationGoal * cosmeticRadius, position + orientationGoal * (cosmeticRadius + 0.1f), 0.05f, Rgba8::RED);
		DebugDrawLine(position, position + orientationAbsolute * (cosmeticRadius + 0.2f), 0.1f, Rgba8::BLUE);
		DebugDrawLine(position + turretGoal * cosmeticRadius, position + turretGoal * (cosmeticRadius + 0.05f), 0.1f, Rgba8::BLUE);
	}


	for (int i = 1; i < m_allEntities.size(); ++i)
	{
		if (m_allEntities[i] == nullptr)
		{
			continue;
		}
		if (!m_allEntities[i]->m_isDead && !m_allEntities[i]->m_isGarbage)
		{
				Vec2 position = m_allEntities[i]->m_position;
				Vec2 forward = Vec2(1.0f, 0.0f).GetRotatedDegrees(m_allEntities[i]->m_orientationDegrees);
				Vec2 left = forward.GetRotated90Degrees();
				Vec2 velocity = m_allEntities[i]->m_velocity;
				float cosmeticRadius = m_allEntities[i]->m_cosmeticRadius;
				float physicsRadius = m_allEntities[i]->m_physicsRadius;

			if (m_allEntities[i]->m_entityType != EntityType::_EVIL_BULLET && m_allEntities[i]->m_entityType != EntityType::_GOOD_BULLET && m_allEntities[i]->m_entityType != EntityType::_EVIL_SCORPIO)
			{
// 				DebugDrawRing(position, physicsRadius - 0.1f, 0.05f, Rgba8::CYAN);
// 				DebugDrawLine(position, position + forward * cosmeticRadius * 2.f, 0.03f, Rgba8::RED);
// 				DebugDrawLine(position, position + left * cosmeticRadius * 2.f, 0.03f, Rgba8::GREEN);

				DebugDrawRing(position, cosmeticRadius, 0.05f, Rgba8::MAGENTA);
				DebugDrawRing(position, physicsRadius, 0.05f, Rgba8::CYAN);
				DebugDrawLine(position, position + forward * cosmeticRadius * 2.f, 0.05f, Rgba8::RED);
				DebugDrawLine(position, position + left * cosmeticRadius * 2.f, 0.05f, Rgba8::GREEN);
			}

			if (m_allEntities[i]->m_entityType == EntityType::_EVIL_SCORPIO)
			{
				Scorpio* scorpio = dynamic_cast<Scorpio*>(m_allEntities[i]);
				Vec2 orientation = scorpio->GetTurretOrientation();
				Vec2 leftOrientation = orientation.GetRotated90Degrees();
				DebugDrawRing(position, cosmeticRadius, 0.05f, Rgba8::MAGENTA);
				DebugDrawRing(position, physicsRadius, 0.05f, Rgba8::CYAN);
				DebugDrawLine(position, position + orientation * cosmeticRadius * 2.f, 0.03f, Rgba8::RED);
				DebugDrawLine(position, position + leftOrientation * cosmeticRadius, 0.03f, Rgba8::GREEN);
			}

// 			if (m_allEntities[i]->m_entityType == EntityType::_EVIL_LEO)
// 			{
// 				Leo* leo = dynamic_cast<Leo*>(m_allEntities[i]);
// 				Vec2 targetPosition = leo->GetLastSeenPosition();
// 				Vec2 nextWayPointPosition = leo->GetNextWayPointPosition();
// 				DebugDrawRing(targetPosition, 0.1f, 0.01f, Rgba8::MAGENTA);
// 				DebugDrawRing(nextWayPointPosition, 0.1f, 0.01f, Rgba8::RED);
// 				DebugDrawLine(targetPosition, leo->m_position, 0.01f, Rgba8::WHITE);
// 			}
// 			if (m_allEntities[i]->m_entityType == EntityType::_EVIL_ARIES)
// 			{
// 				Aries* aries = dynamic_cast<Aries*>(m_allEntities[i]);
// 				Vec2 targetPosition = aries->GetLastSeenPosition();
// 				Vec2 nextWayPointPosition = aries->GetNextWayPointPosition();
// 				DebugDrawRing(targetPosition, 0.1f, 0.01f, Rgba8::MAGENTA);
// 				DebugDrawRing(nextWayPointPosition, 0.1f, 0.01f, Rgba8::RED);
// 				DebugDrawLine(targetPosition, aries->m_position, 0.01f, Rgba8::WHITE);
// 			}
		}
	}
}

void Map::AddEntityInMap(Entity& entity)
{
	entity.m_map = this;
	AddEntityIntoCertainVector(m_allEntities, entity);
	AddEntityIntoCertainVector(m_entityListsByType[(int)entity.m_entityType], entity);
	AddEntityIntoCertainVector(m_factionEntities[(int)entity.m_entityFaction], entity);
}

void Map::AddEntityIntoCertainVector(EntityList& listToAdd, Entity& entity)
{
	for (int i = 0; i < listToAdd.size(); i++)
	{
		if (listToAdd[i] == nullptr)
		{
			listToAdd[i] = &entity;
			return;
		}
	}
	listToAdd.push_back(&entity);
}

void Map::RemoveEntityFromMap(Entity& entity)
{
	for (int i = 0; i < m_allEntities.size(); i++)
	{
		if (m_allEntities[i] == &entity)
		{
			m_allEntities[i] = nullptr;
		}
	}

	for (int j = 0; j < m_entityListsByType[(int)entity.m_entityType].size(); j++)
	{
		if (m_entityListsByType[(int)entity.m_entityType][j] == &entity)
		{
			m_entityListsByType[(int)entity.m_entityType][j] = nullptr;
		}
	}

	for (int k = 0; k < m_factionEntities[(int)entity.m_entityFaction].size(); k++)
	{
		if (m_factionEntities[(int)entity.m_entityFaction][k] == &entity)
		{
			m_factionEntities[(int)entity.m_entityFaction][k] = nullptr;
		}
	}
}


void Map::SetStartEntityNum(int scorpioNum, int leoNum, int ariesNum)
{
	m_mapDefinition.m_scorpioNum = scorpioNum;
	m_mapDefinition.m_leoNum = leoNum;
	m_mapDefinition.m_ariesNum = ariesNum;
}

IntVec2 Map::GenerateTilePositionInsideDimensions() const
{
	int spawnTilePosX = g_theRNG->RollRandomIntInRange(1, m_mapDefinition.m_dimensions.x - 2);
	int spawnTilePosY = g_theRNG->RollRandomIntInRange(1, m_mapDefinition.m_dimensions.y - 2);
	return IntVec2(spawnTilePosX, spawnTilePosY);
}

// void Map::AddWalkableTileTypes(TileType type)
// {
// 	m_walkableTypes.push_back(type);
// }
// 
// void Map::AddNonWalkableTileTypes(TileType type)
// {
// 	m_nonwalkableTypes.push_back(type);
// }

Vec2 Map::GeneratePositionBaseOnDimensions() const
{
	int spawnTilePosX = g_theRNG->RollRandomIntInRange(1, m_mapDefinition.m_dimensions.x - 2);
	int spawnTilePosY = g_theRNG->RollRandomIntInRange(1, m_mapDefinition.m_dimensions.y - 2);

	float insideTilePosX = g_theRNG->RollRandomFloatInRange(0.0f, 1.f);
	float insideTilePosY = g_theRNG->RollRandomFloatInRange(0.0f, 1.f);

	return Vec2(spawnTilePosX + insideTilePosX, spawnTilePosY + insideTilePosY);

}

Vec2 Map::GeneratePositionNotInSolid() const
{
	Vec2 pos = GeneratePositionBaseOnDimensions();

	while (GetTileByPos(pos).GetTileDef()->m_isSolid)
	{
		pos = GeneratePositionBaseOnDimensions();
	}

	return pos;
}

Tile Map::GetTileByIndex(int tileIndex) const
{
	return m_tiles[tileIndex];
}

IntVec2 Map::GetDimensions() const
{
	return m_mapDefinition.m_dimensions;
}

int Map::GetTilesSize() const
{
	return (int)m_tiles.size();
}

bool Map::PopulateDistanceField(TileHeatMap& out_distanceField, IntVec2 startCoords, float maxCost, bool treatWaterAsSolid, bool treatScorpioAsSolid)
{
	UNUSED(treatWaterAsSolid);
	out_distanceField.SetAllValues(maxCost);
	int startTileIndex = GetTileIndexByPos(GetDimensions().y, startCoords);
	out_distanceField.SetValueByIndex(startTileIndex, 0.0f);

	for (int valueIndex = 0; valueIndex < out_distanceField.GetValuesSize(); ++valueIndex)
	{
		if (m_tiles[valueIndex].GetTileDef()->m_isSolid)
		{
			out_distanceField.SetValueByIndex(valueIndex, maxCost);
			continue;
		}
	}


	bool mapBeenChange = true;
	float currentLowestValue = 0.0f;
	while (mapBeenChange && currentLowestValue < maxCost)
	{
		mapBeenChange = false;

		for (int valueIndex = 0; valueIndex < out_distanceField.GetValuesSize(); ++valueIndex)
		{
			if (out_distanceField.GetHeatValueByIndex(valueIndex) == currentLowestValue)
			{
				if (valueIndex - GetDimensions().y > 0 &&
					valueIndex - GetDimensions().y < out_distanceField.GetValuesSize() &&
					m_tiles[valueIndex - GetDimensions().y].GetTileDef()->m_isSolid == false &&
					out_distanceField.GetHeatValueByIndex(valueIndex - GetDimensions().y) >= currentLowestValue)
				{
					if (!treatScorpioAsSolid || !IsOverlapWithScorpio(m_tiles[valueIndex - GetDimensions().y].GetWorldPosCenter()))
					{
						out_distanceField.SetValueByIndex(valueIndex - GetDimensions().y, currentLowestValue + 1.0f);
						mapBeenChange = true;
					}
				}

				if (valueIndex + GetDimensions().y > 0 &&
					valueIndex + GetDimensions().y < out_distanceField.GetValuesSize() &&
					m_tiles[valueIndex + GetDimensions().y].GetTileDef()->m_isSolid == false &&
					out_distanceField.GetHeatValueByIndex(valueIndex + GetDimensions().y) >= currentLowestValue)
				{
					if (!treatScorpioAsSolid || !IsOverlapWithScorpio(m_tiles[valueIndex + GetDimensions().y].GetWorldPosCenter()))
					{
						out_distanceField.SetValueByIndex(valueIndex + GetDimensions().y, currentLowestValue + 1.0f);
						mapBeenChange = true;
					}
				}

				if (valueIndex + 1 > 0 &&
					valueIndex + 1 < out_distanceField.GetValuesSize() &&
					m_tiles[valueIndex + 1].GetTileDef()->m_isSolid == false &&
					out_distanceField.GetHeatValueByIndex(valueIndex + 1) >= currentLowestValue)
				{
					if (!treatScorpioAsSolid || !IsOverlapWithScorpio(m_tiles[valueIndex + 1].GetWorldPosCenter()))
					{
						out_distanceField.SetValueByIndex(valueIndex + 1, currentLowestValue + 1.0f);
						mapBeenChange = true;
					}
				}

				if (valueIndex - 1 > 0 &&
					valueIndex - 1 < out_distanceField.GetValuesSize() &&
					m_tiles[valueIndex - 1].GetTileDef()->m_isSolid == false &&
					out_distanceField.GetHeatValueByIndex(valueIndex - 1) >= currentLowestValue)
				{
					if (!treatScorpioAsSolid || !IsOverlapWithScorpio(m_tiles[valueIndex - 1].GetWorldPosCenter()))
					{
						out_distanceField.SetValueByIndex(valueIndex - 1, currentLowestValue + 1.0f);
						mapBeenChange = true;
					}
				}

			}
		}
		if (mapBeenChange)
		{
			currentLowestValue += 1.0f;
		}
	}
	Vec2& exitPosition = m_mapDefinition.m_exitPosition;
	IntVec2 exitPositionInt = IntVec2(RoundDownToInt(exitPosition.x), RoundDownToInt(exitPosition.y));
	int index = GetTileIndexByPos(m_mapDefinition.m_dimensions.y, exitPositionInt);

	return out_distanceField.GetHeatValueByIndex(index) != maxCost;
}

bool Map::IsOverlapWithScorpio(Vec2 const& comparePosition) const
{
	for (int i = 0; i < m_entityListsByType[(int)EntityType::_EVIL_SCORPIO].size(); ++i)
	{
		if (m_entityListsByType[(int)EntityType::_EVIL_SCORPIO][i] == nullptr)
		{
			continue;
		}

		if (IsPointInsideDisc2D(comparePosition,
			m_entityListsByType[(int)EntityType::_EVIL_SCORPIO][i]->m_position,
			m_entityListsByType[(int)EntityType::_EVIL_SCORPIO][i]->m_physicsRadius))
		{
			return true;
		}
	}

	return false;
}

void Map::SwitchRenderMode()
{
	switch (m_currentMode)
	{
	case MapRenderMode::MAP_MODE_NORMAL:
		m_currentMode = MapRenderMode::MAP_MODE_HEATMAP_NOTILES;
		break;
	case MapRenderMode::MAP_MODE_HEATMAP_NOTILES:
		m_currentMode = MapRenderMode::MAP_MODE_NORMAL;
		break;
	case MapRenderMode::MAP_MODE_NUM:
		break;
	default:
		break;
	}
}

Tile Map::GetTileByPos(Vec2 const& pointPos) const
{
	//if (pointPos.x < 0.0f || pointPos .y < 0.0f ||
	//	pointPos.x > (float)m_dimensions.x || pointPos.y >(float)m_dimensions.y)
	//{

	//}
	int tilePosX = static_cast<int>(RoundDownToInt(pointPos.x));
	int tilePosY = static_cast<int>(RoundDownToInt(pointPos.y));

	IntVec2 tilePos = IntVec2(tilePosX, tilePosY);
	int tileIndex = GetTileIndexByPos(GetDimensions().y, tilePos);
	Tile tile = GetTileByIndex(tileIndex);
	return tile;
}

Entity* Map::GetPlayer() const
{
	return m_entityListsByType[(int)EntityType::ENTITY_TYPE_GOOD_PLAYER][0];
}

std::string Map::GetMapName() const
{
	return m_mapDefinition.m_name;
}

std::vector<int> Map::GetAllScorpioAndSolidTilesIndex() const
{
	std::vector<int> indexList;
	indexList.reserve(100);

	for (int scorpioIndex = 0; scorpioIndex < m_entityListsByType[(int)EntityType::_EVIL_SCORPIO].size(); ++scorpioIndex)
	{
		if (m_entityListsByType[(int)EntityType::_EVIL_SCORPIO][scorpioIndex] != nullptr &&
			!m_entityListsByType[(int)EntityType::_EVIL_SCORPIO][scorpioIndex]->m_isGarbage &&
			m_entityListsByType[(int)EntityType::_EVIL_SCORPIO][scorpioIndex]->IsAlive())
		{

			int scorpioIntPosX = RoundDownToInt(m_entityListsByType[(int)EntityType::_EVIL_SCORPIO][scorpioIndex]->m_position.x);
			int scorpioIntPosY = RoundDownToInt(m_entityListsByType[(int)EntityType::_EVIL_SCORPIO][scorpioIndex]->m_position.y);

			IntVec2 intPos = IntVec2(scorpioIntPosX, scorpioIntPosY);

			int tileIndex = GetTileIndexByPos(m_mapDefinition.m_dimensions.y, intPos);

			indexList.push_back(tileIndex);
		}
	}

	for (int tileIndex = 0; tileIndex < m_tiles.size(); ++tileIndex)
	{
		if (m_tiles[tileIndex].GetTileDef()->m_isSolid)
		{
			indexList.push_back(tileIndex);
		}
	}

	return indexList;
}

bool Map::IsPointInSolid(Vec2 const& point)
{
	int tilePosX = static_cast<int>(RoundDownToInt(point.x));
	int tilePosY = static_cast<int>(RoundDownToInt(point.y));

	if (tilePosX >= m_mapDefinition.m_dimensions.x || tilePosX < 0)
	{
		return false;
	}

	if (tilePosY >= m_mapDefinition.m_dimensions.y || tilePosY < 0)
	{
		return false;
	}
	IntVec2 tilePos = IntVec2(tilePosX, tilePosY);
	return IsTileSolid(tilePos);
}

bool Map::IsTileSolid(IntVec2 const& tilePos)
{
	int tileIndex = GetTileIndexByPos(GetDimensions().y, tilePos);
	if (tileIndex < 0 || tileIndex >= m_tiles.size())
	{
		return false;
	}

	Tile tile = GetTileByIndex(tileIndex);
	return tile.GetTileDef()->m_isSolid;
}

RaycastHitResult Map::RaycastVsTiles(Vec2 const& startPostion, Vec2 const& rayDirectionNormal, float maxDistance)
{
	Vec2 currentRayPosition = startPostion;
	Vec2 lastRayPosition = currentRayPosition;
	int	 m_raycasySteps = 100;
	float singleStep = maxDistance / m_raycasySteps;
	RaycastHitResult hitInfo = RaycastHitResult();

	for (int i = 0; i < m_raycasySteps; i++)
	{
		currentRayPosition += rayDirectionNormal * singleStep;
		if (IsPointInSolid(currentRayPosition))
		{
			hitInfo.m_hitAnything = true;
			hitInfo.m_hitPoint = lastRayPosition;
			int tilePosXCurrent = static_cast<int>(RoundDownToInt(currentRayPosition.x));
			int tilePosYCurrent = static_cast<int>(RoundDownToInt(currentRayPosition.y));
			int tilePosXLast = static_cast<int>(RoundDownToInt(lastRayPosition.x));
			int tilePosYLast = static_cast<int>(RoundDownToInt(lastRayPosition.y));

			hitInfo.m_hitNormal = Vec2((float)(tilePosXLast - tilePosXCurrent), (float)(tilePosYLast - tilePosYCurrent));
			return hitInfo;
		}

		lastRayPosition = currentRayPosition;
	}
	hitInfo.m_hitAnything = false;
	hitInfo.m_hitPoint = currentRayPosition;
	return hitInfo;
}

bool Map::HasLineOfSight(Entity* startEntity, Entity* endEntity, float maxDistance)
{
	if (endEntity->m_isDead || endEntity->m_isGarbage)
	{
		return false;
	}

	Vec2 currentRayPosition = startEntity->m_position;
	Vec2 toPlayerDirection = (endEntity->m_position - startEntity->m_position).GetNormalized();
	int	 m_raycasySteps = 50;
	float singleStep = maxDistance / m_raycasySteps;

	for (int i = 0; i < m_raycasySteps; i++)
	{
		currentRayPosition += toPlayerDirection * singleStep;

		if (IsPointInSolid(currentRayPosition))
		{
			return false;
		}

		if (IsPointInsideDisc2D(currentRayPosition, endEntity->m_position, endEntity->m_physicsRadius))
		{
			return true;
		}

	}

	return false;
}


MapDefinition::MapDefinition()
{
}


void MapDefinition::InitializeMapDefs()
{
	XmlDocument doc;
	doc.LoadFile("Data/Definitions/MapDefinitions.xml");
	XmlElement* rootElement = doc.RootElement();

	XmlElement* childElementIterator = rootElement->FirstChildElement();

	while (childElementIterator)
	{
		NamedStrings mapDefinitionNameString = NamedStrings();

		mapDefinitionNameString.PopulateFromXmlElementAttributes(*childElementIterator);

		MapDefinition mapDefinition = MapDefinition();

		mapDefinition.m_name = mapDefinitionNameString.GetValue("name", "NOT_LOADED");
		mapDefinition.m_dimensions = mapDefinitionNameString.GetValue("dimensions", IntVec2(1, 1));
		mapDefinition.m_fillType = TileDefinition::s_definitions.find(mapDefinitionNameString.GetValue("fillTileType", "NOT_LOADED"))->second;
		mapDefinition.m_edgeType = TileDefinition::s_definitions.find(mapDefinitionNameString.GetValue("edgeTileType", "NOT_LOADED"))->second;

		mapDefinition.m_wormTypes.reserve(2);
		mapDefinition.m_wormCounts.reserve(2);
		mapDefinition.m_wormMaxLength.reserve(2);

		mapDefinition.m_wormTypes.push_back(TileDefinition::s_definitions.find(mapDefinitionNameString.GetValue("worm1TileType", "NOT_LOADED"))->second);
		mapDefinition.m_wormCounts.push_back(mapDefinitionNameString.GetValue("worm1Count", 1));
		mapDefinition.m_wormMaxLength.push_back(mapDefinitionNameString.GetValue("worm1MaxLength", 1));

		mapDefinition.m_wormTypes.push_back(TileDefinition::s_definitions.find(mapDefinitionNameString.GetValue("worm2TileType", "NOT_LOADED"))->second);
		mapDefinition.m_wormCounts.push_back(mapDefinitionNameString.GetValue("worm2Count", 1));
		mapDefinition.m_wormMaxLength.push_back(mapDefinitionNameString.GetValue("worm2MaxLength", 1));

		mapDefinition.m_startFloorTileType = TileDefinition::s_definitions.find(mapDefinitionNameString.GetValue("startFloorTileType", "NOT_LOADED"))->second;
		mapDefinition.m_startBunkerTileType = TileDefinition::s_definitions.find(mapDefinitionNameString.GetValue("startBunkerTileType", "NOT_LOADED"))->second;
		mapDefinition.m_endFloorTileType = TileDefinition::s_definitions.find(mapDefinitionNameString.GetValue("endFloorTileType", "NOT_LOADED"))->second;
		mapDefinition.m_endBunkerTileType = TileDefinition::s_definitions.find(mapDefinitionNameString.GetValue("endBunkerTileType", "NOT_LOADED"))->second;

		mapDefinition.m_leoNum = mapDefinitionNameString.GetValue("leoCount", 1);
		mapDefinition.m_ariesNum = mapDefinitionNameString.GetValue("ariesCount", 1);
		mapDefinition.m_scorpioNum = mapDefinitionNameString.GetValue("scorpioCount", 1);
		mapDefinition.m_taurusNum = mapDefinitionNameString.GetValue("taurusCount", 1);
		mapDefinition.m_capricornNum = mapDefinitionNameString.GetValue("capricornCount", 1);
		mapDefinition.m_sagittariusNum = mapDefinitionNameString.GetValue("sagittariusCount", 1);

		s_mapDefinitions[mapDefinition.m_name] = mapDefinition;

		childElementIterator = childElementIterator->NextSiblingElement();
	}
}