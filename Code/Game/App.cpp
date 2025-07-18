#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"
//#include "Engine/Core/EventSystem.hpp"
//#include "Engine/Core/DevConsole.hpp"

App::App()
{
}

App::~App()
{
}

void App::Startup()
{
	XmlDocument doc;
	doc.LoadFile("Data/GameConfig.xml");
	XmlElement* rootElement = doc.RootElement();
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*rootElement);

	InputSystemConfig inputConfig;
	g_theInput = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_aspectRatio = 2.0f;
	windowConfig.m_inputSystem = g_theInput;
	windowConfig.m_windowTitle = "SD1_A8: Libra";
	g_theWindow = new Window(windowConfig);

	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(rendererConfig);

	AudioSystemConfig audioSystemConfig;
	g_theAudio = new AudioSystem(audioSystemConfig);

// 	DevConsoleConfig devConsoleConfig;
// 	devConsoleConfig.m_font = std::string("Data/Fonts/SquirrelFixedFont");
// 	devConsoleConfig.m_numLines = 35.0f;
// 	g_theConsole = new DevConsole(devConsoleConfig);
// 
// 	EventSystemConfig eventSystemConfig;
// 	g_theEventSystem = new EventSystem(eventSystemConfig);

	g_theInput->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_theAudio->Startup();

	g_theGame = new Game();


//	GetEventSystem();
}

void App::RunMainLoop()
{
	while (!IsQuitting())
	{
		RunFrame();
	}
}

void App::Shutdown() 
{
	delete g_theGame;
	g_theGame = nullptr;

	g_theAudio->ShutDown();
    g_theRenderer->ShutDown();
	g_theWindow->ShutDown();
	g_theInput->ShutDown();

	delete g_theAudio;
	g_theAudio = nullptr;

    delete g_theRenderer;
    g_theRenderer = nullptr;

	delete g_theWindow;
	g_theWindow = nullptr; 

	delete g_theInput;
	g_theInput = nullptr;

}

void App::RunFrame() 
{
	float m_timeNow = static_cast<float>(GetCurrentTimeSeconds());
	float deltaSeconds = m_timeNow - m_timeLastFrameStart;
	m_timeLastFrameStart = m_timeNow;

	if (g_theInput->IsKeyDown(KEYCODE_F8))
	{
		ReloadGame();
	}

	if (g_theInput->IsKeyDown('T') && g_theInput->IsKeyDown('Y'))
	{
		deltaSeconds *= 8.0f;
	}
	else
	{
		if (g_theInput->IsKeyDown('T'))
		{
			deltaSeconds *= 0.1f;
		}

		if (g_theInput->IsKeyDown('Y'))
		{
			deltaSeconds *= 4.0f;
		}
	}

    BeginFrame();
    Update(deltaSeconds);
    Render();
    EndFrame();
}

void App::HandleQuitRequested() 
{
    m_isQuitting = true;
}

void App::BeginFrame()
{
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theAudio->BeginFrame();
	//g_theConsole->BeginFrame();
}

void App::Update(float deltaSeconds)
{
	g_theGame->Update(deltaSeconds);
}

void App::Render() const
{
	if (IsQuitting())
	{
		return;
	}
	g_theGame->Render();
}


void App::EndFrame()
{
	g_theInput->EndFrame();
	g_theWindow->EndFrame();
	g_theRenderer->EndFrame();
	g_theAudio->EndFrame();
	//g_theConsole->EndFrame();
}

void App::ClearGameInstance()
{
	g_theGame = nullptr;
}

void App::ReloadGame()
{
	delete g_theGame;
	g_theGame = new Game();
}
