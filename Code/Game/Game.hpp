#pragma once
#include "Game/Gamecommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/Player.hpp"
#include "Game/Map.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Texture.hpp"

class Game 
{
public:
	Game();
	~Game();

	void					Update(float deltaseconds);
	void					Render();
	Player* const			GetPlayer() const;
	Map* const				GetCurrentMap() const;
	SpriteSheet*			GetTileSpriteSheet() const;
	bool					GetNoClipMode() const;
	void					InitializeData();
	void					PauseResumeGame();
	void					PlaySound(int soundIndex);
	void					PutPlayerIntoMap(Map* map);
	void					PutPlayerIntoNextMap();

private:
	void					SpawnDefinitions();
	void					RenderMap() const;
	void					RenderAttract() const;
	void					SpawnPlayerAndMap();
	void					UpdateCameras(float& deltaSeconds);
	void					UpdateAttract(float& deltaSeconds);
	void					UpdateMap(float deltaSeconds);
	void					UpdatePlayer(float deltaSeconds);
	int						GetNextMapIndex() const;
	void					UpdateDeveloperCheats();

	void					DebugRender() const;
	void					PauseMode() const;
	void					RenderVictory() const;
	void					KeyBodadInput(float deltaSeconds);
	void					ControllerInput(float deltaSeconds);
	void					EsePressed();

public:
	bool					m_isAttractMode = true;
	bool					g_isDebugDraw = false;
	RandomNumberGenerator	m_rng;
	float					m_numTilesInViewVertically = 8.0f;

private:
	Camera*					m_worldCamera;
	Camera*					m_attractCamera;

	Texture*				m_victorySceneTexture;
	Texture*				m_bitmapFontTexture;
	Texture*				m_attractTexture;
	Texture*				m_tileSpriteTexture;
	Texture*				m_dieTexture;
	SpriteSheet*			m_tileSpriteSheet;
	GameStatus				m_currentGameStatus = GameStatus::STATUS_ATTRACT;
	bool					m_isPaused = false;
	bool					m_isSlowMo = false;
	bool					m_inDebugCamera = false;
	bool					m_noneClip = false;
	Player*					m_player;
	Map*					m_currentMap = nullptr;
	RandomNumberGenerator*	m_RNG = nullptr;
	int						m_currentMapIndex = 0;
	std::vector<Map*>		m_maps;

	SoundID					m_gameplayMusic;
	SoundID					m_attractMusic;
	SoundID					m_clickMusic;
	SoundID					m_pauseMusic;
	SoundID					m_unpauseMusic;
	SoundID					m_victoryMusic;

	SoundID					m_playerShoot;
	SoundID					m_enemyShoot;
	SoundID					m_playerHit;
	SoundID					m_enemyHit;
	SoundID					m_playerDied;
	SoundID					m_enemyDied;
	SoundID					m_bulletBounce;

	SoundPlaybackID			m_gameSound;
	SoundPlaybackID			m_attractSound;
	SoundPlaybackID			m_victorySound;
};




