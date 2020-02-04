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
#include "Game/Point.hpp"
#include "Game/ConvexShape.hpp"

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
	m_entities = std::vector<Entity*>();
	m_entities.push_back(new Point(this));
	m_entities.push_back(new ConvexShape2D(this));
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

	for (int ent_idx = 0; ent_idx < static_cast<int>(m_entities.size()); ++ent_idx)
	{
		delete m_entities[ent_idx];
		m_entities[ent_idx] = nullptr;
	}
	
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

	m_mousePos = g_theWindow->GetMousePosition(WORLD_BOUNDS);

	UpdateEntities(delta_seconds);
}

void Game::UpdateEntities(double delta_seconds)
{
	for (int ent_idx = 0; ent_idx < static_cast<int>(m_entities.size()); ++ent_idx)
	{
		m_entities[ent_idx]->Update(delta_seconds);
	}
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

// 	g_theRenderer->BindModelMatrix(m_quadTransform);
// 	g_theRenderer->BindMaterial(*m_woodMaterial);
// 	g_theRenderer->DrawMesh(*m_quad);

	RenderEntities();
	g_imGUI->Render();

	g_theRenderer->EndCamera(m_gameCamera);
	g_theDebugRenderer->RenderToCamera(m_gameCamera);

}

void Game::RenderEntities() const
{
	//would like to populate a buffer and do one single draw call, but till then

	for (int ent_idx = 0; ent_idx < static_cast<int>(m_entities.size()); ++ent_idx)
	{
		m_entities[ent_idx]->Render();
	}
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
	switch (key_code)
	{
		
		default:
			return false;
	}
		
}


void Game::SetDeveloperMode(const bool on_or_off)
{
	m_inDevMode = on_or_off;
}

Vec2 Game::GetMousePosition() const
{
	return m_mousePos;
}

bool Game::InDeveloperMode() const
{
	return m_inDevMode;
}


void Game::GarbageCollection() const
{
	// USED TO CLEAN UP UNUSED ENTITIES
}


void Game::InitCamera()
{
	m_gameCamera = new Camera();
	m_gameCamera->SetColorTarget(nullptr); // when binding, if nullptr, use the backbuffer
	//m_gameCamera->SetPerspectiveProjection(m_camFOVDegrees, WORLD_ASPECT, 0.1f, 100.0f);
	m_gameCamera->SetOrthoView(WORLD_BL_CORNER, WORLD_TR_CORNER);
}


void Game::InitGameObjs()
{
	//Get all the materials for the game objs
	m_woodMaterial = g_theRenderer->CreateOrGetMaterial("wood.mat");
	m_woodMaterial->m_shader->SetDepth(COMPARE_LESS_EQUAL, true);
	m_defaultShader = g_theRenderer->CreateOrGetShader("default_lit.hlsl");

	//Get the mesh for all the game objs
	CPUMesh quad_mesh;
	CpuMeshAddQuad(&quad_mesh, AABB2(-10.0f, -10.0f, 10.0f, 10.0f));
	m_quad = new GPUMesh(g_theRenderer);
	m_quad->CreateFromCPUMesh<Vertex_Lit>(quad_mesh); // we won't be updated this;
}
