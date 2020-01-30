#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Core/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/Vertex_Lit.hpp"
#include "Engine/Core/Callstack.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/Renderer/ImGUISystem.hpp"

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

#include <vector>

UNITTEST("Is Test", nullptr, 0)
{
	return true;
}


UNITTEST("Is false", nullptr, 2)
{
	const Callstack cs = Callstack(2);

	const bool result = false;

	if constexpr (!result)
	{
		std::vector<std::string> callstack_output = ToStringCollection(cs);
		DebugOutputPrintCollection(callstack_output);
	}

	return result;
}


Game::Game()
{
}


Game::~Game()
{
}


void Game::Startup()
{
	InitCamera();
	InitGameObjs();
}


void Game::Shutdown()
{
	delete m_gameCamera;
	m_gameCamera = nullptr;

	delete m_quad;
	m_quad = nullptr;
}


void Game::BeginFrame() const
{
	// Feed inputs to dear imgui, start new frame
	g_imGUI->BeginFrame();
	ImGui::NewFrame();
}


void Game::Update(const double delta_seconds)
{
	m_time += static_cast<float>(delta_seconds);
	m_currentFrame++;

	ImGui::Text("Hello\nWorld"); ImGui::SameLine();
}


void Game::Render() const
{
	ColorTargetView* rtv = g_theRenderer->GetFrameColorTarget();
	m_gameCamera->SetColorTarget(rtv);

	// Move the camera to where it is in the scene
	// (right now, no rotation (looking forward), set 10 back (so looking at 0,0,0)
	const Matrix44 cam_model = Matrix44::ForEulerZXY(m_camEuler, m_camPosition);
	m_gameCamera->SetModelMatrix(cam_model);

	g_theRenderer->BeginCamera(m_gameCamera);
	g_theRenderer->ClearScreen(Rgba::BLACK);
	g_theRenderer->ClearDepthStencilTarget(1.0f);

	g_theRenderer->BindModelMatrix(m_quadTransform);
	g_theRenderer->BindMaterial(*m_woodMaterial);
	g_theRenderer->DrawMesh(*m_quad);

	g_imGUI->Render();

	g_theRenderer->EndCamera(m_gameCamera);
	g_theDebugRenderer->RenderToCamera(m_gameCamera);

}

void Game::EndFrame() const
{
	g_imGUI->EndFrame();
}

bool Game::HandleKeyPressed(const unsigned char key_code)
{
	switch (key_code)
	{
	default:
		return false;
	}
}


bool Game::HandleKeyReleased(const unsigned char key_code)
{
	UNUSED(key_code);
	return true;
}


void Game::SetDeveloperMode(const bool on_or_off)
{
	m_inDevMode = on_or_off;
}


void Game::GarbageCollection() const
{
	// USED TO CLEAN UP UNUSED ENTITIES
}


void Game::InitCamera()
{
	m_gameCamera = new Camera();
	m_gameCamera->SetColorTarget(nullptr); // when binding, if nullptr, use the backbuffer
	m_gameCamera->SetPerspectiveProjection(m_camFOVDegrees, WORLD_ASPECT, 0.1f, 100.0f);
}


void Game::InitGameObjs()
{
	//Get all the materials for the game objs
	m_woodMaterial = g_theRenderer->CreateOrGetMaterial("wood.mat");
	m_woodMaterial->m_shader->SetDepth(COMPARE_LESS_EQUAL, true);
	m_defaultShader = g_theRenderer->CreateOrGetShader("default_lit.hlsl");

	//Get the mesh for all the game objs
	CPUMesh quad_mesh;
	CpuMeshAddQuad(&quad_mesh, AABB2(-1.0f, -1.0f, 1.0f, 1.0f));
	m_quad = new GPUMesh(g_theRenderer);
	m_quad->CreateFromCPUMesh<Vertex_Lit>(quad_mesh); // we won't be updated this;
}
