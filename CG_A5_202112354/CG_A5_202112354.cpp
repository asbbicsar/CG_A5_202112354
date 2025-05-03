//
//  sphere_scene.c
//  Rasterizer
//

#include <Windows.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/freeglut.h>

#define GLFW_INCLUDE_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <stdio.h>
#include <math.h>
#define M_PI 3.14159265358979

using namespace glm;

int     gNumVertices = 0;    // Number of 3D vertices.
int     gNumTriangles = 0;    // Number of triangles.
int* gIndexBuffer = NULL; // Vertex indices for the triangles.
vec3* gVertexArray = NULL;

int Width = 512;
int Height = 512;
std::vector<float> OutputImage;

mat4 M_m_scale = mat4{
	2.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 2.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 2.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};
mat4 M_m_translate = mat4{
	1.0f, 0.0f,  0.0f, 0.0f,
	0.0f, 1.0f,  0.0f, 0.0f,
	0.0f, 0.0f,  1.0f, 0.0f,
	0.0f, 0.0f,	-7.0f, 1.0f
};
mat4 M_m = M_m_translate * M_m_scale;

mat4 M_cam_inverse = mat4(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
);
mat4 M_cam = glm::inverse(M_cam_inverse);

float l = -0.1f, r = 0.1f, b = -0.1f, t = 0.1f, n = -0.1f, f = -1000.0f;
mat4 M_P = mat4{
	n,		0.0f,	0.0f,	0.0f,
	0.0f,	n,		0.0f,	0.0f,
	0.0f,	0.0f,	n + f,	1.0f,
	0.0f,	0.0f,	-f * n,	0.0f
};
mat4 M_orth = mat4{
	2 / (r - l),		0.0f,			0.0f,			0.0f,
	0.0f,			2 / (t - b),		0.0f,			0.0f,
	0.0f,			0.0f,			2 / (n - f),		0.0f,
	-(r + l) / (r - l),	-(t + b) / (t - b),	-(n + f) / (n - f),	1.0f
};
mat4 M_pers = M_orth * M_P;

mat4 M_vp = mat4{
	Width / 2.0f,		0.0f,				0.0f,	0.0f,
	0.0f,			Height / 2.0f,		0.0f,	0.0f,
	0.0f,			0.0f,				1.0f,	0.0f,
	(Width - 1) / 2.0f,	(Height - 1) / 2.0f,	0.0f,	1.0f
};

mat4 M = M_vp * M_pers * M_cam * M_m;

void create_scene()
{
	int width = 32;
	int height = 16;

	float theta, phi;
	int t;

	gNumVertices = (height - 2) * width + 2;
	gNumTriangles = (height - 2) * (width - 1) * 2;

	// Allocate an array for gNumVertices vertices.
	gVertexArray = new vec3[gNumVertices];
	gIndexBuffer = new int[3 * gNumTriangles];

	t = 0;
	for (int j = 1; j < height - 1; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			theta = (float)j / (height - 1) * M_PI;
			phi = (float)i / (width - 1) * M_PI * 2;

			float   x = sinf(theta) * cosf(phi);
			float   y = cosf(theta);
			float   z = -sinf(theta) * sinf(phi);

			// TODO: Set vertex t in the vertex array to {x, y, z}.
			gVertexArray[t].x = x;
			gVertexArray[t].y = y;
			gVertexArray[t].z = z;
			t++;
		}
	}

	// Set vertex t in the vertex array to {0, 1, 0}.
	gVertexArray[t].x = 0.0f;
	gVertexArray[t].y = 1.0f;
	gVertexArray[t].z = 0.0f;
	t++;

	// Set vertex t in the vertex array to {0, -1, 0}.
	gVertexArray[t].x = 0.0f;
	gVertexArray[t].y = -1.0f;
	gVertexArray[t].z = 0.0f;
	t++;

	t = 0;
	for (int j = 0; j < height - 3; ++j)
	{
		for (int i = 0; i < width - 1; ++i)
		{
			gIndexBuffer[t++] = j * width + i;
			gIndexBuffer[t++] = (j + 1) * width + (i + 1);
			gIndexBuffer[t++] = j * width + (i + 1);
			gIndexBuffer[t++] = j * width + i;
			gIndexBuffer[t++] = (j + 1) * width + i;
			gIndexBuffer[t++] = (j + 1) * width + (i + 1);
		}
	}
	for (int i = 0; i < width - 1; ++i)
	{
		gIndexBuffer[t++] = (height - 2) * width;
		gIndexBuffer[t++] = i;
		gIndexBuffer[t++] = i + 1;
		gIndexBuffer[t++] = (height - 2) * width + 1;
		gIndexBuffer[t++] = (height - 3) * width + (i + 1);
		gIndexBuffer[t++] = (height - 3) * width + i;
	}

	// The index buffer has now been generated. Here's how to use to determine
	// the vertices of a triangle. Suppose you want to determine the vertices
	// of triangle i, with 0 <= i < gNumTriangles. Define:
	//
	// k0 = gIndexBuffer[3*i + 0]
	// k1 = gIndexBuffer[3*i + 1]
	// k2 = gIndexBuffer[3*i + 2]
	//
	// Now, the vertices of triangle i are at positions k0, k1, and k2 (in that
	// order) in the vertex array (which you should allocate yourself at line
	// 27).
	//
	// Note that this assumes 0-based indexing of arrays (as used in C/C++,
	// Java, etc.) If your language uses 1-based indexing, you will have to
	// add 1 to k0, k1, and k2.
}

void render()
{
	//Create our image. We don't want to do this in 
	//the main loop since this may be too slow and we 
	//want a responsive display of our beautiful image.
	//Instead we draw to another buffer and copy this to the 
	//framebuffer using glDrawPixels(...) every refresh
	OutputImage.clear();
	create_scene();

	std::vector<std::vector<float>> zbuffer(Width, std::vector<float>(Height, std::numeric_limits<float>::infinity()));
	std::vector<std::vector<vec3>> framebuffer(Width, std::vector<vec3>(Height));


	for (int j = 0; j < Height; ++j)
	{
		for (int i = 0; i < Width; ++i)
		{
			framebuffer[i][j] = vec3(0.0f, 0.0f, 0.0f);
			zbuffer[i][j] = std::numeric_limits<float>::infinity();
		}
	}

	for (int tri = 0; tri < gNumTriangles; ++tri)
	{
		int k0 = gIndexBuffer[3 * tri + 0];
		int k1 = gIndexBuffer[3 * tri + 1];
		int k2 = gIndexBuffer[3 * tri + 2]; //get indices

		vec4 v0 = M * vec4(gVertexArray[k0].x, gVertexArray[k0].y, gVertexArray[k0].z, 1.0f);
		vec4 v1 = M * vec4(gVertexArray[k1].x, gVertexArray[k1].y, gVertexArray[k1].z, 1.0f);
		vec4 v2 = M * vec4(gVertexArray[k2].x, gVertexArray[k2].y, gVertexArray[k2].z, 1.0f); //get vertices

		v0 /= v0.w;
		v1 /= v1.w;
		v2 /= v2.w;

		int xmin = floor(std::min({ v0.x, v1.x, v2.x }));
		int xmax = ceil(std::max({ v0.x, v1.x, v2.x }));
		int ymin = floor(std::min({ v0.y, v1.y, v2.y }));
		int ymax = ceil(std::max({ v0.y, v1.y, v2.y }));  //set range of rasterization

		mat2 A_2(
			v1.x - v0.x, v1.y - v0.y,
			v2.x - v0.x, v2.y - v0.y
		);

		vec2 dx = inverse(A_2) * vec2(1.0f, 0.0f);
		vec2 dy = inverse(A_2) * vec2(0.0f, 1.0f);
		int n = xmax - xmin + 1;
		vec2 origin = vec2(xmin + 0.5f - v0.x, ymin + 0.5f - v0.y);
		vec2 sol = inverse(A_2) * origin;
		float beta = sol.x;
		float gamma = sol.y;
		float alpha = 1.0f - beta - gamma; //calculate dx, dy about beta, gamma by linear system

		for (int y = ymin; y <= ymax; ++y) {
			for (int x = xmin; x <= xmax; ++x) {
				if (beta >= 0 && gamma >= 0 && alpha >= 0) {
					float z = alpha * v0.z + beta * v1.z + gamma * v2.z;
					if (x >= 0 && y >= 0 && std::abs(z) < std::abs(zbuffer[x][y])) { //apply zbuffer
						zbuffer[x][y] = z;
						framebuffer[x][y] = vec3(1.0f, 1.0f, 1.0f);
					}
				}
				beta += dx.x;
				gamma += dx.y;
				alpha = 1.0f - beta - gamma;
			}
			beta += (dy.x - n * dx.x);
			gamma += (dy.y - n * dx.y);
			alpha = 1.0f - beta - gamma;
		}
	}
	for (int j = 0; j < Height; ++j)
	{
		for (int i = 0; i < Width; ++i)
		{
			vec3 color = framebuffer[i][j];
			OutputImage.push_back(color.x);
			OutputImage.push_back(color.y);
			OutputImage.push_back(color.z);
		}
	}
}

void resize_callback(GLFWwindow*, int nw, int nh)
{
	//This is called in response to the window resizing.
	//The new width and height are passed in so we make 
	//any necessary changes:
	Width = nw;
	Height = nh;
	//Tell the viewport to use all of our screen estate
	glViewport(0, 0, nw, nh);

	//This is not necessary, we're just working in 2d so
	//why not let our spaces reflect it?
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0, static_cast<double>(Width)
		, 0.0, static_cast<double>(Height)
		, 1.0, -1.0);

	//Reserve memory for our render so that we don't do 
	//excessive allocations and render the image
	OutputImage.reserve(Width * Height * 3);
	render();
}


int main(int argc, char* argv[])
{
	// -------------------------------------------------
	// Initialize Window
	// -------------------------------------------------

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(Width, Height, "OpenGL Viewer", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	//We have an opengl context now. Everything from here on out 
	//is just managing our window or opengl directly.

	//Tell the opengl state machine we don't want it to make 
	//any assumptions about how pixels are aligned in memory 
	//during transfers between host and device (like glDrawPixels(...) )
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	//We call our resize function once to set everything up initially
	//after registering it as a callback with glfw
	glfwSetFramebufferSizeCallback(window, resize_callback);
	resize_callback(NULL, Width, Height);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		//Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);

		// -------------------------------------------------------------
		//Rendering begins!
		glDrawPixels(Width, Height, GL_RGB, GL_FLOAT, &OutputImage[0]);
		//&OutputImage[0]: viewer window
		//and ends.
		// -------------------------------------------------------------

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();

		//Close when the user hits 'q' or escape
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS
			|| glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}