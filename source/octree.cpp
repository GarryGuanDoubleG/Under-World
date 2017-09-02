#include "game.hpp"

#define OCTREE_ACTIVE 1
#define OCTREE_INUSE 2
#define OCTREE_LEAF 4
#define OCTREE_INNER 8
#define OCTREE_PSUEDO 16

#define MATERIAL_SOLID 1
#define MATERIAL_AIR 0

const float QEF_ERROR = 1e-6f;
const int QEF_SWEEPS = 4;

const glm::vec3 CHILD_MIN_OFFSETS[] =
{
	// needs to match the vertMap from Dual Contouring impl
	glm::vec3(0, 0, 0),
	glm::vec3(0, 0, 1),
	glm::vec3(0, 1, 0),
	glm::vec3(0, 1, 1),
	glm::vec3(1, 0, 0),
	glm::vec3(1, 0, 1),
	glm::vec3(1, 1, 0),
	glm::vec3(1, 1, 1),
};

// ----------------------------------------------------------------------------
// data from the original DC impl, drives the contouring process
const int edgevmap[12][2] =
{
	{ 0,4 },{ 1,5 },{ 2,6 },{ 3,7 },	// x-axis 
	{ 0,2 },{ 1,3 },{ 4,6 },{ 5,7 },	// y-axis
	{ 0,1 },{ 2,3 },{ 4,5 },{ 6,7 }		// z-axis
};
const int edgemask[3] = { 5, 3, 6 };
const int vertMap[8][3] =
{
	{ 0,0,0 },
	{ 0,0,1 },
	{ 0,1,0 },
	{ 0,1,1 },
	{ 1,0,0 },
	{ 1,0,1 },
	{ 1,1,0 },
	{ 1,1,1 }
};
const int faceMap[6][4] = { { 4, 8, 5, 9 },{ 6, 10, 7, 11 },{ 0, 8, 1, 10 },{ 2, 9, 3, 11 },{ 0, 4, 2, 6 },{ 1, 5, 3, 7 } };
const int cellProcFaceMask[12][3] = { { 0,4,0 },{ 1,5,0 },{ 2,6,0 },{ 3,7,0 },{ 0,2,1 },{ 4,6,1 },{ 1,3,1 },{ 5,7,1 },{ 0,1,2 },{ 2,3,2 },{ 4,5,2 },{ 6,7,2 } };
const int cellProcEdgeMask[6][5] = { { 0,1,2,3,0 },{ 4,5,6,7,0 },{ 0,4,1,5,1 },{ 2,6,3,7,1 },{ 0,2,4,6,2 },{ 1,3,5,7,2 } };
const int faceProcFaceMask[3][4][3] = {
	{ { 4,0,0 },{ 5,1,0 },{ 6,2,0 },{ 7,3,0 } },
	{ { 2,0,1 },{ 6,4,1 },{ 3,1,1 },{ 7,5,1 } },
	{ { 1,0,2 },{ 3,2,2 },{ 5,4,2 },{ 7,6,2 } }
};
const int faceProcEdgeMask[3][4][6] = {
	{ { 1,4,0,5,1,1 },{ 1,6,2,7,3,1 },{ 0,4,6,0,2,2 },{ 0,5,7,1,3,2 } },
	{ { 0,2,3,0,1,0 },{ 0,6,7,4,5,0 },{ 1,2,0,6,4,2 },{ 1,3,1,7,5,2 } },
	{ { 1,1,0,3,2,0 },{ 1,5,4,7,6,0 },{ 0,1,5,0,4,1 },{ 0,3,7,2,6,1 } }
};
const int edgeProcEdgeMask[3][2][5] = {
	{ { 3,2,1,0,0 },{ 7,6,5,4,0 } },
	{ { 5,1,4,0,1 },{ 7,3,6,2,1 } },
	{ { 6,4,2,0,2 },{ 7,5,3,1,2 } },
};
const int g_processEdgeMask[3][4] = { { 3,2,1,0 },{ 7,5,6,4 },{ 11,10,9,8 } };

Octree::Octree() : m_flag (0)
{
}

void Octree::InitNode(glm::vec3 minPos, int size)
{
	m_minPos = minPos;
	m_size = size;

	m_flag = OCTREE_INUSE;
}

void Octree::DestroyNode()
{
//	if (m_flag & OCTREE_INNER)
//	{
//		for (int i = 0; i < 8; i++)
//		{
////			Octree *child = (m_children + sizeof(Octree) * i);
//			child->DestroyNode();
//		}
//	}

	m_flag = m_flag >> 8;
}

glm::vec3 Octree::FindZeroSurface(const glm::vec3 p0, const glm::vec3 p1)
{
	float minValue = 100000.f;
	float t = 0.f;
	float currentT = 0.f;
	const int steps = 8;
	const float increment = 1.f / (float)steps;
	while (currentT <= 1.f)
	{
		const glm::vec3 p = p0 + ((p1 - p0) * currentT);
		const float density = glm::abs(Density_Func(p));
		if (density < minValue)
		{
			minValue = density;
			t = currentT;
		}

		currentT += increment;
	}

	return p0 + ((p1 - p0) * t);
}

glm::vec3 Octree::CalculateNormals(const glm::vec3 pos)
{
	const float H = 0.1f;

	if (pos.y <= 1.0f)
	{
		return glm::vec3(0.0f, 1.0f, 0.0f);
	}

	//finite difference method to get partial derivatives
	const float dx = Density_Func(pos + glm::vec3(H, 0.f, 0.f)) - Density_Func(pos - glm::vec3(H, 0.f, 0.f));
	const float dy = Density_Func(pos + glm::vec3(0.f, H, 0.f)) - Density_Func(pos - glm::vec3(0.f, H, 0.f));
	const float dz = Density_Func(pos + glm::vec3(0.f, 0.f, H)) - Density_Func(pos - glm::vec3(0.f, 0.f, H));

	return glm::normalize(glm::vec3(dx, dy, dz));
}

Octree * Octree::BuildTree(glm::vec3 chunkPos, glm::vec3 chunkSize, float voxelSize, const vector<float> &noiseValues) 
{
	vector<Octree *> tree;

	for (int x = 0; x < chunkSize.x; x++)
	for (int z = 0; z < chunkSize.z; z++)
	{
		//floor the noise value
		float height = (int)noiseValues[GETINDEXXZ(x, z)];
		if (height < chunkPos.y || height > chunkPos.y + chunkSize.y * voxelSize) 
			continue;

		float y = (int)height / (int)voxelSize;
		 
		//world voxel pos = chunk pos + local pos
		Octree *leaf = new Octree();
		leaf->InitNode(glm::vec3(chunkPos.x, chunkPos.y, chunkPos.z) + glm::vec3(x, y, z) * voxelSize, voxelSize);

		if (leaf->BuildLeaf())
			tree.push_back(leaf);
		else
		{
			leaf->DestroyNode();
			delete leaf;
		}
	}

	return BottomUpTreeGen(tree, chunkPos);
}

Octree * Octree::BottomUpTreeGen(vector<Octree *> &tree, const glm::vec3 &chunkPos)
{
	if (tree.size() < 1) return nullptr;

	while (tree.size() > 1)
	{
		vector<Octree *> parents;

		for (vector<Octree*>::iterator it = tree.begin(); it < tree.end(); ++it)
		{
			Octree *currNode = *it;
			bool foundParent = false;
			int parentSize = currNode->m_size << 1; //twice the size

			glm::ivec3 currPos = currNode->m_minPos;
			glm::ivec3 floorChunkPos = chunkPos;
			glm::vec3 parentPos = currPos - ((currPos - floorChunkPos) % parentSize);

			int x = currPos.x >= parentPos.x && currPos.x < parentPos.x + currNode->m_size ? 0 : 1;
			int y = currPos.y >= parentPos.y && currPos.y < parentPos.y + currNode->m_size ? 0 : 1;
			int z = currPos.z >= parentPos.z && currPos.z < parentPos.z + currNode->m_size ? 0 : 1;

			int index = 4 * x + 2 * y + z;

			for (vector<Octree *>::iterator it = parents.begin(); it != parents.end(); it++)
			{
				Octree * parent = *it;
				if (parent->m_minPos == parentPos)
				{
					foundParent = true;
					parent->m_children[index] = currNode;
					parent->m_childMask |= 1 << index;
					break;
				}
			}
			if (!foundParent)
			{
				Octree *parent = new Octree();
				parent->InitNode(parentPos, parentSize);
				parent->m_flag |= OCTREE_ACTIVE | OCTREE_INNER;
				parent->m_childMask |= 1 << index;
				parent->m_children[index] = currNode;


				parents.push_back(parent);
			}
		}

		tree = parents;
	}

	return tree[0];
}

bool Octree::BuildLeaf()
{
	int corners = 0;

	for (int i = 0; i < 8; i++)
	{
		glm::ivec3 cornerPos = m_minPos + CHILD_MIN_OFFSETS[i] * (float)m_size;
		const float density = Density_Func(glm::vec3(cornerPos));
		const int material = density < 0.f ? MATERIAL_SOLID : MATERIAL_AIR;
		corners |= (material << i);
	}

	if (corners == 0 || corners == 255)
	{
		// voxel is full inside or outside the volume
		//m_type = Node_Leaf;
		m_flag &= ~(OCTREE_ACTIVE);

		return false;
	}

	//build leaf node if node contains isosurface
	const int MAX_CROSSINGS = 6;
	int edgeCount = 0;
	glm::vec3 averageNormal(0.f);
	svd::QefSolver qef;

	for (int i = 0; i < 12 && edgeCount < MAX_CROSSINGS; i++)
	{
		const int c1 = edgevmap[i][0];
		const int c2 = edgevmap[i][1];

		const int m1 = (corners >> c1) & 1;
		const int m2 = (corners >> c2) & 1;

		if ((m1 == MATERIAL_AIR && m2 == MATERIAL_AIR) ||
			(m1 == MATERIAL_SOLID && m2 == MATERIAL_SOLID))
		{
			continue; //no change in sign density
		}

		const glm::vec3 p1 = glm::vec3(m_minPos + CHILD_MIN_OFFSETS[c1] * (float)m_size);
		const glm::vec3 p2 = glm::vec3(m_minPos + CHILD_MIN_OFFSETS[c2] * (float)m_size);

		const glm::vec3 p = FindZeroSurface(p1, p2);
		const glm::vec3 n = CalculateNormals(p);

		qef.add(p.x, p.y, p.z, n.x, n.y, n.z);

		averageNormal += n;

		edgeCount++;
	}

	svd::Vec3 qefPosition;
	qef.solve(qefPosition, QEF_ERROR, QEF_SWEEPS, QEF_ERROR);

	m_drawInfo = new OctreeDrawInfo;
	m_drawInfo->position = glm::vec3(qefPosition.x, qefPosition.y, qefPosition.z);
	m_drawInfo->qef = qef.getData();

	const glm::vec3 min = glm::vec3(m_minPos);
	const glm::vec3 max = glm::vec3(m_minPos + glm::vec3(m_size));

	if (m_drawInfo->position.x < min.x || m_drawInfo->position.x > max.x ||
		m_drawInfo->position.y < min.y || m_drawInfo->position.y > max.y ||
		m_drawInfo->position.z < min.z || m_drawInfo->position.z > max.z)
	{
		const auto& mp = qef.getMassPoint();
		m_drawInfo->position = glm::vec3(mp.x, mp.y, mp.z);
	}

	m_drawInfo->averageNormal = glm::normalize(averageNormal / (float)edgeCount);
	m_drawInfo->corners = corners;
	m_drawInfo->type = 0;

	m_flag |= OCTREE_ACTIVE;
	m_flag |= OCTREE_LEAF;

	return true;
}

void Octree::GenerateVertexIndices(vector<VoxelVertex> &voxelVerts)
{
	if (~m_flag & OCTREE_ACTIVE)
		return;

	if (~m_flag & OCTREE_LEAF)
	{
		for (int i = 0; i < 8; i++)
		{
			if (m_childMask & 1 << i)
			{
				m_children[i]->GenerateVertexIndices(voxelVerts);
			}
		}
	}

	if (~m_flag & OCTREE_INNER)
	{
		if (!m_drawInfo)
		{
			printf("Error! Could not add vertex!\n");
			exit(EXIT_FAILURE);
		}

		m_drawInfo->index = voxelVerts.size();
		voxelVerts.push_back(VoxelVertex(m_drawInfo->position, m_drawInfo->averageNormal, m_drawInfo->type));
	}
}

void Octree::ContourProcessEdge(std::vector<GLuint> &m_tri_indices, Octree* node[4], int dir)
{
	int minSize = 1000000;		// arbitrary big number
	int minIndex = 0;
	int indices[4] = { -1, -1, -1, -1 };
	bool flip = false;
	bool signChange[4] = { false, false, false, false };

	for (int i = 0; i < 4; i++)
	{
		const int edge = g_processEdgeMask[dir][i];
		const int c1 = edgevmap[edge][0];
		const int c2 = edgevmap[edge][1];

		const int m1 = (node[i]->m_drawInfo->corners >> c1) & 1;
		const int m2 = (node[i]->m_drawInfo->corners >> c2) & 1;

		if (node[i]->m_size < minSize)
		{
			minSize = node[i]->m_size;
			minIndex = i;
			flip = m1 != MATERIAL_AIR;
		}


		indices[i] = node[i]->m_drawInfo->index;

		signChange[i] =
			(m1 == MATERIAL_AIR && m2 != MATERIAL_AIR) ||
			(m1 != MATERIAL_AIR && m2 == MATERIAL_AIR);
	}

	if (signChange[minIndex])
	{
		if (!flip)
		{
			m_tri_indices.push_back(indices[0]);
			m_tri_indices.push_back(indices[1]);
			m_tri_indices.push_back(indices[3]);

			m_tri_indices.push_back(indices[0]);
			m_tri_indices.push_back(indices[3]);
			m_tri_indices.push_back(indices[2]);
		}
		else
		{
			m_tri_indices.push_back(indices[0]);
			m_tri_indices.push_back(indices[3]);
			m_tri_indices.push_back(indices[1]);

			m_tri_indices.push_back(indices[0]);
			m_tri_indices.push_back(indices[2]);
			m_tri_indices.push_back(indices[3]);
		}
	}
}

void Octree::ContourEdgeProc(std::vector<GLuint> &m_tri_indices, Octree* node[4], int dir)
{
	if (!node[0] || !node[1] || !node[2] || !node[3])
	{
		return;
	}
	if ((~node[0]->m_flag & OCTREE_ACTIVE) || (~node[1]->m_flag & OCTREE_ACTIVE)
		|| (~node[2]->m_flag & OCTREE_ACTIVE) || (~node[3]->m_flag & OCTREE_ACTIVE))
	{
		return;
	}


	if (~node[0]->m_flag & OCTREE_INNER &&
		~node[1]->m_flag & OCTREE_INNER &&
		~node[2]->m_flag & OCTREE_INNER &&
		~node[3]->m_flag & OCTREE_INNER)
	{
		ContourProcessEdge(m_tri_indices, node, dir);
	}
	else
	{
		for (int i = 0; i < 2; i++)
		{
			Octree* edgeNodes[4];
			const int c[4] =
			{
				edgeProcEdgeMask[dir][i][0],
				edgeProcEdgeMask[dir][i][1],
				edgeProcEdgeMask[dir][i][2],
				edgeProcEdgeMask[dir][i][3],
			};

			for (int j = 0; j < 4; j++)
			{
				if (node[j]->m_flag & OCTREE_LEAF)
				{
					edgeNodes[j] = node[j];
				}
				else
				{
					edgeNodes[j] = node[j]->m_children[c[j]];
				}
			}

			ContourEdgeProc(m_tri_indices, edgeNodes, edgeProcEdgeMask[dir][i][4]);
		}
	}
}

void Octree::ContourFaceProc(std::vector<GLuint> &m_tri_indices, Octree* node[2], int dir)
{
	if (!node[0] || !node[1]) return;
	if ((~node[0]->m_flag & OCTREE_ACTIVE) || (~node[1]->m_flag & OCTREE_ACTIVE)) return;

	if (node[0]->m_flag & OCTREE_INNER || node[1]->m_flag & OCTREE_INNER)
	{
		for (int i = 0; i < 4; i++)
		{
			Octree* faceNodes[2];
			const int c[2] =
			{
				faceProcFaceMask[dir][i][0],
				faceProcFaceMask[dir][i][1],
			};

			for (int j = 0; j < 2; j++)
			{
				if (~node[j]->m_flag & OCTREE_INNER)
				{
					faceNodes[j] = node[j];
				}
				else
				{
					faceNodes[j] = node[j]->m_children[c[j]];
				}
			}

			ContourFaceProc(m_tri_indices, faceNodes, faceProcFaceMask[dir][i][2]);
		}

		const int orders[2][4] =
		{
			{ 0, 0, 1, 1 },
			{ 0, 1, 0, 1 },
		};
		for (int i = 0; i < 4; i++)
		{
			Octree* edgeNodes[4];
			const int c[4] =
			{
				faceProcEdgeMask[dir][i][1],
				faceProcEdgeMask[dir][i][2],
				faceProcEdgeMask[dir][i][3],
				faceProcEdgeMask[dir][i][4],
			};

			const int* order = orders[faceProcEdgeMask[dir][i][0]];
			for (int j = 0; j < 4; j++)
			{
				if (node[order[j]]->m_flag & OCTREE_LEAF ||
					node[order[j]]->m_flag & OCTREE_PSUEDO)
				{
					edgeNodes[j] = node[order[j]];
				}
				else
				{
					edgeNodes[j] = node[order[j]]->m_children[c[j]];
				}
			}

			ContourEdgeProc(m_tri_indices, edgeNodes, faceProcEdgeMask[dir][i][5]);
		}
	}
}

void Octree::ContourCellProc(std::vector<GLuint> &m_tri_indices)
{
	if (!this || ~m_flag & OCTREE_ACTIVE)
	{
		return;
	}

	if (m_flag & OCTREE_INNER)
	{
		for (int i = 0; i < 8; i++)
		{
			if (m_children[i] && m_children[i]->m_flag & OCTREE_ACTIVE)
			{
				m_children[i]->ContourCellProc(m_tri_indices);
			}
		}

		for (int i = 0; i < 12; i++)
		{
			Octree* faceNodes[2];
			const int c[2] = { cellProcFaceMask[i][0], cellProcFaceMask[i][1] };

			faceNodes[0] = m_children[c[0]];
			faceNodes[1] = m_children[c[1]];

			ContourFaceProc(m_tri_indices, faceNodes, cellProcFaceMask[i][2]);
		}

		for (int i = 0; i < 6; i++)
		{
			Octree* edgeNodes[4];
			const int c[4] =
			{
				cellProcEdgeMask[i][0],
				cellProcEdgeMask[i][1],
				cellProcEdgeMask[i][2],
				cellProcEdgeMask[i][3],
			};

			for (int j = 0; j < 4; j++)
			{
				edgeNodes[j] = m_children[c[j]];
			}

			ContourEdgeProc(m_tri_indices, edgeNodes, cellProcEdgeMask[i][4]);
		}
	}
}

