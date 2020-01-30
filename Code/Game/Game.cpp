#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Math/Matrix44.hpp"
#include <vector>
#include "Engine/Core/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Vertex_Lit.hpp"
#include "Engine/Core/Callstack.hpp"
#include "Engine/Architecture/CircularBuffer.hpp"
#include "Engine/Renderer/ImGUISystem.hpp"
#include "Engine/Tools/Profiler.hpp"


UNITTEST("Is Test", nullptr, 0)
{
	return true; 
}

UNITTEST("Is false", nullptr, 2)
{
	Callstack cs = Callstack(2);

	const bool result = false;

	if(!result)
	{
		std::vector<std::string> callstack_output;
		callstack_output = ToStringCollection(cs);
		DebugOutputPrintCollection(callstack_output);
	}
	
	return result; 
}

Game::Game()
{
	m_inDevMode = false;
	m_time = 0.0f;
}

Game::~Game()
{
}

void Game::Startup()
{	
	//LogHook(&OutputLog);

// 	g_theDevConsole->ExecuteCommandString("logEnbAll");
// 	g_theDevConsole->ExecuteCommandString("log tag=cf msg=HelloWorld");
// 	g_theDevConsole->ExecuteCommandString("logDisAll");
// 	g_theDevConsole->ExecuteCommandString("log tag=cf msg=FilteredTest");
// 	g_theDevConsole->ExecuteCommandString("logEnb tag=test");
// 	g_theDevConsole->ExecuteCommandString("log tag=cf msg=ThisMessageShouldNotShowUp");
// 	g_theDevConsole->ExecuteCommandString("log tag=test msg=ThisMessageShouldShowUp");
// 	g_theDevConsole->ExecuteCommandString("logEnbAll");
// 	g_theDevConsole->ExecuteCommandString("logDis tag=test");
// 	g_theDevConsole->ExecuteCommandString("log tag=cf msg=ThisMessageShouldNowShowUp");
// 	g_theDevConsole->ExecuteCommandString("log tag=test msg=SoShouldThisOne");


	m_gameCamera = new Camera();
	m_gameCamera->SetColorTarget(nullptr); // when binding, if nullptr, use the backbuffer

	// set projection; 
	m_gameCamera->SetPerspectiveProjection(m_camFOVDegrees, WORLD_ASPECT, 0.1f, 100.0f);

// 	m_boxMaterial = new Material(g_theRenderer);
// 	m_boxMaterial->SetShader("default_lit.hlsl");
// 	m_boxMaterial->m_shader->SetDepth(COMPARE_LESS_EQUAL, true);
// 	m_boxMaterial->SetTextureView(TEXTURE_MAP_DIFFUSE, "woodcrate.jpg");

// 	m_testMaterial = new Material(g_theRenderer);
// 	m_testMaterial->SetShader("default_lit.hlsl");
// 	m_testMaterial->m_shader->SetDepth(COMPARE_LESS_EQUAL, true);
// 	m_testMaterial->SetTextureView(TEXTURE_MAP_DIFFUSE, "Test_StbiFlippedAndOpenGL.png");

// 	m_globeMaterial = new Material(g_theRenderer);
// 	m_globeMaterial->SetShader("default_lit.hlsl");
// 	m_globeMaterial->m_shader->SetDepth(COMPARE_LESS_EQUAL, true);
// 	m_globeMaterial->SetTextureView(TEXTURE_MAP_DIFFUSE, "2k_earth_daymap.jpg");

	m_normalsMaterial = new Material(g_theRenderer);
	m_normalsMaterial->SetShader("normals_test.hlsl");
	m_normalsMaterial->m_shader->SetDepth(COMPARE_LESS_EQUAL, true);

	m_tangentsMaterial = new Material(g_theRenderer);
	m_tangentsMaterial->SetShader("tangents_test.hlsl");
	m_tangentsMaterial->m_shader->SetDepth(COMPARE_LESS_EQUAL, true);

	m_bitangentsMaterial = new Material(g_theRenderer);
	m_bitangentsMaterial->SetShader("bitangents_test.hlsl");
	m_bitangentsMaterial->m_shader->SetDepth(COMPARE_LESS_EQUAL, true);

	m_couchMaterial = g_theRenderer->CreateOrGetMaterial("couch.mat");
	m_couchMaterial->m_shader->SetDepth(COMPARE_LESS_EQUAL, true);

	m_pebblesMaterial = g_theRenderer->CreateOrGetMaterial("pebbles.mat");
	m_pebblesMaterial->m_shader->SetDepth(COMPARE_LESS_EQUAL, true);

	m_woodMaterial = g_theRenderer->CreateOrGetMaterial("wood.mat");
	m_woodMaterial->m_shader->SetDepth(COMPARE_LESS_EQUAL, true);

// 	m_tangentSpaceMaterial = g_theRenderer->CreateOrGetMaterial("couch.mat");
// 	m_tangentSpaceMaterial->SetShader("tbn_test.hlsl");
// 	m_tangentSpaceMaterial->m_shader->SetDepth(COMPARE_LESS_EQUAL, true);

	m_defaultShader = g_theRenderer->CreateOrGetShader("default_lit.hlsl");
	// says only write color if depth is less-equal to whatever was already there
	// and that we also want to update the depth when we do so; 

	// create a cube (centered at zero, with sides 2 length)
	CPUMesh cube_mesh;
	CpuMeshAddCube(&cube_mesh, AABB3(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 1.0f)));
	m_cube = new GPUMesh(g_theRenderer);
	m_cube->CreateFromCPUMesh<Vertex_Lit>(cube_mesh); // we won't be updated this;

	CPUMesh quad_mesh;
	CpuMeshAddQuad(&quad_mesh, AABB2(-1.0f, -1.0f, 1.0f, 1.0f));
	m_quad = new GPUMesh(g_theRenderer);
	m_quad->CreateFromCPUMesh<Vertex_Lit>(quad_mesh); // we won't be updated this;

	CPUMesh sphere_mesh;
	CpuMeshAddUVSphere(&sphere_mesh, Vec3(0.0f, 0.0f, 0.0f), 4.0f);
	m_sphere = new GPUMesh(g_theRenderer);
	m_sphere->CreateFromCPUMesh<Vertex_Lit>(sphere_mesh); // we won't be updated this;

	////////////////////////////////////////////////////////////////////////////////////////
	LightT directional = g_theRenderer->m_lightContainer.m_lights[0];
	directional.m_color = Vec3(1.0f, 0.0f, 1.0f);
	directional.m_intensity = 0.5f;
	directional.m_position = Vec3(-5.0f, 0.0f, -2.0f);
	directional.m_isDirectional = 1.0f;
	directional.m_direction = Vec3(1.0f, 1.0f, 1.0f);
	g_theRenderer->UpdateLightAtIndex(0, directional);

	m_pointLights1.m_color = Vec3(1.0f, 0.0f, 1.0f);
	m_pointLights1.m_intensity = 0.5f;
	m_pointLights1.m_position = Vec3(-5.0f, 0.0f, -2.0f);
	m_pointLights1.m_isDirectional = 0.0f;
	m_pointLights1.m_direction = Vec3(1.0f, 1.0f, 1.0f);
	g_theRenderer->AddLight(m_pointLights1);

	m_pointLights2.m_color = Vec3(0.0f, 0.0f, 1.0f);
	m_pointLights2.m_intensity = 0.5f;
	m_pointLights2.m_position = Vec3(-5.0f, 0.0f, -2.0f);
	m_pointLights2.m_isDirectional = 0.0f;
	m_pointLights2.m_direction = Vec3(1.0f, 1.0f, 1.0f);
	m_pointLights2.m_diffuseAttenuation = Vec3(0.0f, 0.0f, 1.0f);
	m_pointLights2.m_specularAttenuation = Vec3(1.0f, 1.0f, 0.0f);
	g_theRenderer->AddLight(m_pointLights2);

	m_pointLights3.m_color = Vec3(0.0f, 1.0f, 1.0f);
	m_pointLights3.m_intensity = 0.5f;
	m_pointLights3.m_position = Vec3(-5.0f, 0.0f, -2.0f);
	m_pointLights3.m_isDirectional = 0.0f;
	m_pointLights3.m_direction = Vec3(1.0f, 1.0f, 1.0f);
	m_pointLights3.m_diffuseAttenuation = Vec3(0.0f, 0.0f, 1.0f);
	m_pointLights3.m_specularAttenuation = Vec3(1.0f, 1.0f, 0.0f);
	g_theRenderer->AddLight(m_pointLights3);

	m_pointLights4.m_color = Vec3(1.0f, 0.0f, 0.0f);
	m_pointLights4.m_intensity = 0.5f;
	m_pointLights4.m_position = Vec3(-5.0f, 0.0f, -2.0f);
	m_pointLights4.m_isDirectional = 0.0f;
	m_pointLights4.m_direction = Vec3(1.0f, 1.0f, 1.0f);
	m_pointLights4.m_diffuseAttenuation = Vec3(0.0f, 0.0f, 1.0f);
	m_pointLights4.m_specularAttenuation = Vec3(1.0f, 1.0f, 0.0f);
	g_theRenderer->AddLight(m_pointLights4);

//  	g_theRenderer->BindShader(m_defaultShader);
//  	g_theRenderer->BindSampler(SAMPLE_MODEL_LINEAR);
}

void Game::Shutdown()
{
	delete m_gameCamera;
	m_gameCamera = nullptr;

	delete m_cube;
	m_cube = nullptr;

	delete m_quad;
	m_quad = nullptr;

	delete m_sphere;
	m_sphere = nullptr;
}


void Game::Update(const double delta_seconds)
{
	g_profiler->ProfilePush("Game::Update");
	m_time += static_cast<float>(delta_seconds);
	m_currentFrame++;
	m_deltaMovement = m_cameraMovementScale * static_cast<float>(delta_seconds);

	const IntVec2 mouse_movement = g_theWindow->GetClientMouseRelativeMovement();
	const Vec3 mouse_movement_float = Vec3(mouse_movement.y * m_mouseMovementScale,
	                                       mouse_movement.x * m_mouseMovementScale, 0.0f);
	m_camEuler += mouse_movement_float;
	m_cubeTransform = Matrix44::ForEulerZXY(Vec3(-m_time * 19.0f, 0.0f, 0.0f), Vec3(-5.0f, 0.0f, 0.0f));
	m_sphereTransform = Matrix44::ForEulerZXY(Vec3(0.0f, m_time, 0.0f), Vec3(5.0f, 0.0f, 0.0f));

	g_theDebugRenderer->DebugRenderMessagef(0.0f, Rgba::YELLOW, Rgba::YELLOW, "current frame: %d", m_currentFrame);
	g_theDebugRenderer->DebugRenderMessagef(0.0f, Rgba::YELLOW, Rgba::YELLOW, "current time: %f", GetCurrentTimeSecondsF());

	float sin_time = SinDegrees(m_time * 29.7067174262f);
	float cos_time = CosDegrees(m_time * 34.3064415949f);

	m_pl1Position = Vec3( -4.0f, (cos_time - sin_time), (-1.0f * sin_time - cos_time));
	g_theDebugRenderer->DebugRenderPoint(0.0f, DEBUG_RENDER_USE_DEPTH, m_pl1Position, Rgba(m_pointLights1.m_color), Rgba(m_pointLights1.m_color)); 

	sin_time = sin_time * 2.0f;
	cos_time = cos_time * 2.0f;
	m_pl2Position = Vec3( -2.0f, (cos_time - sin_time), (-1.0f * sin_time - cos_time));
	g_theDebugRenderer->DebugRenderPoint(0.0f, DEBUG_RENDER_USE_DEPTH, m_pl2Position, Rgba(m_pointLights2.m_color), Rgba(m_pointLights2.m_color)); 

	sin_time = sin_time * 1.5f;
	cos_time = cos_time * 1.5f;
	m_pl3Position = Vec3( 2.0f, (cos_time - sin_time), (-1.0f * sin_time - cos_time));
	g_theDebugRenderer->DebugRenderPoint(0.0f, DEBUG_RENDER_USE_DEPTH, m_pl3Position, Rgba(m_pointLights3.m_color), Rgba(m_pointLights3.m_color)); 

	sin_time = sin_time * 1.75f;
	cos_time = cos_time * 1.75f;
	m_pl4Position = Vec3( 4.0f, (cos_time - sin_time), (-1.0f * sin_time - cos_time));
	g_theDebugRenderer->DebugRenderPoint(0.0f, DEBUG_RENDER_USE_DEPTH, m_pl4Position, Rgba(m_pointLights4.m_color), Rgba(m_pointLights4.m_color)); 

	m_pointLights1.m_position = m_pl1Position;
	g_theRenderer->UpdateLightAtIndex(1, m_pointLights1);
	m_pointLights2.m_position = m_pl2Position;
	g_theRenderer->UpdateLightAtIndex(2, m_pointLights2);
	m_pointLights3.m_position = m_pl3Position;
	g_theRenderer->UpdateLightAtIndex(3, m_pointLights3);
	m_pointLights4.m_position = m_pl4Position;
	g_theRenderer->UpdateLightAtIndex(4, m_pointLights4);
	
	g_profiler->ProfilePop();
}

void Game::UpdateImGUI() const
{

	// Feed inputs to dear imgui, start new frame
	g_imGUI->BeginFrame();
	ImGui::NewFrame();

	// Any application code here
	ImGui::Text("Hello, world!");

	static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
	ImGui::PlotHistogram("Histogram", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 1.0f, ImVec2(0,80));


	struct Funcs
	{
		static float Sin(void*, int i) { return sinf(i * 0.1f); }
		static float Saw(void*, int i) { return (i & 1) ? 1.0f : -1.0f; }
	};
	
	static int func_type = 0;
	static int begin_i = 100, end_i = 1000;
	ImGui::Separator();
	ImGui::SetNextItemWidth(100);
	ImGui::Combo("func", &func_type, "Sin\0Saw\0");
	ImGui::SameLine();
	ImGui::DragIntRange2("Sample range", &begin_i, &end_i, 5, 0, 0, "Min: %d units", "Max: %d units");
	float (*func)(void*, int) = (func_type == 0) ? Funcs::Sin : Funcs::Saw;
	ImGui::PlotHistogram("Histogram", func, NULL, begin_i, end_i, NULL, -1.0f, 1.0f, ImVec2(0,80));
	
	// Render dear imgui into screen
	g_imGUI->Render();
	g_imGUI->EndFrame();
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

 	g_theRenderer->BindModelMatrix(m_cubeTransform);
	switch(m_testshaders)
	{
	case 0:
		g_theRenderer->BindMaterial(*m_couchMaterial);
		break;
	case 1:
		g_theRenderer->BindMaterial(*m_normalsMaterial);
		break;
	case 2:
		g_theRenderer->BindMaterial(*m_tangentsMaterial);
		break;
	case 3:
		g_theRenderer->BindMaterial(*m_bitangentsMaterial);
		break;
	}	
	g_theRenderer->DrawMesh(*m_cube);

 	g_theRenderer->BindModelMatrix(m_quadTransform);
	switch(m_testshaders)
	{
	case 0:
		g_theRenderer->BindMaterial(*m_woodMaterial);
		break;
	case 1:
		g_theRenderer->BindMaterial(*m_normalsMaterial);
		break;
	case 2:
		g_theRenderer->BindMaterial(*m_tangentsMaterial);
		break;
	case 3:
		g_theRenderer->BindMaterial(*m_bitangentsMaterial);
		break;
	}	
	g_theRenderer->DrawMesh(*m_quad);

 	g_theRenderer->BindModelMatrix(m_sphereTransform);

	switch(m_testshaders)
	{
	case 0:
		g_theRenderer->BindMaterial(*m_pebblesMaterial);
		break;
	case 1:
		g_theRenderer->BindMaterial(*m_normalsMaterial);
		break;
	case 2:
		g_theRenderer->BindMaterial(*m_tangentsMaterial);
		break;
	case 3:
		g_theRenderer->BindMaterial(*m_bitangentsMaterial);
		break;
	}	
	g_theRenderer->DrawMesh(*m_sphere);


	g_theRenderer->EndCamera(m_gameCamera);
	g_theDebugRenderer->RenderToCamera(m_gameCamera);
}

bool Game::HandleKeyPressed(const unsigned char key_code)
{
	switch (key_code)
	{
	case SHIFT_KEY:
		{
		LightT directional = g_theRenderer->m_lightContainer.m_lights[0];
		directional.m_position = m_camPosition;
		g_theRenderer->UpdateLightAtIndex(0, directional);

		return true;
		}
	case LEFT_ARROW:
		{
		LightT directional = g_theRenderer->m_lightContainer.m_lights[0];
		directional.m_direction.y = directional.m_direction.y - 1.0f;
		g_theRenderer->UpdateLightAtIndex(0, directional);

		return true;
		}
	case RIGHT_ARROW:
	{
		LightT directional = g_theRenderer->m_lightContainer.m_lights[0];
		directional.m_direction.y = directional.m_direction.y + 1.0f;
		g_theRenderer->UpdateLightAtIndex(0, directional);

		return true;
	}
	case UP_ARROW:
	{
		LightT directional = g_theRenderer->m_lightContainer.m_lights[0];
		directional.m_direction.z = directional.m_direction.z + 1.0f;
		g_theRenderer->UpdateLightAtIndex(0, directional);

		return true;
	}
	case DOWN_ARROW:
	{
		LightT directional = g_theRenderer->m_lightContainer.m_lights[0];
		directional.m_direction.z = directional.m_direction.z - 1.0f;
		g_theRenderer->UpdateLightAtIndex(0, directional);

		return true;
	}
	case W_KEY:
		{
			MoveCamera(Vec3(0.0f, 0.0f, 1.0f));
			return true;
		}
	case A_KEY:
		{
			MoveCamera(Vec3(-1.0f, 0.0f, 0.0f));
			return true;
		}
	case S_KEY:
		{
			MoveCamera(Vec3(0.0f, 0.0f, -1.0f));
			return true;
		}
	case D_KEY:
		{
			MoveCamera(Vec3(1.0f, 0.0f, 0.0f));
			return true;
		}
	case SPACE_BAR:
		{
			m_testshaders = ++m_testshaders % 4;
			return true;
		}
	case N_KEY:
		{
			g_theRenderer->m_lightContainer.m_specFactor = ClampFloat(g_theRenderer->m_lightContainer.m_specFactor - 0.1f,
				0.0f, 1.0f);
			return true;
		}
	case M_KEY:
	{
		g_theRenderer->m_lightContainer.m_specFactor = ClampFloat(g_theRenderer->m_lightContainer.m_specFactor + 0.1f,
			0.0f, 1.0f);
			return true;
	}
	case V_KEY:
	{
		g_theRenderer->m_lightContainer.m_specPower = ClampFloat(g_theRenderer->m_lightContainer.m_specPower - 0.1f,
			0.0f, 1.0f);
		return true;
	}
	case B_KEY:
	{
		g_theRenderer->m_lightContainer.m_specPower = ClampFloat(g_theRenderer->m_lightContainer.m_specPower + 0.1f,
			0.0f, 100.0f);
		return true;
	}
	case NUM_1_KEY:
		{
		g_theRenderer->m_lightContainer.m_ambient.x = ClampFloat(g_theRenderer->m_lightContainer.m_ambient.x - 0.1f,
			0.0f, 1.0f);
		return true;
		}
	case NUM_2_KEY:
	{
		g_theRenderer->m_lightContainer.m_ambient.x = ClampFloat(g_theRenderer->m_lightContainer.m_ambient.x + 0.1f,
			0.0f, 1.0f);
		return true;
	}
	case NUM_3_KEY:
	{
		g_theRenderer->m_lightContainer.m_ambient.y = ClampFloat(g_theRenderer->m_lightContainer.m_ambient.y - 0.1f,
			0.0f, 1.0f);
		return true;
	}
	case NUM_4_KEY:
	{
		g_theRenderer->m_lightContainer.m_ambient.y = ClampFloat(g_theRenderer->m_lightContainer.m_ambient.y + 0.1f,
			0.0f, 1.0f);
		return true;
	}
	case NUM_5_KEY:
	{
		g_theRenderer->m_lightContainer.m_ambient.z = ClampFloat(g_theRenderer->m_lightContainer.m_ambient.z - 0.1f,
			0.0f, 1.0f);
		return true;
	}
	case NUM_6_KEY:
	{
		g_theRenderer->m_lightContainer.m_ambient.z = ClampFloat(g_theRenderer->m_lightContainer.m_ambient.z + 0.1f,
			0.0f, 1.0f);
		return true;
	}
	case NUM_7_KEY:
	{
		g_theRenderer->m_lightContainer.m_ambient.w = ClampFloat(g_theRenderer->m_lightContainer.m_ambient.w - 0.1f,
			0.0f, 1.0f);
		return true;
	}
	case NUM_8_KEY:
	{
		g_theRenderer->m_lightContainer.m_ambient.w = ClampFloat(g_theRenderer->m_lightContainer.m_ambient.w + 0.1f,
			0.0f, 1.0f);
		return true;
	}
	case NUM_9_KEY:
	{
		g_theRenderer->m_lightContainer.m_emissiveFactor = ClampFloat(g_theRenderer->m_lightContainer.m_emissiveFactor - 0.1f,0.0f, 1.0f);
		return true;
	}
	case NUM_0_KEY:
	{
		g_theRenderer->m_lightContainer.m_emissiveFactor = ClampFloat(g_theRenderer->m_lightContainer.m_emissiveFactor + 0.1f,0.0f, 1.0f);
		return true;
	}
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
}

void Game::MoveCamera(const Vec3& local_movement)
{
	const Matrix44 camera_model = m_gameCamera->GetModelMatrix();
	const Vec3 movement = camera_model.GetTransformVector3D(local_movement * m_deltaMovement);
	m_camPosition += movement;
}