#include "game.hpp"

Mesh::Mesh(vector<Vertex>& vertices, vector<GLuint>& indices, vector<Texture>& textures)
{
	this->m_vertices = vertices;
	this->m_indices = indices;
	this->m_textures = textures;

	this->MeshInit();
	this->m_instanced = GL_FALSE;
}

void Mesh::Draw(Shader *shader)
{
	//bind textures
	glBindVertexArray(this->m_vao);

	for (GLuint i = 0; i < m_textures.size(); i++)
	{
		m_textures[i].Bind(GL_TEXTURE0 + i);

		switch (m_textures[i].GetTexType())
		{
		case Diffuse:
			shader->SetUniform1i("textureDiffuse", i);
			break;
		case Specular:
			shader->SetUniform1i("textureSpecular", i);
			break;
		case Ambient:
			shader->SetUniform1i("textureAmbient", i);
			break;
		}
	}

	//draw the mesh
	if (m_instanced)
		glDrawElementsInstanced(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0, m_instanceCount);
	else
		glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);

	// Unbind the textures after drawing
	for (auto tex : m_textures)
		tex.Unbind();
}
/**
* @brief binds all the vertex data to a vertex array object
*/
void Mesh::MeshInit()
{
	glGenVertexArrays(1, &this->m_vao);
	glGenBuffers(1, &this->m_vbo);
	glGenBuffers(1, &this->m_ebo);

	glBindVertexArray(this->m_vao);

	glBindBuffer(GL_ARRAY_BUFFER, this->m_vbo);
	glBufferData(GL_ARRAY_BUFFER, this->m_vertices.size() * sizeof(Vertex), &this->m_vertices[0], GL_STATIC_DRAW);

	//setup for indices drawing
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->m_indices.size() * sizeof(GLuint), &this->m_indices[0], GL_STATIC_DRAW);

	//location 0 should be verts
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	//now normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
	//now textures
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, uv));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::MeshInitRigged()
{
	//glGenVertexArrays(1, &this->m_vao);
	//glGenBuffers(1, &this->m_vbo);
	//glGenBuffers(1, &this->m_ebo);

	//glBindVertexArray(this->m_vao);

	//glBindBuffer(GL_ARRAY_BUFFER, this->m_vbo);
	//glBufferData(GL_ARRAY_BUFFER, this->m_boneVertices.size() * sizeof(BoneVertex), &this->m_boneVertices[0], GL_STATIC_DRAW);

	////setup for indices drawing
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_ebo);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

	////location 0 should be verts
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BoneVertex), (GLvoid*)0);
	////now normals
	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(BoneVertex), (GLvoid*)offsetof(BoneVertex, normal));
	////now textures
	//glEnableVertexAttribArray(2);
	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(BoneVertex), (GLvoid*)offsetof(BoneVertex, uv));

	//glEnableVertexAttribArray(3);
	//glVertexAttribIPointer(3, 4, GL_INT, sizeof(BoneVertex), (GLvoid*)offsetof(BoneVertex, boneIds));

	//glEnableVertexAttribArray(4);
	//glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(BoneVertex), (GLvoid*)offsetof(BoneVertex, weights));

	//glBindVertexArray(0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
/**
* @brief additional buffer for instance rendering
* @param intstanceBuffer bind buffer with instance world position data
* @param amount number of times to instance render this mesh
*/
void Mesh::MeshSetInstance(GLuint instanceBuffer, GLuint amount)
{
	m_instanceCount = amount;
	m_instanced = GL_TRUE;

	glBindVertexArray(m_vao);

	glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);

	GLsizei vec4Size = sizeof(glm::vec4);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (GLvoid*)0);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (GLvoid*)(vec4Size));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (GLvoid*)(2 * vec4Size));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (GLvoid*)(3 * vec4Size));

	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}