#ifndef AGL_GRAPHICS_CAMERA2D
#define AGL_GRAPHICS_CAMERA2D

// CAMERA 2D ----------------------------------------------------------------------------

#include <glm/glm.hpp>

class Camera2D{

	public:
		glm::vec2 m_pos;
		float m_width, m_height;
		float m_zoom;
		float m_rotation;
	
		glm::mat4 ViewMatrix();
		glm::vec2 ScreenToWorld(glm::vec2 screenCoords);
		glm::vec2 WorldToScreen(glm::vec2 worldCoords);
};

#endif