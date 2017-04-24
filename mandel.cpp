#include <iostream>
#include <vector>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
using std::cout;
using std::endl;
static GLFWwindow* window;
static int wwidth = 500;
static int wheight = 500;
static GLuint gfx_program;
static GLuint vbo;
static GLint resolution_U;
static GLint zoom_U;
static GLint center_U;
static char cmd = 0;
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS) return;
	if (key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_Q)
		glfwSetWindowShouldClose(window, GL_TRUE);
	cmd = key - GLFW_KEY_A + 'a';
}
static void window_size_callback(GLFWwindow* window, int width, int height) {
	wwidth = width;
	wheight  = height;
	glViewport(0, 0, width, height);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wwidth, wheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
}
static std::string VERT = R"(
#version 330
void main(){}
)";
static std::string GEOM = R"(
#version 330
precision highp float;
layout(points) in;
layout(triangle_strip, max_vertices=6) out;
out vec2 p;
void main() {
	const vec2 p0 = vec2(-1.,-1.);
	const vec2 p1 = vec2(-1., 1.);
	gl_Position = vec4(p0,0.,1.);
	p = p0*.5+.5;
	EmitVertex(); // 0
	gl_Position = vec4(p1,0.,1.);
	p = p1*.5+.5;
	EmitVertex(); // 1
	gl_Position = vec4(-p1,0.,1.);
	p = -p1*.5+.5;
	EmitVertex(); // 2
	EndPrimitive();

	gl_Position = vec4(-p1,0.,1.);
	p = -p1*.5+.5;
	EmitVertex(); // 2
	gl_Position = vec4(p1,0.,1.);
	p = p1*.5+.5;
	EmitVertex(); // 1
	gl_Position = vec4(-p0,0.,1.);
	p = -p0*.5+.5;
	EmitVertex(); // 3
	EndPrimitive();
}
)";
static std::string FRAG = R"(
#version 330
precision highp float;
uniform vec2 R;
uniform float Z;
uniform vec2 C;
out vec4 c;
vec4 bg=vec4(0.);
vec4 fg=vec4(0.,204./255.,1.,1.);
void main() {
	vec2 u=gl_FragCoord.xy/R*2.-1.;
	u.x*=R.x/R.y;u.x-=.5;u/=Z;u-=C;
	vec2 z=vec2(0);
	int s=0;
	for (int i=0;i<64;i++) {
		z = vec2(z.x*z.x-z.y*z.y, 2.*z.x*z.y);
		z += u;
		if (dot(z,z)>4.) break;
		s++;
	}
	c = mix(bg,fg,float(s)/64.);
}
)";
bool compile_shader() {
	// Create fullscreen shader to display output
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	char* s0 = (char*)VERT.data();
	glShaderSource(vs, 1, &s0, NULL);
	glCompileShader(vs);

	GLint isCompiled = 0;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(vs, maxLength, &maxLength, &errorLog[0]);

		for (auto c : errorLog)
			cout << c;
		cout << endl;

		glDeleteShader(vs); // Don't leak the shader.
		return false;
	}

	char* s1 = (char*)FRAG.data();
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &s1, NULL);
	glCompileShader(fs);

	isCompiled = 0;
	glGetShaderiv(fs, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(fs, maxLength, &maxLength, &errorLog[0]);

		for (auto c : errorLog)
			cout << c;
		cout << endl;

		glDeleteShader(fs); // Don't leak the shader.
		return false;
	}


	char* s2 = (char*)GEOM.data();
	GLuint gs = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(gs, 1, &s2, NULL);
	glCompileShader(gs);

	isCompiled = 0;
	glGetShaderiv(gs, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(gs, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(gs, maxLength, &maxLength, &errorLog[0]);

		for (auto c : errorLog)
			cout << c;
		cout << endl;

		glDeleteShader(gs); // Don't leak the shader.
		return false;
	}

	gfx_program = glCreateProgram();
	glAttachShader(gfx_program, gs);
	glAttachShader(gfx_program, fs);
	glAttachShader(gfx_program, vs);
	glLinkProgram(gfx_program);
	GLint isLinked = 0;
	glGetProgramiv(gfx_program, GL_LINK_STATUS, (int *)&isLinked);
	if(isLinked == GL_FALSE) {
		GLint maxLength = 0;
		glGetProgramiv(gfx_program, GL_INFO_LOG_LENGTH, &maxLength);

		//The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(gfx_program, maxLength, &maxLength, &infoLog[0]);

		for (auto c : infoLog)
			cout << c;
		cout << endl;

		//We don't need the program anymore.
		glDeleteProgram(gfx_program);
		//Don't leak shaders either.
		glDeleteShader(vs);
		glDeleteShader(fs);
		glDeleteShader(gs);

		return false;
	}

	return true;
}
void init_graphics() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(wwidth, wheight, "float me", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSwapInterval(1);
	glewExperimental = GL_TRUE;
	glewInit();
	glViewport(0, 0, wwidth, wheight);
	glfwSetKeyCallback(window, key_callback);
	compile_shader();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE); // for nice lines?
	glClearColor(0., 0., 0., 1.);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	int point_indices[1]={0};
	glBufferData(GL_ARRAY_BUFFER, sizeof(int), point_indices, GL_STATIC_DRAW);
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glVertexAttribIPointer(0, 1, GL_INT, 0, NULL);
	resolution_U = glGetUniformLocation(gfx_program, "R");
	center_U = glGetUniformLocation(gfx_program, "C");
	zoom_U = glGetUniformLocation(gfx_program, "Z");
	glUseProgram(gfx_program);
}
int main() {
	float zoom = 1.0;
	float centerX = 0.0;
	float centerY = 0.0;
	init_graphics();
	while (!glfwWindowShouldClose(window)) {
		switch (cmd) {
		case 'a':
			centerX += .1/zoom;
			centerX += .1/zoom;
			break;
		case 'd':
			centerX -= .1/zoom;
			centerX -= .1/zoom;
			break;
		case 'w':
			centerY -= .1/zoom;
			centerY -= .1/zoom;
			break;
		case 's':
			centerY += .1/zoom;
			centerY += .1/zoom;
			break;
		case 'z':
			zoom *= 2.;
			break;
		case 'x':
			zoom /= 2.;
			break;
		case 'r':
			zoom = 1.0;
			centerX = 0.0;
			centerY = 0.0;
			break;
		case 'q':
			return 0;
		}
		cmd = 0;
		glClear(GL_COLOR_BUFFER_BIT);
		glUniform2f(resolution_U, float(wwidth), float(wheight));
		glUniform2f(center_U, centerX, centerY);
		glUniform1f(zoom_U, zoom);
		glDrawArrays(GL_POINTS, 0, 1);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	return 0;
}
