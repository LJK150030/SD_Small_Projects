#pragma once
#include "Engine/Math/Plane2.hpp"
#include "Engine/Math/Segment2.hpp"

#include "Game/ConvexShape.hpp"

#include <vector>

enum PointType
{
	POINT_BEHIND,
	POINT_INFRONT,
	POINT_ONLINE,
};


enum SegmentType
{
	SEGMENT_BEHIND,
	SEGMENT_INFRONT,
	SEGMENT_STRADDLING
};

enum BspHeuristic
{
	HEURISTIC_RANDOM,
	HEURISTIC_SCORE
};

struct BSPNode
{
	BSPNode();
	~BSPNode();

	Plane2 m_split;
	int m_backChildIdx = -1;
	int m_frontChildIdx = -1;
	bool m_isLeaf = false;
	bool m_isSolid = false;
};

class BSPTree
{
public:
	BSPTree();
	~BSPTree();

	void BuildBspTree(BspHeuristic plane_selection, const std::vector<ConvexShape2D*>& geometry_list);

	void Render() const;
	
	void Clear();
	
private:
	void		BuildBspSubTree(int current_node_idx, const std::vector<int>& seg_index_list);
	PointType	ClassifyPoint(const Vec2& point, const Plane2& plane);
	SegmentType	ClassifySegment(const Segment2& shape, const Plane2& plane);
	void		SplitPolygon(const Segment2& shape, const Plane2& plane, int& out_back_shape_idx, int& out_front_shape_idx);
	int			SelectBestSplitterIndex(const std::vector<int>& seg_index_list);

	
private:
	std::vector<BSPNode> m_bspTree;
	std::vector<Segment2> m_sceneSegments;
	BspHeuristic m_heuristicType = HEURISTIC_RANDOM;
};

