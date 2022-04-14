/*
 'Procedural' world with dog, Nef and hierarchical model with diffuse/textures and spline camera 
 and two additional viewports with orth camera for top down views
 CPE 471+476 Cal Poly Z. Wood + S. Sueda + I. Dunn (spline D. McGirr) 

 PRESS:
 'g' : camera runs on spline (not great points - you can fix them)
 'z': wireframe
 'p' : evetually to CULL (you need to fix TODOs)
 **EARLY RELEASE NO GAME CAMERA scroll for pitch and yaw camera

 // View Frustum Culling base code for CPE 476 VFC workshop
// Note data-structure NOT recommended for CPE 476 - 
// object locations in arrays and estimated radii
// use your improved data structures
// 2019 revise with Application and window manag3er, shape and program
// 2022 added spline, textures and HM
*/

#include <iostream>
#include <chrono>
#include <list>
#include <optional>
#include <filesystem>

#include <glad/glad.h>
#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Texture.h"
#include "stb_image.h"
#include "Bezier.h"
#include "Spline.h"
#include "util.h"
#include "transForms.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#define PI 3.1415927
#define numO 80
enum Mat {ruby=0, brass, copper, gold, tone1, tone2, tone3, tone4, shadow};

using namespace std;
using namespace glm;

struct Sphere {
	float r;
	glm::vec3 p;

	std::optional<glm::vec3> intersects(const Sphere& o) const {
		if (glm::length(o.p - p) < o.r + r) {
			return {(o.p * o.r + p * r) / (o.r + r)};
		}
		return {};
	}
};

class GameObject {
	GameObject* parent = nullptr;

	glm::quat rotation = glm::identity<glm::quat>();
	glm::vec3 position = {0, 0, 0};
	glm::vec3 scale = {1, 1, 1};
public:

	virtual ~GameObject(){}

	virtual void update(double delta) {}

	void setPosition(glm::vec3 p) noexcept {
		position = p;
	}

	void setRotation(glm::quat q) noexcept {
		rotation = q;
	}

	void setScale(glm::vec3 s) noexcept {
		scale = s;
	}

	glm::vec3 getPosition() const noexcept {
		return position;
	}

	glm::quat getRotation() const noexcept {
		return rotation;
	}

	glm::vec3 getScale() const noexcept {
		return scale;
	}

	glm::mat4 getLinearTransform() const noexcept {
		glm::mat4 tr = glm::translate(glm::mat4{1.0f}, position) * glm::toMat4(rotation);
		if (parent != nullptr) {
			tr = parent->getLinearTransform() * tr;
		}
		return tr;
	}

	glm::mat4 getModelTransform() const noexcept {
		return getLinearTransform() * glm::scale(glm::mat4{1.0f}, scale);
	}
};

class IGOT : public GameObject {
public:
	float sphere_radius = 1.0f;
	glm::vec3 velocity;

	Sphere getCollider() const {
		return {
			sphere_radius,
			getPosition()
		};
	}

	void onColission(std::shared_ptr<IGOT> other, glm::vec3 p) {}

	void update(double delta) override {

	}

};

class SphereColliders {
public:
	/**
	 * @brief check all colliders for intersections
	 * 
	 * @param objs 
	 */
	void checkAll(std::list<std::shared_ptr<IGOT>>& objs) {
		auto itr = objs.begin();
		while (itr != objs.end()) {
			auto itrb = itr;
			itrb++;
			while (itrb != objs.end()) {
				auto cp = itr->get()->getCollider().intersects(itrb->get()->getCollider());
				if (cp) {
					itr->get()->onColission(*itrb, cp.value());
					itrb->get()->onColission(*itr, cp.value());
				}
			}
			itr++;
		}
	}
};

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	std::list<std::shared_ptr<GameObject>> objs;

	// Our shader program - use this one for Blinn-Phong has diffuse TODO add specular
	std::shared_ptr<Program> prog;
	//Our shader program for textures
	std::shared_ptr<Program> texProg;

	//our geometry - you likely want to revise with nicer design
	shared_ptr<Shape> sphere;
	shared_ptr<Shape> theDog;
	shared_ptr<Shape> nef;
	shared_ptr<Shape> cube;

	//transforms on objects 
	vector<transForms> NefTrans;
	vector<transForms> SnowTrans;

	//more geom (direct) global data for ground plane - direct load constant defined CPU data to GPU (not obj)
	GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	int g_GiboLen;
	GLuint GroundVertexArrayID;

	//textures
	shared_ptr<Texture> texture0;
	shared_ptr<Texture> texture1;	
	shared_ptr<Texture> texture2;

	//light animation data
	float lightTrans = 0;

	float frameTime = 0.001f;

	//camera - likely better to put into its own class
	double g_phi, g_theta;
	vec3 view = vec3(0, 0, 1);
	vec3 strafe = vec3(1, 0, 0);
	vec3 g_eye = vec3(0, 1, 0);
	vec3 g_lookAt = vec3(0, 1, -4);
	quat g_rot = identity<quat>();
	double g_up = 0.0f, g_right = 0.0f;

	// input state
	bool left_button_down = false;

	struct WASD {
		bool w = false,a = false,s = false,d = false;
	} input_keys;

	//spline for camera and anim toggle
	Spline splinepath[2];
	bool goCamera = false;

	//animation data on HM
	float sTheta = 0;
	float eTheta = 0;
	float hTheta = 0;
	float gTrans = 0;

	bool CULL = false;
	int cullCount;

	/* load shader, textures, set up data */
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.72f, .84f, 1.06f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		g_theta = -PI/2.0;

		// Initialize the GLSL program that we will use for local shading
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("MatShine");
		prog->addUniform("lightPos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex"); //silence error

		// Initialize the GLSL program that we will use for texture mapping
		texProg = make_shared<Program>();
		texProg->setVerbose(true);
		texProg->setShaderNames(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/tex_frag0.glsl");
		texProg->init();
		texProg->addUniform("P");
		texProg->addUniform("V");
		texProg->addUniform("M");
		texProg->addUniform("flip");
		texProg->addUniform("Texture0");
		texProg->addUniform("MatShine");
		texProg->addUniform("lightPos");
		texProg->addAttribute("vertPos");
		texProg->addAttribute("vertNor");
		texProg->addAttribute("vertTex");

		//read in a load the texture
		texture0 = make_shared<Texture>();
		texture0->setFilename(resourceDirectory + "/cloud.bmp");
		texture0->init();
		texture0->setUnit(0);
		texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		texture1 = make_shared<Texture>();
		texture1->setFilename(resourceDirectory + "/skyBox/back.jpg");
		texture1->init();
		texture1->setUnit(1);
		texture1->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		texture2 = make_shared<Texture>();
		texture2->setFilename(resourceDirectory + "/cartoonWood.jpg");
		texture2->init();
		texture2->setUnit(2);
		texture2->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

  		// init splines up and down
		splinepath[0] = Spline(glm::vec3(-6,0,5), glm::vec3(-1,-0.5,5), glm::vec3(1, 2, 2), glm::vec3(2,0,2), 5);
		splinepath[1] = Spline(glm::vec3(2,0,2), glm::vec3(3,-2,-1), glm::vec3(-0.25, 0.25, -2), glm::vec3(0,0,5), 5);

  		//allocate the transforms for the different models
		float tx, tz, s, r; 
		float Wscale = 11.0*(numO/10.0);
		srand(time(NULL));

		for (int i=0; i < numO; i++) {
			if(i < 10) {
				Wscale = 18.0;
			} else {
				Wscale = 18.0*(numO/10.0);
			}
			tx = 0.2 + Wscale*niceRand()-Wscale/2.0;
			tz = 0.1 + Wscale*niceRand()-Wscale/2.0;
			r = 6.28*niceRand();

    		//note 'bounding sphere' approximate - fix for your models!
			transForms t1 = transForms(vec3(tx, -0.85, tz), r, 1.0, 1.25, i%8);
			NefTrans.push_back(t1);

			tx = 0.1 + Wscale*niceRand()-Wscale/2.0;
			tz = 0.2 + Wscale*niceRand()-Wscale/2.0;
			r = 6.28*niceRand();
    		//note 'bounding sphere' approximate - fix for your models!
			transForms t2 = transForms(vec3(tx, -0.5, tz), r, 0.5, 1.5, i%8);
			SnowTrans.push_back(t2);
		}

	}

	/* loade meshes */
	void initGeom(const std::string& resourceDirectory)
	{
		//EXAMPLE set up to read one shape from one obj file - convert to read several
		// Initialize mesh
		// Load geometry
 		// Some obj files contain material information.We'll ignore them for this assignment.
		vector<tinyobj::shape_t> TOshapes;
		vector<tinyobj::material_t> objMaterials;
		string errStr;
		//load in the mesh and make the shape(s)
		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/sphereWTex.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			sphere = make_shared<Shape>();
			sphere->createShape(TOshapes[0]);
			sphere->measure();
			sphere->init();
		}

		// Initialize dog mesh.
		vector<tinyobj::shape_t> TOshapesB;
		vector<tinyobj::material_t> objMaterialsB;
		//load in the dogmesh and make the shape(s)
		rc = tinyobj::LoadObj(TOshapesB, objMaterialsB, errStr, (resourceDirectory + "/dog.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {	
			theDog = make_shared<Shape>();
			theDog->createShape(TOshapesB[0]);
			theDog->measure();
			theDog->init();
		}

		//Nefertiti
		vector<tinyobj::shape_t> TOshapesN;
		vector<tinyobj::material_t> objMaterialsN;
    	//load in the mesh and make the shape(s)
		rc = tinyobj::LoadObj(TOshapesN, objMaterialsN, errStr, (resourceDirectory + "/Nefertiti-100K.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			nef = make_shared<Shape>();
			nef->createShape(TOshapesN[0]);
			nef->measure();
			nef->init();
		}

		//code to load in the ground plane (CPU defined data passed to GPU)
		initGround();

		objs.push_back(std::make_shared<IGOT>());
	}

	//directly pass quad for the ground to the GPU
	void initGround() {

		float g_groundSize = 60;
		float g_groundY = -0.25;

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
      		0, 1,
      		1, 1,
      		1, 0 };

      		unsigned short idx[] = {0, 1, 2, 0, 2, 3};

		//generate the ground VAO
      		glGenVertexArrays(1, &GroundVertexArrayID);
      		glBindVertexArray(GroundVertexArrayID);

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

      		glGenBuffers(1, &GIndxBuffObj);
      		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
      		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
      	}

      

	/* helper functions to set model transforms  - you likely need LESS*/
    void SetModel(vec3 trans, float rotY, float rotX, float sc, shared_ptr<Program> curS) {
    	mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
    	mat4 RotX = glm::rotate( glm::mat4(1.0f), rotX, vec3(1, 0, 0));
    	mat4 RotY = glm::rotate( glm::mat4(1.0f), rotY, vec3(0, 1, 0));
    	mat4 ScaleS = glm::scale(glm::mat4(1.0f), vec3(sc));
    	mat4 ctm = Trans*RotX*RotY*ScaleS;
    	glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
    }

	/* alt. model transforms -  - these are insane because they came from p2B and P4*/
    mat4 SetModel(shared_ptr<Program> curS, vec3 trans, float rotY, float rotX, vec3 sc) {
    	mat4 Trans = translate( glm::mat4(1.0f), trans);
    	mat4 RotateY = rotate( glm::mat4(1.0f), rotY, glm::vec3(0.0f, 1, 0));
    	mat4 RotateX = rotate( glm::mat4(1.0f), rotX, glm::vec3(1,0 ,0));
    	mat4 Sc = scale( glm::mat4(1.0f), sc);
    	mat4 ctm = Trans*RotateY*Sc*RotateX;
    	glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
    	return ctm;
    }

    void SetModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
    	glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
    }

    /* model transforms - normal */
    void SetModel(shared_ptr<Program> curS, mat4 m) {
    	glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(m));
    }

	mat4 GetCamTr() {
		glm::mat4 cam_t = glm::rotate(glm::mat4{1.0f}, (float)g_up, glm::vec3{1, 0, 0});
		cam_t = glm::rotate(glm::mat4{1.0f}, (float)g_right, {0, 1, 0}) * cam_t;
		cam_t[3] = glm::vec4{g_eye, 1.0f};
		return cam_t;
	}

	/* helper functions for transforms */
    mat4 GetView(shared_ptr<Program>  shader) {
		glm::mat4 cam_t = GetCamTr();
		cam_t = glm::inverse(cam_t);
    	return cam_t;
    }

    mat4 GetProjectionMatrix() {
    	int width, height;
    	glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
    	float aspect = width/(float)height;
    	mat4 Projection = perspective(radians(50.0f), aspect, 0.1f, 200.0f);
    	return Projection;
    }

    mat4 GetOrthoMatrix() {
    	float wS = 2.5;
    	mat4 ortho = glm::ortho(-15.0f*wS, 15.0f*wS, -15.0f*wS, 15.0f*wS, 2.1f, 100.f);
    	return ortho;
    }

	/* camera controls - this is the camera for the top down view */
    mat4 GetTopView() {
    	mat4 Cam = lookAt(g_eye + vec3(0, 6, 0), g_eye, g_lookAt-g_eye);
    	return Cam;
    }

    void updateUsingCameraPath(float frametime)  {

    	if (goCamera) {
    		if(!splinepath[0].isDone()){
    			splinepath[0].update(frametime);
    			g_eye = splinepath[0].getPosition();
    		} else {
    			splinepath[1].update(frametime);
    			g_eye = splinepath[1].getPosition();
    		}
    	}
    }

/* code to draw the scene - it will be helpful to be able to draw your geometry more than once*/
//may use different shaders
    void drawScene(mat4 P, mat4 V, bool cullFlag) {

    	//For any shader switch be sure to set up ALL data
    	//matrix transforms for scene

		// Draw an array lay out of doggos - textured meshes
    	texProg->bind();
    	glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(P));
    	glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(V));
    	glUniform3f(texProg->getUniform("lightPos"), 2.0+lightTrans, 2.0, 2.9);
    	glUniform1f(texProg->getUniform("MatShine"), 27.9);
    	glUniform1i(texProg->getUniform("flip"), 1);
    	texture2->bind(texProg->getUniform("Texture0"));

    	float dScale = 1.0/(theDog->max.x-theDog->min.x);
    	float sp = 3.0;
    	float off = -3.5;
    	for (int i =0; i < 3; i++) {
    		for (int j=0; j < 3; j++) {
    			mat4 t = glm::translate(mat4(1.0), vec3(off+sp*i, -0.5, off+sp*j));
    			mat4 s = glm::scale(mat4(1.0), vec3(dScale));
    			glUniformMatrix4fv(texProg->getUniform("M"), 1, GL_FALSE, value_ptr(t*s));
    			//TODO look at this conditional and understand it - total hack on size
    			if( !cullFlag || !ViewFrustCull(vec3(off+sp*i, -0.5, off+sp*j), 3.0)) {
    				theDog->draw(texProg);
    			}
    		}
    	}

		auto itr_go = objs.begin();
		while (itr_go != objs.end()) {
			glm::mat4 tr = itr_go->get()->getModelTransform();
			glUniformMatrix4fv(texProg->getUniform("M"), 1, GL_FALSE, value_ptr(tr));
			theDog->draw(texProg);
			itr_go++;
		}
    	
    	//the ground plane is also textured so draw it
    	drawGround(texProg);
    	texProg->unbind();

    	//swap shaders set up all data
		//use the diffuse material shader
    	prog->bind();
		//set up all the matrices
    	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P));
    	glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(V));
    	glUniform3f(prog->getUniform("lightPos"), 2.0+lightTrans, 2.0, 2.9);

    	for (int i=0; i < numO; i++)  {
    		//draw Nefriti, placeholder code to check for culling
    		if( !cullFlag || !ViewFrustCull(NefTrans[i].mTrans, NefTrans[i].mRadius)) {
    			SetMaterial(prog, NefTrans[i].matID);
    			float meshScale = 1.0/(nef->max.x-nef->min.x);
    			SetModel(prog, vec3(NefTrans[i].mTrans.x, NefTrans[i].mTrans.y, NefTrans[i].mTrans.z), NefTrans[i].mRot, -3.14/2.0, vec3(meshScale));
      			//draw the mesh 
    			nef->draw(prog);
      			//fake mesh planar shadow - offsets hardcoded - not ideal should depend on light, etc.
    			SetMaterial(prog, Mat::shadow);
    			SetModel(prog, vec3(NefTrans[i].mTrans.x+0.2, NefTrans[i].mTrans.y-0.15, NefTrans[i].mTrans.z+0.2), NefTrans[i].mRot, -3.14/2.0, vec3(meshScale, 0.1*meshScale, meshScale));
    			nef->draw(prog);
    		}

    		/*now draw the hierarchiucal model - pseudo Baymax*/
    		if( !cullFlag || !ViewFrustCull(SnowTrans[i].mTrans, SnowTrans[i].mRadius)) {
      			/* transforms weird because of code re-use - mostly ignore */
    			mat4 Trans = glm::translate( glm::mat4(1.0f), vec3(SnowTrans[i].mTrans.x, SnowTrans[i].mTrans.y+0.1, SnowTrans[i].mTrans.z));
    			mat4 RotateY = glm::rotate( glm::mat4(1.0f), SnowTrans[i].mRot, glm::vec3(0.0f, 1, 0));
    			mat4 Sc = glm::scale( glm::mat4(1.0f), vec3(SnowTrans[i].mScale));
    			mat4 com = Trans*RotateY*Sc;
    			SetMaterial(prog, SnowTrans[i].matID);
    			drawHierModel(com, prog);

    			//fake mesh planar shadow - offsets hardcoded - not ideal should depend on light, etc.
    			SetMaterial(prog, Mat::shadow);
    			com =  SetModel(prog, vec3(SnowTrans[i].mTrans.x+0.2, SnowTrans[i].mTrans.y-0.5, SnowTrans[i].mTrans.z+0.2), SnowTrans[i].mRot, -3.14/2.0, vec3(0.3, 0.1, 0.3));
    			drawHierModel(com, prog);
    		}
    	} 

        prog->unbind();
    	assert(glGetError() == GL_NO_ERROR);
    }

/* Render including two subwindows with top down views that will help us 'visualize' that our view frustum culling
  is working */
    void render(float frametime) {
		static double posX = 0.0, posY = 0.0;

		this->frameTime = frametime;

		double lastX = posX, lastY = posY;
		glfwGetCursorPos(windowManager->getHandle(), &posX, &posY);
		if (left_button_down)
		{
			g_right -= (posX - lastX) * frameTime * 0.05;
			g_up -= (posY - lastY) * frameTime * 0.04;
			if (g_up > M_PI_2)
				g_up = M_PI_2;
			else if (g_up < -M_PI_2)
				g_up = -M_PI_2;
		}

		const mat4 cam_t = GetCamTr();

		if (input_keys.w) {
			g_eye += vec3(-cam_t[2]) * frameTime * 1.2f;
		}
		if (input_keys.a) {
			g_eye += vec3(-cam_t[0]) * frameTime * 1.2f;
		}
		if (input_keys.s) {
			g_eye += vec3(cam_t[2]) * frameTime * 1.2f;
		}
		if (input_keys.d) {
			g_eye += vec3(cam_t[0]) * frameTime * 1.2f;
		}

  		// Get current frame buffer size.
    	int width, height;
    	glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
    	glViewport(0, 0, width, height);

		// Clear framebuffer.
    	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    	
    	//traditional perspective projection 
    	mat4 PerProj = GetProjectionMatrix();
    	//update the camera position
    	updateUsingCameraPath(frametime);
    	//compute game camera view
    	mat4 GameCamView = GetView(prog);

    	//only extract the planes for the game camaera
    	ExtractVFPlanes(PerProj, GameCamView);

    	cullCount = 0;
    	//draw scene from 'game camera' perspective
    	drawScene(PerProj, GameCamView, CULL);
    	// cout << "cull count: " << cullCount << endl;

    	//draw big background sphere (always)
    	texProg->bind();

    	mat4 scaleM = glm::scale(glm::mat4(1.0f), vec3(48.0));
    	glUniform1i(texProg->getUniform("flip"), 0);
    	texture1->bind(texProg->getUniform("Texture0"));
    	SetModel(texProg, scaleM);
    	sphere->draw(texProg);

    	texProg->unbind();

  		/* draw the complete scene from a top down camera */
  		// mat4 OrthoProj = GetOrthoMatrix();
    	// mat4 TopView = GetTopView();
    	// glClear( GL_DEPTH_BUFFER_BIT);
    	// glViewport(0, 0, 300, 300);
    	// drawScene(OrthoProj, TopView, false);

  		// /* draw the culled scene from a top down camera - nothing culled yet - fix that */
    	// glClear( GL_DEPTH_BUFFER_BIT);
    	// glViewport(0, height-300, 300, 300);
    	// drawScene(OrthoProj, TopView, CULL);

    }



 	/* VFC code starts here TODO - start here and fill in these functions!!!*/
	vec4 Left, Right, Bottom, Top, Near, Far;
	vec4 planes[6];

/*TODO fill in */
void ExtractVFPlanes(mat4 P, mat4 V) {

  /* composite matrix */
  mat4 comp = P*V;
  vec3 n; //use to pull out normal
  float l; //length of normal for plane normalization

  Left.x = comp[0][3] + comp[0][0]; 
  Left.y = comp[1][3] + comp[1][0]; 
  Left.z = comp[2][3] + comp[2][0]; 
  Left.w = comp[3][3] + comp[3][0];
  planes[0] = Left;
//   cout << "Left' " << Left.x << " " << Left.y << " " <<Left.z << " " << Left.w << endl;
  
  Right.x = 0; // see handout to fill in with values from comp
  Right.y = 0; // see handout to fill in with values from comp
  Right.z = 0; // see handout to fill in with values from comp
  Right.w = 0; // see handout to fill in with values from comp
  planes[1] = Right;
//   cout << "Right " << Right.x << " " << Right.y << " " <<Right.z << " " << Right.w << endl;

  Bottom.x = 0; // see handout to fill in with values from comp
  Bottom.y = 0; // see handout to fill in with values from comp
  Bottom.z = 0; // see handout to fill in with values from comp
  Bottom.w = 0; // see handout to fill in with values from comp
  planes[2] = Bottom;
//   cout << "Bottom " << Bottom.x << " " << Bottom.y << " " <<Bottom.z << " " << Bottom.w << endl;
  
  Top.x = 0; // see handout to fill in with values from comp
  Top.y = 0; // see handout to fill in with values from comp
  Top.z = 0; // see handout to fill in with values from comp
  Top.w = 0; // see handout to fill in with values from comp
  planes[3] = Top;
//   cout << "Top " << Top.x << " " << Top.y << " " <<Top.z << " " << Top.w << endl;

  Near.x = 0; // see handout to fill in with values from comp
  Near.y = 0; // see handout to fill in with values from comp
  Near.z = 0; // see handout to fill in with values from comp
  Near.w = 0; // see handout to fill in with values from comp
  planes[4] = Near;
//   cout << "Near " << Near.x << " " << Near.y << " " <<Near.z << " " << Near.w << endl;

  Far.x = 0; // see handout to fill in with values from comp
  Far.y = 0; // see handout to fill in with values from comp
  Far.z = 0; // see handout to fill in with values from comp
  Far.w = 0; // see handout to fill in with values from comp
  planes[5] = Far;
//   cout << "Far " << Far.x << " " << Far.y << " " <<Far.z << " " << Far.w << endl;
}


/* helper function to compute distance to the plane */
/* TODO: fill in */
float DistToPlane(float A, float B, float C, float D, vec3 point) {
  return 0;
}

/* Actual cull on planes */
//returns 1 to CULL
//TODO fill in
int ViewFrustCull(vec3 center, float radius) {

  float dist;

  if (CULL) {
    cout << "testing against all planes" << endl;
    for (int i=0; i < 6; i++) {
      dist = DistToPlane(planes[i].x, planes[i].y, planes[i].z, planes[i].w, center);
      //test against each plane
    }
    return 0; 
  } else {
    return 0;
  }
}

/* code to draw waving hierarchical model */
    void drawHierModel(mat4 ctm, shared_ptr<Program> prog) {
    	auto Model = make_shared<MatrixStack>();
   		// draw mesh 
    	Model->pushMatrix();
    		Model->loadIdentity();
    		Model->multMatrix(ctm);
			/* draw top cube - aka head */
    		Model->pushMatrix();
    			Model->translate(vec3(0, 1.5, 0));
    			Model->scale(vec3(0.5, 0.5, 0.5));
    			SetModel(prog, Model);
    			sphere->draw(prog);
    		Model->popMatrix();
			//draw the torso with these transforms
    		Model->pushMatrix();
    			Model->scale(vec3(1.25, 1.35, 1.25));
    			SetModel(prog, Model);
    			sphere->draw(prog);
    		Model->popMatrix();

			//draw right arm
    		Model->pushMatrix();
			  //place at shoulder
    			Model->translate(vec3(0.75, 0.8, 0));
			  //rotate shoulder joint
    			Model->rotate(sTheta, vec3(0, 0, 1));
			  //move to shoulder joint
    			Model->translate(vec3(0.8, 0, 0));
			    //lower arm
    			Model->pushMatrix();
			 	   //place at upper arm
    				Model->translate(vec3(0.5, 0, 0));
			  	  //elbow rotation
    				Model->rotate(eTheta, vec3(0, 0, 1));
			  	  //move to lower arm pivot point
    				Model->translate(vec3(0.6, 0, 0));
			  	  //now draw the hand
    				Model->pushMatrix();
				  //place on lower arm
    					Model->translate(vec3(0.55, 0, 0));
			  		//rotate hand
    					Model->rotate(hTheta, vec3(0, 0, 1));
			  		//move to hand pivot joint
    					Model->translate(vec3(0.2, 0, 0));
    					Model->scale(vec3(0.3, 0.25, 0.1));
    					SetModel(prog, Model);
    					sphere->draw(prog);
    				Model->popMatrix();
			  	  //draw lower arm
    				Model->scale(vec3(0.6, 0.21, 0.21));
    				SetModel(prog, Model);
    				sphere->draw(prog);
    			Model->popMatrix();
			  //Do final scale ONLY to upper arm then draw
			  //non-uniform scale
    			Model->scale(vec3(0.7, 0.25, 0.25));
    			SetModel(prog, Model);
    			sphere->draw(prog);
    		Model->popMatrix();

			//draw left arm
    		Model->pushMatrix();
			  //place at shoulder
    			Model->translate(vec3(-0.8, 0.8, 0));
			  //rotate shoulder joint
    			Model->rotate(0.7, vec3(0, 0, 1));
			  //move to shoulder joint
    			Model->translate(vec3(-0.8, 0, 0));
			    //now the lower arm 
    			Model->pushMatrix();
			 	   //place on upper arm
    				Model->translate(vec3(-0.5, 0, 0));
			  	  //rotate lower arm
    				Model->rotate(1.5, vec3(0, 0, 1));
			  	  //move to pivot
    				Model->translate(vec3(-0.6, 0, 0));
    				Model->pushMatrix();
				    // place on lower arm
    					Model->translate(vec3(-0.55, 0, 0));
			  		//rotate hand
    					Model->rotate(-0.7, vec3(0, 0, 1));
			  		//move to pivot point on hand
    					Model->translate(vec3(-0.2, 0, 0));
    					Model->scale(vec3(0.3, 0.25, 0.1));
    					SetModel(prog, Model);
    					sphere->draw(prog);
    				Model->popMatrix();
			  	  //draw lower arm
    				Model->scale(vec3(0.6, 0.21, 0.21));
    				SetModel(prog, Model);
    				sphere->draw(prog);
    			Model->popMatrix();
			  //Do final scale ONLY to upper arm then draw
			  //non-uniform scale
    		Model->scale(vec3(0.7, 0.25, 0.25));
    		SetModel(prog, Model);
    		sphere->draw(prog);
    	Model->popMatrix();

    	Model->popMatrix();

    	//animation update example
		sTheta = sin(glfwGetTime());
		eTheta = std::max(0.0, 1.5*sin(glfwGetTime()));
		if (eTheta > 0.3){
			hTheta = std::max(0.0, sin(glfwGetTime()*6));
		}
    }

//code to draw the ground plane
      	void drawGround(shared_ptr<Program> curS) {
      		curS->bind();
      		glBindVertexArray(GroundVertexArrayID);
      		texture0->bind(curS->getUniform("Texture0"));
			//draw the ground plane 
      		SetModel(vec3(0, -1, 0), 0, 0, 1, curS);
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
      		curS->unbind();
      	}

     	//helper function to pass material data to the GPU - note specular data ignored - fix
      	void SetMaterial(shared_ptr<Program> curS, int i) {

      		switch (i) {
    			case 0: //jade
      			glUniform3f(prog->getUniform("MatAmb"), 0.0135, 0.02225,  0.01575);
      			glUniform3f(prog->getUniform("MatDif"), 0.14,  0.39,  0.23);
      			glUniform3f(prog->getUniform("MatSpec"), 0.116228,  0.116228,  0.116228 );
      			glUniform1f(prog->getUniform("MatShine"), 10.0);
      			break;
      			case 1: //brass
      			glUniform3f(prog->getUniform("MatAmb"), 0.3294, 0.2235, 0.02745);
      			glUniform3f(prog->getUniform("MatDif"), 0.7804, 0.5686, 0.11373);
      			glUniform3f(prog->getUniform("MatSpec"), 0.9922, 0.941176, 0.80784);
      			glUniform1f(prog->getUniform("MatShine"), 27.9);
      			break;
      			case 2: //copper
      			glUniform3f(prog->getUniform("MatAmb"), 0.1913, 0.0735, 0.0225);
      			glUniform3f(prog->getUniform("MatDif"), 0.7038, 0.27048, 0.0828);
      			glUniform3f(prog->getUniform("MatSpec"), 0.257, 0.1376, 0.08601);
      			glUniform1f(prog->getUniform("MatShine"), 12.8);
      			break;
      			case 3: // gold
      			glUniform3f(prog->getUniform("MatAmb"), 0.09, 0.07, 0.08);
      			glUniform3f(prog->getUniform("MatDif"), 0.91, 0.92, 0.91);
      			glUniform3f(prog->getUniform("MatSpec"), 1.0, 0.7, 1.0);
      			glUniform1f(prog->getUniform("MatShine"), 100.0);
      			break;
      			case 4: //tone1
				glUniform3f(prog->getUniform("MatAmb"), 199.0/2550.0f, 151.0/2550.0f, 146.0/2550.0f);
				glUniform3f(prog->getUniform("MatDif"), 199.0/255.0f, 151.0/255.0f, 146.0/255.0f);
				glUniform3f(prog->getUniform("MatSpec"), 0.257, 0.1376, 0.08601);
      			glUniform1f(prog->getUniform("MatShine"), 12.8);
				break;
				case 5: 
				glUniform3f(prog->getUniform("MatAmb"), 216.0/2550.0f, 164.0/2550.0f, 151.0/2550.0f);
				glUniform3f(prog->getUniform("MatDif"), 216.0/255.0f, 164.0/255.0f, 151.0/255.0f);
				glUniform3f(prog->getUniform("MatSpec"), 0.257, 0.1376, 0.08601);
      			glUniform1f(prog->getUniform("MatShine"), 12.8);
				break;
		 		case 6: 
		 		glUniform3f(prog->getUniform("MatAmb"), 173.0/2550.0f, 129.0/2550.0f, 111.0/2550.0f);
		 		glUniform3f(prog->getUniform("MatDif"), 173.0/255.0f, 129.0/255.0f, 111.0/255.0f);
		 		glUniform3f(prog->getUniform("MatSpec"), 0.257, 0.1376, 0.08601);
      			glUniform1f(prog->getUniform("MatShine"), 12.8);
		 		break;
		 		case 7: //tone4
        		glUniform3f(prog->getUniform("MatAmb"), 87.0/2550.0f, 49/2550.0f, 31.0/2550.0f);
        		glUniform3f(prog->getUniform("MatDif"), 87.0/255.0f, 49/255.0f, 31.0/255.0f);
        		glUniform3f(prog->getUniform("MatSpec"), 0.257, 0.1376, 0.08601);
      			glUniform1f(prog->getUniform("MatShine"), 12.8);
        		break;
        		case 8: //shadow
      			glUniform3f(prog->getUniform("MatAmb"), 0.12, 0.12, 0.12);
      			glUniform3f(prog->getUniform("MatDif"), 0.0, 0.0, 0.0);
      			glUniform3f(prog->getUniform("MatSpec"), 0.0, 0.0, 0.0);
      			glUniform1f(prog->getUniform("MatShine"), 0);
      			break;

    	}
    }
	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		 float speed = 0.2;
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS){
			lightTrans += 0.5;
		}
		if (key == GLFW_KEY_E && action == GLFW_PRESS){
			lightTrans -= 0.5;
		}
		if (key == GLFW_KEY_A) {
    		input_keys.a = action == GLFW_PRESS || action == GLFW_REPEAT;
    	}
  		if (key == GLFW_KEY_D)
    		input_keys.d = action == GLFW_PRESS || action == GLFW_REPEAT;
  		if (key == GLFW_KEY_W) {
    		input_keys.w = action == GLFW_PRESS || action == GLFW_REPEAT;
  		}
  		if (key == GLFW_KEY_S) {
    		input_keys.s = action == GLFW_PRESS || action == GLFW_REPEAT;
 		 }
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
    		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  		}
		if (key == GLFW_KEY_G && action == GLFW_RELEASE) {
			goCamera = !goCamera;
			splinepath[0].reset();
			splinepath[1].reset();
		}
		if (key == GLFW_KEY_P && action == GLFW_PRESS) { 
    		CULL = !CULL;
 		 }
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
			if(GLFW_PRESS == action)
				left_button_down = true;
			else if(GLFW_RELEASE == action)
				left_button_down = false;
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	/* much of the camera is here */
	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
		cout << "Early release does not include camera" << endl;
	}


};

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	std::cout << std::filesystem::current_path() << std::endl;

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(960, 720);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);

	auto lastTime = chrono::high_resolution_clock::now();
	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// save current time for next frame
		auto nextLastTime = chrono::high_resolution_clock::now();

		// get time since last frame
		float deltaTime =
		chrono::duration_cast<std::chrono::microseconds>(
			chrono::high_resolution_clock::now() - lastTime)
		.count();
		// convert microseconds (weird) to seconds (less weird)
		deltaTime *= 0.000001;

		// reset lastTime so that we can calculate the deltaTime
		// on the next frame
		lastTime = nextLastTime;

		// Render scene.
		application->render(deltaTime);
		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
