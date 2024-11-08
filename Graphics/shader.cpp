#include <Graphics/shader.hpp>
#include <Base/gl.hpp>

#include <SDL2/SDL.h>

// SHADER ----------------------------------------------------------------------------------------


// class Shader{

// 	public:

// 		Shader();
// 		void FromString(const char *_vertStr, const char *_fragStr);
// 		void FromFile(const char *_vertFilePath, const char *_fragFilePath);

// 		unsigned int GetId();
// 		void Set1f(const char *_name, float _x);
// 		void Set2f(const char *_name, float _x, float _y);
// 		void Set3f(const char *_name, float _x, float _y, float _z);
// 		void Set4f(const char *_name, float _x, float _y, float _z, float _w);
// 		void Set1i(const char *_name, int _x);
// 		void SetMatrix4(const char *_name, float *_mat4x4fv);

// 		void Free();
// 		~Shader();

// 	protected:
// 		unsigned int id;
// };


Shader::Shader()
	: m_id(0){
}

void Shader::FromString(const char *vertStr, const char *fragStr){

	int success;
	char log[1024];

	unsigned int vert = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert, 1, &vertStr, 0);
	glCompileShader(vert);

	glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(vert, 1024, 0, log);
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Vertex shader compilation error : %s\n", log);
		glDeleteShader(vert);
		return;
	}

	unsigned int frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag, 1, &fragStr, 0);
	glCompileShader(frag);

	glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(frag, 1024, 0, log);
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Fragment shader compilation error : %s\n", log);
		glDeleteShader(vert);
		glDeleteShader(frag);
		return;
	}

	m_id = glCreateProgram();
	glAttachShader(m_id, vert);
	glAttachShader(m_id, frag);
	glLinkProgram(m_id);

	glGetProgramiv(m_id, GL_LINK_STATUS, &success);
	if(!success){
		glGetProgramInfoLog(frag, 1024, 0, log);
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Shader Link error : %s\n", log);
		glDeleteShader(vert);
		glDeleteShader(frag);
		glDeleteProgram(m_id);
		m_id = 0;
		return;
	}

	glDeleteShader(vert);
	glDeleteShader(frag);
}

void Shader::FromFile(const char *vertFilePath, const char *fragFilePath){

	size_t size;

	char *vertStr = (char*)SDL_LoadFile(vertFilePath, &size);
	if(!vertStr){
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error Loading Shader : (path) %s\n", vertFilePath);
		return;
	}

	char *fragStr = (char*)SDL_LoadFile(fragFilePath, &size);
	if(!fragStr){
		SDL_free(vertStr);
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error Loading Shader : (path) %s\n", fragFilePath);
		return;
	}

	FromString(vertStr, fragStr);

	SDL_free(vertStr);
	SDL_free(fragStr);
}

void Shader::TF_FromString(const char *vertStr, const char *fragStr, int varyingsN, const char **varyings){

	int success;
	char log[1024];

	unsigned int vert = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert, 1, &vertStr, 0);
	glCompileShader(vert);

	glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(vert, 1024, 0, log);
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Vertex shader compilation error : %s\n", log);
		glDeleteShader(vert);
		return;
	}

	unsigned int frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag, 1, &fragStr, 0);
	glCompileShader(frag);

	glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(frag, 1024, 0, log);
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Fragment shader compilation error : %s\n", log);
		glDeleteShader(vert);
		glDeleteShader(frag);
		return;
	}

	m_id = glCreateProgram();
	glAttachShader(m_id, vert);
	glAttachShader(m_id, frag);
	glTransformFeedbackVaryings(m_id, varyingsN, varyings, GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(m_id);

	glGetProgramiv(m_id, GL_LINK_STATUS, &success);
	if(!success){
		glGetProgramInfoLog(frag, 1024, 0, log);
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Shader Link error : %s\n", log);
		glDeleteShader(vert);
		glDeleteShader(frag);
		glDeleteProgram(m_id);
		m_id = 0;
		return;
	}

	glDeleteShader(vert);
	glDeleteShader(frag);
}

void Shader::TF_FromString(const char *vertStr, const char *fragStr, std::vector<std::string> varyings){

	int n = varyings.size();
	const char *c_strs[n];

	for(int i = 0; i < n; i++)
		c_strs[i] = varyings[i].c_str();
	
	TF_FromString(vertStr, fragStr, n, c_strs);
}

void Shader::TF_FromFile(const char *vertFilePath, const char *fragFilePath, int varyingsN, const char **varyings){

	size_t size;

	char *vertStr = (char*)SDL_LoadFile(vertFilePath, &size);
	if(!vertStr){
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error Loading Shader : (path) %s\n", vertFilePath);
		return;
	}

	char *fragStr = (char*)SDL_LoadFile(fragFilePath, &size);
	if(!fragStr){
		SDL_free(vertStr);
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error Loading Shader : (path) %s\n", fragFilePath);
		return;
	}

	TF_FromString(vertStr, fragStr, varyingsN, varyings);

	SDL_free(vertStr);
	SDL_free(fragStr);
}

void Shader::TF_FromFile(const char *vertFilePath, const char *fragFilePath, std::vector<std::string> varyings){

	int n = varyings.size();
	const char *c_strs[n];

	for(int i = 0; i < n; i++)
		c_strs[i] = varyings[i].c_str();

	TF_FromFile(vertFilePath, fragFilePath, n, c_strs);
}

void Shader::Use(){
	glUseProgram(m_id);
}

unsigned int Shader::GetId(){
	return m_id;
}

void Shader::Set1f(const char *_name, float _x){
	glUniform1f(glGetUniformLocation(m_id, _name), _x);
}

void Shader::Set2f(const char *_name, float _x, float _y){
	glUniform2f(glGetUniformLocation(m_id, _name), _x, _y);
}

void Shader::Set3f(const char *_name, float _x, float _y, float _z){
	glUniform3f(glGetUniformLocation(m_id, _name), _x, _y, _z);
}

void Shader::Set4f(const char *_name, float _x, float _y, float _z, float _w){
	glUniform4f(glGetUniformLocation(m_id, _name), _x, _y, _z, _w);
}

void Shader::Set1i(const char *_name, int _x){
	glUniform1i(glGetUniformLocation(m_id, _name), _x);
}

void Shader::SetMatrix4(const char *_name, float *_mat4x4fv){
	glUniformMatrix4fv(glGetUniformLocation(m_id, _name), 1, GL_FALSE, _mat4x4fv);
}

void Shader::Free(){
	glDeleteProgram(m_id);
	m_id = 0;
}

Shader::~Shader(){
	Free();
}