#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"


void DrawLine(const Vec2& start, const Vec2& end, float thickness, const Rgba& tint)
{
	std::vector<Vertex_PCU> lineVerts;
	AddVertsForLine2D(lineVerts, start, end, thickness, tint);
	g_theRenderer->DrawVertexArray(lineVerts);
}
