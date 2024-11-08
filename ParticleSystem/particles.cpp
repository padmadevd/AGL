#include <ParticleSystem/particles.hpp>

#include <Base/gl.hpp>
#include <Graphics/texture2d.hpp>
#include <Graphics/shader.hpp>

#include <cstdio>
#include <cstdlib>
#include <string>

#include <SDL2/SDL_log.h>
#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct ParticleData{

	float data[6];
};

Shader *updateProgram = 0;
Shader *renderProgram = 0;
unsigned int randNoiseTexture = 0;

const char *updateVertCode =
"precision mediump float;\n"
"\n"
"// A Single vertex input, representing a single particle. Input to Transform Feedback\n"
"layout (location = 0) in vec2 iPos;\n"
"layout (location = 1) in vec2 iVelocity;\n"
"layout (location = 2) in float iLife;\n"
"layout (location = 3) in float iAge;\n"
"\n"
"uniform float uDeltaTime;\n"
"uniform sampler2D uRandNoise;\n"
"\n"
"uniform vec2 uOrigin;\n"
"uniform float uDirection;\n"
"uniform vec2 uGravity;\n"
"uniform float uMinSpeed;\n"
"uniform float uMaxSpeed;\n"
"uniform float uLinearSpread;\n"
"uniform float uAngularSpread;\n"
"uniform float uLifeTime;\n"
"\n"
"// A single particle Output from Transform Feedback\n"
"out vec2 vPos;\n"
"out vec2 vVelocity;\n"
"out float vLife;\n"
"out float vAge;\n"
"\n"
"float radian(float degree){\n"
"\n"
"\treturn degree * 0.0174533;\n"
"}\n"
"\n"
"vec2 rotate(vec2 vec, float direction){\n"
"\t\n"
"\tfloat cosT = cos(radian(direction));\n"
"\tfloat sinT = sin(radian(direction));\n"
"\n"
"\tvec2 res = vec2(1.0f);\n"
"\tres.x = vec.x*cosT - vec.y*sinT;\n"
"\tres.y = vec.x*sinT + vec.y*cosT;\n"
"\n"
"\treturn res;\n"
"}\n"
"\n"
"void main(){\n"
"\t\n"
"\t// Particle died. So, Create a new Particle\n"
"\tif(iAge >= iLife){\n"
"\n"
"\t\t// Fetch random value from random noise texture\n"
"\t\tivec2 randNoiseCoord = ivec2(gl_VertexID%512, gl_VertexID/512);\n"
"\t\tvec2 randoms = texelFetch(uRandNoise, randNoiseCoord, 0).rg;\n"
"\t\tfloat rand1 = randoms.x;\n"
"\t\tfloat rand2 = randoms.y;\n"
"\t\tfloat rand3 = (rand1+rand2)/2.0f;\n"
"\n"
"\t\t// Direction of the emitter\n"
"\t\tvec2 dir = vec2(1.0f, 0.0f);\n"
"\t\tdir = rotate(dir, uDirection);\n"
"\n"
"\t\t// linear spread of the spawning of the particle\n"
"\t\tfloat offset = -uLinearSpread + (uLinearSpread - (-uLinearSpread))*rand1;\n"
"\t\tvPos = uOrigin + (rotate(dir, -90.0f)*offset);\n"
"\n"
"\t\t// Velocity of the Particle with angular spread and random speed;\n"
"\t\tfloat dirOffset = -uAngularSpread + (uAngularSpread - (-uAngularSpread))*rand2;\n"
"\t\tfloat speed = uMinSpeed + (uMaxSpeed - uMinSpeed)*rand3;\n"
"\t\tvVelocity = rotate(dir, dirOffset)*speed;\n"
"\n"
"\t\t// life time and initial age of the particle\n"
"\t\tvLife = uLifeTime;\n"
"\t\tvAge = iAge-iLife;\n"
"\n"
"\t}else{\n"
"\t// Particle is alive\n"
"\n"
"\t\t// Update the particle's properties based on newton's laws of physics\n"
"\t\tvPos = iPos + iVelocity*uDeltaTime;\n"
"\t\tvVelocity = iVelocity + uGravity*uDeltaTime;\n"
"\t\tvAge = iAge + uDeltaTime;\n"
"\t\tvLife = iLife;\n"
"\t}\n"
"}";


const char *updateFragCode =
"precision mediump float;\n"
"\n"
"void main(){\n"
"\tdiscard;\n"
"}";

const char *renderVertCode = 
"precision mediump float;\n"
"\n"
"layout (location = 0) in vec2 iPos;\n"
"layout (location = 1) in vec2 iVelocity;\n"
"layout (location = 2) in float iLife;\n"
"layout (location = 3) in float iAge;\n"
"\n"
"layout (location = 4) in vec2 iVertPos;\n"
"layout (location = 5) in vec2 iVertTC;\n"
"\n"
"uniform float sizeStart;\n"
"uniform float sizeEnd;\n"
"\n"
"uniform mat4 view;\n"
"uniform mat4 proj;\n"
"\n"
"out float gradient;\n"
"out vec2 vertTC;\n"
"\n"
"void main(){\n"
"\t\n"
"\tfloat grad = iAge/iLife;\n"
"\tfloat size = (sizeStart + (sizeEnd - sizeStart)*grad);\n"
"\t\n"
"\tvec2 vertPos = iPos + iVertPos*size;\n"
"\n"
"\tgradient = grad;\n"
"\tvertTC = iVertTC;\n"
"\n"
"\tgl_Position = proj*view*vec4(vertPos, 0.0, 1.0);\n"
"}";

const char *renderFragCode =
"precision mediump float;\n"
"\n"
"in float gradient;\n"
"in vec2 vertTC;\n"
"\n"
"uniform sampler2D uTexture;\n"
"uniform vec4 uColorStart;\n"
"uniform vec4 uColorEnd;\n"
"\n"
"out vec4 FragColor;\n"
"\n"
"void main(){\n"
"\n"
"\tvec4 color = (uColorStart + (uColorEnd - uColorStart)*gradient);\n"
"\tFragColor = color*texture(uTexture, vertTC);\n"
"}";

void InitParticleSystem(){

	updateProgram = new Shader;
	renderProgram = new Shader;
	updateProgram->TF_FromString((std::string(glsl_header) + std::string(updateVertCode)).c_str(), (std::string(glsl_header) + std::string(updateFragCode)).c_str(), {"vPos", "vVelocity", "vLife", "vAge"});
	renderProgram->FromString((std::string(glsl_header) + std::string(renderVertCode)).c_str(), (std::string(glsl_header) + std::string(renderFragCode)).c_str());

	float *randomNoise = (float*)malloc(sizeof(float)*512*512*2);
	for(int i = 0; i < 512*512*2; i++){
		randomNoise[i] = (rand() % 101)/100.0f;
	}

	glGenTextures(1, &randNoiseTexture);
	glBindTexture(GL_TEXTURE_2D, randNoiseTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, randomNoise);

	free(randomNoise);
}

Particles* CreateParticles(glm::vec2 origin, float direction, float minSpeed, float maxSpeed, float linearSpread, float angularSpread, float lifeTime, float sizeStart, float sizeEnd, glm::vec4 colorStart, glm::vec4 colorEnd, glm::vec2 gravity, float birthRate, unsigned int count, Texture2D *texture){

	Particles *p = new Particles;

	p->origin = origin;
	p->direction = direction;
	p->minSpeed = minSpeed;
	p->maxSpeed = maxSpeed;
	p->linearSpread = linearSpread;
	p->angularSpread = angularSpread;
	p->lifeTime = lifeTime;
	p->sizeStart = sizeStart;
	p->sizeEnd = sizeEnd;
	p->colorStart = colorStart;
	p->colorEnd = colorEnd;
	p->gravity = gravity;
	p->birthRate = birthRate;
	p->count = count;
	p->bornCount = 0;
	p->texture = texture->GetId();

	ParticleData *initialParticlesData = (ParticleData*)malloc(sizeof(ParticleData)*MAX_PARTICLES);
	for(int i = 0; i < MAX_PARTICLES; i++){

		// Position
		initialParticlesData[i].data[0] = 0;
		initialParticlesData[i].data[1] = 0;

		// Velocity
		initialParticlesData[i].data[2] = 0;
		initialParticlesData[i].data[3] = 0;

		// Age then Life
		initialParticlesData[i].data[4] = lifeTime;
		initialParticlesData[i].data[5] = lifeTime + 0.01;
	}

	glGenBuffers(1, &p->VBOR);
	glBindBuffer(GL_ARRAY_BUFFER, p->VBOR);
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES*sizeof(ParticleData), initialParticlesData, GL_STATIC_DRAW);

	glGenBuffers(1, &p->VBOW);
	glBindBuffer(GL_ARRAY_BUFFER, p->VBOW);
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES*sizeof(ParticleData), initialParticlesData, GL_STATIC_DRAW);

	free(initialParticlesData);

	float w = texture->GetWidth();
	float h = texture->GetHeight();

	float quad[] = {
		-0.5f*w, -0.5f*h, 0.f, 0.f,
		 0.5f*w, -0.5f*h, 1.f, 0.f,
		 0.5f*w,  0.5f*h, 1.f, 1.f,
		 0.5f*w,  0.5f*h, 1.f, 1.f,
		-0.5f*w,  0.5f*h, 0.f, 1.f,
		-0.5f*w, -0.5f*h, 0.f, 0.f
	};

	glGenBuffers(1, &p->VBOQ);
	glBindBuffer(GL_ARRAY_BUFFER, p->VBOQ);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*6*4, quad, GL_STATIC_DRAW);

	// VERTEX ARRAY U1 ---------------------------------------------------------------------------->
	glGenVertexArrays(1, &p->VAO_U1);
	glBindVertexArray(p->VAO_U1);

	glBindBuffer(GL_ARRAY_BUFFER, p->VBOR);

	// layout (location = 0) in vec2 iPos;
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*6, 0);
	glEnableVertexAttribArray(0);

	// layout (location = 1) in vec2 iVelocity;
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*6, (const void*)(sizeof(float)*2));
	glEnableVertexAttribArray(1);

	// layout (location = 2) in float iLife;
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float)*6, (const void*)(sizeof(float)*4));
	glEnableVertexAttribArray(2);

	// layout (location = 3) in float iAge;
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float)*6, (const void*)(sizeof(float)*5));
	glEnableVertexAttribArray(3);


	// VERTEX ARRAY U2 ---------------------------------------------------------------------------->
	glGenVertexArrays(1, &p->VAO_U2);
	glBindVertexArray(p->VAO_U2);

	glBindBuffer(GL_ARRAY_BUFFER, p->VBOW);

	// layout (location = 0) in vec2 iPos;
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*6, 0);
	glEnableVertexAttribArray(0);

	// layout (location = 1) in vec2 iVelocity;
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*6, (const void*)(sizeof(float)*2));
	glEnableVertexAttribArray(1);

	// layout (location = 2) in float iLife;
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float)*6, (const void*)(sizeof(float)*4));
	glEnableVertexAttribArray(2);

	// layout (location = 3) in float iAge;
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float)*6, (const void*)(sizeof(float)*5));
	glEnableVertexAttribArray(3);


	// VERTEX ARRAY R1 ---------------------------------------------------------------------------->
	glGenVertexArrays(1, &p->VAO_R1);
	glBindVertexArray(p->VAO_R1);

	glBindBuffer(GL_ARRAY_BUFFER, p->VBOQ);
	// layout (location = 4) in vec2 iVertPos;
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (const void*)(sizeof(float)*0));
	glEnableVertexAttribArray(4);
	// layout (location = 5) in vec2 iVertTC;
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (const void*)(sizeof(float)*2));
	glEnableVertexAttribArray(5);

	glBindBuffer(GL_ARRAY_BUFFER, p->VBOW);
	// layout (location = 0) in vec2 iPos;
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*6, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribDivisor(0, 1);
	// layout (location = 1) in vec2 iVelocity;
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*6, (const void*)(sizeof(float)*2));
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1);
	// layout (location = 2) in float iLife;
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float)*6, (const void*)(sizeof(float)*4));
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 1);
	// layout (location = 3) in float iAge;
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float)*6, (const void*)(sizeof(float)*5));
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);


	// VERTEX ARRAY R2 ---------------------------------------------------------------------------->
	glGenVertexArrays(1, &p->VAO_R2);
	glBindVertexArray(p->VAO_R2);

	glBindBuffer(GL_ARRAY_BUFFER, p->VBOQ);
	// layout (location = 4) in vec2 iVertPos;
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (const void*)(sizeof(float)*0));
	glEnableVertexAttribArray(4);
	// layout (location = 5) in vec2 iVertTC;
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (const void*)(sizeof(float)*2));
	glEnableVertexAttribArray(5);

	glBindBuffer(GL_ARRAY_BUFFER, p->VBOR);
	// layout (location = 0) in vec2 iPos;
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*6, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribDivisor(0, 1);
	// layout (location = 1) in vec2 iVelocity;
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*6, (const void*)(sizeof(float)*2));
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1);
	// layout (location = 2) in float iLife;
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float)*6, (const void*)(sizeof(float)*4));
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 1);
	// layout (location = 3) in float iAge;
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float)*6, (const void*)(sizeof(float)*5));
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return p;
}

void UpdateParticles(Particles *p, float deltaTime){

	if(p->bornCount < p->count)
		p->bornCount += p->birthRate*deltaTime;

	if(p->bornCount > p->count)
		p->bornCount = p->count; 

	glEnable(GL_RASTERIZER_DISCARD);
	glBindVertexArray(p->VAO_U1);
	updateProgram->Use();

	//SET UNIFORMS ------------------------------------------------------->
	
	// uniform float uDeltaTime;
	updateProgram->Set1f("uDeltaTime", deltaTime);
	// uniform sampler2D uRandNoise;
	updateProgram->Set1i("uRandNoise", 0);
	// uniform vec2 uOrigin;
	updateProgram->Set2f("uOrigin",  p->origin.x,  p->origin.y);
	// uniform float uDirection;
	updateProgram->Set1f("uDirection", p->direction);
	// uniform vec2 uGravity;
	updateProgram->Set2f("uGravity", p->gravity.x, p->gravity.y);
	// uniform float uMinSpeed;
	updateProgram->Set1f( "uMinSpeed", p->minSpeed);
	// uniform float uMaxSpeed;
	updateProgram->Set1f("uMaxSpeed", p->maxSpeed);
	// uniform float uLinearSpread;
	updateProgram->Set1f("uLinearSpread", p->linearSpread);
	// uniform float uAngularSpread;
	updateProgram->Set1f("uAngularSpread", p->angularSpread);
	// uniform float uLifeTime
	updateProgram->Set1f("uLifeTime", p->lifeTime);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, randNoiseTexture);

	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, p->VBOW);
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, p->bornCount);

	glEndTransformFeedback();
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	glDisable(GL_RASTERIZER_DISCARD);

	unsigned int temp = p->VAO_U1;
	p->VAO_U1 = p->VAO_U2;
	p->VAO_U2 = temp;

	temp = p->VBOR;
	p->VBOR = p->VBOW;
	p->VBOW = temp;
}

void RenderParticles(Particles *p, Camera2D cam, bool flipY){

	glBindVertexArray(p->VAO_R1);
	renderProgram->Use();

	//uniform float sizeStart;
	renderProgram->Set1f("sizeStart", p->sizeStart);
	// uniform float sizeEnd;
	renderProgram->Set1f("sizeEnd", p->sizeEnd);
	// uniform Sampler2D uTexture;
	renderProgram->Set1i("uTexture", 0);
	// uniform vec4 uColorStart;
	renderProgram->Set4f("uColorStart", p->colorStart.x, p->colorStart.y, p->colorStart.z, p->colorStart.w);
	// uniform vec4 uColorEnd;
	renderProgram->Set4f("uColorEnd", p->colorEnd.x, p->colorEnd.y, p->colorEnd.z, p->colorEnd.w);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, p->texture);

	glm::mat4 view = cam.ViewMatrix();
	renderProgram->SetMatrix4("view", glm::value_ptr(view));

	// ORTHO PROJECTION MATRIX
	glm::mat4 ortho;
	if(flipY)
		ortho = glm::orthoNO(-cam.m_width/2.f, -cam.m_width/2.f+cam.m_width, -cam.m_height/2.f, -cam.m_height/2.f+cam.m_height, 0.f, 100.f);
	else
		ortho = glm::orthoNO(-cam.m_width/2.f, -cam.m_width/2.f+cam.m_width, -cam.m_height/2.f+cam.m_height, -cam.m_height/2.f, 0.f, 100.f);

	renderProgram->SetMatrix4("proj", glm::value_ptr(ortho));

	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, p->bornCount);

	glUseProgram(0);
	glBindVertexArray(0);

	unsigned int temp = p->VAO_R1;
	p->VAO_R1 = p->VAO_R2;
	p->VAO_R2 = temp;
}

void FreeParticles(Particles *p){

	glDeleteBuffers(1, &p->VBOR);
	glDeleteBuffers(1, &p->VBOW);
	glDeleteBuffers(1, &p->VBOQ);
	glDeleteVertexArrays(1, &p->VAO_U1);
	glDeleteVertexArrays(1, &p->VAO_U2);
	glDeleteVertexArrays(1, &p->VAO_R1);
	glDeleteVertexArrays(1, &p->VAO_R2);

	delete p;
}

void QuitParticleSystem(){

	delete updateProgram;
	delete renderProgram;
	glDeleteTextures(1, &randNoiseTexture);
}