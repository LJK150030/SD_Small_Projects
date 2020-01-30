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
#include "Engine/Memory/ObjectAllocator.hpp"
#include "Engine/Tools/Profiler.hpp"
#include "Engine/Tools/Reports.hpp"
#include "Scripting/Python/python.hpp"


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

App::App(): m_theGame(nullptr)
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
	//UnitTestsRunAllCategories(1);
	
	EngineStartup();
	g_profiler->ProfilerResume();
	UnitTestsRun("LoggingSystem", 0);
	g_theWindow->SetMouseMode(MOUSE_MODE_ABSOLUTE);
// 	g_theWindow->LockMouse();
// 	g_theWindow->ShowMouse();
// 	g_theWindow->HideMouse();
	
	m_theGame = new Game;
	
	m_devCamera = new Camera();
	m_devCamera->SetColorTarget(nullptr);
	m_devCamera->SetOrthoView( Vec2(0.0f, 0.0),	Vec2((WORLD_HEIGHT * WORLD_ASPECT), (WORLD_HEIGHT)) );

	m_theGame->Startup();

	g_theEventSystem->SubscribeEventCallbackFunction("quit", QuitRequest);
	g_theEventSystem->SubscribeEventCallbackFunction("ShowMemAlloc", PrintMemAlloc);
	g_theEventSystem->SubscribeEventCallbackFunction("LogMemAlloc", LogMemAlloc);
	DevConPrintMemTrackType();


// 
// 	g_profilerPool = new(TrackedAllocator::s_instance) ObjectAllocator<dummy>();
// 	g_profilerPool->Init(&TrackedAllocator::s_instance, 10);
// 
// 
// 	dummy* one = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
// 	one->color = Rgba::MAGENTA;
// 	dummy* two = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
// 	dummy* three = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
// 	dummy* four = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
// 	dummy* five = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
// 	dummy* six = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
// 	dummy* seven = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
// 	seven->idenity = Vec3(1.0f, 2.0f, 3.0f);
// 	dummy* eight = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
// 	dummy* nine = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
// 	// 	dummy* ten = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy*)));
// 	// 	*ten = 10;
// 
// 	dummy* one2 = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
// 	dummy* two2 = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
// 	dummy* three2 = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
// 	dummy* four2 = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
// 	dummy* five2 = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
// 	dummy* six2 = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
// 	dummy* seven2 = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
// 
// 	g_profilerPool->Free(four2);
// 
// 	dummy* eight2 = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
// 	dummy* nine2 = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
// 	dummy* ten2 = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
// 
// 	g_profilerPool->Free(one2);
// 
// 	dummy* eleven2 = static_cast<dummy*>(g_profilerPool->Alloc(sizeof(dummy)));
	PythonSystemStartup();
}

void App::Shutdown()
{
/*	g_profilerPool->DeInit();*/
	Reports report;
	report.DebugPrintReport();

	m_theGame->Shutdown();
	PythonSystemShutdown();
	EngineShutdown();
}

void App::RunFrame()
{
	g_profiler->ProfileBeginFrame("RunFrame");
	BeginFrame();
	Update();
	Render();
	EndFrame();
	g_profiler->ProfileEndFrame();
}

void App::BeginFrame() const
{
	g_profiler->ProfilePush("App::BeginFrame");
	g_theRenderer->BeginFrame();
	g_theEventSystem->BeginFrame();
	g_theDevConsole->BeginFrame();
	g_theDebugRenderer->BeginFrame();
	g_theAudio->BeginFrame();
	g_profiler->ProfilePop();
}

void App::Update()
{
	g_profiler->ProfilePush("App::Update");
	const double current_time = GetCurrentTimeSeconds();
	//double delta_seconds = ClampDouble(current_time - m_timeLastFrame, 0.0, 0.1);
	double delta_seconds = current_time - m_timeLastFrame;;
	m_timeLastFrame = current_time;
	

	g_theClock->Step(delta_seconds);
	g_theDevConsole->Update(g_theClock->m_frameTime);
	m_theGame->Update(g_theClock->m_frameTime);
	g_profiler->ProfilePop();
}

void App::Render() const
{
	// Draw a line from the bottom-left corner of the screen (0,0) to the center of the screen (50,50)
	m_theGame->Render();
	g_theDebugRenderer->RenderToScreen();

	m_theGame->UpdateImGUI();

	if(DEV_CONSOLE_IN_USE)
	{
		m_devCamera->SetColorTarget(g_theRenderer->GetFrameColorTarget());

		m_devCamera->SetModelMatrix( Matrix44::IDENTITY );
		g_theRenderer->BeginCamera(m_devCamera);
		g_theRenderer->ClearDepthStencilTarget( 1.0f );

		g_theDevConsole->Render(g_theRenderer, *m_devCamera);

		g_theRenderer->EndCamera(m_devCamera);
	}
}

void App::EndFrame() const
{
	// "Present" the back buffer by swapping the front (visible) and back (working) screen buffers
	g_theRenderer->EndFrame();
	g_theEventSystem->EndFrame();
	g_theDevConsole->EndFrame();
	g_theDebugRenderer->EndFrame();
	g_theAudio->EndFrame();
}


bool App::HandleQuitRequested()
{
	m_isQuitting = true;
	return true;
}

void App::HardRestart()
{
	Shutdown();
	delete m_theGame;
	m_theGame = nullptr;
	Startup();
}

bool App::HandleKeyPressed(const unsigned char key_code)
{
	switch (key_code)
	{
	case ESC_KEY:
		if (!DEV_CONSOLE_IN_USE)
			m_isQuitting = true;
		return true;

	case T_KEY:
		if (!DEV_CONSOLE_IN_USE)
			m_isSlowMo = true;
		return true;

	case P_KEY:
		if (!DEV_CONSOLE_IN_USE)
			m_isPaused = true;
		return true;

	case Q_KEY:
		g_profiler->ProfilerPause();
		return true;

	case E_KEY:
		g_profiler->ProfilerResume();
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
	case T_KEY:
		m_isSlowMo = false;
		return true;

	case P_KEY:
		m_isPaused = false;
		return true;

	case F1_KEY:
		m_theGame->SetDeveloperMode(false);
		return true;


	default:
		return m_theGame->HandleKeyReleased(key_code);
	}
}