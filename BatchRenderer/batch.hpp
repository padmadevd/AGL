#ifndef AGL_VERTEX_BATCH
#define AGL_VERTEX_BATCH

#include <Graphics/texture2d.hpp>

struct Vertex{
	float x, y;
	float r, g, b, a;
	float s, t;
	float texUnit;
};

struct Batch{

    public:

        Batch(uint32_t vertexLimit, uint32_t indexLimit, uint32_t texLimit);
        bool AddTri(Vertex *tri, Texture2D *tex);
        bool AddQuad(Vertex *quad, Texture2D *tex);
        void SubmitDraw();
        ~Batch();

    protected:
        uint32_t m_vertexLimit, m_indexLimit, m_texLimit;
        Vertex *m_vertices;
        int m_verticesN;
        unsigned int *m_indices;
        int m_indicesN;
        unsigned int *m_texIDs;
        int m_texIDsN;

	    unsigned int m_VAO, m_VBO, m_EBO;
};

#endif