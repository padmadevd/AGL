#ifndef AGL_PARTICLES
#define AGL_PARTICLES

#include <Graphics/camera2d.hpp>
#include <Graphics/texture2d.hpp>

#include <glm/glm.hpp>

#define MAX_PARTICLES 5000

struct Particles;

struct Particles{

	glm::vec2 origin;
	float direction;
	float minSpeed;
	float maxSpeed;
	float linearSpread;
	float angularSpread;
	float lifeTime;
	float birthRate;
	glm::vec2 gravity;
	float bornCount;
	unsigned int count;

	float sizeStart;
	float sizeEnd;

	glm::vec4 colorStart;
	glm::vec4 colorEnd;

	unsigned int texture;

	unsigned int VAO_U1, VAO_U2, VAO_R1, VAO_R2;
	unsigned int VBOQ, VBOR, VBOW;
};

void InitParticleSystem();

Particles* CreateParticles(glm::vec2 origin, float direction, float minSpeed, float maxSpeed, float linearSpread, float angularSpread, float lifeTime, float sizeStart, float sizeEnd, glm::vec4 colorStart, glm::vec4 colorEnd, glm::vec2 gravity, float birthRate, unsigned int count, Texture2D *texture);

void UpdateParticles(Particles *p, float deltaTime);
void RenderParticles(Particles *p, Camera2D cam, bool flipY);
void FreeParticles(Particles *p);

void QuitParticleSystem();

#endif