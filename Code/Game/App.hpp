#pragma once
#include "Engine/Renderer/Camera.hpp"

class Game;

class App
{
public:
	App();
	~App();

	void		Startup();
	void		BeginFrame();
	void		RunMainLoop();
	void		RunFrame();

// 	void		GetDevConsole() const;
// 	void		GetEventSystem();

	void		Update(float deltaSeconds);
	void		Render() const;

	void		HandleQuitRequested();
	bool		IsQuitting() const { return m_isQuitting; }

	void		ClearGameInstance();
	void		ReloadGame();

	void		Shutdown();
	void		EndFrame();

public:
	bool		m_isPaused = false;

private:
	float		m_timeLastFrameStart = 0.0f;
	bool		m_isQuitting = false;
};

