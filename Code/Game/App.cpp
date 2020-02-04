#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Memory/Mem.hpp"


STATIC bool App::QuitRequest(EventArgs& args)
{
	UNUSED(args);
	g_theApp->HandleQuitRequested();
	return true;
}


STATIC bool App::PrintMemAlloc(EventArgs& args)
{
	UNUSED(args);
	DevConPrintMemTrack();
	return true;
}


STATIC bool App::LogMemAlloc(EventArgs& args)
{
	UNUSED(args);
	MemTrackLogLiveAllocations();
	return true;
}


App::App() : m_theGame(nullptr)
{
	ParseXmlFileToNamedString(g_gameConfigBlackboard, "Data/GameConfig.xml");
}


App::~App()
{
	delete m_theGame;
	m_theGame = nullptr;

	delete m_devCamera;
	m_devCamera = nullptr;
}


void App::Startup()
{
	EngineStartup();
	g_theWindow->SetMouseMode(MOUSE_MODE_ABSOLUTE);

	m_theGame = new Game;

	m_devCamera = new Camera();
	m_devCamera->SetColorTarget(nullptr);
	m_devCamera->SetOrthoView(Vec2(0.0f, 0.0), Vec2((WORLD_HEIGHT * WORLD_ASPECT), (WORLD_HEIGHT)));

	m_theGame->Startup();

	g_theEventSystem->SubscribeEventCallbackFunction("quit", QuitRequest);
	g_theEventSystem->SubscribeEventCallbackFunction("ShowMemAlloc", PrintMemAlloc);
	g_theEventSystem->SubscribeEventCallbackFunction("LogMemAlloc", LogMemAlloc);
	DevConPrintMemTrackType();
}


void App::Shutdown()
{
	m_theGame->Shutdown();
	EngineShutdown();
}


void App::RunFrame()
{
	BeginFrame();
	Update();
	Render();
	EndFrame();
}


void App::BeginFrame() const
{
	g_theRenderer->BeginFrame();
	g_theEventSystem->BeginFrame();
	g_theDevConsole->BeginFrame();
	g_theDebugRenderer->BeginFrame();
	g_theAudio->BeginFrame();
	m_theGame->BeginFrame();
}


void App::Update()
{
	const double current_time = GetCurrentTimeSeconds();
	double delta_seconds = current_time - m_timeLastFrame;;
	m_timeLastFrame = current_time;

	g_theClock->Step(delta_seconds);
	g_theDevConsole->Update(g_theClock->m_frameTime);
	m_theGame->Update(g_theClock->m_frameTime);
}


void App::Render() const
{
	// Draw a line from the bottom-left corner of the screen (0,0) to the center of the screen (50,50)
	m_theGame->Render();
	g_theDebugRenderer->RenderToScreen();

	if (DEV_CONSOLE_IN_USE)
	{
		m_devCamera->SetColorTarget(g_theRenderer->GetFrameColorTarget());

		m_devCamera->SetModelMatrix(Matrix44::IDENTITY);
		g_theRenderer->BeginCamera(m_devCamera);
		g_theRenderer->ClearDepthStencilTarget(1.0f);

		g_theDevConsole->Render(g_theRenderer, *m_devCamera);

		g_theRenderer->EndCamera(m_devCamera);
	}
}


void App::EndFrame() const
{
	g_theRenderer->EndFrame();
	g_theEventSystem->EndFrame();
	g_theDevConsole->EndFrame();
	g_theDebugRenderer->EndFrame();
	g_theAudio->EndFrame();
	m_theGame->EndFrame();
}


bool App::HandleQuitRequested()
{
	m_isQuitting = true;
	return true;
}

void App::HardRestart()
{
	m_theGame->Shutdown();
	delete m_theGame;
	m_theGame = nullptr;
	m_theGame = new Game;
	m_theGame->Startup();
}


bool App::HandleKeyPressed(const unsigned char key_code)
{
	switch (key_code)
	{
	case ESC_KEY:
		if (!DEV_CONSOLE_IN_USE)
			m_isQuitting = true;
		return true;

	case F1_KEY:
		if (!DEV_CONSOLE_IN_USE)
			m_theGame->SetDeveloperMode(true);
		return true;

	case F8_KEY:
		if (!DEV_CONSOLE_IN_USE)
			HardRestart();
		return true;

	case TILDE_KEY:
		DEV_CONSOLE_IN_USE = !DEV_CONSOLE_IN_USE;
		return true;

	default:
		if (!DEV_CONSOLE_IN_USE)
			return m_theGame->HandleKeyPressed(key_code);

		return false;
	}
}


bool App::HandleKeyReleased(const unsigned char key_code)
{
	switch (key_code)
	{
	case F1_KEY:
		m_theGame->SetDeveloperMode(false);
		return true;


	default:
		return m_theGame->HandleKeyReleased(key_code);
	}
}