#include <Graphics/camera2d.hpp>

#include <glm/gtc/matrix_transform.hpp>

// CAMERA 2D ---------------------------------------------------------------------------------------------

// class Camera2D{

// 	public:
// 		glm::vec2 pos;
// 		float width, height;
// 		float zoom;
// 		float rotation;
	
// 		glm::mat4 ViewMatrix();
// 		glm::vec2 ScreenToWorld(glm::vec2 screenCoords);
// 		glm::vec2 WorldToScreen(glm::vec2 worldCoords);
// };

glm::mat4 Camera2D::ViewMatrix(){

	return glm::lookAt(glm::vec3(m_pos, 0.0f), glm::vec3(m_pos, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec2 Camera2D::ScreenToWorld(glm::vec2 screenCoords){

	glm::vec2 wc;
	wc.x = screenCoords.x - m_width/2 + m_pos.x;
	wc.y = screenCoords.y - m_height/2 + m_pos.y;
	return wc;
}

glm::vec2 Camera2D::WorldToScreen(glm::vec2 worldCoords){

	glm::vec2 sc;
	sc.x = worldCoords.x - m_width/2  + m_pos.x;
	sc.y = worldCoords.y - m_height/2 + m_pos.y;
	return sc;
}