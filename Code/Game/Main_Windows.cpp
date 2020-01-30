#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Memory/Mem.hpp"
#include "ThirdParty/imGUI/imgui.h"

#define WINDOW_TITLE "Protogame3D"


extern App* g_theApp = nullptr;

// The WndProc function for imGUI third party tool
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wPram, LPARAM lParam);

//-----------------------------------------------------------------------------------------------
// Handles Windows (Win32) messages/events; i.e. the OS is trying to tell us something happened.
// This function is called by Windows whenever we ask it for notifications
//-----------------------------------------------------------------------------------------------
bool WindowsMessageHandlingProcedure(void* window_handle, const uint32_t wm_message_code, const uintptr_t w_param, uintptr_t l_param)
{
	UNREFERENCED_PARAMETER(window_handle); 
	UNUSED(l_param);

	if(g_imGUI != nullptr)
	{
		bool imguiHandled = ImGui_ImplWin32_WndProcHandler((HWND) window_handle, wm_message_code, w_param, l_param);
		UNUSED(imguiHandled);
	}
	
	switch (wm_message_code)
	{
		case WM_ACTIVATE:
		{
			g_theWindow->LockMouse();
			return true;
		}

		case BN_KILLFOCUS:
		{
			g_theWindow->UnlockMouse();
			return true;
		}
	
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
		case WM_CLOSE:
		{
			g_theApp->HandleQuitRequested();
			return true; // "Consumes" this message (tells Windows "okay, we handled it")
		}

		// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
		case WM_KEYDOWN:
		{
			const unsigned char asKey = static_cast<unsigned char>(w_param);

			if(DEV_CONSOLE_IN_USE)
			{
				if(g_theDevConsole->HandleKey(asKey))
					return true;
			}
			if (asKey == VK_ESCAPE && !DEV_CONSOLE_IN_USE)
			{
				g_theApp->HandleQuitRequested();
				return true; // "Consumes" this message (tells Windows "okay, we handled it")
			}
			if(g_imGUI != nullptr)
			{
				const ImGuiIO& io = ImGui::GetIO();
				if(io.WantCaptureKeyboard)
				{
					return false;
				}
			}
			if(g_theApp->HandleKeyPressed(asKey))
				return true;
			break;
		}
		// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
		case WM_KEYUP:
		{
			unsigned char asKey = (unsigned char)w_param;
			if(g_imGUI != nullptr)
			{
				const ImGuiIO& io = ImGui::GetIO();
				if(io.WantCaptureKeyboard)
				{
					return false;
				}
			}
			if(g_theApp->HandleKeyReleased(asKey))
				return true;
			break;
		}
		case WM_CHAR:
		{
			if(DEV_CONSOLE_IN_USE)
			{
				const unsigned char asKey = static_cast<unsigned char>(w_param);
				if(g_theDevConsole->HandleChar(asKey))
					return true;
				break;
			}
			if(g_imGUI != nullptr)
			{
				const ImGuiIO& io = ImGui::GetIO();
				if(io.WantCaptureKeyboard)
				{
					return false;
				}
			}
		}
		case WM_RBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
			{
				if(g_imGUI != nullptr)
				{
					const ImGuiIO& io = ImGui::GetIO();
					if(io.WantCaptureKeyboard)
					{
						return false;
					}
				}
			}
	}

	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return false;

}

//-----------------------------------------------------------------------------------------------
void CreateWindowAndRenderContext(float clientAspect)
{
	g_theWindow = new WindowContext();
	g_theWindow->CreateTheWindow( WINDOW_TITLE, clientAspect, .90f, WindowsMessageHandlingProcedure );

	// this is the end of the windows part
	g_theRenderer = new RenderContext(g_theWindow);
}

//-----------------------------------------------------------------------------------------------
// Processes all Windows messages (WM_xxx) for this app that have queued up since last frame.
// For each message in the queue, our WindowsMessageHandlingProcedure (or "WinProc") function
//	is called, telling us what happened (key up/down, minimized/restored, gained/lost focus, etc.)
void RunMessagePump()
{
	g_theWindow->BeginFrame(); 
}

//-----------------------------------------------------------------------------------------------
// One "frame" of the game.  Generally: Input, Update, Render.  We call this 60+ times per second.
//
void RunFrame()
{
	RunMessagePump();
	g_theApp->RunFrame();	
}

//-----------------------------------------------------------------------------------------------
void Startup()
{
	tinyxml2::XMLDocument config;
	config.LoadFile("Data/GameConfig.xml");
	XmlElement* root = config.RootElement();
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*root);
	const float world_aspect = g_gameConfigBlackboard.GetValue("windowAspect", WORLD_ASPECT);
	CreateWindowAndRenderContext( world_aspect );
	g_theApp = new App();
	g_theApp->Startup();
}

//-----------------------------------------------------------------------------------------------
void Shutdown()
{
	//	Destroy the global App instance
	g_theApp->Shutdown();

	delete g_theApp;		
	g_theApp = nullptr;

	delete g_theWindow; 
	g_theWindow = nullptr; 
}

//-----------------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE application_instance_handle, HINSTANCE prev_instance, LPSTR command_line_string, int show_cmd)
{
	UNUSED( application_instance_handle ); 
	UNUSED( command_line_string );
	UNUSED( prev_instance );
	UNUSED( show_cmd );
	
	Startup();
	
	// This bit benefits the most from making it a class - knowing when a processing messages 
	// results int he window itself being closed so we can stop processing.
	while( !g_theApp->IsQuitting()) 
	{
		RunFrame();
		Sleep(0);
	}

	Shutdown();
	
	MemTrackLogLiveAllocations();

	return 0;
}