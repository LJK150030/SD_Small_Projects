#include "Game/BSPTree.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Ray2.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

BSPNode::BSPNode() = default;
BSPNode::~BSPNode()
{
}


BSPTree::BSPTree()
{
	m_bspTree = std::vector<BSPNode>();
	m_sceneSegments = std::vector<Segment2>();

	m_material = g_theRenderer->CreateOrGetMaterial("white.mat");
}


BSPTree::~BSPTree()
{
	Clear();
}


void BSPTree::BuildBspTree(BspHeuristic plane_selection, const std::vector<ConvexShape2D*>& geometry_list)
{
	m_heuristicType = plane_selection;
	Clear();

	const int num_geometry = static_cast<int>(geometry_list.size());
// 	m_sceneSegments.reserve(num_geometry * 5 + 4);
// 	m_sceneSegments.emplace_back(WORLD_BOUNDS.mins, Vec2(WORLD_BOUNDS.maxs.x, WORLD_BOUNDS.mins.y));
// 	m_sceneSegments.emplace_back(Vec2(WORLD_BOUNDS.maxs.x, WORLD_BOUNDS.mins.y), WORLD_BOUNDS.maxs);
// 	m_sceneSegments.emplace_back(WORLD_BOUNDS.maxs, Vec2(WORLD_BOUNDS.mins.x, WORLD_BOUNDS.maxs.y));
// 	m_sceneSegments.emplace_back(Vec2(WORLD_BOUNDS.mins.x, WORLD_BOUNDS.maxs.y), WORLD_BOUNDS.mins);

	std::vector<int> seg_indexes = std::vector<int>();
	seg_indexes.reserve(num_geometry * 5);
	int current_num_segs = 0;

	for(int geometry_idx = 0; geometry_idx < num_geometry; ++geometry_idx)
	{
		std::vector<Segment2> convex_segments = geometry_list[geometry_idx]->GetWorldConvexSegments();
		const int num_segments = static_cast<int>(convex_segments.size());

		for(int segments_idx = 0; segments_idx < num_segments; ++segments_idx)
		{
			m_sceneSegments.emplace_back(convex_segments[segments_idx]);
			seg_indexes.push_back(current_num_segs);
			++current_num_segs;
		}
	}

	m_bspTree.reserve(num_geometry * 5);
	m_bspTree.emplace_back(); //root node
	
	BuildBspSubTree(0, seg_indexes, -1);

	//don't need this info any more
	m_sceneSegments.clear();

	//walk the tree to add rendering objs
	AddRenderObjs(0);
	
	//walk the tree to set the type
	SettingSpaceTypes(0);
}

bool BSPTree::CanSee(const Vec2& start, const Vec2& end, Vec2& out_end)
{
	const bool result = CanSee(start, end, out_end, 0);
	return result;
}


void BSPTree::Render() const
{
	RenderLowestNodes(0);
}


void BSPTree::BuildBspSubTree(int current_node_idx, const std::vector<int>& seg_index_list, int parent_idx)
{
	std::vector<int> front_idx_list = std::vector<int>();
	std::vector<int> back_idx_list = std::vector<int>();

	const int best_split_idx = SelectBestSplitterIndex(seg_index_list, parent_idx);
	const int best_world_split_idx = seg_index_list[best_split_idx];
	const Segment2 best_split = m_sceneSegments[best_world_split_idx];
	const Plane2 split(best_split.m_start, best_split.m_end);
	m_bspTree[current_node_idx].m_split = split;
	m_bspTree[current_node_idx].m_segment = best_split;
	

	const int max_segments = static_cast<int>(seg_index_list.size());
	for(int seg_idx = 0; seg_idx < max_segments; ++seg_idx)
	{
		if(seg_idx != best_split_idx)
		{
			const int test_seg_index = seg_index_list[seg_idx];
			const Segment2 test_segment = m_sceneSegments[test_seg_index];
			SegmentType test_type = ClassifySegment(test_segment, m_bspTree[current_node_idx].m_split);

			switch(test_type)
			{
				case SEGMENT_BEHIND:
				{
					back_idx_list.push_back(test_seg_index);
					break;
				}
				case SEGMENT_INFRONT:
				{
					front_idx_list.push_back(test_seg_index);
					break;
				}
				case SEGMENT_STRADDLING:
				{
					int back_seg_idx = -1;
					int front_seg_idx = -1;

					SplitPolygon(test_segment, split, back_seg_idx, front_seg_idx);
					back_idx_list.push_back(back_seg_idx);
					front_idx_list.push_back(front_seg_idx);
					break;
				}
			}
		}
	}


	const int front_list_size = front_idx_list.size();
	if(front_list_size == 0)
	{
		m_bspTree.emplace_back();

		const int leaf_idx = static_cast<int>(m_bspTree.size()) - 1;
		m_bspTree[leaf_idx].m_parentIdx = current_node_idx;
		m_bspTree[leaf_idx].m_isLeaf = true;
		m_bspTree[leaf_idx].m_spaceType = SPACE_FREE;

		m_bspTree[current_node_idx].m_frontChildIdx = leaf_idx;
	}
	else
	{
		m_bspTree.emplace_back();

		const int node_idx = static_cast<int>(m_bspTree.size()) - 1;
		m_bspTree[node_idx].m_parentIdx = current_node_idx;
		m_bspTree[node_idx].m_isLeaf = false;
		m_bspTree[node_idx].m_spaceType = SPACE_FREE;

		m_bspTree[current_node_idx].m_frontChildIdx = node_idx;
		BuildBspSubTree(node_idx, front_idx_list, current_node_idx);
	}

	const int back_list_size = back_idx_list.size();
	if (back_list_size == 0)
	{
		m_bspTree.emplace_back();

		const int leaf_idx = static_cast<int>(m_bspTree.size()) - 1;
		m_bspTree[leaf_idx].m_parentIdx = current_node_idx;
		m_bspTree[leaf_idx].m_isLeaf = true;
		m_bspTree[leaf_idx].m_spaceType = SPACE_SOLID;

		m_bspTree[current_node_idx].m_backChildIdx = leaf_idx;
	}
	else
	{
		m_bspTree.emplace_back();

		const int node_idx = static_cast<int>(m_bspTree.size()) - 1;
		m_bspTree[node_idx].m_parentIdx = current_node_idx;
		m_bspTree[node_idx].m_isLeaf = false;
		m_bspTree[node_idx].m_spaceType = SPACE_FREE;

		m_bspTree[current_node_idx].m_backChildIdx = node_idx;
		BuildBspSubTree(node_idx, back_idx_list, current_node_idx);
	}
}


void BSPTree::Clear()
{
	int num_nodes = static_cast<int>(m_bspTree.size());
	for(int node_idx = 0; node_idx < num_nodes; ++node_idx)
	{
		delete m_bspTree[node_idx].m_mesh;
		m_bspTree[node_idx].m_mesh = nullptr;
	}

	m_bspTree.clear();
	m_sceneSegments.clear();
}


PointType BSPTree::ClassifyPoint(const Vec2& point, const Plane2& plane)
{
	const Vec2 point_on_plane = plane.PointOnPlane();
	const Vec2 dir = point_on_plane - point;
	const float result = DotProduct(dir, plane.m_normal);

	if (result < -0.001f)
	{
		return POINT_INFRONT;
	}
	if(result > 0.001f)
	{
		return POINT_BEHIND;
	}

	return POINT_ONLINE;
}


SegmentType BSPTree::ClassifySegment(const Segment2& shape, const Plane2& plane)
{
	int num_points_behind = 0;
	int num_points_online = 0;
	int num_points_infront = 0;

	//we know the size of a segment will always be 2
	std::vector<Vec2> verts;
	verts.reserve(2);
	verts.push_back(shape.m_start);
	verts.push_back(shape.m_end);
	
	const int num_verts = static_cast<int>(verts.size());
	for(int vert_idx = 0; vert_idx < num_verts; ++vert_idx)
	{
		const PointType point_type = ClassifyPoint(verts[vert_idx], plane);
		switch(point_type)
		{
			case POINT_BEHIND:
			{
				num_points_behind++;
				break;
			}
			case POINT_INFRONT:
			{
				num_points_infront++;
				break;
			}
			case POINT_ONLINE:
			{
				num_points_behind++;
				num_points_online++;
				num_points_infront++;
				break;
			}
		}
	}

	if (num_points_behind == num_verts)
	{
		return SEGMENT_BEHIND;
	}

	if (num_points_infront == num_verts)
	{
		return SEGMENT_INFRONT;
	}

	if (num_points_online == num_verts)
	{
		return SEGMENT_INFRONT;
	}

	return SEGMENT_STRADDLING;
}


void BSPTree::SplitPolygon(const Segment2& shape, const Plane2& plane, int& out_back_shape_idx,
	int& out_front_shape_idx)
{
	// using ray vs plane
	// the ray will come from the segment, and we are certain that it will intersect
	

	PointType start_type = ClassifyPoint(shape.m_start, plane);
	PointType end_type = ClassifyPoint(shape.m_end, plane);

	//We have taken care of points on the line, so this segment must be straddling
	if(start_type == POINT_INFRONT)
	{
		const Vec2 dir = shape.m_end - shape.m_start;
		Ray2 ray(shape.m_start, dir);
		float t[2];
		Raycast(t, ray, plane);

		Vec2 intersection = ray.PointAtTime(t[0]);

		m_sceneSegments.emplace_back(shape.m_start, intersection);
		out_front_shape_idx = static_cast<int>(m_sceneSegments.size()) - 1;

		m_sceneSegments.emplace_back(intersection, shape.m_end);
		out_back_shape_idx = static_cast<int>(m_sceneSegments.size()) - 1;
	}
	else if(end_type == POINT_INFRONT)
	{
		const Vec2 dir = shape.m_start - shape.m_end;
		Ray2 ray(shape.m_end, dir);
		float t[2];
		Raycast(t, ray, plane);

		Vec2 intersection = ray.PointAtTime(t[0]);

		m_sceneSegments.emplace_back(intersection, shape.m_end);
		out_front_shape_idx = static_cast<int>(m_sceneSegments.size()) - 1;

		m_sceneSegments.emplace_back(shape.m_start, intersection);
		out_back_shape_idx = static_cast<int>(m_sceneSegments.size()) - 1;
	}

}


void BSPTree::WalkTreeInOrder(int current_node_idx)
{
	BSPNode current_node = m_bspTree[current_node_idx];

	
	if(current_node.m_backChildIdx != -1)
	{
		WalkTreeInOrder(current_node.m_backChildIdx);
	}

	//TODO: add convex hull from the parents

	
	if (current_node.m_frontChildIdx != -1)
	{
		WalkTreeInOrder(current_node.m_frontChildIdx);
	}
	
}

//walking tree in pre order
void BSPTree::AddRenderObjs(int current_node_idx)
{
	SetMesh(current_node_idx);

	BSPNode current_node = m_bspTree[current_node_idx];
	
	if (current_node.m_backChildIdx != -1)
	{
		AddRenderObjs(current_node.m_backChildIdx);
	}

	if (current_node.m_frontChildIdx != -1)
	{
		AddRenderObjs(current_node.m_frontChildIdx);
	}
}

//walking tree in post order
void BSPTree::SettingSpaceTypes(int current_node_idx)
{
	BSPNode& current_node = m_bspTree[current_node_idx];


	if (current_node.m_backChildIdx != -1)
	{
		SettingSpaceTypes(current_node.m_backChildIdx);
	}

	if (current_node.m_frontChildIdx != -1)
	{
		SettingSpaceTypes(current_node.m_frontChildIdx);
	}

	SetType(current_node_idx);
}


// walking tree in post order
void BSPTree::RenderLowestNodes(int current_node_idx) const
{
	BSPNode current_node = m_bspTree[current_node_idx];


	if (current_node.m_backChildIdx != -1)
	{
		RenderLowestNodes(current_node.m_backChildIdx);
	}

	if (current_node.m_frontChildIdx != -1)
	{
		RenderLowestNodes(current_node.m_frontChildIdx);
	}

	RenderNode(current_node_idx);
}

void BSPTree::SetMesh(int current_node_idx)
{
	BSPNode& current_node = m_bspTree[current_node_idx];
	int hue_step = 50;
	float line_thickness = 0.32f;
	
	std::vector<int> ancestry_idx = std::vector<int>();
	int parent_idx = current_node.m_parentIdx;
	
	while(parent_idx != -1)
	{
		ancestry_idx.push_back(parent_idx);
		parent_idx = m_bspTree[parent_idx].m_parentIdx;
	}

	int num_ancestors = static_cast<int>(ancestry_idx.size());
	
	if(num_ancestors >= 2)
	{
		if (current_node.m_isLeaf)
		{
// 			std::vector<Plane2> convex_hull = std::vector<Plane2>();
// 			convex_hull.reserve(num_ancestors);
// 
// 			for(int plane_idx = 0; plane_idx < num_ancestors; ++plane_idx)
// 			{
// 				const int ancestor_idx = ancestry_idx[plane_idx];
// 				convex_hull.push_back(m_bspTree[ancestor_idx].m_split);
// 			}
// 
// 			ConvexPolygon2D poly(convex_hull);
// 			CPUMesh poly_mesh;
// 			Rgba color(static_cast<float>(hue_step * num_ancestors));
// 			color.a = 0.5f;
// 			int triangle_set = num_ancestors - 2;
// 
// 			CPUMesh convex_mesh;
// 			for (int convex_itr = 0; convex_itr < triangle_set; ++convex_itr)
// 			{
// 				CpuMeshAddTriangle(
// 					&convex_mesh,
// 					true,
// 					poly.m_points[0],
// 					poly.m_points[convex_itr + 1],
// 					poly.m_points[convex_itr + 2],
// 					color,
// 					convex_itr);
// 			}
// 
// 			current_node.m_mesh = new GPUMesh(g_theRenderer);
// 			current_node.m_mesh->CreateFromCPUMesh<Vertex_PCU>(convex_mesh); // we won't be updated this;
		}
		else
		{
			Plane2 split_plane = m_bspTree[current_node_idx].m_split;
			Plane2 parent_plan = m_bspTree[ancestry_idx[0]].m_split;

			Vec2 start = Vec2::ZERO;
			split_plane.Intersection(start, parent_plan);

			Vec2 dir = m_bspTree[current_node_idx].m_segment.GetCenter() - start;
			dir.Normalize();
			
			Ray2 ray(start, dir);
			float smallest_t = INFINITY;

			for(int anc_idx = 1; anc_idx < num_ancestors; ++anc_idx)
			{
				int node_idx = ancestry_idx[anc_idx];
				Plane2 plane = m_bspTree[node_idx].m_split;
				float t[2];

				Raycast(t, ray, plane, false);

				if(t[0] < smallest_t)
				{
					smallest_t = t[0];
				}
			}

			Vec2 end;
			if(smallest_t != INFINITY)
			{
				end = ray.PointAtTime(smallest_t);
			}
			else
			{
				end = start + dir * 200.0f;
			}

			CPUMesh ray_mesh;
			Rgba color(static_cast<float>(hue_step * num_ancestors));

			CpuMeshAddLine(&ray_mesh, false, start, end, line_thickness, color);
			current_node.m_mesh = new GPUMesh(g_theRenderer);
			current_node.m_mesh->CreateFromCPUMesh<Vertex_PCU>(ray_mesh);
		}
	}
	else if(num_ancestors == 1)
	{
		if (current_node.m_isLeaf)
		{
			//TODO make convex hull
		}
		else
		{
			Plane2 split_plane = m_bspTree[current_node_idx].m_split;
			Plane2 parent_plan = m_bspTree[ancestry_idx[0]].m_split;

			Vec2 start = Vec2::ZERO;
			split_plane.Intersection(start, parent_plan);

			
			Vec2 dir = m_bspTree[current_node_idx].m_segment.GetCenter() - start;
			dir.Normalize();
			
			Vec2 end = start + dir * 200.0f;

			CPUMesh ray_mesh;
			Rgba color(static_cast<float>(hue_step * num_ancestors));
			
			CpuMeshAddLine(&ray_mesh, false, start, end, line_thickness, color);
			current_node.m_mesh = new GPUMesh(g_theRenderer);
			current_node.m_mesh->CreateFromCPUMesh<Vertex_PCU>(ray_mesh);
		}
	}
	else
	{
		//root node
		if(current_node.m_isLeaf)
		{
// 			CPUMesh box_mesh;
// 
// 			Rgba color = Rgba::GetRandomColor();
// 			
// 			CpuMeshAddQuad(&box_mesh, false, color, WORLD_BOUNDS);
// 			current_node.m_mesh = new GPUMesh(g_theRenderer);
// 			current_node.m_mesh->CreateFromCPUMesh<Vertex_PCU>(box_mesh);
		}
		else
		{
			CPUMesh plane_mesh;

			Vec2 dir = current_node.m_split.GetDirection();
			Vec2 start = current_node.m_split.PointOnPlane() - dir * 100.0f;
			Vec2 end = current_node.m_split.PointOnPlane() + dir * 100.0f;

			Rgba color(static_cast<float>(hue_step * num_ancestors));
			
			CpuMeshAddLine(&plane_mesh, start, end, line_thickness, color);
			current_node.m_mesh = new GPUMesh(g_theRenderer);
			current_node.m_mesh->CreateFromCPUMesh<Vertex_PCU>(plane_mesh);
		}
	}
}

void BSPTree::SetType(int current_node_idx)
{
	BSPNode& current_node = m_bspTree[current_node_idx];
	if(!current_node.m_isLeaf)
	{
		int back_child_idx = current_node.m_backChildIdx;
		int front_child_idx = current_node.m_frontChildIdx;

		SpaceType back_type = m_bspTree[back_child_idx].m_spaceType;
		SpaceType front_type = m_bspTree[front_child_idx].m_spaceType;

		if(back_type == SPACE_SOLID && front_type == SPACE_SOLID)
		{
			current_node.m_spaceType = SPACE_SOLID;
		}
		else if(back_type == SPACE_FREE && front_type == SPACE_FREE)
		{
			current_node.m_spaceType = SPACE_FREE;
		}
		else
		{
			current_node.m_spaceType = SPACE_MIXED;
		}
	}
}


void BSPTree::RenderNode(int current_node_idx) const
{
	if (m_bspTree[current_node_idx].m_mesh)
	{
		const Matrix44 model_matrix = Matrix44::IDENTITY;

		g_theRenderer->BindModelMatrix(model_matrix);
		g_theRenderer->BindMaterial(*m_material);
		g_theRenderer->DrawMesh(*m_bspTree[current_node_idx].m_mesh);
	}
}


bool BSPTree::CanSee(const Vec2& start, const Vec2& end, Vec2& out_end, int current_node_idx)
{
	float t_val;
	Vec2 intersection;
	bool is_open_space;
	BSPNode& current_node = m_bspTree[current_node_idx];

	//for either start or end
	if(current_node.m_isLeaf)
	{
		//nothing alters our path
		is_open_space = current_node.m_spaceType == SPACE_FREE;
		return is_open_space;
	}

	
	const PointType start_type = ClassifyPoint(start, current_node.m_split);
	const PointType end_type = ClassifyPoint(end, current_node.m_split);

	// assume on  the line means we are in front of the split
	if(start_type == POINT_ONLINE && end_type == POINT_ONLINE)
	{
		is_open_space = CanSee(start, end, out_end, current_node.m_frontChildIdx);
		return is_open_space;
	}

	// in the case of straddling
	if(start_type == POINT_INFRONT && end_type == POINT_BEHIND)
	{
		GetIntersection(start, end, current_node.m_split, intersection, t_val);
		const bool can_see_front = CanSee(start, intersection, out_end, current_node.m_frontChildIdx);
		const bool can_see_behind = CanSee(intersection, end, out_end, current_node.m_backChildIdx);

		if(!can_see_behind)
		{
			out_end = intersection;
		}
		
		is_open_space = can_see_front && can_see_behind;
		return is_open_space;
	}

	// or other way of straddling
	if(start_type == POINT_BEHIND && end_type == POINT_INFRONT)
	{
		GetIntersection(start, end, current_node.m_split, intersection, t_val);
		const bool can_see_front = CanSee(intersection, end, out_end, current_node.m_frontChildIdx);
		const bool can_see_back = CanSee(start, intersection, out_end, current_node.m_backChildIdx);

		if (!can_see_front)
		{
			out_end = intersection;
		}
		
		is_open_space = can_see_front && can_see_back;
		return is_open_space;
	}

	// Lastly, either point is on top of the plane
	if(start_type == POINT_INFRONT || end_type == POINT_INFRONT)
	{
		is_open_space = CanSee(start, end, out_end, current_node.m_frontChildIdx);
		return is_open_space;
	}

	// last check
	is_open_space = CanSee(start, end, out_end, current_node.m_backChildIdx);
	out_end = intersection;
	return is_open_space;
}


int BSPTree::SelectBestSplitterIndex(const std::vector<int>& seg_index_list, int parent_idx)
{
	int	max_segments = static_cast<int>(seg_index_list.size());
	int	select_segment_idx = -1;

	switch(m_heuristicType)
	{
		//todo add a third heuristic that focuses on the three aspects
		// 1. How many segments can you align with +++ (if you pick this segment, how many other segments are exactly the same)
		// 2. How many splits do you make -- (what I have earlier, how many points from a segments do you split up)
		// 3. How unbalanced are you - (same as earlier, how many full segments do you "evenly" split)

		case HEURISTIC_SCORE:
		{
			float	best_score = INFINITY;

			for (int split_idx = 0; split_idx < max_segments; ++split_idx)
			{
				float score, splits, back_faces, front_faces, dot;
				splits = back_faces = front_faces = dot = 0.0f;


				const int scene_seg_idx = seg_index_list[split_idx];
				const Segment2 seg(m_sceneSegments[scene_seg_idx]);
				const Plane2 split(seg.m_start, seg.m_end);

				for (int segment_idx = 0; segment_idx < max_segments; ++segment_idx)
				{
					if (split_idx != segment_idx)
					{
						const Segment2 test_seg(m_sceneSegments[segment_idx]);
						const SegmentType type = ClassifySegment(test_seg, split);

						switch (type)
						{
						case SEGMENT_BEHIND:
						{
							++back_faces;
							break;
						}
						case SEGMENT_INFRONT:
						{
							++front_faces;
							break;
						}
						case SEGMENT_STRADDLING:
						{
							++splits;
							break;
						}
						}
					}


					if (parent_idx != -1)
					{
						const Vec2 split_normal = split.m_normal;
						const Vec2 parent_normal = m_bspTree[parent_idx].m_split.m_normal;
						const float similarity = DotProduct(split_normal, parent_normal);
						dot = similarity;
					}

					score = Abs(front_faces - back_faces) * 7.0f +
						Abs(dot) * 19.0f +
						splits * 31.0f;

					if (score < best_score)
					{
						best_score = score;
						select_segment_idx = split_idx;
					}
				}
				break;
			}
		}

		case HEURISTIC_RANDOM:
		default:
		{
			int idx = g_randomNumberGenerator.GetRandomIntInRange(0, max_segments - 1);
			select_segment_idx = idx;
			break;
		}
	}

	return select_segment_idx;
}


bool BSPTree::GetIntersection(const Vec2& start, const Vec2& end, const Plane2& plane, Vec2& intersection, float& t)
{
	//TODO: go over math
	Vec2 dir = end - start;
	const float line_length = DotProduct(dir, plane.m_normal);
	if (IsZero(line_length))
	{
		return false;
	}

	Vec2 line = plane.PointOnPlane() - start;
	float dist_from_point = DotProduct(line, plane.m_normal);
	t = dist_from_point / line_length;


	if (t < 0.0f)
	{
		return false;
	}
	else
	{
		if (t > 1.0f)
		{
			return false;
		}
	}

	intersection = start + dir * t;
	return true;
}