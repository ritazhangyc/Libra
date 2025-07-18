#include "Game/Tile.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include <map>
#include <string>


std::map<std::string, TileDefinition> TileDefinition::s_definitions;

Tile::Tile(IntVec2 const& tilePosition, TileDefinition const& tileDef)
	:m_tileCoords(tilePosition)
	,m_tileDef( &tileDef )
{
}

Tile::Tile(const Tile& copyFrom)
	:m_tileCoords(copyFrom.m_tileCoords)
	,m_tileDef(copyFrom.m_tileDef)
{
}

Tile::~Tile()
{

}


IntVec2	const Tile::GetTilePosition() const
{
	return m_tileCoords;
}

TileDefinition const* Tile::GetTileDef() const
{
	return m_tileDef;
}

int	const Tile::GetTilePositionX() const
{
	return m_tileCoords.x;
}

int const Tile::GetTilePositionY() const
{
	return m_tileCoords.y;
}

Vec2 const Tile::GetWorldPosMin() const
{
	return Vec2( static_cast<float>( m_tileCoords.x ), static_cast<float>( m_tileCoords.y ) );
}

Vec2 const Tile::GetWorldPosMax() const
{
	return Vec2( static_cast<float>( m_tileCoords.x+1 ), static_cast<float>( m_tileCoords.y+1 ) );
}

AABB2 const Tile::GetBounds() const
{
	AABB2 bounds;
	bounds.m_mins = GetWorldPosMin();
	bounds.m_maxs = GetWorldPosMax();
	return bounds;
}

Rgba8 const Tile::GetColor() const
{
	return m_tileDef->m_tintColor;
}

void Tile::SetTilePosition(int pos_x, int pos_y)
{
	m_tileCoords.x = pos_x;
	m_tileCoords.y = pos_y;
}

void Tile::SetTileDef(TileDefinition& tileDef)
{
	m_tileDef = &tileDef;
}

Vec2 const Tile::GetWorldPosCenter() const
{
	return GetWorldPosMin() + Vec2( 0.5f, 0.5f );
}

void TileDefinition::InitializeTileDefs()
{
	XmlDocument doc;
	doc.LoadFile("Data/Definitions/TileDefinitions.xml");
	XmlElement* rootElement = doc.RootElement();

	XmlElement* childElementIterator = rootElement->FirstChildElement();

	while (childElementIterator)
	{
		NamedStrings tileDefinitionNameString = NamedStrings();

		tileDefinitionNameString.PopulateFromXmlElementAttributes(*childElementIterator);

		TileDefinition tileDefinition = TileDefinition();

		tileDefinition.m_name = tileDefinitionNameString.GetValue("name", "NOT_LOADED");
		tileDefinition.m_isSolid = tileDefinitionNameString.GetValue("isSolid", false);
		tileDefinition.m_spriteCoords = tileDefinitionNameString.GetValue("spriteCoords", IntVec2(0, 0));
		tileDefinition.m_tintColor = tileDefinitionNameString.GetValue("tint", Rgba8::WHITE);

		s_definitions[tileDefinitionNameString.GetValue("name", "NOT_LOADED")] = tileDefinition;

		childElementIterator = childElementIterator->NextSiblingElement();
	}
}
