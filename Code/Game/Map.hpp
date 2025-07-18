#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Tile.hpp"
#include <map>

class Tile;
class Entity;
class TileHeatMap;


typedef std::vector<Entity*> EntityList;

enum class MapRenderMode
{
	MAP_MODE_NORMAL,
	MAP_MODE_HEATMAP_NOTILES,
	MAP_MODE_NUM
};

class MapDefinition
{
public:

	MapDefinition();

	std::string										m_name;
	IntVec2											m_dimensions = IntVec2(0, 0);

	TileDefinition m_fillType;
	TileDefinition m_edgeType;
	TileDefinition m_startFloorTileType;
	TileDefinition m_startBunkerTileType;
	TileDefinition m_endFloorTileType;
	TileDefinition m_endBunkerTileType;

	std::vector<TileDefinition>						m_wormTypes;
	std::vector<int>								m_wormCounts;
	std::vector<int>								m_wormMaxLength;

	Vec2 m_exitPosition = Vec2(0.0f, 0.0f);

	int	m_scorpioNum = 0;
	int	m_leoNum = 0;
	int	m_ariesNum = 0;
	int	m_taurusNum = 0;
	int	m_capricornNum = 0;
	int	m_sagittariusNum = 0;

	static void										InitializeMapDefs();
	static std::map<std::string, MapDefinition>		s_mapDefinitions;
};

class Map
{
public:

	Map();
//	Map(int width, int height);
	Map(MapDefinition const& mapDefition);
	~Map();

	void						StartUp();
	void						SpawnFixedValues();
	void						SpawnAccessibleMap();

	void						SpawnMaps();
	void						SpawnWorm();
	void						SpawnStartExitArea();
	void						SpawnEntities();
	void						SpawnNewEntity(EntityType type, Vec2 const& position, float orientationDegrees);

	void						Update(float deltaSeconds);
	void						UpdateAllEntities(float deltaSeconds);
	void						CheckEntityCollisionWithWall();
	void						CheckEntityCollisionWithEntity();
	void						CheckEntityWithCertainTile(Entity* entity, int tileIndex);
	void						CheckBulletCollisionWithEntities();
	void						CheckPlayerPositionOverlapwithExit();
	void						ClearAllGarbage();

	void						Render() const;
	void						RenderTiles() const;
	void						RenderEntities() const;

	void						RenderDistanceFieldMap() const;
	void						RenderDeveloperMode() const;

	void						AddEntityInMap(Entity& entity);
	void						AddEntityIntoCertainVector(EntityList& listToAdd, Entity& entity);
	void						RemoveEntityFromMap(Entity& entity);
	void						SetStartEntityNum(int scorpioNum, int leoNum, int ariesNum);
	
// 	void						AddWalkableTileTypes(TileType type);
// 	void						AddNonWalkableTileTypes(TileType type);
	int							GetTilesSize() const;

	bool						PopulateDistanceField(TileHeatMap& out_distanceField, IntVec2 startCoords, float maxConst = 999.0f, bool treatWaterAsSolid = true, bool treatScorpioAsSolid = false);
	bool						IsOverlapWithScorpio(Vec2 const& comparePosition) const;
	void						SwitchRenderMode();

	IntVec2						GenerateTilePositionInsideDimensions() const;
	Vec2						GeneratePositionBaseOnDimensions() const;
	Vec2						GeneratePositionNotInSolid() const;
	Tile						GetTileByIndex(int tileIndex) const;
	IntVec2						GetDimensions() const;
	Tile						GetTileByPos(Vec2 const& pointPos) const;
	Entity*						GetPlayer() const;
	std::string					GetMapName() const;
	std::vector<int>			GetAllScorpioAndSolidTilesIndex() const;

	bool						IsPointInSolid(Vec2 const& point);
	bool						IsTileSolid(IntVec2 const& tilePos);
	RaycastHitResult			RaycastVsTiles(Vec2 const& startPostion, Vec2 const& rayDirectionNormal, float maxDistance);
	bool						HasLineOfSight(Entity* startEntity, Entity* endEntity, float maxDistance);

public:
	std::vector<Tile>			m_tiles;			// Note: this is NOT a 2D array!
	std::vector<TileType>		m_walkableTypes;
	std::vector<TileType>		m_nonwalkableTypes;
	IntVec2						m_dimensions;		// # of tiles wide (x) and high (y)
	EntityList					m_allEntities;
	EntityList					m_entityListsByType[NUM_ENTITY_TYPES];
	EntityList					m_factionEntities[(int)EntityFaction::FACTION_NUM];
	Vec2						m_exitPosition = Vec2(0.0f, 0.0f);

private:
	int							m_scorpioNum;
	int							m_leoNum;
	int							m_ariesNum;
	MapDefinition 				m_mapDefinition;
	MapRenderMode				m_currentMode;
	TileHeatMap*				m_dijkstraHeatMap;
};


