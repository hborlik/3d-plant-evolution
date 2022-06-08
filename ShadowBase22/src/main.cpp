/* CPE 476: ZJW Base code for shadow mapping lab */
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

class Application : public EventCallbacks {

  public:
	 WindowManager * windowManager = nullptr;

	 shared_ptr<Program> DepthProg;
	 shared_ptr<Program> DepthProgDebug;
	 shared_ptr<Program> ShadowProg;
	 shared_ptr<Program> DebugProg;

	 shared_ptr<Shape> world;
	 shared_ptr<Shape> dog;
   shared_ptr<Shape> cube;

	 shared_ptr<Texture> texture0, texture1, texture2, texture3;

	 bool DEBUG_LIGHT = false;
	 bool GEOM_DEBUG = false;
   bool SHADOW = true;

	 GLuint depthMapFBO;
	 const GLuint S_WIDTH = 1024, S_HEIGHT = 1024;
	 GLuint depthMap;

	 vec3 g_light = vec3(3, 5, 5);

   //camera
   bool MOVEF = false;
   bool MOVEB = false;
   bool MOVER = false;
   bool MOVEL = false;
	 double g_phi, g_theta;
	 vec3 view = vec3(0, 0, 1);
	 vec3 strafe = vec3(1, 0, 0);
	 vec3 g_eye = vec3(0, 1, 0);
	 vec3 g_lookAt = vec3(0, 1, -4);

   mat4 bias_mat{
      glm::vec4{0.5, 0, 0, 0}, 
      glm::vec4{0, 0.5, 0, 0}, 
      glm::vec4{0, 0, 0.5, 0},
      glm::vec4{0.5, 0.5, 0.499, 0}};

	 //global data for ground plane
   int g_GiboLen;
	 GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	 //geometry for texture render
	 GLuint quad_VertexArrayID;
	 GLuint quad_vertexbuffer;

	 static void error_callback(int error, const char *description) {
		  cerr << description << endl;
	 }

  /* set up the FBO for storing the light's depth */
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
  	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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
  	texture0->setFilename(resourceDirectory + "/cartoonWood.jpg");
  	texture0->init();
  	texture0->setUnit(0);
  	texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

  	texture1 = make_shared<Texture>();
  	texture1->setFilename(resourceDirectory + "/world.jpg");
  	texture1->init();
  	texture1->setUnit(0);
  	texture1->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

  	texture2 = make_shared<Texture>();
  	texture2->setFilename(resourceDirectory + "/flowers.jpg");
  	texture2->init();
  	texture2->setUnit(0);
  	texture2->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

    texture3 = make_shared<Texture>();
    texture3->setFilename(resourceDirectory + "/cartoonSky.png");
    texture3->init();
    texture3->setUnit(0);
    texture3->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	 /// Add uniform and attributes to each of the programs
  	DepthProg->addUniform("LP");
  	DepthProg->addUniform("LV");
  	DepthProg->addUniform("M");
  	DepthProg->addAttribute("vertPos");
	 //un-needed, better solution to modifying shape
  	DepthProg->addAttribute("vertNor");
  	DepthProg->addAttribute("vertTex");

  	DepthProgDebug->addUniform("LP");
  	DepthProgDebug->addUniform("LV");
  	DepthProgDebug->addUniform("M");
  	DepthProgDebug->addAttribute("vertPos");
	 //un-needed, better solution to modifying shape
  	DepthProgDebug->addAttribute("vertNor");
  	DepthProgDebug->addAttribute("vertTex");

  	ShadowProg->addUniform("P");
  	ShadowProg->addUniform("M");
  	ShadowProg->addUniform("V");
    ShadowProg->addUniform("LS");
  	ShadowProg->addUniform("lightDir");
  	ShadowProg->addAttribute("vertPos");
  	ShadowProg->addAttribute("vertNor");
  	ShadowProg->addAttribute("vertTex");
  	ShadowProg->addUniform("Texture0");
  	ShadowProg->addUniform("shadowDepth");

  	DebugProg->addUniform("texBuf");
  	DebugProg->addAttribute("vertPos");

  	initShadow();
  }

  void SetProjectionMatrix(shared_ptr<Program> curShade) {
    int width, height;
    glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
  	float aspect = width/(float)height;
  	mat4 Projection = perspective(radians(50.0f), aspect, 0.1f, 100.0f);
  	glUniformMatrix4fv(curShade->getUniform("P"), 1, GL_FALSE, value_ptr(Projection));
  }

/* TODO fix */
  mat4 SetOrthoMatrix(shared_ptr<Program> curShade) {
  	mat4 ortho = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.1f, 100.f);

    glUniformMatrix4fv(curShade->getUniform("LP"), 1, GL_FALSE, value_ptr(ortho));
  	return ortho;
  }

/* TODO fix */
  mat4 SetLightView(shared_ptr<Program> curShade, vec3 pos, vec3 LA, vec3 up) {
  	mat4 Cam = glm::lookAt(pos, LA, up); 

    glUniformMatrix4fv(curShade->getUniform("LV"), 1, GL_FALSE, value_ptr(Cam));
  	return Cam;
  }

  /* camera controls - do not change */
  void SetView(shared_ptr<Program> curShade) {
    mat4 Cam = glm::lookAt(g_eye, g_lookAt, vec3(0, 1, 0));
    glUniformMatrix4fv(curShade->getUniform("V"), 1, GL_FALSE, value_ptr(Cam));
  }

/* model transforms */
  void SetModel(vec3 trans, float rotY, float rotX, float sc, shared_ptr<Program> curS) {
  	mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
  	mat4 RotX = glm::rotate( glm::mat4(1.0f), rotX, vec3(1, 0, 0));
  	mat4 RotY = glm::rotate( glm::mat4(1.0f), rotY, vec3(0, 1, 0));
  	mat4 ScaleS = glm::scale(glm::mat4(1.0f), vec3(sc));
  	mat4 ctm = Trans*RotX*RotY*ScaleS;
  	glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
  }

  /* model transforms - normal */
  void SetModel(shared_ptr<Program> curS, mat4 m) {
    glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(m));
  }

/*
Draw the dog, sphere and ground plane
Textures can be turned on an off (as shadow map depth drawing does not need textures)
*/
  void drawScene(shared_ptr<Program> shader, GLint texID, int TexOn) {

  	if (TexOn) {
  		texture0->bind(texID);
  	}
	 //draw the dog mesh 	
  	SetModel(vec3(-1, 0, -5), 0, 0, 1.0/(dog->max.x-dog->min.x), shader);
  	dog->draw(shader);

    //draw a circle of doggos
    float tx, tz, theta = 0;
    mat4 transO = translate(glm::mat4(1.0f), vec3(-dog->min.x, -dog->min.y, -dog->min.z));
    mat4 scaleUnit = scale(glm::mat4(1.0f), vec3(1.0/(dog->max.x-dog->min.x)));
    mat4 scaleCube = scale(glm::mat4(1.0f), vec3(2.5, 0.5, 2.5));
    mat4 tranCube = translate(glm::mat4(1.0f), vec3(0.25, -0.26, 0.25));
    for (int i = 0; i < 10; i++) {
      tx = (9.0f) * sin(theta);
      tz = (9.0f) * cos(theta);
      mat4 trans;
      mat4 r1, r2;
      trans = translate(glm::mat4(1.0f), vec3(tx, -0.4f, tz));
      r2 = rotate(glm::mat4(1.0f), 3.14f + theta, vec3(0, 1, 0));
      SetModel(shader, trans*r2*scaleUnit*transO);
      dog->draw(shader);
      SetModel(shader, trans*r2*tranCube*scaleCube);
      cube->draw(shader);
      theta += 6.28f / 10.f;
    }


  	if (TexOn) {
  		texture3->bind(texID);
  	}
	 //draw the world sphere	
  	SetModel(vec3(1, 0, -5), 0, 0, 1, shader);
  	world->draw(shader);
    if (TexOn) {
      texture1->bind(texID);
    }
    SetModel(vec3(1, 1.5, -5), 0, 0, 0.5, shader);
    world->draw(shader);







  	if (TexOn) { 
  		texture2->bind(texID);
  	}	
	 //draw the ground plane	
  	SetModel(vec3(0, 0.5, 0), 0, 0, 1, shader);
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

  }

  void updateCamera() {
    float speed = 0.2;
    if (MOVEL){
      g_eye -= speed*strafe;
      g_lookAt -= speed*strafe;
    } else if (MOVER) {
     g_eye += speed*strafe;
      g_lookAt += speed*strafe;
    } else if (MOVEF) {
     g_eye -= speed*view;
     g_lookAt -= speed*view;
    } else if (MOVEB) {
     g_eye += speed*view;
     g_lookAt += speed*view;
    }
  }


  bool wireframe  = false;
/* let's draw */
  void render() {

    // Get current frame buffer size.
    int width, height;
    glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
    vec3 lightLA = vec3(0.0);
    vec3 lightUp = vec3(0, 1, 0);

    //camera movement
    updateCamera();

    mat4 LO, LV, LSpace;

    if (wireframe) {
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    } else {
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }

    //Note in this code we generate a shadow map each frame
    if (SHADOW) {
	   //set up light's depth map
  	 glViewport(0, 0, S_WIDTH, S_HEIGHT);

      //sets up the output to be out FBO
  	 glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  	 glClear(GL_DEPTH_BUFFER_BIT);
  	 glCullFace(GL_FRONT);

		  //set up shadow shader and render the scene
  	 DepthProg->bind();
		  //TODO you will need to fix these
  	 LO = SetOrthoMatrix(DepthProg);
  	 LV = SetLightView(DepthProg, g_light, lightLA, lightUp);
  	 drawScene(DepthProg, 0, 0);
  	 DepthProg->unbind();

      //set culling back to normal
  	 glCullFace(GL_BACK);

      //this sets the output back to the screen
  	 glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    //Second pass, now draw the scene (or do debug drawing)
    glViewport(0, 0, width, height);
	 // Clear framebuffer.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    if (DEBUG_LIGHT) {	
          glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		  /* code to draw the light depth buffer */
		  //geometry style debug on light - test transforms, draw geometry from light
		  //perspective
  		if (GEOM_DEBUG) { 
  			DepthProgDebug->bind();
        //render scene from light's point of view
  			SetOrthoMatrix(DepthProgDebug);
  			SetLightView(DepthProgDebug, g_light, lightLA, lightUp);
  			drawScene(DepthProgDebug, ShadowProg->getUniform("Texture0"), 0);
  			DepthProgDebug->unbind();
  		} else {
        //actually draw the light depth map after testing geometry - texture test
  			DebugProg->bind();
        /* TODO uncomment this code */
        /* do you understand why you are sending a texture in this DEBUG test */
        
  			glActiveTexture(GL_TEXTURE0);
  			glBindTexture(GL_TEXTURE_2D, depthMap);
  			glUniform1i(DebugProg->getUniform("texBuf"), 0);
        
        //draw the quad
  			glEnableVertexAttribArray(0);
  			glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
  			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
  			glDrawArrays(GL_TRIANGLES, 0, 6);
  			glDisableVertexAttribArray(0);
  			DebugProg->unbind();
  		}
  	} else {
		  //If not debugging render the scene like normal
		  //set up shadow shader
  		ShadowProg->bind();
		  /* also set up light depth map  as texture input*/
  		glActiveTexture(GL_TEXTURE1);
  		glBindTexture(GL_TEXTURE_2D, depthMap);
  		glUniform1i(ShadowProg->getUniform("shadowDepth"), 1);
  		glUniform3f(ShadowProg->getUniform("lightDir"), g_light.x, g_light.y, g_light.z);
		  //render scene
  		SetProjectionMatrix(ShadowProg);
  		SetView(ShadowProg);
		  //TODO: is there other uniform data that must be sent?
      //send light space
      LSpace = bias_mat*LO*LV;
      glUniformMatrix4fv(ShadowProg->getUniform("LS"), 1, GL_FALSE, value_ptr(LSpace));

  		drawScene(ShadowProg, ShadowProg->getUniform("Texture0"), 1);
  		ShadowProg->unbind();
  	}
  }

  void mouseCallback(GLFWwindow *window, int button, int action, int mods) {
    double posX, posY;

    if (action == GLFW_PRESS) {
       glfwGetCursorPos(window, &posX, &posY);
       cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
    }
  }

  void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
      MOVEL = true;
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
      MOVER = true;
    }
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
      MOVEF = true;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
      MOVEB = true;
    }
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
      g_light.x += 0.25;
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
      g_light.x -= 0.25;
    if (key == GLFW_KEY_L && action == GLFW_PRESS)
      DEBUG_LIGHT = !DEBUG_LIGHT;
    if (key == GLFW_KEY_G && action == GLFW_PRESS)
      GEOM_DEBUG = !GEOM_DEBUG;
    if (action == GLFW_RELEASE){
      MOVER = MOVEF = MOVEB = MOVEL = false;
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
      wireframe = true;
    }
    if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
      wireframe = false;
    }
  }

  void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
    vec3 diff, newV;
    g_theta += deltaX;
    g_phi += deltaY;
    newV.x = cosf(g_phi*(3.14/180.0))*cosf(g_theta*(3.14/180.0));
    newV.y = -1.0*sinf(g_phi*(3.14/180.0));
    newV.z = 1.0*cosf(g_phi*(3.14/180.0))*cosf((90.0-g_theta)*(3.14/180.0));
    diff = (g_lookAt - g_eye) - newV;
    g_lookAt = g_lookAt - diff;
    view = g_eye - g_lookAt;
    strafe = cross(vec3(0, 1,0), view);
  }

  void resizeCallback(GLFWwindow *window, int width, int height) {
   glViewport(0, 0, width, height);
  }

  /* code to read in meshes and define the ground plane */
  void initGeom(const std::string& resourceDirectory) {

    vector<tinyobj::shape_t> TOshapes;
    vector<tinyobj::material_t> objMaterials;
    string errStr;
    //load in the mesh and make the shape(s)
    bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/dog.obj").c_str());
    if (!rc) {
      cerr << errStr << endl;
    } else {
      dog = make_shared<Shape>();
      dog->createShape(TOshapes[0]);
      dog->measure();
      dog->init();
    }
    
    vector<tinyobj::shape_t> TOshapes2;
    vector<tinyobj::material_t> objMaterials2;
    //load in the mesh and make the shape(s)
    rc = tinyobj::LoadObj(TOshapes2, objMaterials2, errStr, (resourceDirectory + "/sphere.obj").c_str());
    if (!rc) {
      cerr << errStr << endl;
    } else {
      world = make_shared<Shape>();
      world->createShape(TOshapes2[0]);
      world->measure();
      world->init();
    }

    vector<tinyobj::shape_t> TOshapes3;
    vector<tinyobj::material_t> objMaterials3;
    //load in the mesh and make the shape(s)
    rc = tinyobj::LoadObj(TOshapes3, objMaterials3, errStr, (resourceDirectory + "/cube.obj").c_str());
    if (!rc) {
      cerr << errStr << endl;
    } else {
      cube = make_shared<Shape>();
      cube->createShape(TOshapes3[0]);
      cube->measure();
      cube->init();
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
      0, 1,
      1, 1,
      1, 0 
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

      glGenBuffers(1, &GIndxBuffObj);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

      initQuad();
    }

  /*geometry set up for a quad for texturing to validate depth */
  /* This is an XY quad for texturing a picture to view */
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


};

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
 std::string resourceDir = "../resources";

 if (argc >= 2) {
  resourceDir = argv[1];
  }

  Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.
	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state
	application->init(resourceDir);
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

