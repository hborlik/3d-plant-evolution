/* Base code for normal mapping 476 2022 - note uses a modified Shape.h/cpp to support BTN*/
/* This code is incomplete - follow tasks listed in handout */
#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Texture.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

void initQuad();

using namespace std;
using namespace glm;


class Application : public EventCallbacks
{

public:
	WindowManager * windowManager = nullptr;

	shared_ptr<Program> DepthProg;
	shared_ptr<Program> DepthProgDebug;
	shared_ptr<Program> ShadowProg;
	shared_ptr<Program> DebugProg;

	//Our shader program - use this one for Blinn-Phong has diffuse TODO add specular
	//std::shared_ptr<Program> diffuse;

	shared_ptr<Shape> cube;

	std::vector<shared_ptr<Shape>> dummyMesh;
	vec3 gD_trans;
	float gD_scale;

	shared_ptr<Texture> texture0;
	shared_ptr<Texture> texture1;
	shared_ptr<Texture> flowers;
	shared_ptr<Texture> wood;

	int DEBUG_LIGHT = 0;
	int GEOM_DEBUG = 1;
	int g_GiboLen;
	int g_width, g_height;
	int shadow = 1;
	int FirstTime = 1;

	GLuint depthMapFBO;
	const GLuint S_WIDTH = 1024, S_HEIGHT = 1024;
	GLuint depthMap;

	float g_Camtrans = -2.5;
	vec3 g_light = vec3(1, 1, 1);
	float updateDir = 0.5;
	double g_phi, g_theta;

	float gObjRot = 0;

	vec3 view = vec3(0, 0, 1);
	vec3 strafe = vec3(1, 0, 0);
	vec3 g_eye = vec3(0, 1, 0);
	vec3 g_lookAt = vec3(0, 1, -4);

	//global data for ground plane
	GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj, GrndTanBO, GrndBNBO;
	//geometry for texture render
	GLuint quad_VertexArrayID;
	GLuint quad_vertexbuffer;

	static void error_callback(int error, const char *description) {
		cerr << description << endl;
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods) {
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
	}

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		float speed = 0.2;

		if (key == GLFW_KEY_A && action == GLFW_PRESS) {
			g_eye -= speed * strafe;
			g_lookAt -= speed * strafe;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			g_eye += speed * strafe;
			g_lookAt += speed * strafe;
		}
		if (key == GLFW_KEY_W && action == GLFW_PRESS) {
			g_eye -= speed * view;
			g_lookAt -= speed * view;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS) {
			g_eye += speed * view;
			g_lookAt += speed * view;
		}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS)
			g_light.x += 0.25;
		if (key == GLFW_KEY_E && action == GLFW_PRESS)
			g_light.x -= 0.25;
		if (key == GLFW_KEY_M && action == GLFW_PRESS)
			gObjRot += 0.25;
		if (key == GLFW_KEY_N && action == GLFW_PRESS)
			gObjRot -= 0.25;
		if (key == GLFW_KEY_L && action == GLFW_PRESS)
			DEBUG_LIGHT = !DEBUG_LIGHT;
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}


	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
		vec3 diff, newV;
		//cout << "xDel + yDel " << deltaX << " " << deltaY << endl;
		g_theta += deltaX;
		g_phi += deltaY;
		newV.x = cosf(g_phi * (3.14 / 180.0)) * cosf(g_theta * (3.14 / 180.0));
		newV.y = -1.0 * sinf(g_phi * (3.14 / 180.0));
		newV.z = 1.0 * cosf(g_phi * (3.14 / 180.0)) * cosf((90.0 - g_theta) * (3.14 / 180.0));
		diff.x = (g_lookAt.x - g_eye.x) - newV.x;
		diff.y = (g_lookAt.y - g_eye.y) - newV.y;
		diff.z = (g_lookAt.z - g_eye.z) - newV.z;
		g_lookAt.x = g_lookAt.x - diff.x;
		g_lookAt.y = g_lookAt.y - diff.y;
		g_lookAt.z = g_lookAt.z - diff.z;
		view = g_eye - g_lookAt;
		strafe = cross(vec3(0, 1, 0), view);
	}

	void resizeCallback(GLFWwindow *window, int width, int height) {
		glViewport(0, 0, width, height);
	}

	/* code to read in meshes and define the ground plane */
	void initGeom(const std::string& resourceDirectory) {

		vector<tinyobj::shape_t> TOshapes2;
		vector<tinyobj::material_t> objMaterials2;
		string errStr;
		//load in the mesh and make the shape(s)
		bool rc = tinyobj::LoadObj(TOshapes2, objMaterials2, errStr, (resourceDirectory + "/cube.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			cube = make_shared<Shape>();
			cube->createShape(TOshapes2[0]);
			cube->measure();
			//create the BTN
			cout << "the cube: " << glGetError() << endl;
			cube->init(true);
		}

		/*read in the multi-shape dummy */
		vec3 Gmin, Gmax;
		vector<tinyobj::shape_t> TOshapes1;
		vector<tinyobj::material_t> objMaterials1;
		rc = tinyobj::LoadObj(TOshapes1, objMaterials1, errStr, (resourceDirectory + "/dummy.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			Gmin = vec3(1.1754E+38F);
			Gmax = vec3(-1.1754E+38F);
			for (int i = 0; i < TOshapes1.size(); i++) {
				// Initialize each mesh.
				shared_ptr<Shape> s;
				s = make_shared<Shape>();
				s->createShape(TOshapes1[i]);
				s->measure();
				s->init(false);
				//some record keeping for global scale and translate
				if (s->min.x < Gmin.x) Gmin.x = s->min.x;
				if (s->min.y < Gmin.y) Gmin.y = s->min.y;
				if (s->min.z < Gmin.z) Gmin.z = s->min.z;
				if (s->max.x > Gmax.x) Gmax.x = s->max.x;
				if (s->max.y > Gmax.y) Gmax.y = s->max.y;
				if (s->max.z > Gmax.z) Gmax.z = s->max.z;

				dummyMesh.push_back(s);
			}
			gD_trans = Gmin + 0.5f * (Gmax - Gmin);

			if (Gmax.x > Gmax.y && Gmax.x > Gmax.z)
				gD_scale = 2.0 / (Gmax.x - Gmin.x);
			else if (Gmax.y > Gmax.x && Gmax.y > Gmax.z)
				gD_scale = 2.0 / (Gmax.y - Gmin.y);
			else
				gD_scale = 2.0 / (Gmax.z - Gmin.z);
		}

		float g_groundSize = 20;
		float g_groundY = -1.5;

		// A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
		float GrndPos[] = {
			-g_groundSize, g_groundY, -g_groundSize,
			-g_groundSize, g_groundY,  g_groundSize,
			g_groundSize, g_groundY,  g_groundSize,
			g_groundSize, g_groundY, -g_groundSize
		};

		float GrndNorm[] = {
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0
		};

		static GLfloat GrndTex[] = {
			0, 0, // back
			0, 4,
			4, 4,
			4, 0
		};

		float GTBO[] = {
			1, 0, 0,
			1, 0, 0,
			1, 0, 0,
			1, 0, 0,
			1, 0, 0,
			1, 0, 0
		};

		float GBNBO[] = {
			0, 0, -1,
			0, 0, -1,
			0, 0, -1,
			0, 0, -1,
			0, 0, -1,
			0, 0, -1
		};

		unsigned short idx[] = {0, 1, 2, 0, 2, 3};

		GLuint VertexArrayID;
		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		g_GiboLen = 6;
		glGenBuffers(1, &GrndBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

		glGenBuffers(1, &GrndNorBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);

		glGenBuffers(1, &GrndTexBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

		glGenBuffers(1, &GrndTanBO);
		glBindBuffer(GL_ARRAY_BUFFER, GrndTanBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GTBO), GTBO, GL_STATIC_DRAW);

		glGenBuffers(1, &GrndBNBO);
		glBindBuffer(GL_ARRAY_BUFFER, GrndBNBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GBNBO), GBNBO, GL_STATIC_DRAW);


		glGenBuffers(1, &GIndxBuffObj);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

		initQuad();
	}

	/**** geometry set up for a quad *****/
	void initQuad() {

		//now set up a simple quad for rendering FBO
		glGenVertexArrays(1, &quad_VertexArrayID);
		glBindVertexArray(quad_VertexArrayID);

		static const GLfloat g_quad_vertex_buffer_data[] = {
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			1.0f,  1.0f, 0.0f,
		};

		glGenBuffers(1, &quad_vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

	}


	/* set up the FBO for the light's depth */
	void initShadow() {

		//generate the FBO for the shadow depth
		glGenFramebuffers(1, &depthMapFBO);

		//generate the texture
		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, S_WIDTH, S_HEIGHT,
		             0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//bind with framebuffer's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}


	void init(const std::string& resourceDirectory) {
		GLSL::checkVersion();

		g_phi = 0;
		g_theta = -90;

		// Set background color.
		glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL programs
		DepthProg = make_shared<Program>();
		DepthProg->setVerbose(true);
		DepthProg->setShaderNames(resourceDirectory + "/depth_vert.glsl", resourceDirectory + "/depth_frag.glsl");
		DepthProg->init();

		DepthProgDebug = make_shared<Program>();
		DepthProgDebug->setVerbose(true);
		DepthProgDebug->setShaderNames(resourceDirectory + "/depth_vertDebug.glsl", resourceDirectory + "/depth_fragDebug.glsl");
		DepthProgDebug->init();

		ShadowProg = make_shared<Program>();
		ShadowProg->setVerbose(true);
		ShadowProg->setShaderNames(resourceDirectory + "/shadow_vert.glsl", resourceDirectory + "/shadow_frag.glsl");
		ShadowProg->init();

		DebugProg = make_shared<Program>();
		DebugProg->setVerbose(true);
		DebugProg->setShaderNames(resourceDirectory + "/pass_vert.glsl", resourceDirectory + "/pass_texfrag.glsl");
		DebugProg->init();


		//////////////////////////////////////////////////////
		// Intialize textures
		//////////////////////////////////////////////////////
		texture0 = make_shared<Texture>();
		texture0->setFilename(resourceDirectory + "/bump1.bmp");
		texture0->init();
		texture0->setUnit(0);
		texture0->setWrapModes(GL_REPEAT, GL_REPEAT);

		texture1 = make_shared<Texture>();
		texture1->setFilename(resourceDirectory + "/brick.bmp");
		texture1->init();
		texture1->setUnit(1);
		texture1->setWrapModes(GL_REPEAT, GL_REPEAT);

		flowers = make_shared<Texture>();
		flowers->setFilename(resourceDirectory + "/flowers.jpg");
		flowers->init();
		flowers->setUnit(1);
		flowers->setWrapModes(GL_REPEAT, GL_REPEAT);

		wood = make_shared<Texture>();
		wood->setFilename(resourceDirectory + "/cartoonWood.jpg");
		wood->init();
		wood->setUnit(1);
		wood->setWrapModes(GL_REPEAT, GL_REPEAT);

		/// Add uniform and attributes to each of the programs
		DepthProg->addUniform("LP");
		DepthProg->addUniform("LV");
		DepthProg->addUniform("M");
		DepthProg->addAttribute("vertPos");
		//un-needed, but easier then modifying shape
		DepthProg->addAttribute("vertNor");
		DepthProg->addAttribute("vertTex");

		DepthProgDebug->addUniform("LP");
		DepthProgDebug->addUniform("LV");
		DepthProgDebug->addUniform("M");
		DepthProgDebug->addAttribute("vertPos");
		//un-needed, but easier then modifying shape
		DepthProgDebug->addAttribute("vertNor");
		DepthProgDebug->addAttribute("vertTex");

		ShadowProg->addUniform("P");
		ShadowProg->addUniform("M");
		ShadowProg->addUniform("V");
		ShadowProg->addUniform("LS");
		ShadowProg->addUniform("normMapFlag");
		ShadowProg->addUniform("lightDir");
		ShadowProg->addAttribute("vertPos");
		ShadowProg->addAttribute("vertNor");
		ShadowProg->addAttribute("vertTex");
		ShadowProg->addAttribute("vertTan");
		ShadowProg->addAttribute("vertBN");
		ShadowProg->addUniform("normalTex");
		ShadowProg->addUniform("colorTex");
		ShadowProg->addUniform("shadowDepth");

		DebugProg->addUniform("texBuf");
		DebugProg->addAttribute("vertPos");

		cout << "end of init: " << glGetError() << endl;
		initShadow();
	}

	void SetProjectionMatrix(shared_ptr<Program> curShade) {
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		mat4 Projection = perspective(radians(50.0f), aspect, 0.1f, 100.0f);
		glUniformMatrix4fv(curShade->getUniform("P"), 1, GL_FALSE, value_ptr(Projection));
	}

	mat4 SetOrthoMatrix(shared_ptr<Program> curShade) {
		mat4 OP = glm::ortho(-10.0, 10.0, -10.0, 10.0, -10.0, 30.0);
		glUniformMatrix4fv(curShade->getUniform("LP"), 1, GL_FALSE, value_ptr(OP));
		return OP;
	}

	/* camera controls - do not change */
	void SetView(shared_ptr<Program> curShade) {
		mat4 Cam = glm::lookAt(g_eye, g_lookAt, vec3(0, 1, 0));
		glUniformMatrix4fv(curShade->getUniform("V"), 1, GL_FALSE, value_ptr(Cam));
	}

	mat4 SetLightView(shared_ptr<Program> curShade, vec3 pos, vec3 LA, vec3 up) {
		mat4 Cam = glm::lookAt(pos, LA, up);
		//fill in the glUniform call to send to the right shader!
		glUniformMatrix4fv(curShade->getUniform("LV"), 1, GL_FALSE, value_ptr(Cam));
		return Cam;
	}

	/* model transforms */
	void SetModel(vec3 trans, float rotY, float rotX, float sc, shared_ptr<Program> curS) {
		mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
		mat4 RotX = glm::rotate( glm::mat4(1.0f), rotX, vec3(1, 0, 0));
		mat4 RotY = glm::rotate( glm::mat4(1.0f), rotY, vec3(0, 1, 0));
		mat4 ScaleS = glm::scale(glm::mat4(1.0f), vec3(sc));
		mat4 ctm = Trans * RotX * RotY * ScaleS;
		glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
	}

	void SetModel(mat4 ctm, shared_ptr<Program> curS) {
		glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
	}

	/*
	Draw the cube and ground plane
	Textures can be turned on an off (as shadow map depth drawing does not need textures)
	*/
	void drawScene(shared_ptr<Program> shader, GLint texID0, GLint texID1, bool TexOn, bool norMap) {

		//for cubes and ground plane attempt to normalmap
		glUniform1i(shader->getUniform("normMapFlag"), 1);
		if (TexOn) {
			texture0->bind(texID0);
			texture1->bind(texID1);
		}
		//draw the cube
		SetModel(vec3(1.5, -0.4, -5), gObjRot+1.2, 0, 1.3, shader);
		//draw with BTN data if final shading pass
		cube->draw(shader, true && norMap);

		SetModel(vec3(-2, -0.4, -5), gObjRot, 0, 1.3, shader);
		cube->draw(shader, true && norMap);

		if (TexOn) {
			texture0->bind(texID0);
			flowers->bind(texID1);
		}
		//draw the ground plane only if not shadow mapping
		glUniform1i(shader->getUniform("normMapFlag"), 0);
		SetModel(vec3(0, 0.25, 0), 0, 0, 1, shader);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

		// draw!
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
		glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);


		if (TexOn) {
			texture0->bind(texID0);
			wood->bind(texID1);
		}
		//for dummy don't normal map
		glUniform1i(shader->getUniform("normMapFlag"), 0);
		/* wanting more geometry int the scene */
		mat4 tranOrig = glm::translate(mat4(1.0), -1.0f * gD_trans);
		mat4 sc = glm::scale(mat4(1.0), vec3(gD_scale * 1.7));
		mat4 rot = glm::rotate(glm::mat4(1.0f), -radians(90.0f), vec3(1, 0, 0));
		mat4 rot2 = glm::rotate(glm::mat4(1.0f), -radians(90.0f), vec3(0, 1, 0));

		mat4 Dmod = sc*rot2*rot*tranOrig;
		mat4 ctm;
		float tx, tz, theta = 0.0;
		float diffX = dummyMesh[21]->max.x-dummyMesh[21]->min.x;
		float diffZ = dummyMesh[21]->max.z-dummyMesh[21]->min.z;	
		vec3 pivotS = vec3(dummyMesh[21]->min.x+0.5*(diffX), dummyMesh[21]->min.y, dummyMesh[21]->min.z+ 0.5*diffZ);
		//shoulder trans
		mat4 shoulderTO = translate(mat4(1.0), -1.0f*vec3(pivotS));
		mat4 rotS = glm::rotate(glm::mat4(1.0f), 2.0f*sin((float)glfwGetTime()), vec3(1, 0, 0));
		mat4 ctmS = translate(mat4(1.0), pivotS)*rotS*shoulderTO;
		for (int i = 0; i < 10; i++) {
			tx = (6.0f) * sin(theta);
			tz = (6.0f) * cos(theta);
			mat4 trans;
			mat4 r1, r2;
			trans = translate(glm::mat4(1.0f), vec3(tx, 0.43f, tz));
			r2 = rotate(glm::mat4(1.0f), 3.14f + theta, vec3(0, 1, 0));
			ctm = trans*r2*Dmod;
			int id = shader->getUniform("M");
			for (int j = 0; j < dummyMesh.size(); j++) {
				//left shoulder
				if (j >= 21 && j <= 26) {
					glUniformMatrix4fv(shader->getUniform("M"), 1, GL_FALSE, value_ptr(ctm*ctmS));
					dummyMesh[j]->draw(shader, false);
				} else {
					glUniformMatrix4fv(shader->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
					dummyMesh[j]->draw(shader, false);
				}
			}
			theta += 6.28f / 10.f;
		}

	}

	/* let's draw */
	void render() {

		mat4 LSpace;
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

		if (shadow) {
			//set up light's depth map
			glViewport(0, 0, S_WIDTH, S_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			glCullFace(GL_FRONT);

			//set up shadow shader
			//render scene
			DepthProg->bind();
			mat4 LO = SetOrthoMatrix(DepthProg);
			mat4 LV = SetLightView(DepthProg, g_light, vec3(0, 0, 0), vec3(0, 1, 0));

			drawScene(DepthProg, 0, 0, false, false);
			DepthProg->unbind();
			glCullFace(GL_BACK);

			LSpace = LO * LV;

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}


		glViewport(0, 0, width, height);
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//now render the scene like normal
		//set up shadow shader
		ShadowProg->bind();
		/* also set up light depth map */
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glUniform1i(ShadowProg->getUniform("shadowDepth"), 2);
		glUniform3f(ShadowProg->getUniform("lightDir"), g_light.x, g_light.y, g_light.z);
		//render scene
		SetProjectionMatrix(ShadowProg);
		SetView(ShadowProg);
		glUniformMatrix4fv(ShadowProg->getUniform("LS"), 1, GL_FALSE, value_ptr(LSpace)) ;
		drawScene(ShadowProg, ShadowProg->getUniform("normalTex"), ShadowProg->getUniform("colorTex"), true, true);
		ShadowProg->unbind();

	}

	
};

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(800, 600);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	cout << "init G" << glGetError() << endl;
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle())) {
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}

