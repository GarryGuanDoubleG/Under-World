#pragma once
#include "qef.hpp"
#define GETINDEXXYZ(x,y,z) ((4 * x) + (2 * y) + z)
#define GETINDEXXZ(x,z) ((2 * x) + z)

typedef struct VoxelVertex_S
{
	VoxelVertex_S() {};
	VoxelVertex_S(glm::vec3 pos, glm::vec3 norm) : position(pos), normal(norm) {};
	VoxelVertex_S(glm::vec3 pos, glm::vec3 norm, int type) : position(pos), normal(norm), textureID(type) {};

	glm::vec3 position;
	glm::vec3 normal;
	int textureID;
}VoxelVertex;

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


class Octree
{
	Uint8 m_flag;
	Uint8 m_childMask;

	int m_size;
	glm::vec3 m_minPos;

	Octree * m_children[8];
	OctreeDrawInfo *m_drawInfo;
public:
	Octree();

	void InitNode(glm::vec3 minPos, int size);
	void DestroyNode();

	glm::vec3 FindZeroSurface(const glm::vec3 p0, const glm::vec3 p1);
	glm::vec3 CalculateNormals(const glm::vec3 pos);
	Octree * BuildTree(glm::vec3 chunkPos, glm::vec3 chunkSize, float voxelSize, const vector<float> &noiseValues);
	Octree * BottomUpTreeGen(vector<Octree*> &leafNodes, const glm::vec3 &chunkPos);
	bool BuildLeaf();

	void GenerateVertexIndices(vector<VoxelVertex>& voxelVerts);
	void ContourProcessEdge(std::vector<GLuint>& m_tri_indices, Octree * node[4], int dir);
	void ContourEdgeProc(std::vector<GLuint>& m_tri_indices, Octree * node[4], int dir);
	void ContourFaceProc(std::vector<GLuint>& m_tri_indices, Octree * node[2], int dir);
	void ContourCellProc(std::vector<GLuint>& m_tri_indices);
};