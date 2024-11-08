#ifndef AGL_GRAPHICS_SHADER
#define AGL_GRAPHICS_SHADER

#include <vector>
#include <string>

// SHADER -------------------------------------------------------------------------

class Shader{

	public:

		Shader();
		void FromString(const char *vertStr, const char *fragStr);
		void FromFile(const char *vertFilePath, const char *fragFilePath);

		void TF_FromString(const char *vertStr, const char *fragStr, int varyingsN, const char **varyings);
		void TF_FromString(const char *vertStr, const char *fragStr, std::vector<std::string> varyings);

		void TF_FromFile(const char *vertFilePath, const char *fragFilePath, int varyingsN, const char **varyings);
		void TF_FromFile(const char *vertFilePath, const char *fragFilePath, std::vector<std::string> varyings);

		void Use();
		unsigned int GetId();
		void Set1f(const char *name, float x);
		void Set2f(const char *name, float x, float y);
		void Set3f(const char *name, float x, float y, float z);
		void Set4f(const char *name, float x, float y, float z, float w);
		void Set1i(const char *name, int x);
		void SetMatrix4(const char *name, float *mat4x4fv);

		void Free();
		~Shader();

	protected:
		unsigned int m_id;

		friend class BatchRenderer2D;
};

#endif