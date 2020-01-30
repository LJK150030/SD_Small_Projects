#pragma once
#include "Game/Game.hpp"

struct Rgba;

class App
{
public:
	App();
	~App();
	void Startup();
	void Shutdown();
	void RunFrame();
	bool IsQuitting() const { return m_isQuitting; }
	bool HandleKeyPressed(unsigned char key_code);
	bool HandleKeyReleased(unsigned char key_code);
	bool HandleQuitRequested();
	void HardRestart();

	static bool QuitRequest(EventArgs& args);
	static bool PrintMemAlloc(EventArgs& args);
	static bool LogMemAlloc(EventArgs& args);

private:
	void BeginFrame() const;
	void Update();
	void Render() const;
	void EndFrame() const;

private:
	bool m_isQuitting = false;

	double m_timeLastFrame = 0.0f;
	Game* m_theGame;

	Camera* m_devCamera = nullptr;
};

//----------------------------------------
