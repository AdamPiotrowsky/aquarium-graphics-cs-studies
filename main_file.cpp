/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "myCube.h"
#include "myTeapot.h"

float speed_x = 0; //angular speed in radians
float speed_y = 0; //angular speed in radians
float aspectRatio = 1;
GLuint tex0;
GLuint sky; //uchwyt na teksturê
GLuint water;
GLuint top;
GLuint green;
GLuint rocks;
GLuint nemo;

ShaderProgram* sp; //Pointer to the shader program

std::vector<glm::vec4> fishVerts;
std::vector<glm::vec4> fishNorms;
std::vector<glm::vec2> fishTexCoords;
std::vector<unsigned int> fishIndices;

std::vector<glm::vec4> algaeVerts;
std::vector<glm::vec4> algaeNorms;
std::vector<glm::vec2> algaeTexCoords;
std::vector<unsigned int> algaeIndices;

std::vector<glm::vec4> fishPaperVerts;
std::vector<glm::vec4> fishPaperNorms;
std::vector<glm::vec2> fishPaperTexCoords;
std::vector<unsigned int> fishPaperIndices;

// x,y,z, isRotated, changeX, changeY, changeZ
float speed_fish[] = {
	0.3f, -0.3f, 0.0f, 1.0f, 0.015f, 0.01f, 0.004f,
	1.1f, 0.55f, 0.7f, 1.0f, 0.012f, 0.012f, 0.005f,
	2.0f, 0.4f, 0.0f, 1.0f, 0.03f, 0.01f, 0.01f,
	-1.0f, 0.0f, 0.5f, 1.0f, 0.02f, 0.007f, 0.003f,
	0.5f, 0.2f, 0.2f, 1.0f, 0.01f, 0.01f, 0.005f,
	0.4f, -0.1f, 0.3f, 1.0f, 0.025f, 0.03f, 0.002f,
	1.2f, 0.5f, 0.7f, 1.0f, 0.01f, 0.01f, 0.003f,
	1.8f, -0.5f, 0.0f, 1.0f, 0.03f, -0.008f, 0.004f,
	-1.3f, 0.0f, 0.3f, 1.0f, 0.01f, 0.02f, 0.005f,
	0.7f, 0.2f, 0.2f, 1.0f, 0.015f, 0.01f, 0.005f,
	1.4f, -0.1f, 0.3f, 1.0f, 0.025f, 0.02f, 0.002f,
	-1.1f, 0.2f, 0.7f, 1.0f, 0.01f, 0.01f, 0.003f
};
int fish_speed_length = 84;

//Uncomment to draw a cube
float* vertices = myCubeVertices;
float* texCoords = myCubeTexCoords;
float* colors = myCubeColors;
float* normals = myCubeNormals;
int vertexCount = myCubeVertexCount;


void loadModel(std::string plik, 
		std::vector<glm::vec4>& verts,
		std::vector<glm::vec4>& norms,
		std::vector<glm::vec2>& texCoords,
		std::vector<unsigned int>& indices) {
	using namespace std;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(plik, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
	cout << importer.GetErrorString() << endl;

	auto mesh = scene->mMeshes[0];

	for (int i = 0; i < mesh->mNumVertices; i++)
	{
		aiVector3D vertex = mesh->mVertices[i];
		verts.push_back(glm::vec4(vertex.x, vertex.y, vertex.z, 1));

		aiVector3D normal = mesh->mNormals[i];
		norms.push_back(glm::vec4(normal.x, normal.y, normal.z, 0));

		//unsigned int liczba_zest = mesh->GetNumUVChannels();
		//unsigned int wymiar_wsp_tex = mesh->mNumUVComponents[0];
		aiVector3D texCoord = mesh->mTextureCoords[0][i];
		texCoords.push_back(glm::vec2(texCoord.x, texCoord.y));
	}

	for (int i = 0; i < mesh->mNumFaces; i++) {
		aiFace& face = mesh->mFaces[i];

		for (int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}
}

//Error processing callback procedure
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}
GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);
	//Wczytanie do pamiêci komputera
	std::vector<unsigned char> image; //Alokuj wektor do wczytania obrazka
	unsigned width, height; //Zmienne do których wczytamy wymiary obrazka
	//Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);
	//Import do pamiêci karty graficznej
	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pamiêci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return tex;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT) speed_x = -PI / 2;
		if (key == GLFW_KEY_RIGHT) speed_x = PI / 2;
		if (key == GLFW_KEY_UP) speed_y = PI / 2;
		if (key == GLFW_KEY_DOWN) speed_y = -PI / 2;
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT) speed_x = 0;
		if (key == GLFW_KEY_RIGHT) speed_x = 0;
		if (key == GLFW_KEY_UP) speed_y = 0;
		if (key == GLFW_KEY_DOWN) speed_y = 0;
	}
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	if (height == 0) return;
	aspectRatio = (float)width / (float)height;
	glViewport(0, 0, width, height);
}


//Initialization code procedure
void initOpenGLProgram(GLFWwindow* window) {
	//************Place any code here that needs to be executed once, at the program start************
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	tex0 = readTexture("tiger.png");
	sky = readTexture("tiger.png");
	water = readTexture("woda.png");
	top = readTexture("gora.png");
	green = readTexture("green1.png");
	rocks = readTexture("rocks.png");
	nemo = readTexture("fish_pic1.png");
	sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");
	loadModel("algae.glb", algaeVerts, algaeNorms, algaeTexCoords, algaeIndices);
	loadModel("ryba_test.glb", fishVerts, fishNorms, fishTexCoords, fishIndices);
	//loadModel("zolw.glb", grassVerts, grassNorms, grassTexCoords, grassIndices);
}

//Release resources allocated by the program
void freeOpenGLProgram(GLFWwindow* window) {
	//************Place any code here that needs to be executed once, after the main loop ends************
	delete sp;
}

void drawFish(glm::mat4 M, float angle_x, float angle_y, float angle_z, float rotateFish_x, GLuint tex) {
	glm::mat4 M1;

	glEnableVertexAttribArray(sp->a("vertex")); //Enable sending data to the attribute vertex
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, fishVerts.data()); //Specify source of the data for the attribute vertex

	glEnableVertexAttribArray(sp->a("normal")); //Enable sending data to the attribute color
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, fishNorms.data()); //Specify source of the data for the attribute normal

	glEnableVertexAttribArray(sp->a("texCoord0"));
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, fishTexCoords.data());//odpowiednia tablica

	//FOR OPACITY
	glEnableVertexAttribArray(sp->a("color")); //Enable sending data to the attribute color
	glVertexAttribPointer(sp->a("color"), 4, GL_FLOAT, false, 0, myTeapotColors); //Specify source of the data for the attribute color

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex);




	M1 = glm::translate(M, glm::vec3(angle_x, angle_y, angle_z));
	M1 = glm::rotate(M1, PI * rotateFish_x, glm::vec3(0.0f, 1.0f, 0.0f));
	M1 = glm::scale(M1, glm::vec3(0.02f, 0.02f, 0.02f));


	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M1));
	//glDrawArrays(GL_TRIANGLES, 0, myTeapotVertexCount); //Draw the object
	glDrawElements(GL_TRIANGLES, fishIndices.size(), GL_UNSIGNED_INT, fishIndices.data());

	glDisableVertexAttribArray(sp->a("vertex")); //Disable sending data to the attribute vertex
	glDisableVertexAttribArray(sp->a("color")); //Disable sending data to the attribute color
	glDisableVertexAttribArray(sp->a("normal")); //Disable sending data to the attribute normal
	glDisableVertexAttribArray(sp->a("texCoord0"));
}

void drawAlgaeFromM(glm::mat4 M) {

	glEnableVertexAttribArray(sp->a("vertex")); //Enable sending data to the attribute vertex
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, algaeVerts.data()); //Specify source of the data for the attribute vertex

	glEnableVertexAttribArray(sp->a("normal")); //Enable sending data to the attribute color
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, algaeNorms.data()); //Specify source of the data for the attribute normal

	glEnableVertexAttribArray(sp->a("texCoord0"));
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, algaeTexCoords.data());//odpowiednia tablica

	//FOR OPACITY
	glEnableVertexAttribArray(sp->a("color")); //Enable sending data to the attribute color
	glVertexAttribPointer(sp->a("color"), 4, GL_FLOAT, false, 0, myTeapotColors); //Specify source of the data for the attribute color

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, green);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, green);

	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));
	glDrawElements(GL_TRIANGLES, algaeIndices.size(), GL_UNSIGNED_INT, algaeIndices.data());

	glDisableVertexAttribArray(sp->a("vertex")); //Disable sending data to the attribute vertex
	glDisableVertexAttribArray(sp->a("color")); //Disable sending data to the attribute color
	glDisableVertexAttribArray(sp->a("normal")); //Disable sending data to the attribute normal
	glDisableVertexAttribArray(sp->a("texCoord0"));

	/*glEnableVertexAttribArray(sp->a("vertex")); //Enable sending data to the attribute vertex
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, algaeVerts.data()); //Specify source of the data for the attribute vertex

	glEnableVertexAttribArray(sp->a("normal")); //Enable sending data to the attribute color
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, algaeNorms.data()); //Specify source of the data for the attribute normal

	glEnableVertexAttribArray(sp->a("texCoord0"));
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, algaeTexCoords.data());//odpowiednia tablica

	//FOR OPACITY
	glEnableVertexAttribArray(sp->a("color")); //Enable sending data to the attribute color
	glVertexAttribPointer(sp->a("color"), 4, GL_FLOAT, false, 0, myTeapotColors); //Specify source of the data for the attribute color

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, green);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, green);

	M1 = glm::translate(backupM, glm::vec3(-1.0f, -1.4f, 0.0f));
	M1 = glm::scale(M1, glm::vec3(0.02f, 0.02f, 0.02f));


	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M1));
	//glDrawArrays(GL_TRIANGLES, 0, myTeapotVertexCount); //Draw the object
	glDrawElements(GL_TRIANGLES, algaeIndices.size(), GL_UNSIGNED_INT, algaeIndices.data());

	glDisableVertexAttribArray(sp->a("vertex")); //Disable sending data to the attribute vertex
	glDisableVertexAttribArray(sp->a("color")); //Disable sending data to the attribute color
	glDisableVertexAttribArray(sp->a("normal")); //Disable sending data to the attribute normal
	glDisableVertexAttribArray(sp->a("texCoord0"));*/
}


void drawAquarium(glm::mat4 M) {

	glm::mat4 backupM = glm::translate(M, glm::vec3(0.0f, 0.0f, 0.0f));
	glm::mat4 M1;
	for (int i = 0; i < 6; i++) {
		glEnableVertexAttribArray(sp->a("vertex")); //Enable sending data to the attribute vertex
		glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, vertices); //Specify source of the data for the attribute vertex

		//FOR OPACITY
		glEnableVertexAttribArray(sp->a("color")); //Enable sending data to the attribute color
		glVertexAttribPointer(sp->a("color"), 4, GL_FLOAT, false, 0, colors); //Specify source of the data for the attribute color

		glEnableVertexAttribArray(sp->a("normal")); //Enable sending data to the attribute color
		glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, normals); //Specify source of the data for the attribute normal

		glEnableVertexAttribArray(sp->a("texCoord0"));
		glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, texCoords);//odpowiednia tablica

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, water);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, water);

		//podloga
		if (i == 0) {
			M1 = glm::translate(backupM, glm::vec3(0.0f, -1.5f, 0.0f));
			M1 = glm::scale(M1, glm::vec3(2.5f, 0.02f, 1.0f));
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, rocks);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, rocks);
			glEnableVertexAttribArray(sp->a("color")); //Enable sending data to the attribute color
			glVertexAttribPointer(sp->a("color"), 4, GL_FLOAT, false, 0, myTeapotColors); //Specify source of the data for the attribute color
		}
		//woda
		if (i == 1) {
			M1 = glm::translate(backupM, glm::vec3(0.0f, 1.2f, 0.0f));
			M1 = glm::scale(M1, glm::vec3(2.5f, 0.005f, 1.0f));
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, top);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, sky);
		}
		//sciana 1
		if (i == 2) {
			M1 = glm::translate(backupM, glm::vec3(2.5f, 0.0f, 0.0f));
			M1 = glm::scale(M1, glm::vec3(0.02f, 1.5f, 1.0f));
		}
		if (i == 3) {
			M1 = glm::translate(backupM, glm::vec3(-2.5f, 0.0f, 0.0f));
			M1 = glm::scale(M1, glm::vec3(0.02f, 1.5f, 1.0f));
		}
		if (i == 4) {
			M1 = glm::translate(backupM, glm::vec3(0.0f, 0.0f, 1.0f));
			M1 = glm::scale(M1, glm::vec3(2.5f, 1.5f, 0.02f));
		}
		if (i == 5) {
			M1 = glm::translate(backupM, glm::vec3(0.0f, 0.0f, -1.0f));
			M1 = glm::scale(M1, glm::vec3(2.5f, 1.5f, 0.02f));
		}



		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M1));
		glDrawArrays(GL_TRIANGLES, 0, vertexCount); //Draw the object

		glDisableVertexAttribArray(sp->a("vertex")); //Disable sending data to the attribute vertex
		glDisableVertexAttribArray(sp->a("color")); //Disable sending data to the attribute color
		glDisableVertexAttribArray(sp->a("normal")); //Disable sending data to the attribute normal
		glDisableVertexAttribArray(sp->a("texCoord0"));
	}

}

//Drawing procedure
void drawScene(GLFWwindow* window, float angle_x, float angle_y) {
	//************Place any code here that draws something inside the window******************l

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 V = glm::lookAt(
		glm::vec3(0.0f, 0.0f, -9.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)); //compute view matrix
	glm::mat4 P = glm::perspective(50.0f * PI / 180.0f, 1.0f, 1.0f, 50.0f); //compute projection matrix

	sp->use();//activate shading program
	//Send parameters to graphics card
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));

	glm::mat4 M = glm::mat4(1.0f);
	M = glm::rotate(M, angle_y, glm::vec3(1.0f, 0.0f, 0.0f)); //Compute model matrix
	M = glm::rotate(M, angle_x, glm::vec3(0.0f, 1.0f, 0.0f)); //Compute model matrix
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));
	glUniform1i(sp->u("textureMap0"), 0); //drawScene
	glUniform1i(sp->u("textureMap1"), 1); //drawScene

	//drawAlgae(M);

	for (int i = 0; i < fish_speed_length; i += 7) {
		GLuint tex;
		if (i / 7 == 0 || i / 7 == 1 || i / 7 == 2) tex = top;
		if (i / 7 == 3 || i / 7 == 4 || i / 7 == 5) tex = nemo;
		if (i / 7 == 6 || i / 7 == 7 || i / 7 == 8) tex = tex0;
		if (i / 7 == 9 || i / 7 == 10 || i / 7 == 11) tex = green;

		drawFish(M, speed_fish[i], speed_fish[i + 1], speed_fish[i + 2], speed_fish[i + 3], tex );
	}

	glm::mat4 M1 = glm::translate(M, glm::vec3(1.5f, -1.4f, 0.3f));
	M1 = glm::scale(M1, glm::vec3(0.02f, 0.02f, 0.02f));
	drawAlgaeFromM(M1);

	M1 = glm::translate(M, glm::vec3(-0.7f, -1.4f, -0.5f));
	M1 = glm::scale(M1, glm::vec3(0.01f, 0.01f, 0.01f));
	drawAlgaeFromM(M1);

	M1 = glm::translate(M, glm::vec3(-1.3f, -1.4f, 0.4f));
	M1 = glm::scale(M1, glm::vec3(0.015f, 0.015f, 0.015f));
	drawAlgaeFromM(M1);

	M1 = glm::translate(M, glm::vec3(0.1f, -1.4f, 0.4f));
	M1 = glm::scale(M1, glm::vec3(0.005f, 0.025f, 0.007f));
	drawAlgaeFromM(M1);

	M1 = glm::translate(M, glm::vec3(-1.8f, -1.4f, -0.7f));
	M1 = glm::scale(M1, glm::vec3(0.005f, 0.027f, 0.003f));
	drawAlgaeFromM(M1);

	M1 = glm::translate(M, glm::vec3(0.90f, -1.45f, -0.2f));
	M1 = glm::scale(M1, glm::vec3(0.025f, 0.001f, 0.025f));
	drawAlgaeFromM(M1);

	M1 = glm::translate(M, glm::vec3(1.9f, -1.4f, -0.7f));
	M1 = glm::scale(M1, glm::vec3(0.005f, 0.032f, 0.003f));
	drawAlgaeFromM(M1);



	//last!!!
	drawAquarium(M);



	glfwSwapBuffers(window); //Copy back buffer to front buffer
}

int main(void)
{
	GLFWwindow* window; //Pointer to object that represents the application window

	glfwSetErrorCallback(error_callback);//Register error processing callback procedure

	if (!glfwInit()) { //Initialize GLFW library
		fprintf(stderr, "Can't initialize GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Create a window 500pxx500px titled "OpenGL" and an OpenGL context associated with it.

	if (!window) //If no window is opened then close the program
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Since this moment OpenGL context corresponding to the window is active and all OpenGL calls will refer to this context.
	glfwSwapInterval(1); //During vsync wait for the first refresh

	GLenum err;
	if ((err = glewInit()) != GLEW_OK) { //Initialize GLEW library
		fprintf(stderr, "Can't initialize GLEW: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Call initialization procedure


	float angle_x = 0; //current rotation angle of the object, x axis
	float angle_y = 0; //current rotation angle of the object, y axis
	glfwSetTime(0); //Zero the timer
	//Main application loop
	float change_x = 0.01f;
	float change_y = 0.01f;
	float change_z = 0.01f;
	while (!glfwWindowShouldClose(window)) //As long as the window shouldnt be closed yet...
	{
		angle_x += speed_x * glfwGetTime(); //Add angle by which the object was rotated in the previous iteration
		angle_y += speed_y * glfwGetTime(); //Add angle by which the object was rotated in the previous iteration
		for (int i = 0; i < fish_speed_length; i += 7) {
			float speed_fish_x = speed_fish[i];
			float speed_fish_y = speed_fish[i + 1];
			float speed_fish_z = speed_fish[i + 2];
			float rotateFish_x = speed_fish[i + 3];
			float change_x = speed_fish[i + 4];
			float change_y = speed_fish[i + 5];
			float change_z = speed_fish[i + 6];

			speed_fish_x += change_x;
			speed_fish_y += change_y;
			speed_fish_z += change_z;

			if (speed_fish_x > 2.2f || speed_fish_x < -2.2f) {
				if (rotateFish_x < 0.1) rotateFish_x = 1.0f;
				else rotateFish_x = 0.0f;
				change_x = -change_x;
			}
			if (speed_fish_y > 0.8f || speed_fish_y < -0.8f) change_y = -change_y;
			if (speed_fish_z > 0.8f || speed_fish_z < -0.8f) change_z = -change_z;

			speed_fish[i] = speed_fish_x;
			speed_fish[i + 1] = speed_fish_y;
			speed_fish[i + 2] = speed_fish_z;
			speed_fish[i + 3] = rotateFish_x;
			speed_fish[i + 4] = change_x;
			speed_fish[i + 5] = change_y;
			speed_fish[i + 6] = change_z;

		}

		glfwSetTime(0); //Zero the timer
		drawScene(window, angle_x, angle_y); //Execute drawing procedure
		glfwPollEvents(); //Process callback procedures corresponding to the events that took place up to now
	}
	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Delete OpenGL context and the window.
	glfwTerminate(); //Free GLFW resources
	exit(EXIT_SUCCESS);
}
