#pragma once
#include "qef.hpp"

#define OCTREE_ACTIVE 1
#define OCTREE_INUSE 2
#define OCTREE_LEAF 4
#define OCTREE_INNER 8
#define OCTREE_PSUEDO 16

#define GETINDEXXYZ(x,y,z) ((4 * x) + (2 * y) + z)
#define GETINDEXXZ(x,z) ((2 * x) + z)
#define GETINDEXCHUNK(chunkSize, a, b, c) ((chunkSize.x * chunkSize.y * a) + (chunkSize.y * b) + c)


const glm::vec3 CHILD_MIN_OFFSETS[] =
{
	// needs to match the vertunordered_map from Dual Contouring impl
	glm::vec3(0, 0, 0),
	glm::vec3(0, 0, 1),
	glm::vec3(0, 1, 0),
	glm::vec3(0, 1, 1),
	glm::vec3(1, 0, 0),
	glm::vec3(1, 0, 1),
	glm::vec3(1, 1, 0),
	glm::vec3(1, 1, 1),
};

typedef struct VoxelVertex_S
{
	VoxelVertex_S() {};
	VoxelVertex_S(glm::vec3 pos, glm::vec3 norm) : position(pos), normal(norm) {};
	VoxelVertex_S(glm::vec3 pos, glm::vec3 norm, int type) : position(pos), normal(norm), textureID(type) {};

	glm::vec3 position;
	glm::vec3 normal;
	int textureID;
}VoxelVertex;

struct EdgeInfo
{
	EdgeInfo() {};
	EdgeInfo(glm::vec3 p, glm::vec3 n) :pos(p), normal(n) {};
	glm::vec3 pos;
	glm::vec3 normal;
};

struct OctreeDrawInfo
{
	OctreeDrawInfo()
		: index(-1)
		, corners(0)
	{
	}

	int    index;
	int    corners;
	GLint	type;
	glm::vec3   position;
	glm::vec3   averageNormal;
	svd::QefData	qef;
};

class Octree;

glm::vec3 FindIntersection(Density::DensityType type, const glm::vec3 &p0, const glm::vec3 &p1);
glm::vec3 CalculateNormals(Density::DensityType type, const glm::vec3 &pos);


void FindEdgeCrossing(Octree *node, const unordered_map<glm::vec3, EdgeInfo> &hermite_map);

Octree * BottomUpTreeGen(const unordered_map<glm::vec3, Octree *> &unordered_map, const glm::vec3 &chunkPos);

class Octree
{
public:
	int m_index;
	int m_corners;
	glm::vec3 m_position;
	glm::vec3 m_normal;
	GLint m_type;

	Octree * m_children[8];

	Uint8 m_flag;
	Uint8 m_childMask;

	int m_size;
	glm::vec3 m_minPos;
public:
	Octree();

	void InitNode(glm::vec3 minPos, int size, int corners);
	void DestroyNode();

	void GenerateVertexIndices(vector<VoxelVertex>& voxelVerts);
	void ContourProcessEdge(std::vector<GLuint>& m_tri_indices, Octree * node[4], int dir);
	void ContourEdgeProc(std::vector<GLuint>& m_tri_indices, Octree * node[4], int dir);
	void ContourFaceProc(std::vector<GLuint>& m_tri_indices, Octree * node[2], int dir);
	void ContourCellProc(std::vector<GLuint>& m_tri_indices);
};