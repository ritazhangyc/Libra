#include "Game/GameCommon.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"

#include <math.h>

constexpr float PI = 3.1415926535897932384626433832795f;

Window* g_theWindow = nullptr;
Renderer* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Camera* g_theCamera = nullptr;
App* g_theApp = nullptr;
Game* g_theGame = nullptr;
RandomNumberGenerator* g_theRNG = nullptr;
BitmapFont* g_testFont = nullptr;

void DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color) 
{
    float halfThickness = 0.5f * thickness;
    float innerRadius = radius - halfThickness;
    float outerRadius = radius + halfThickness;
    constexpr int NUM_SIDES = 32;
    constexpr int NUM_TRIS = 2 * NUM_SIDES;
    constexpr int NUM_VERTS = 3 * NUM_TRIS;
    Vertex_PCU verts[NUM_VERTS];
    constexpr float DEGREES_PER_SIDE = 360.f / static_cast<float>(NUM_SIDES);
    for (int sideNum = 0; sideNum < NUM_SIDES; ++sideNum)
    {
        float startDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum);
        float endDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum + 1);
        float cosStart = cosf(startDegrees * PI/180.f);
        float sinStart = sinf(startDegrees * PI/180.f);
        float cosEnd = cosf(endDegrees * PI / 180.f);
        float sinEnd = sinf(endDegrees * PI / 180.f);
        Vec3 innerStartPos (center.x + innerRadius * cosStart, center.y + innerRadius * sinStart, 0.f);
        Vec3 outerStartPos (center.x + outerRadius * cosStart, center.y +outerRadius * sinStart, 0.f);
        Vec3 innerEndPos (center.x + innerRadius * cosEnd, center.y + innerRadius * sinEnd, 0.f);
        Vec3 outerEndPos (center.x + outerRadius * cosEnd, center.y + outerRadius * sinEnd, 0.f);

        int vertIndexA = 6 * sideNum;
        int vertIndexB = vertIndexA + 1;
        int vertIndexC = vertIndexA + 2;
        int vertIndexD = vertIndexA + 3;
        int vertIndexE = vertIndexA + 4;
        int vertIndexF = vertIndexA + 5;

        verts[vertIndexA].m_position = innerEndPos;
        verts[vertIndexB].m_position = innerStartPos;
        verts[vertIndexC].m_position = outerStartPos;

        verts[vertIndexA].m_color = color;
        verts[vertIndexB].m_color = color;
        verts[vertIndexC].m_color = color;

        verts[vertIndexD].m_position = innerEndPos;
        verts[vertIndexE].m_position = outerEndPos;
        verts[vertIndexF].m_position = outerStartPos;

        verts[vertIndexD].m_color = color;
        verts[vertIndexE].m_color = color;
        verts[vertIndexF].m_color = color;
    }

	g_theRenderer->BindTexture(nullptr);
    g_theRenderer->DrawVertexArray(NUM_VERTS, &verts[0]);

}


void DebugDrawLine(Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color)
{
    Vertex_PCU lineVerts[6];
    Vec2 SE = end - start;
    float h = thickness * 0.5f;
    Vec2 se = SE.GetNormalized();
    Vec2 fwd = se * h;
    Vec2 left = fwd.GetRotated90Degrees();
    Vec2 EL = end + fwd + left;
    Vec2 ER = end + fwd - left;
    Vec2 SL = start - fwd + left;
    Vec2 SR = start - fwd - left;

    lineVerts[0] = Vertex_PCU(Vec3(EL.x, EL.y, 0.f), color, Vec2(0.5f, 0.5f));
    lineVerts[1] = Vertex_PCU(Vec3(ER.x, ER.y, 0.f), color, Vec2(0.5f, 0.5f));
    lineVerts[2] = Vertex_PCU(Vec3(SR.x, SR.y, 0.f), color, Vec2(0.5f, 0.5f));

    lineVerts[3] = Vertex_PCU(Vec3(SL.x, SL.y, 0.f), color, Vec2(0.5f, 0.5f));
    lineVerts[4] = Vertex_PCU(Vec3(SR.x, SR.y, 0.f), color, Vec2(0.5f, 0.5f));
    lineVerts[5] = Vertex_PCU(Vec3(EL.x, EL.y, 0.f), color, Vec2(0.5f, 0.5f));

    g_theRenderer->DrawVertexArray(6, &lineVerts[0]);
}


int GetTileIndexByPos(int mapHeight, IntVec2 const& pos)
{
	return mapHeight * pos.x + (pos.y);
}

bool IsIntValueInsideVector(int value, const std::vector<int>& vectorList)
{
	for (int i : vectorList)
	{
		if (i == value)
		{
			return true;
		}
	}
	return false;
}
