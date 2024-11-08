#ifndef AGL_PHYSICS_POLYGON2D
#define AGL_PHYSICS_POLYGON2D

#include <glm/glm.hpp>
#include <vector>

class Polygon2D{

    public:

        inline void Add(glm::vec2 point){
            m_points.push_back(point);
        }
        // inline void Add(glm::vec2 &point){
        //     m_points.push_back(point);
        // }
        inline void Add(std::vector<glm::vec2> points){
            m_points.insert(m_points.end(), points.begin(), points.end());
        }
        inline void Add(std::vector<glm::vec2> &points){
            m_points.insert(m_points.end(), points.begin(), points.end());
        }
        glm::vec2 &At(int i);

        inline size_t size(){
            return m_points.size();
        }

        bool IsReflex(int i);
        void MakeCCW();

    public:

        std::vector<glm::vec2> m_points;
};

void Decompose(Polygon2D &polygon, std::vector<Polygon2D> &subPolys);

#endif