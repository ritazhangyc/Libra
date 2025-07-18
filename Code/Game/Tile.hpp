#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include <map>

class TileDefinition
{
public:

	TileDefinition();
	~TileDefinition();

public:
	std::string							m_name;
	AABB2								m_spriteSheetUVS;
	Rgba8								m_tintColor;
	IntVec2								m_spriteCoords;
	bool								m_isSolid;

	static std::map<std::string, TileDefinition>	s_definitions;
	static void										InitializeTileDefs();
};

class Tile
{
public:
	Tile(IntVec2 const& tilePosition, TileDefinition const& tileDef);
	Tile(const Tile& copyFrom);
	~Tile();

	IntVec2	const			GetTilePosition() const;
	TileDefinition const*	GetTileDef() const;

	int	const		GetTilePositionX() const;
	int const		GetTilePositionY() const;
	Vec2 const		GetWorldPosMin() const;
	Vec2 const		GetWorldPosMax() const;
	Vec2 const		GetWorldPosCenter() const;

	AABB2 const		GetBounds() const;
	Rgba8 const		GetColor() const;

	void			SetTilePosition(int pos_x, int pos_y);
	void			SetTileDef(TileDefinition& tileDef);

private:

	IntVec2					m_tileCoords;
	TileDefinition const*	m_tileDef = nullptr;

// 	Vec2			m_worldPosMin;
// 	Vec2			m_worldPosMax;

};

constexpr int eachTileSize = sizeof(Tile);

