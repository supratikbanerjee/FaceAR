#include <windows.h>
#include <vector>
#include <glad/glad.h>
#include <iostream>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <Eigen/Sparse>
#include <Eigen/Dense>
#include <GLFW/glfw3.h>
#include "Calibration.h"
#include "LandmarkDetector.h"
#include "Shader.h"
#include "Camera.h"
#include "ObjectLoader.h"
#include <future>

int width = 800;
int height = 450;

float* weight;

std::vector<unsigned int> indices;
std::vector < float > vertex_count;


GLuint positionID;
GLuint normalID;
GLuint vp_vbo1 = 0;
GLuint vn_vbo1 = 0;
GLuint EBO = 0;
GLuint vao = 0;

unsigned int ModelCount = 0;
std::vector < glm::mat4 > local;
std::vector < glm::mat4 > global;
std::vector < glm::vec3 > objc;

glm::vec3 lpos;

ObjectLoader load;
Calibration calibrate;
Camera camera(glm::vec3(0.0f, 20.0f, 60.0f));


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
	{
		calibrate.isCalibrate = true;
		//calibrate.Calibrated = false;
		printf("Starting Calibration\n");
	}
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
	{
		calibrate.isCalibrate = false;
		printf("Stoping Calibration\n");
	}
}


void generateObjectBuffer(Eigen::VectorXf &blendfaces, Eigen::VectorXf &Nblendfaces, Shader &shader)
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vp_vbo1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo1);
	glBufferData(GL_ARRAY_BUFFER, blendfaces.size() * sizeof(float), blendfaces.data(), GL_DYNAMIC_DRAW);

	glGenBuffers(1, &vn_vbo1);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo1);
	glBufferData(GL_ARRAY_BUFFER, Nblendfaces.size() * sizeof(float), Nblendfaces.data(), GL_DYNAMIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	shader.setAttribVec3(positionID, vp_vbo1);
	shader.setAttribVec3(normalID, vn_vbo1);

	glBindVertexArray(0);
}


void updateObjectBuffer(Eigen::VectorXf &blendfaces, Eigen::VectorXf &Nblendfaces, Shader &shader)
{
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo1);
	glBufferData(GL_ARRAY_BUFFER, blendfaces.size() * sizeof(float), blendfaces.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo1);
	glBufferData(GL_ARRAY_BUFFER, Nblendfaces.size() * sizeof(float), Nblendfaces.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	shader.setAttribVec3(positionID, vp_vbo1);
	shader.setAttribVec3(normalID, vn_vbo1);
}

void blendFaces(Eigen::VectorXf &blendfaces, Eigen::MatrixXf &facesXf, Eigen::VectorXf &weightsXf) 
{
	blendfaces = facesXf.transpose() * weightsXf;
}

void NblendFaces(Eigen::VectorXf &Nblendfaces, Eigen::MatrixXf &facesXf, Eigen::VectorXf &weightsXf) 
{
	Nblendfaces = facesXf.transpose() * weightsXf;
}

void manageScene(float* weight)
{
	for (int i = 0; i < ModelCount; i++)
		global[i] = local[i];
	//std::cout << weight[7] << std::endl;
	local[0] = glm::rotate(glm::mat4(1.0f), weight[7], glm::vec3(0.0f, 1.0f, 0.0f));
	local[0] = glm::rotate(local[0], weight[8], glm::vec3(1.0f, 0.0f, 0.0f));
	local[0] = glm::rotate(local[0], weight[9], glm::vec3(0.0f, 0.0f, 1.0f));
	global[0] = local[0];
}

void display(Shader &shader, float* weight) {

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader.use();
	glBindVertexArray(vao);

	float rough = 1.0f;

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 persp_proj = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);

	shader.setVec3("camView", camera.GetCameraPosition());
	rough = 0.15f;
	shader.setFloat("shine_rough", rough);
	shader.setMat4("proj", persp_proj);
	shader.setMat4("view", view);

	for (int i = 0; i < ModelCount; i++)
	{
		shader.setVec3("objColor", objc[i]);
		manageScene(weight);
		shader.setMat4("model", global[i]);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		//glDrawArrays(GL_POINT, index_p,0);
	}
}

void init(Shader &shader, Eigen::VectorXf &out_blendWeights, Eigen::MatrixXf &out_Blendshapes, Eigen::MatrixXf &out_BlendshapesNormals, Eigen::VectorXf& out_Neutral)
{
	shader.use();
	positionID = shader.getAttribLocation("vertex_position1");
	normalID = shader.getAttribLocation("vertex_normals1");
	lpos = glm::vec3(0.0f, 20.0f, 20.0f);
	shader.setVec3("position", lpos);
	//GLuint shaderProgramID = CompileShaders();

	const char *c_obj[] = { "cubet.obj" };
	/*const char *obj[] = { "neutral.obj","Mery_jaw_open.obj", "Mery_kiss.obj", "Mery_l_brow_lower.obj", \
  "Mery_l_brow_narrow.obj", "Mery_l_brow_raise.obj", "Mery_l_eye_closed.obj", "Mery_l_eye_lower_open.obj",\
  "Mery_l_eye_upper_open.obj", "Mery_l_nose_wrinkle.obj","Mery_l_puff.obj", "Mery_l_sad.obj", "Mery_l_smile.obj",\
  "Mery_l_suck.obj","Mery_r_brow_lower.obj", "Mery_r_brow_narrow.obj", "Mery_r_brow_raise.obj", \
  "Mery_r_eye_closed.obj", "Mery_r_eye_lower_open.obj","Mery_r_eye_upper_open.obj","Mery_r_nose_wrinkle.obj",\
  "Mery_r_puff.obj","Mery_r_sad.obj","Mery_r_smile.obj","Mery_r_suck.obj" };*/
	const char *obj[] = { "neutral.obj","Mery_l_brow_raise.obj", "Mery_r_brow_raise.obj", "Mery_jaw_open.obj", "Mery_l_smile.obj" , "Mery_r_smile.obj", "Mery_l_eye_closed.obj", "Mery_r_eye_closed.obj"};
	//objc.push_back(glm::vec3(0.95, 0.45, 0.25));
	objc.push_back(glm::vec3(1.0, 1.0, 1.0));
	objc.push_back(glm::vec3(1.0, 0.0, 0.0));
	unsigned int objs = (sizeof(obj) / sizeof(obj[0]));

	ModelCount = 1;

	std::vector <std::vector< glm::vec2 >> uvs;
	std::vector <std::vector< glm::vec3 >> vertices;
	std::vector <std::vector< glm::vec3 >> normals;

	std::vector< glm::vec3 > vert_temp;
	std::vector< glm::vec2 > uv_temp;
	std::vector< glm::vec3 > norm_temp;

	std::vector< glm::vec3 > vert_out;
	std::vector< glm::vec2 > uv_out;
	std::vector< glm::vec3 > norm_out;


	for (int i = 0; i < objs; i++)
	{
		global.push_back(glm::mat4(1.0f));
		local.push_back(glm::mat4(1.0f));
		//local[i] = glm::scale(local[i], glm::vec3(0.05f, 0.05f, 0.05f));
		bool res = load.loadOBJ(obj[i], vert_temp, uv_temp, norm_temp);
		indices.clear();
		load.indexVBO(vert_temp, uv_temp, norm_temp, indices, vert_out, uv_out, norm_out);
		vertices.push_back(vert_out);
		uvs.push_back(uv_out);
		normals.push_back(norm_out);
		// std::cout << vert_temp.size() << std::endl;
		vert_temp.clear();
		uv_temp.clear();
		norm_temp.clear();
		vert_out.clear();
		uv_out.clear();
		norm_out.clear();
		//std::cout << vert_temp.size() << std::endl;
		vertex_count.push_back(vertices[0].size());
		//printf("count %d\n", vertices[i].size());
	}
	printf("%d\n", indices.size());

	Eigen::VectorXf blendWeights(objs);
	Eigen::MatrixXf Blendshapes(objs, (int)vertex_count[0] * 3);
	Eigen::MatrixXf BlendshapesNormals(objs, (int)vertex_count[0] * 3);
	Eigen::VectorXf neutralV((int)vertex_count[0] * 3);
	Eigen::VectorXf neutralN((int)vertex_count[0] * 3);

	for (int i = 0; i < objs; i++)
		blendWeights(i) = 0;
	blendWeights(0) = 1.0;

	int v = 0;

	for (int i = 0; i < vertex_count[0] * 3; i += 3)
	{
		neutralV(i) = Blendshapes(0, i) = vertices[0][v].x;
		neutralV(i + 1) = Blendshapes(0, i + 1) = vertices[0][v].y;
		neutralV(i + 2) = Blendshapes(0, i + 2) = vertices[0][v].z;

		neutralN(i) = BlendshapesNormals(0, i) = normals[0][v].x;
		neutralN(i + 1) = BlendshapesNormals(0, i + 1) = normals[0][v].y;
		neutralN(i + 2) = BlendshapesNormals(0, i + 2) = normals[0][v].z;
		v++;
	}

	for (int k = 1; k < objs; k++)
	{
		v = 0;
		for (int i = 0; i < vertex_count[0] * 3; i += 3)
		{
			Blendshapes(k, i) = vertices[k][v].x - Blendshapes(0, i);
			Blendshapes(k, i + 1) = vertices[k][v].y - Blendshapes(0, i + 1);
			Blendshapes(k, i + 2) = vertices[k][v].z - Blendshapes(0, i + 2);

			BlendshapesNormals(k, i) = normals[k][v].x - BlendshapesNormals(0, i);
			BlendshapesNormals(k, i + 1) = normals[k][v].y - BlendshapesNormals(0, i + 1);
			BlendshapesNormals(k, i + 2) = normals[k][v].z - BlendshapesNormals(0, i + 2);
			v++;
		}
	}

	out_blendWeights = blendWeights;
	out_Blendshapes = Blendshapes;
	out_BlendshapesNormals = BlendshapesNormals;
	out_Neutral = neutralV;
	generateObjectBuffer(neutralV, neutralN, shader);
}


int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, "Xtreme Render Engine FaceAR Plugin", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "FaceAR Plugin Error" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	Shader shader("../Shaders/rta.vs", "../Shaders/oren-nayar_cook-torrance_frag.glsl");
	LandmarkDetector landmark = LandmarkDetector(&calibrate);

	Eigen::MatrixXf Blendshapes;
	Eigen::MatrixXf BlendshapesNormals;
	Eigen::VectorXf blendWeights;

	Eigen::VectorXf blendfaces;
	Eigen::VectorXf Nblendfaces;
	Eigen::VectorXf Neutral;

	glm::vec3 position;
	glm::vec3 first_pos;
	glm::vec3 last_pos;

	init(shader, blendWeights, Blendshapes, BlendshapesNormals, Neutral);

	while (!glfwWindowShouldClose(window))
	{
		//std::thread th1(landmark.Capture(), NULL);
		weight = landmark.Capture();
		//printf("%f %f\n", weight[0], weight[1]);

		glfwPollEvents();
		processInput(window);
		for (int i = 0; i < 7; i++)
		{
			weight[i] = weight[i] < 0 ? 0 : weight[i];
			weight[i] = weight[i] > 1 ? 1 : weight[i];
			blendWeights(i+1) = weight[i];
		}

		display(shader, weight);
		blendFaces(blendfaces, Blendshapes, blendWeights);
		NblendFaces(Nblendfaces, BlendshapesNormals, blendWeights);
		updateObjectBuffer(blendfaces, Nblendfaces, shader);
		glfwSwapInterval(0);
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	getchar();
	return 0;
}