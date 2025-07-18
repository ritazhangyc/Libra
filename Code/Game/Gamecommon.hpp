#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <vector>


enum class TileType
{
	TILE_TYPE_NONE = -1,
	TILE_TYPE_GRASS = 0,
	TILE_TYPE_STONE = 1,
	TILE_TYPE_SOIL = 2,
	TILE_TYPE_BLOCK = 3,
	TILE_TYPE_FLOOR = 4,
	TILE_TYPE_EXIT = 5,
	TILE_TYPE_START = 6,
	TILE_NUM
};


enum class GameStatus
{
	STATUS_ATTRACT = 0,
	STATUS_NORMAL,
	STATUS_PAUSE,
	STATUS_VICTORY,
	STATUS_DEFEAT,
	STATUS_TYPE_NUM
};

class RaycastHitResult
{
public:
	bool		m_hitAnything = false;
	Vec2		m_hitPoint = Vec2(0.0f, 0.0f);
	Vec2		m_hitNormal = Vec2(0.0f, 0.0f);
};

class Renderer;
class Window;
class InputSystem;
class AudioSystem;
class App;
class Game;
class SpriteSheet;
class SpriteDefinition;
class BitmapFont;
class Texture;
class Camera;
class Tile;
class Entity;

struct AABB2;
struct Vertex_PCU;
struct Vec2;
struct Rgba8;
struct IntVec2;
struct Vec2;
struct Vec3;
struct LineSegment2;
struct Capsule2;
struct Triangle2;
struct OBB2;
struct Triangle2;

extern Renderer*				g_theRenderer; 
extern Window*					g_theWindow;
extern InputSystem*				g_theInput;
extern AudioSystem*				g_theAudio;
extern App*						g_theApp;
extern Game*					g_theGame;
extern RandomNumberGenerator*	g_theRNG;
extern BitmapFont* g_testFont;

extern bool		g_isDebugDraw;

const	  Vec2	PLAYER_START_POSITION = Vec2(1.5f, 1.5f);
constexpr float	PLAYER_MAX_SPEED = 1.0f;
constexpr float PLAYER_TURN_RATE = 180.0f;
constexpr float PLAYER_PHYSIC_RADIUS = 0.38f;
constexpr float PLAYER_COSMETIC_RADIUS = 0.6f;
constexpr float TURRET_TURN_RATE = 360.0f;

constexpr int	PLAYER_HEALTH = 10;
constexpr float	PLAYER_SHOOT_INTERVAL = 0.1f;

constexpr float SCORPIO_PHYSIC_RADIUS = 0.4f;
constexpr float SCORPIO_COSMETIC_RADIUS = 0.4f;
constexpr float SCORPIO_TURN_RATE = 30.0f;
constexpr float SCORPIO_MAX_DISTANCE = 10.0f;
constexpr int	SCORPIO_HEALTH = 5;

constexpr float LEO_PHYSIC_RADIUS = 0.4f;
constexpr float LEO_COSMETIC_RADIUS = 0.5f;
constexpr float	LEO_MOVE_SPEED = 2.0f;
constexpr float LEO_TURN_RATE = 30.0f;
constexpr float LEO_MAX_DISTANCE = 10.0f;
constexpr int	LEO_HEALTH = 5;

constexpr float ARIES_PHYSIC_RADIUS = 0.3f;
constexpr float ARIES_COSMETIC_RADIUS = 0.4f;
constexpr float	ARIES_MOVE_SPEED = 2.0f;
constexpr float ARIES_TURN_RATE = 30.0f;
constexpr float ARIES_MAX_DISTANCE = 10.0f;
constexpr int	ARIES_HEALTH = 10;

constexpr float BULLET_PHYSIC_RADIUS = 0.1f;
constexpr float BULLET_COSMETIC_HEIGHT = 0.075f;
constexpr float BULLET_COSMETIC_WIDTH = 0.15f;
constexpr float	BULLET_MOVE_SPEED = 8.0f;
constexpr int	GOOD_BULLET_DAMAGE = 1;
constexpr int	BAD_BULLET_DAMAGE = 1;

constexpr int	MAP1_WIDTH_SIZE = 20;
constexpr int   MAP1_HEIGHT_SIZE = 30;
constexpr int	MAP1_ARIES = 5;
constexpr int	MAP1_LEO = 5;
constexpr int	MAP1_SCORPIO = 10;
constexpr int	NUM_ENTITY_TYPES = 6;

constexpr int	MAP2_WIDTH_SIZE = 50;
constexpr int   MAP2_HEIGHT_SIZE = 20;
constexpr int	MAP2_ARIES = 15;
constexpr int	MAP2_LEO = 15;
constexpr int	MAP2_SCORPIO = 15;

constexpr int	MAP3_WIDTH_SIZE = 30;
constexpr int   MAP3_HEIGHT_SIZE = 30;
constexpr int	MAP3_ARIES = 10;
constexpr int	MAP3_LEO = 10;
constexpr int	MAP3_SCORPIO = 10;


void DebugDrawLine(Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color);
void DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color);
int	 GetTileIndexByPos(int mapHeight, IntVec2 const& pos);
bool IsIntValueInsideVector(int value, const std::vector<int>& vectorList);
