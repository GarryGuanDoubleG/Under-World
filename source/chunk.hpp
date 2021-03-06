#pragma once

enum Side : Uint8
{
	TOP = 0,
	BOTTOM,
	LEFT,
	RIGHT,
	BACK,
	FRONT,
	FRONT_RIGHT
};

const glm::ivec3 g_neighbors[6] =
{
	glm::ivec3(-1,0,0),
	glm::ivec3(1,0,0),
	glm::ivec3(0,-1,0),
	glm::ivec3(0, 1,0),
	glm::ivec3(0,0,-1),
	glm::ivec3(0,0, 1)
};
//for selection func
typedef std::function<bool(const glm::ivec3&, const glm::ivec3&)> FilterNodesFunc;


class Chunk
{
	Octree * m_root;

	int m_flag;
	Density::DensityType m_densityType;

	int m_voxelSize;
	float m_invVoxelSize; // inverse voxel size (1 / voxelSize)
	glm::vec3 m_chunkSize;
	glm::vec3 m_position;

	//proc mesh
	GLuint m_vao, m_vbo, m_ebo;
public:
	bool m_init;
	float heightMapTime, materialTime, findVoxelTime, hermiteTime, meshTime;
	Chunk *m_neighbors[7];
	glm::ivec3 m_chunkIndex;

	unordered_map<glm::vec3, EdgeInfo> m_hermiteMap;
	vector<int> m_materialIndices;
	unordered_map<glm::vec3, Octree*> m_nodeMap;

	vector<VoxelVertex> m_vertices;
	vector<GLuint> m_triIndices;
	vector<GLboolean> m_flipVerts;
public:
	Chunk();
	~Chunk();

	bool Init(glm::ivec3 chunkIndices, glm::vec3 chunkSize, int voxelSize);
	bool ClearBufferedData();
	glm::vec3 GetPosition();

	void AssignNeighbor(Chunk * chunk, Side side);

	void FindActiveVoxels();

	void GenerateMesh();
	void GetNodesInRange(const Chunk * chunk, const glm::ivec3 minrange, const glm::ivec3 maxrange, vector<Octree*> &outputNodes);
	vector<Octree*> FindSeamNodes();
	void GenerateSeam();
	vector<Octree*> findNodes(FilterNodesFunc filterFunc);
	void GenerateMesh(vector<float>& heightmap);
	void GenerateHermiteField();//uses 3D noise for size^3 grid
	void GenerateHermiteHeightMap2D(vector<float> &heightmap);//uses 2d heightmap with O(size^2)
	void BindMesh();
	
	void Render();
};