#include "Game/BSPTree.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Ray2.hpp"

BSPNode::BSPNode() {}
BSPNode::~BSPNode() {}


BSPTree::BSPTree()
{
	m_bspTree = std::vector<BSPNode>();
	m_sceneSegments = std::vector<Segment2>();
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
	m_sceneSegments.reserve(num_geometry * 5);

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
	
	BuildBspSubTree(0, seg_indexes);
}

void BSPTree::Render() const
{
}


void BSPTree::BuildBspSubTree(int current_node_idx, const std::vector<int>& seg_index_list)
{
	std::vector<int> front_idx_list = std::vector<int>();
	std::vector<int> back_idx_list = std::vector<int>();

	const int best_split_idx = SelectBestSplitterIndex(seg_index_list);
	const int best_world_split_idx = seg_index_list[best_split_idx];
	const Segment2 best_split = m_sceneSegments[best_world_split_idx];
	const Plane2 split(best_split.m_start, best_split.m_end);
	m_bspTree[current_node_idx].m_split = split;

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
		m_bspTree[leaf_idx].m_isLeaf = true;
		m_bspTree[leaf_idx].m_isSolid = false;

		m_bspTree[current_node_idx].m_frontChildIdx = leaf_idx;
	}
	else
	{
		m_bspTree.emplace_back();

		const int node_idx = static_cast<int>(m_bspTree.size()) - 1;
		m_bspTree[node_idx].m_isLeaf = false;
		m_bspTree[node_idx].m_isSolid = false;

		m_bspTree[current_node_idx].m_frontChildIdx = node_idx;
		BuildBspSubTree(node_idx, front_idx_list);
	}

	const int back_list_size = back_idx_list.size();
	if (back_list_size == 0)
	{
		m_bspTree.emplace_back();

		const int leaf_idx = static_cast<int>(m_bspTree.size()) - 1;
		m_bspTree[leaf_idx].m_isLeaf = true;
		m_bspTree[leaf_idx].m_isSolid = true;

		m_bspTree[current_node_idx].m_backChildIdx = leaf_idx;
	}
	else
	{
		m_bspTree.emplace_back();

		const int node_idx = static_cast<int>(m_bspTree.size()) - 1;
		m_bspTree[node_idx].m_isLeaf = false;
		m_bspTree[node_idx].m_isSolid = false;

		m_bspTree[current_node_idx].m_backChildIdx = node_idx;
		BuildBspSubTree(node_idx, back_idx_list);
	}
}


void BSPTree::Clear()
{
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


int BSPTree::SelectBestSplitterIndex(const std::vector<int>& seg_index_list)
{
	int	max_segments = static_cast<int>(seg_index_list.size());
	int	select_segment_idx = -1;

	
	switch(m_heuristicType)
	{
		case HEURISTIC_SCORE:
			{
				int	best_score = INT_MAX;
				int score, splits, back_faces, front_faces;
				score = splits = back_faces = front_faces = 0;
				
				for(int split_idx = 0; split_idx < max_segments; ++split_idx)
				{
					const int scene_seg_idx = seg_index_list[split_idx];
					const Segment2 seg(m_sceneSegments[scene_seg_idx]);
					const Plane2 split(seg.m_start, seg.m_end);

					for (int segment_idx = 0; segment_idx < max_segments; ++segment_idx)
					{
						if(split_idx != segment_idx)
						{
							const SegmentType type = ClassifySegment(seg, split);

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
					}

					score = Abs(front_faces - back_faces)*2 + (splits * 8);

					if(score < best_score)
					{
						best_score = score;
						select_segment_idx = split_idx;
					}
				}
				break;
			}
			
		case HEURISTIC_RANDOM:
		default:
			{
				int idx = g_randomNumberGenerator.GetRandomIntInRange(0, max_segments-1);
				select_segment_idx = idx;
				break;
			}
	}

	return select_segment_idx;
}

