struct Mesh {
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
};

struct Camera {

};

class Object {
	GLuint shaders;
	GLuint VAO;
	GLuint vertexVBO;
	GLuint normalVBO;
	glm::mat4* ModelMatrix;
	glm::mat4* ViewMatrix;
	glm::mat4* ProjectionMatrix;
	glm::vec3* Position;
	Mesh* mesh;

	public:
		Object();

};



void render_skybox(GLuint shaders, mat4, mat4, mat4, GLuint, GLuint skyboxTexture);
void render_triangle(GLuint shaders, mat4, mat4, mat4, GLuint);
void render_water(GLuint shaders, glm::mat4 ModelMatrix, glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix, glm::vec3 Position, GLuint vertexVbo, GLuint normalVbo, GLuint vao, int size);
void render_particles(GLuint shaders, mat4 ModelMatrix, mat4 ViewMatrix, mat4 ProjectionMatrix, vec3 Position, GLuint vbo, GLuint vao, int n_particles);
void render_text(GLint freetype_shdrs, const char *text, float x, float y, float sx, float sy, FT_Library library, FT_Face face);

void render(Object obj);