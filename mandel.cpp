#include <iostream>
#include <vector>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
using std::cout;
using std::endl;
static int wwidth = 500;
static int wheight = 500;
static float zoom = 1.0;
static float centerX = 0.0;
static float centerY = 0.0;
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS) return;
	if (key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_Q)
		glfwSetWindowShouldClose(window, GL_TRUE);
	switch (key - GLFW_KEY_A + 'a') {
	case 'a':
		centerX += .5/zoom;
		break;
	case 'd':
		centerX -= .5/zoom;
		break;
	case 'w':
		centerY -= .5/zoom;
		break;
	case 's':
		centerY += .5/zoom;
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
	}
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
vec4 bg=vec4(0);
vec4 fg=vec4(1);
void main() {
	vec2 u=gl_FragCoord.xy/R*2.-1.;
	u.x*=R.x/R.y;u.x-=.5;u/=Z;u-=C;
	vec2 z=vec2(0);
	int s=0;
	for (int i=0;i<200;i++) {
		z = vec2(z.x*z.x-z.y*z.y, 2.*z.x*z.y);
		z += u;
		if (dot(z,z)>4.) break;
		s++;
	}
	c = mix(bg,fg,float(s)/200.);
}
)";
bool compile_shader() {
	GLuint vs, gs, fs;
	auto cs = [](char* s, GLuint& sn, GLenum stype) {
		sn = glCreateShader(stype);
		glShaderSource(sn, 1, &s, NULL);
		glCompileShader(sn);
		GLint isCompiled = 0;
		glGetShaderiv(sn, GL_COMPILE_STATUS, &isCompiled);
		if(isCompiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(sn, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> errorLog(maxLength);
			glGetShaderInfoLog(sn, maxLength, &maxLength, &errorLog[0]);
			for (auto c : errorLog)
				cout << c;
			cout << endl;
			glDeleteShader(sn);
			return false;
		}
	};
	cs((char*)VERT.data(), vs, GL_VERTEX_SHADER);
	cs((char*)GEOM.data(), gs, GL_GEOMETRY_SHADER);
	cs((char*)FRAG.data(), fs, GL_FRAGMENT_SHADER);
	GLuint gfx_program = glCreateProgram();
	glAttachShader(gfx_program, gs);
	glAttachShader(gfx_program, fs);
	glAttachShader(gfx_program, vs);
	glLinkProgram(gfx_program);
	GLint isLinked = 0;
	glGetProgramiv(gfx_program, GL_LINK_STATUS, (int *)&isLinked);
	if(isLinked == GL_FALSE) {
		GLint maxLength = 0;
		glGetProgramiv(gfx_program, GL_INFO_LOG_LENGTH, &maxLength);
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(gfx_program, maxLength, &maxLength, &infoLog[0]);
		for (auto c : infoLog)
			cout << c;
		cout << endl;
		glDeleteProgram(gfx_program);
		glDeleteShader(vs);
		glDeleteShader(fs);
		glDeleteShader(gs);
		return false;
	}
	return true;
}
int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(wwidth, wheight, "float me", NULL, NULL);
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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glClearColor(0., 0., 0., 1.);
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	int point_indices[1]={0};
	glBufferData(GL_ARRAY_BUFFER, sizeof(int), point_indices, GL_STATIC_DRAW);
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glVertexAttribIPointer(0, 1, GL_INT, 0, NULL);
	GLint resolution_U = glGetUniformLocation(gfx_program, "R");
	GLint center_U = glGetUniformLocation(gfx_program, "C");
	GLint zoom_U = glGetUniformLocation(gfx_program, "Z");
	glUseProgram(gfx_program);
	while (!glfwWindowShouldClose(window)) {
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
