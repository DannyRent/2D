#include "stdafx.h"
#include "objLoader.h"

struct verts
{
	float x, y;
	float r, g, b;
};

struct mesh
{

	verts *vertexArray;
	GLuint numTriangles, numVerticies, vertex_buffer, shaderProgram;

	// Create a rectangle whose origin is the bottom left point.
	void makeRectangle(float width, float height)
	{
		if (vertexArray != nullptr)
		{
			error_callback(0,"Mesh called MakeRectangle with vertex data already set. Deleting previous mesh data.\n");
			delete[] vertexArray;
			numTriangles = 0;
			numVerticies = 0;
			vertex_buffer = 0;
		}

		numTriangles = 2;
		numVerticies = 6;

		vertexArray = new verts[6];
		vertexArray[0] = {0    , 0     , 1, 0, 0};
		vertexArray[1] = {width, 0     , 1, 0, 0};
		vertexArray[2] = {0    , height, 0, 0, 1};
		vertexArray[3] = {0    , height, 1, 0, 0};
		vertexArray[4] = {width, height, 0, 1, 0};
		vertexArray[5] = {width, 0     , 0, 0, 1};


		//This is inneficient but w/e.
		for (int i = 0; i < 6; i++)
		{
			vertexArray[i].x -= width / 2;
		}

	}

	bool upload()
	{
		if (vertexArray == nullptr)
		{
			error_callback(0,"Mesh: upload() called without vertex data! Call ignored.\n");
			return false;
		}
		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts) * numVerticies, vertexArray, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		delete[] vertexArray;
		vertexArray = nullptr;
		return true;
	}

	bool draw()
	{
		if (vertexArray != nullptr || vertex_buffer == 0)
		{
			error_callback(0,"Mesh: draw() called with an invalid buffer, or with a populated vertexArray. Call ignored.\n");
			return false;
		}
		GLint vpos_location, vcol_location;

		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		// Specify the locations of position and color atributes used in the shader program.
		vpos_location = glGetAttribLocation(shaderProgram, "vPos");
		vcol_location = glGetAttribLocation(shaderProgram, "vCol");

		// Specify how the buffer data is formated for use by the shader program.
		glEnableVertexAttribArray(vpos_location);

		// There are two position floats, and there are five total elements in each vertex/array row.
		glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
		glEnableVertexAttribArray(vcol_location);

		// There are three color floats, and there are five total elements in each vertex/array row.
		// There is also an offset of two floats at the start of each vertex/array row.
		// This is to skip over the two position floats that begin each vertex.
		glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 2));

		glDrawArrays(GL_TRIANGLES, 0, numVerticies);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return true;
	}

	mesh(): numTriangles{0}, numVerticies{0}, vertexArray{nullptr}, vertex_buffer{0}, shaderProgram{0}
	{}


};

mesh triangleMesh_NoIndex()
{
	GLuint xDivisions = 4;
	GLuint yDivisions = 1;

	float width{1},height{1};
	float dx{width/xDivisions},dy{height/yDivisions};

	int totalTriangles = 2*(xDivisions*yDivisions);

	float curX, nextX, curY, nextY;

	mesh gridMesh;
	gridMesh.numTriangles = totalTriangles;
	gridMesh.numVerticies = totalTriangles*3;
	gridMesh.vertexArray = new verts[gridMesh.numVerticies];


	int ithVertex = 0;
	for (int yi {0}; yi < yDivisions; yi++)
	{
		for (int xi {0}; xi < xDivisions; xi++)
		{
			curX = (xi)*dx;
			nextX = (xi+1)*dx;

			curY = (yi)*dy;
			nextY = (yi+1)*dy;

			/*---------------------- Left triangle of square ---------------------*/
			fprintf(stderr, "[%d,%d | %d](%f,%f) ", xi,yi, ithVertex,curX, curY);
			gridMesh.vertexArray[ithVertex++] = {curX, curY,1,0,0};

			// [Duplicate Vertex 1]
			fprintf(stderr, "(%f,%f) ", curX, nextY);
			gridMesh.vertexArray[ithVertex++] = {curX, nextY,0,0,1};

			//[Duplicate Vertex 2]
			fprintf(stderr, "(%f,%f)\n", nextX, curY);
			gridMesh.vertexArray[ithVertex++] = {nextX, curY,0,1,0};



			/*---------------------- Right triangle of square ---------------------*/
			// [Duplicate Vertex 1]
			fprintf(stderr, "[%d,%d | %d](%f,%f) ", xi, yi, ithVertex, curX, nextY);
			gridMesh.vertexArray[ithVertex++] = {curX, nextY,0,1,0};

			//[Duplicate Vertex 2]
			fprintf(stderr, "(%f,%f) ", nextX, curY);
			gridMesh.vertexArray[ithVertex++] = {nextX, curY,0,0,1};

			fprintf(stderr, "(%f,%f)\n", nextX, nextY);
			gridMesh.vertexArray[ithVertex++] = {nextX, nextY,1,0,0};
		}

	}

	return gridMesh;
}

mesh triangleMesh_Index()
{
	GLuint xDivisions = 2;
	GLuint yDivisions = 2;

	float width{1}, height{1};
	float dx{width / xDivisions}, dy{height / yDivisions};

	int totalTriangles = 2 * (xDivisions*yDivisions);

	float curX, nextX, curY, nextY;

	mesh gridMesh;
	gridMesh.numTriangles = totalTriangles;
	gridMesh.numVerticies = totalTriangles * 2;
	gridMesh.vertexArray = new verts[gridMesh.numVerticies];

	GLuint *indexArray = new GLuint[totalTriangles*3];


	// YOU NEED SEPERATE INDEX AND VERTEX COUNTS FOR THE VERTEX AND INDEX ARRAY!!!!
	// THE INDEX COUNT INCREASES FASTER THAN THE VERTEX COUNT, AND YOU LEAVE DEAD SPACE IN THE VERTEX ARRAY!
	int ithVertex = 0;
	int ithIndex = 0;

	for (int yi{0}; yi < yDivisions; yi++)
	{
		for (int xi{0}; xi < xDivisions; xi++)
		{
			curX = (xi)*dx;
			nextX = (xi + 1)*dx;

			curY = (yi)*dy;
			nextY = (yi + 1)*dy;

			/*---------------------- Left triangle of square ---------------------*/
			indexArray[ithIndex++] = ithVertex;
			gridMesh.vertexArray[ithVertex++] = {curX, curY,1,0,0};

			
			// [Duplicate Vertex 1]
			indexArray[ithIndex++] = ithVertex;
			gridMesh.vertexArray[ithVertex++] = {curX, nextY,0,0,1};

			
			//[Duplicate Vertex 2]
			indexArray[ithIndex++] = ithVertex;
			gridMesh.vertexArray[ithVertex++] = {nextX, curY,0,1,0};
	

			// [Duplicate Vertex 1]
			indexArray[ithIndex++] = ithIndex-2;

			//[Duplicate Vertex 2]
			indexArray[ithIndex++] = ithIndex-2;
			
			/*---------------------- Right triangle of square ---------------------*/
			// [Duplicate Vertex 1]
			//fprintf(stderr, "[%d,%d | %d](%f,%f) ", xi, yi, ithVertex, curX, nextY);
			//gridMesh.vertexArray[ithVertex++] = {curX, nextY,0,1,0};

			//[Duplicate Vertex 2]
			//fprintf(stderr, "(%f,%f) ", nextX, curY);
			//gridMesh.vertexArray[ithVertex++] = {nextX, curY,0,0,1};
			
			indexArray[ithIndex++] = ithVertex;
			gridMesh.vertexArray[ithVertex++] = {nextX, nextY,1,0,0};

		}

	}

	fprintf(stderr, "( %d", indexArray[0]);
	for(int i = 1; i < totalTriangles * 3; i++)
	{
		if (i % 3)
		{
			fprintf(stderr, " %d", indexArray[i]);
		}
		else
		{
			fprintf(stderr, " ) ( %d", indexArray[i]);
		}	
	}
	fprintf(stderr, ")\n\n");



	fprintf(stderr, "( %f", gridMesh.vertexArray[0]);
	for (int i = 1; i < totalTriangles * 2; i++)
	{
		if (i % 3)
		{
			fprintf(stderr, " %f", gridMesh.vertexArray[i]);
		}
		else
		{
			fprintf(stderr, " ) ( %f", gridMesh.vertexArray[i]);
		}
	}
	fprintf(stderr, ")\n\n");


	return gridMesh;
}

struct globalData
{
	GLFWwindow* window;
	GLuint vertex_shader, fragment_shader, program;
	GLint mvp_location;

	// Mainloop variables.
	float ratio;
	int width, height;
	mat4x4 m, p, mvp;
};

globalData globals;

void init(),simpleInit(),draw(),mainloop();


void init()
{
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	globals.window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);


	if (!globals.window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwSetKeyCallback(globals.window, key_callback);
	glfwMakeContextCurrent(globals.window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);


	/*---------------------- Shader Setup ---------------------*/
	// Vertex (position)
	{
		globals.vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(globals.vertex_shader, 1, &vertex_shader_text, NULL);
		glCompileShader(globals.vertex_shader);
	}

	// Fragment (color pixel)
	{
		globals.fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(globals.fragment_shader, 1, &fragment_shader_text, NULL);
		glCompileShader(globals.fragment_shader);
	}


	// Shader program
	{
		globals.program = glCreateProgram();
		glAttachShader(globals.program, globals.vertex_shader);
		glAttachShader(globals.program, globals.fragment_shader);
		glLinkProgram(globals.program);
	}


	// Specify the locations of position and color atributes used in the shader program.
	globals.mvp_location = glGetUniformLocation(globals.program, "MVP");

	simpleInit();
}

void mainloop()
{
	glEnable(GL_DEPTH_TEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Ask for nicest perspective correction

	/*---------------------- Application Mainloop ---------------------*/
	while (!glfwWindowShouldClose(globals.window))
	{
		glUseProgram(globals.program);
		glUniformMatrix4fv(globals.mvp_location, 1, GL_FALSE, (const GLfloat*)globals.mvp);

		// Resize the window at each draw.
		{
			glfwGetFramebufferSize(globals.window, &globals.width, &globals.height);
			globals.ratio = globals.width / (float)globals.height;
			glViewport(0, 0, globals.width, globals.height);
		}

		// Clear the screen and setup the orientation of the objects in the scene.
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			mat4x4_identity(globals.m);

			mat4x4_translate(globals.m, 0, .5, -3);
			//mat4x4_rotate_Z(globals.m, globals.m, (float)glfwGetTime());
			mat4x4_rotate_Y(globals.m, globals.m, (float)glfwGetTime());
			//mat4x4_ortho(globals.p, -globals.ratio, globals.ratio, -1.f, 1.f, 15.f, -10.f);
			mat4x4_perspective(globals.p, 45, (float)globals.width / (float)globals.height, 0.001, 5);
			mat4x4_mul(globals.mvp, globals.p, globals.m);
			// Draw
			draw();
		}


		glfwSwapBuffers(globals.window);
		glfwPollEvents();
	}
	glfwDestroyWindow(globals.window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}



vector<objObject> obj;

GLuint vertex_buffer, IndexBufferId;
void simpleInit()
{
	loadObject(std::string(DIR_ROOT) + "data\\mesh\\smallBread.obj", obj);
	obj[0].consolidate();

	//bool upload()
		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* 8 * obj[0].compiledVertex.size() , &obj[0].compiledVertex[0], GL_STATIC_DRAW);
		
		
			GLint vpos_location, vcol_location;
			vpos_location = glGetAttribLocation(globals.program, "vPos");
			vcol_location = glGetAttribLocation(globals.program, "vCol");

			glEnableVertexAttribArray(vpos_location);
			glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (void*)0);

			glEnableVertexAttribArray(vcol_location);
			glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (void*)(sizeof(GLfloat) * 5));

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &IndexBufferId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj[0].indexList.size()*sizeof(GLuint), &obj[0].indexList[0], GL_STATIC_DRAW);
	


		//glDrawArrays(GL_TRIANGLES, 0, numVerticies);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void draw()
{
	//bool draw()

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId);
	
	glClearColor(1, 1, 1, 1);

	// Draw the triangles !
	glDrawElements(
		GL_TRIANGLES,      // mode
		obj[0].indexList.size(),    // count
		GL_UNSIGNED_INT ,   // type
		(void*)0           // element array buffer offset
	);
}



// One: Window, shader program.
int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	//RUN_ALL_TESTS();

	init();
	mainloop();

	return 0;
}