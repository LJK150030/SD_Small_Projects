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
#include "Game/BSPTree.hpp"

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


Game::Game(): m_mouseEntity(this), m_movableRay(this)
{
	m_convexShapes = std::vector<ConvexShape2D*>();
	m_convexShapes.reserve(MAX_SHAPES);
	for(int shape_idx = 0; shape_idx < m_currentNumConvexShapes; ++shape_idx)
	{
		m_convexShapes.push_back(new ConvexShape2D(this));
	}
	

	
	m_invisibleRays = std::vector <Ray2>();
	m_invisibleRays.reserve(MAX_RAYS);
	for(int ray_idx = 0; ray_idx < m_currentNumRays; ++ray_idx)
	{
		float x = g_randomNumberGenerator.GetRandomFloatInRange(0.0, WORLD_HEIGHT * WORLD_ASPECT);
		float y = g_randomNumberGenerator.GetRandomFloatInRange(0.0, WORLD_HEIGHT);

		Vec2 dir = g_randomNumberGenerator.GetRandomVec2InsideUnitCircle();

		
		m_invisibleRays.push_back(Ray2(Vec2(x, y), dir));
	}
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
	for (int ent_idx = 0; ent_idx < static_cast<int>(m_convexShapes.size()); ++ent_idx)
	{
		delete m_convexShapes[ent_idx];
		m_convexShapes[ent_idx] = nullptr;
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

	const float fps = 1.0f / static_cast<float>(delta_seconds);

	ImGui::Text("CurrentFrame: %i", m_currentFrame);
	ImGui::Text("FPS: %f", CalcAverageTick(fps));
	ImGui::Text("Num Shapes: %i", m_currentNumConvexShapes);
	ImGui::Text("Num Rays: %i", m_currentNumRays);

	m_mousePos = g_theWindow->GetMousePosition(WORLD_BOUNDS);

	UpdateEntities(delta_seconds);
	
	m_numHits = 0;
	for(int ray_idx = 0; ray_idx < m_currentNumRays; ++ray_idx)
	{
		for(int convex_idx = 0; convex_idx < m_currentNumConvexShapes; ++convex_idx)
		{
			float t_val[] = { 0.0f };
			const bool hit = RayToConvexShape(m_invisibleRays[ray_idx], *m_convexShapes[convex_idx]);

			if (hit)
			{
				++m_numHits;
				break;
			}
		}
	}

	if(m_sceneUpdated)
	{
		m_bspSet = false;
	}

}

void Game::UpdateEntities(double delta_seconds)
{
	m_mouseEntity.Update(static_cast<float>(delta_seconds));
	m_movableRay.PreUpdate();
	
	MouseCollisionTest(m_selectedShapes);


	float smallest_contact_point = INFINITY;
	int closest_convx_idx = -1;
	int closest_plain_idx = -1;
	for (int ent_idx = 0; ent_idx < static_cast<int>(m_convexShapes.size()); ++ent_idx)
	{
		m_convexShapes[ent_idx]->Update(static_cast<float>(delta_seconds));

		float t_val[] = { 0.0f };
		int plain_idx = -1;
		const bool hit = m_movableRay.CollideWithConvexShape(t_val, &plain_idx, *m_convexShapes[ent_idx]);

		if (hit)
		{
			if (smallest_contact_point > t_val[0])
			{
				smallest_contact_point = t_val[0];
				closest_convx_idx = ent_idx;
				closest_plain_idx = plain_idx;
			}
		}
	}

	if (smallest_contact_point != INFINITY)
	{
		m_movableRay.SetEnd(smallest_contact_point, m_convexShapes[closest_convx_idx], closest_plain_idx);
	}
	
	
	m_movableRay.Update(static_cast<float>(delta_seconds));
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

	RenderEntities();

	if(m_bspSet)
	{
		m_bspTree.Render();
	}
	
	g_imGUI->Render();

	g_theRenderer->EndCamera(m_gameCamera);
	g_theDebugRenderer->RenderToCamera(m_gameCamera);

}

void Game::RenderEntities() const
{
	//would like to populate a buffer and do one single draw call, but till then
	for (int ent_idx = 0; ent_idx < static_cast<int>(m_convexShapes.size()); ++ent_idx)
	{
		m_convexShapes[ent_idx]->Render();
	}

	m_movableRay.Render();
	m_mouseEntity.Render();
}

void Game::EndFrame() const
{
	g_imGUI->EndFrame();
}

bool Game::HandleKeyPressed(const unsigned char key_code)
{
	switch (key_code)
	{
		//updating shape
		case A_KEY: // rotate CCW
		{
			for (int hover_id = 0; hover_id < static_cast<int>(m_selectedShapes.size()); ++hover_id)
			{
				m_selectedShapes[hover_id]->AddRotationDegrees(-5.0f);
			}

			m_sceneUpdated = true;
			break;
		}
		case S_KEY: // rotate CW
		{
			for (int hover_id = 0; hover_id < static_cast<int>(m_selectedShapes.size()); ++hover_id)
			{
				m_selectedShapes[hover_id]->AddRotationDegrees(5.0f);
			}

			m_sceneUpdated = true;
			break;
		}
		case Z_KEY: // scale down
		{
			for (int hover_id = 0; hover_id < static_cast<int>(m_selectedShapes.size()); ++hover_id)
			{
				m_selectedShapes[hover_id]->AddScalarValue(-1.0f);
			}

			m_sceneUpdated = true;
			break;
		}
		case X_KEY: // scale up
		{
			for (int hover_id = 0; hover_id < static_cast<int>(m_selectedShapes.size()); ++hover_id)
			{
				m_selectedShapes[hover_id]->AddScalarValue(1.0f);
			}

			m_sceneUpdated = true;
			break;
		}
		case NUM_1_KEY: // move start point
		{
			m_movableRay.SetStart(m_mousePos);
			break;
		}
		case NUM_2_KEY: // move end point
		{
			m_movableRay.SetEnd(m_mousePos);
			break;
		}
		default:
		{
			return false;
		}
	}

	return true;
}


bool Game::HandleKeyReleased(const unsigned char key_code)
{
	switch (key_code)
	{
		//--------------------------------------
		//updating quantity
		case Q_KEY: // half the number of shapes
		{
			m_currentNumConvexShapes = ClampInt(
				m_currentNumConvexShapes / 2,
				MIN_SHAPES, 
				MAX_SHAPES
			);

			UpdateNumberOfShapes();
			m_sceneUpdated = true;
			break;
		}
		case W_KEY: // double the number of shapes
		{
			m_currentNumConvexShapes = ClampInt(
				m_currentNumConvexShapes * 2,
				MIN_SHAPES, 
				MAX_SHAPES
			);

			UpdateNumberOfShapes();
			m_sceneUpdated = true;
			break;
		}
		case E_KEY: // half the number of shapes
		{
			m_currentNumRays = ClampInt(
				m_currentNumRays / 2,
				MIN_RAYS,
				MAX_RAYS
			);
			UpdateNumberOfRays();
			break;
		}
		case R_KEY: // double the number of shapes
		{
			m_currentNumRays = ClampInt(
				m_currentNumRays * 2,
				MIN_RAYS,
				MAX_RAYS
			);
			UpdateNumberOfRays();
			break;
		}
		case D_KEY: // re-roll the scene and build BSP tree
		{
			const int cur_rays = m_currentNumRays;
			m_currentNumRays = 1;
			UpdateNumberOfRays();
			m_currentNumRays = cur_rays;
			UpdateNumberOfRays();
				
			const int cur_shapes = m_currentNumConvexShapes;
			m_currentNumConvexShapes = 1;
			UpdateNumberOfShapes();
			m_currentNumConvexShapes = cur_shapes;
			UpdateNumberOfShapes();

			m_bspTree.BuildBspTree(HEURISTIC_RANDOM, m_convexShapes);
			m_sceneUpdated = false;
			m_bspSet = true;
			break;
		}
		case F2_KEY:
		{
			m_bspTree.BuildBspTree(HEURISTIC_RANDOM, m_convexShapes);
			m_sceneUpdated = false;
			m_bspSet = true;
			break;
		}
		
		default:
		{
			return false;
		}
	}
	
	return true;
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

void Game::UpdateNumberOfShapes()
{
	const int num_shapes_in_vec = static_cast<int>(m_convexShapes.size());
	const int current_index = num_shapes_in_vec - 1;

	if(num_shapes_in_vec == m_currentNumConvexShapes)
	{
		return;
	}

	if(num_shapes_in_vec < m_currentNumConvexShapes) // we need to add more
	{
		const int difference = m_currentNumConvexShapes - num_shapes_in_vec;
		
		for(int shape_adding = 0; shape_adding < difference; ++shape_adding)
		{
			m_convexShapes.emplace_back(new ConvexShape2D(this));
		}
	}
	else // we need to "remove" some
	{
		const int difference = num_shapes_in_vec - m_currentNumConvexShapes;

		for (int shape_removing = 0; shape_removing < difference; ++shape_removing)
		{
			ConvexShape2D* current_shape = m_convexShapes[current_index - shape_removing];
			delete current_shape;
			current_shape = nullptr;
			m_convexShapes.pop_back();
		}
	}

}


void Game::UpdateNumberOfRays()
{
	const int num_rays_in_vec = static_cast<int>(m_invisibleRays.size());
	const int current_index = num_rays_in_vec - 1;

	if (num_rays_in_vec == m_currentNumRays)
	{
		return;
	}

	if (num_rays_in_vec < m_currentNumRays) // we need to add more
	{
		const int difference = m_currentNumRays - num_rays_in_vec;

		for (int shape_adding = 0; shape_adding < difference; ++shape_adding)
		{
			float x = g_randomNumberGenerator.GetRandomFloatInRange(0.0, WORLD_HEIGHT * WORLD_ASPECT);
			float y = g_randomNumberGenerator.GetRandomFloatInRange(0.0, WORLD_HEIGHT);

			Vec2 dir = g_randomNumberGenerator.GetRandomVec2InsideUnitCircle();


			m_invisibleRays.push_back(Ray2(Vec2(x, y), dir));
		}
	}
	else // we need to "remove" some
	{
		const int difference = num_rays_in_vec - m_currentNumRays;

		for (int shape_removing = 0; shape_removing < difference; ++shape_removing)
		{
			m_invisibleRays.pop_back();
		}
	}
}

void Game::MouseCollisionTest(std::vector<ConvexShape2D*>& out)
{
	out.clear();
	
	//TODO: from naive attempt to Space partitioning
	for(int shape_idx = 0; shape_idx < static_cast<int>(m_convexShapes.size()); ++shape_idx)
	{
		if(m_convexShapes[shape_idx]->CollisionFromPoint(m_mousePos))
		{
			out.push_back(m_convexShapes[shape_idx]);
		}
	}
	
	
}

bool Game::RayToConvexShape(const Ray2& ray, const ConvexShape2D& shape)
{
	std::vector<Plane2> planes = shape.GetLocalConvexPlanes();
	float t_vals[2];

	// check if inside
	if (shape.IsPointInsideShape(ray.m_pos))
	{
		return true;
	}

	// mid check
	bool mid_test = false;
	const uint num_hits = Raycast(t_vals, ray, shape.GetPosition(), shape.GetScale());
	mid_test = num_hits > 0;
	

	if (mid_test)
	{
		float smallest_t_val = INFINITY;
		int plane_intersection_idx = -1;

		//check all planes if they intersect.
		for (int plane_idx = 0; plane_idx < static_cast<int>(planes.size()); ++plane_idx)
		{
			float t_vals[2];

			//world ray cast, to local ray cast
			Vec2 local_pos = PointToLocalSpace(ray.m_pos, planes[plane_idx].m_normal, planes[plane_idx].GetDirection(), planes[plane_idx].PointOnPlane());
			Vec2 local_dir = VectorToLocalSpace(ray.m_pos, planes[plane_idx].m_normal, planes[plane_idx].GetDirection());

			Ray2 local_ray_cast(local_pos, local_dir);

			uint num_hits = Raycast(t_vals, local_ray_cast, planes[plane_idx]);

			if (num_hits > 0)
			{
				if (t_vals[0] < smallest_t_val)
				{
					smallest_t_val = t_vals[0];
					plane_intersection_idx = plane_idx;
				}
			}
		}

		if(smallest_t_val != INFINITY)
		{
			return true;
		}
	}

	return false;
}
