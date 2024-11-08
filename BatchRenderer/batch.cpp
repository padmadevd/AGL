#include <BatchRenderer/batch.hpp>

#include <cstdlib>

Batch::Batch(uint32_t vertexLimit, uint32_t indexLimit, uint32_t texLimit)
	: m_vertexLimit(vertexLimit), m_indexLimit(indexLimit), m_texLimit(texLimit){

    m_verticesN = 0;
	m_indicesN = 0;
	m_texIDsN = 0;

	m_vertices = (Vertex*)malloc(sizeof(Vertex)*vertexLimit);
	m_indices = (unsigned int*)malloc(sizeof(unsigned int)*indexLimit);
	m_texIDs = (unsigned int*)malloc(sizeof(unsigned int)*texLimit);

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*vertexLimit, nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, x)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, r)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, s)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texUnit)));
	glEnableVertexAttribArray(3);

	glGenBuffers(1, &m_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indexLimit, nullptr, GL_DYNAMIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

bool Batch::AddTri(Vertex *tri, Texture2D *tex){

    if(m_verticesN + 3 > m_vertexLimit || m_indicesN + 3 > m_indexLimit)
		return false;

	int t = -1;
	if(tex){
		for(t = 0; t < m_texIDsN; t++){
			if(m_texIDs[t] == tex->m_id)
				break;
		}
	}

	if(tex && t >= m_texLimit)
		return false;

	if(tex && t >= m_texIDsN){
		m_texIDs[t] = tex->m_id;
		m_texIDsN += 1;
	}

	tri[0].texUnit = t;
	tri[1].texUnit = t;
	tri[2].texUnit = t;

	m_vertices[m_verticesN+0] = tri[0];
	m_vertices[m_verticesN+1] = tri[1];
	m_vertices[m_verticesN+2] = tri[2];

	m_indices[m_indicesN+0] = m_verticesN+0;
	m_indices[m_indicesN+1] = m_verticesN+1;
	m_indices[m_indicesN+2] = m_verticesN+2;
	
	m_verticesN += 3;
	m_indicesN += 3;

	return true;
}

bool Batch::AddQuad(Vertex *quad, Texture2D *tex){

    if(m_verticesN + 4 > m_vertexLimit || m_indicesN + 6 > m_indexLimit)
		return false;

	int t = -1;
	if(tex){
		for(t = 0; t < m_texIDsN; t++){
			if(m_texIDs[t] == tex->m_id)
				break;
		}
	}

	if(tex && t >= m_texLimit)
		return false;

	if(tex && t >= m_texIDsN){
		m_texIDs[t] = tex->m_id;
		m_texIDsN += 1;
	}

	quad[0].texUnit = t;
	quad[1].texUnit = t;
	quad[2].texUnit = t;
	quad[3].texUnit = t;

	m_vertices[m_verticesN+0] = quad[0];
	m_vertices[m_verticesN+1] = quad[1];
	m_vertices[m_verticesN+2] = quad[2];
	m_vertices[m_verticesN+3] = quad[3];

	m_indices[m_indicesN+0] = m_verticesN+0;
	m_indices[m_indicesN+1] = m_verticesN+1;
	m_indices[m_indicesN+2] = m_verticesN+2;
	m_indices[m_indicesN+3] = m_verticesN+2;
	m_indices[m_indicesN+4] = m_verticesN+3;
	m_indices[m_indicesN+5] = m_verticesN+0;
	
	m_verticesN += 4;
	m_indicesN += 6;

	return true;
}

void Batch::SubmitDraw(){

    for(int t = 0; t < m_texIDsN; t++){
		glActiveTexture(GL_TEXTURE0+t);
		glBindTexture(GL_TEXTURE_2D, m_texIDs[t]);
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex)*m_verticesN, m_vertices);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned int)*m_indicesN, m_indices);

	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_indicesN, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_verticesN = 0;
    m_indicesN = 0;
	m_texIDsN = 0;
}

Batch::~Batch(){

    glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_EBO);
	glDeleteVertexArrays(1, &m_VAO);

	free(m_vertices);
	free(m_indices);
	free(m_texIDs);
}