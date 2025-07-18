#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Gamecommon.hpp"
#include "Game/Tile.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

extern App* g_theApp;

TileDefinition::TileDefinition()
{

}

TileDefinition::~TileDefinition()
{

}

Game::Game()
	:m_player(nullptr)
{
	m_RNG = new RandomNumberGenerator();

	m_worldCamera = new Camera();
	m_attractCamera = new Camera();

	m_worldCamera->SetDimensions(Vec2(2.0f * m_numTilesInViewVertically, m_numTilesInViewVertically));
	m_attractCamera->SetOrthoView(Vec2(0.f, 0.f), Vec2(g_gameConfigBlackboard.GetValue("screenSizeX", 1600.0f), g_gameConfigBlackboard.GetValue("screenSizeY", 800.0f)));
	
	m_currentGameStatus = GameStatus::STATUS_ATTRACT;

	InitializeData();
	SpawnDefinitions();
	m_attractSound = g_theAudio->StartSound(m_attractMusic, true);
	MapDefinition::InitializeMapDefs();
}
   

Game::~Game() 
{
	g_theAudio->StopSound(m_attractSound);
	g_theAudio->StopSound(m_gameSound);
	g_theAudio->StopSound(m_victorySound);
	g_theApp->ClearGameInstance();
}


void Game::SpawnDefinitions()
{
	m_tileSpriteSheet = new SpriteSheet(*m_tileSpriteTexture, IntVec2(8, 8));
	TileDefinition::InitializeTileDefs();
}

void Game::Update(float deltaSeconds)
{
	if (m_currentGameStatus == GameStatus::STATUS_ATTRACT)
	{
		UpdateAttract(deltaSeconds);
		return;
	}

	KeyBodadInput(deltaSeconds);
	ControllerInput(deltaSeconds);
	UpdatePlayer(deltaSeconds);
	UpdateMap(deltaSeconds);

}



void Game::Render()
{
	switch (m_currentGameStatus)
	{
	case GameStatus::STATUS_ATTRACT:
		g_theRenderer->BeginCamera(*m_attractCamera);
		g_theRenderer->ClearScreen(Rgba8::BLACK);
		RenderAttract();
		g_theRenderer->EndCamera(*m_attractCamera);
		break;

	case GameStatus::STATUS_NORMAL:
		g_theRenderer->BeginCamera(*m_worldCamera);
		g_theRenderer->ClearScreen(Rgba8::BLACK);
		RenderMap();
		UpdateDeveloperCheats();
		g_theRenderer->EndCamera(*m_worldCamera);
		break;

	case GameStatus::STATUS_PAUSE:
		g_theRenderer->BeginCamera(*m_worldCamera);
		g_theRenderer->ClearScreen(Rgba8::BLACK);
		RenderMap();
		UpdateDeveloperCheats();
		PauseMode();
		g_theRenderer->EndCamera(*m_worldCamera);
		break;

	case GameStatus::STATUS_VICTORY:
		g_theRenderer->BeginCamera(*m_attractCamera);
		g_theRenderer->ClearScreen(Rgba8::BLACK);
		RenderVictory();
		g_theRenderer->EndCamera(*m_attractCamera);
		break;
	}

}

void Game::RenderVictory() const
{
	std::vector<Vertex_PCU> verts;
	AABB2 bounds = AABB2(m_attractCamera->GetOrthoBottomLeft(), m_attractCamera->GetOrthoTopRight());

	AddVertsForAABB2D(verts, bounds, Rgba8::WHITE);
	TransformVertexArrayXY3D((int)verts.size(), verts.data(), 1.0f, 0.0f, Vec2(0.0f, 0.0f));

	g_theRenderer->BindTexture(m_victorySceneTexture);
	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
}

bool Game::GetNoClipMode() const
{
	return m_noneClip;
}

Player* const Game::GetPlayer() const
{
	return m_player;
}

Map* const Game::GetCurrentMap() const
{
	return m_currentMap;
}

SpriteSheet* Game::GetTileSpriteSheet() const
{
	return m_tileSpriteSheet;
}

void Game::RenderMap() const
{
// 	if (m_currentMap)
// 	{
// 		Vec2 mapMaxs = Vec2(static_cast<float>(m_currentMap->m_dimensions.x), static_cast<float>(m_currentMap->m_dimensions.y));
// 		m_worldCamera->FollowPlayerInBounds(m_player->m_position, Vec2(0.0f, 0.0f), mapMaxs);
// 		m_currentMap->Render();
// 	}
	if (m_currentMap)
	{
		m_worldCamera->FollowPlayerInBounds(m_player->m_position, Vec2(0.0f, 0.0f), Vec2(static_cast<float>(m_currentMap->GetDimensions().x), static_cast<float>(m_currentMap->GetDimensions().y)));
		m_currentMap->Render();
	}
}

void Game::RenderAttract() const 
{

	std::vector<Vertex_PCU> verts;
	AABB2 bound = AABB2(m_attractCamera->GetOrthoBottomLeft(), m_attractCamera->GetOrthoTopRight());

	AddVertsForAABB2D(verts, bound, Rgba8::WHITE);
	TransformVertexArrayXY3D((int)verts.size(), verts.data(), 1.0f, 0.f, Vec2(0.0f, 0.0f));

	g_theRenderer->BindTexture(m_attractTexture);
	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());

	DebugDrawRing(Vec2(700.f, 500.f), 200.f, 15.f, Rgba8(255, 127, 0));
	
	std::vector<Vertex_PCU> textVerts;
	// g_testFont->AddVertsForText2D(textVerts, Vec2(30.f, 20.f), 30.f, "C++ so hard - yuchen");
	AABB2 box = AABB2(Vec2(0.f, 0.f), Vec2(400.f, 200.f));
	std::string text = "C++ so hard \n yuchen :)";
	g_testFont->AddVertsForTextInBox2D(textVerts, text, box, 100, Rgba8::YELLOW);
	g_theRenderer->BindTexture(&g_testFont->GetTexture());
	g_theRenderer->DrawVertexArray((int)textVerts.size(), textVerts.data());
}

void Game::InitializeData()
{
	m_victorySceneTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/VictoryScreen.jpg");
	m_tileSpriteTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Terrain_8x8.png");
	m_attractTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/AttractScreen.png");
	m_dieTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/YouDiedScreen.png");

	m_attractMusic = g_theAudio->CreateOrGetSound("Data/Audio/AttractMusic.mp3");
	m_gameplayMusic = g_theAudio->CreateOrGetSound("Data/Audio/GameplayMusic.mp3");
	m_victoryMusic = g_theAudio->CreateOrGetSound("Data/Audio/Victory.mp3");

	m_clickMusic = g_theAudio->CreateOrGetSound("Data/Audio/Click.mp3");
	m_pauseMusic = g_theAudio->CreateOrGetSound("Data/Audio/Pause.mp3");
	m_unpauseMusic = g_theAudio->CreateOrGetSound("Data/Audio/Unpause.mp3");

	m_playerShoot = g_theAudio->CreateOrGetSound("Data/Audio/PlayerShootNormal.ogg");
	m_enemyShoot = g_theAudio->CreateOrGetSound("Data/Audio/EnemyShoot.wav");
	m_playerHit = g_theAudio->CreateOrGetSound("Data/Audio/PlayerHit.wav");
	m_enemyHit = g_theAudio->CreateOrGetSound("Data/Audio/EnemyHit.wav");
	m_playerDied = g_theAudio->CreateOrGetSound("Data/Audio/EnemyDied.wav");
	m_enemyDied = g_theAudio->CreateOrGetSound("Data/Audio/EnemyDied.wav");
	m_bulletBounce = g_theAudio->CreateOrGetSound("Data/Audio/BulletBounce.wav");

	g_testFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
}

void Game::PlaySound(int soundIndex)
{
	switch (soundIndex)
	{
	case 0:
		g_theAudio->StartSound(m_playerShoot);
		break;
	case 1:
		g_theAudio->StartSound(m_enemyShoot);
		break;
	case 2:
		g_theAudio->StartSound(m_playerHit);
		break;
	case 3:
		g_theAudio->StartSound(m_enemyHit);
		break;
	case 4:
		g_theAudio->StartSound(m_playerDied);
		break;
	case 5:
		g_theAudio->StartSound(m_enemyDied);
		break;
	case 6:
		g_theAudio->StartSound(m_bulletBounce);
		break;
	}
}


void Game::SpawnPlayerAndMap()
{
	if (m_currentMap == nullptr)
	{
		std::string allMapsNames = g_gameConfigBlackboard.GetValue("maps", "NOT_LOADED");

		Strings mapNameList = SplitStringOnDelimiter(allMapsNames, ',');

		m_maps.reserve(mapNameList.size());

		for (int mapIndex = 0; mapIndex < mapNameList.size(); ++mapIndex)
		{
			MapDefinition mapDef = MapDefinition::s_mapDefinitions[mapNameList[mapIndex]];

			Map* map = new Map(mapDef);

			if (m_maps.size() == 0)
			{
				m_currentMap = map;
				Vec2 playerStartPosition = g_gameConfigBlackboard.GetValue("playerStartPosition", Vec2(0.f, 0.f));
				m_player = new Player(m_currentMap, playerStartPosition, 0.0f);
				m_currentMap->AddEntityInMap(*m_player);
			}
			m_maps.push_back(map);
		}

		m_currentMap->StartUp();
	}
}

void Game::PutPlayerIntoMap(Map* map)
{
	if (m_currentMap != nullptr)
	{
		m_currentMap->RemoveEntityFromMap(*m_player);
	}

	m_currentMap = map;

	if (m_inDebugCamera)
	{
		float mapRatio = m_currentMap->GetDimensions().GetAspect();
		if (mapRatio > 2.0f)
		{
			m_worldCamera->SetDimensions(Vec2((float)m_currentMap->GetDimensions().x, (float)m_currentMap->GetDimensions().x / g_theWindow->GetAspect()));
		}
		else
		{
			m_worldCamera->SetDimensions(Vec2((float)m_currentMap->GetDimensions().y * g_theWindow->GetAspect(), (float)m_currentMap->GetDimensions().y));
		}
	}

	if (m_player != nullptr)
	{
		m_player->m_map = m_currentMap;
		m_currentMap->AddEntityInMap(*m_player);
		m_player->m_position = g_gameConfigBlackboard.GetValue("playerStartPosition",Vec2(0.f, 0.f));
	}
	m_currentMap->StartUp();

}

void Game::PutPlayerIntoNextMap()
{
	if (m_currentMapIndex == m_maps.size() - 1)
	{
		m_currentGameStatus = GameStatus::STATUS_VICTORY;
		g_theAudio->StopSound(m_gameSound);
		m_victorySound = g_theAudio->StartSound(m_victoryMusic);
	}
	else
	{
		m_currentMapIndex = GetNextMapIndex();
		PutPlayerIntoMap(m_maps[m_currentMapIndex]);
	}
}

int Game::GetNextMapIndex() const
{
	if (m_currentMapIndex < m_maps.size() - 1)
	{
		return m_currentMapIndex + 1;
	}
	return 0;
}

void Game::UpdateCameras(float& deltaSeconds)
{
    UNUSED(deltaSeconds);
}

void Game::UpdateAttract(float& deltaSeconds)
{
	UNUSED(deltaSeconds);
	XboxController XbCtrl = g_theInput->GetController(0);

	if (g_theInput->IsKeyDown(KEYCODE_SPACE) && !g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
		m_currentGameStatus = GameStatus::STATUS_NORMAL;
		g_theAudio->StopSound(m_attractSound);
		
		SpawnPlayerAndMap();
	}

	if (g_theInput->IsKeyDown('P') && !g_theInput->WasKeyJustPressed('P'))
	{
		m_currentGameStatus = GameStatus::STATUS_NORMAL;
		g_theAudio->StopSound(m_attractSound);
	
		SpawnPlayerAndMap();
	}

	if (g_theInput->IsKeyDown(KEYCODE_ESC) && !g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theAudio->StopSound(m_gameSound);
		g_theApp->HandleQuitRequested();
	}

	if (XbCtrl.IsButtonDown(XboxButtonID::XBOX_BUTTON_START) && !XbCtrl.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START))
	{
		m_currentGameStatus = GameStatus::STATUS_NORMAL;
		g_theAudio->StopSound(m_attractSound);

		SpawnPlayerAndMap();
	}


}

void Game::UpdateMap(float deltaSeconds)
{
	if (m_currentGameStatus != GameStatus::STATUS_NORMAL)
	{
		return;
	}
	if (m_currentMap != nullptr)
	{
		m_currentMap->Update(deltaSeconds);
	}
}

void Game::UpdatePlayer(float deltaSeconds)
{
	if (m_isPaused)
	{
		return;
	}
	if (GetPlayer())
	{
		m_player->RunFrame(deltaSeconds);
	}
}


void Game::UpdateDeveloperCheats()
{
	if (!g_isDebugDraw)
	{
		return;
	}

	g_theRenderer->BindTexture(nullptr);

	m_currentMap->RenderDeveloperMode();
}

void Game::DebugRender() const
{
    if (!g_isDebugDraw)
    {
        return;
    }
}

void Game::PauseMode() const
{
	Vertex_PCU PauseMenuVertsGlobal[]
	{
		Vertex_PCU(1.f, 1.f, 0, 0, 0, 50),
		Vertex_PCU(1.f, -1.f, 0, 0, 0, 50),
		Vertex_PCU(-1.f, -1.f, 0, 0, 0, 50),
					
		Vertex_PCU(1.f, 1.f, 0, 0, 0, 50),
		Vertex_PCU(-1.f, 1.f, 0, 0, 0, 50),
		Vertex_PCU(-1.f, -1.f, 0, 0, 0, 50),
	};

	int vertNum = sizeof(PauseMenuVertsGlobal) / sizeof(PauseMenuVertsGlobal[0]);
	TransformVertexArrayXY3D(vertNum, PauseMenuVertsGlobal, m_worldCamera->GetHalfDimension().x, 0.0f, m_worldCamera->GetOrthoCenter());
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(vertNum, PauseMenuVertsGlobal);
}

void Game::PauseResumeGame()
{
	if (m_currentGameStatus == GameStatus::STATUS_NORMAL)
	{
		m_currentGameStatus = GameStatus::STATUS_PAUSE;
		g_theAudio->StartSound(m_pauseMusic);
		g_theAudio->SetSoundPlaybackSpeed(m_gameSound, 0.0f);
	}
	else if (m_currentGameStatus == GameStatus::STATUS_PAUSE)
	{
		m_currentGameStatus = GameStatus::STATUS_NORMAL;
		g_theAudio->StartSound(m_unpauseMusic);
		g_theAudio->SetSoundPlaybackSpeed(m_gameSound, 1.0f);
	}
}

void Game::KeyBodadInput(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	if (g_theInput->IsKeyDown('P') && !g_theInput->WasKeyJustPressed('P'))
	{
		if (m_currentGameStatus == GameStatus::STATUS_VICTORY)
		{
			g_theApp->ReloadGame();
		}
		g_theGame->PauseResumeGame();
	}

	if (g_theInput->IsKeyDown('N') && !g_theInput->WasKeyJustPressed('N'))
	{
		if (GetPlayer()->m_isDead)
		{
			m_player->RespawnPlayer();
			if (m_currentGameStatus == GameStatus::STATUS_PAUSE)
			{
				m_currentGameStatus = GameStatus::STATUS_NORMAL;
				g_theAudio->StartSound(m_unpauseMusic);
				g_theAudio->SetSoundPlaybackSpeed(m_gameSound, 1.0f);
			}
		}
	}

	if (g_theInput->IsKeyDown(KEYCODE_ESC) && !g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		if (m_currentGameStatus == GameStatus::STATUS_VICTORY)
		{
			g_theApp->ReloadGame();
		}
		EsePressed();
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		g_isDebugDraw = !g_isDebugDraw;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
	{
		m_player->InvincibleModeOrNot();
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
	{
		m_noneClip = !m_noneClip;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
	{
		if (m_inDebugCamera)
		{
			m_worldCamera->SetDimensions(Vec2(2.0f * m_numTilesInViewVertically, m_numTilesInViewVertically));
			m_inDebugCamera = false;
		}
		else
		{
			float mapRatio = (float)m_currentMap->GetDimensions().x / (float)m_currentMap->GetDimensions().y;
			if (mapRatio > 2.0f)
			{
				m_worldCamera->SetDimensions(Vec2((float)m_currentMap->GetDimensions().x, (float)m_currentMap->GetDimensions().x / g_theWindow->GetAspect()));
			}
			else
			{
				m_worldCamera->SetDimensions(Vec2((float)m_currentMap->GetDimensions().y * g_theWindow->GetAspect(), (float)m_currentMap->GetDimensions().y));
			}
			m_inDebugCamera = true;
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_currentMap->SwitchRenderMode();
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F9))
	{
		PutPlayerIntoNextMap();
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
		if (m_currentGameStatus == GameStatus::STATUS_VICTORY)
		{
			g_theApp->ReloadGame();
		}
	}
}

void Game::ControllerInput(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	XboxController XbCtrl = g_theInput->GetController(0);

	if (XbCtrl.IsButtonDown(XboxButtonID::XBOX_BUTTON_START) && !XbCtrl.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START))
	{
		// m_isPaused = !m_isPaused;
		g_theGame->PauseResumeGame();
	}

	if (XbCtrl.IsButtonDown(XboxButtonID::XBOX_BUTTON_BACK) && !XbCtrl.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_BACK))
	{
		EsePressed();
	}
}

void Game::EsePressed()
{
	if (m_currentGameStatus != GameStatus::STATUS_PAUSE && m_currentGameStatus != GameStatus::STATUS_ATTRACT)
	{
		PauseResumeGame();
	}
	else if (m_currentGameStatus == GameStatus::STATUS_PAUSE && m_currentGameStatus != GameStatus::STATUS_ATTRACT)
	{
		g_theApp->ReloadGame();
	}

}
