	/* Modified Base code for normal mapping 476 2022 ZJW - to include freetype font rendering
	  heavily modified reference code to support freetype from: 
	  https://learnopengl.com/In-Practice/Text-Rendering
	 - note for normal mapping uses a modified Shape.h/cpp to support BTN*/
	/*TODO you may need to change this line, where this points - where is the arial file for you? */
	//if (FT_New_Face(ft, "/System/Library/Fonts/Supplemental/Arial.ttf", 0, &face)) {
	/* This code is incomplete in terms of normal mapping - follow tasks listed in handout */
	#include <iostream>
	#include <glad/glad.h>

	#include "GLSL.h"
	#include "Program.h"
	#include "Shape.h"
	#include "MatrixStack.h"
	#include "WindowManager.h"
	#include "Texture.h"

	#include <ft2build.h>
	#include FT_FREETYPE_H  

	#define TINYOBJLOADER_IMPLEMENTATION
	#include <tiny_obj_loader/tiny_obj_loader.h>

	// value_ptr for glm
	#include <glm/gtc/type_ptr.hpp>
	#include <glm/gtc/matrix_transform.hpp>

	using namespace std;
	using namespace glm;
	using namespace std;
	using namespace glm;

	// settings
	const unsigned int SCR_WIDTH = 400;
	const unsigned int SCR_HEIGHT = 300;

	//define a type for use with freetype
	struct Character {
	    unsigned int TextureID; // ID handle of the glyph texture
	    glm::ivec2   Size;      // Size of glyph
	    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
	    unsigned int Advance;   // Horizontal offset to advance to next glyph
	};


	class Application : public EventCallbacks
	{

	public:
		WindowManager * windowManager = nullptr;

		shared_ptr<Program> DepthProg;
		shared_ptr<Program> DepthProgDebug;
		shared_ptr<Program> ShadowProg;
		shared_ptr<Program> DebugProg;
		shared_ptr<Program> textProg; //new set of shaders for the text rendering

		shared_ptr<Shape> cube;

		std::vector<shared_ptr<Shape>> fatcatMesh;
		vec3 gD_trans;
		float gD_scale;

		//several textures
		shared_ptr<Texture> texture0;
		shared_ptr<Texture> texture1;
		shared_ptr<Texture> flowers;
		shared_ptr<Texture> wood;
		shared_ptr<Texture> pupil;
		shared_ptr<Texture> eyeColor;

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


		//Free type data
		FT_Library ft;
		FT_Face face;
		std::map<GLchar, Character> Characters;
		unsigned int TextVAO, TextVBO;

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

			/*read in the multi-shape fat cat */
			vec3 Gmin, Gmax;
			vector<tinyobj::shape_t> TOshapes1;
			vector<tinyobj::material_t> objMaterials1;
			rc = tinyobj::LoadObj(TOshapes1, objMaterials1, errStr, (resourceDirectory + "/fat_cat.obj").c_str());
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

					fatcatMesh.push_back(s);
				}
				gD_trans = Gmin + 0.5f * (Gmax - Gmin);

				if (Gmax.x > Gmax.y && Gmax.x > Gmax.z)
					gD_scale = 2.0 / (Gmax.x - Gmin.x);
				else if (Gmax.y > Gmax.x && Gmax.y > Gmax.z)
					gD_scale = 2.0 / (Gmax.y - Gmin.y);
				else
					gD_scale = 2.0 / (Gmax.z - Gmin.z);
			}


			initGround();

			//init billboard for light depth map visualization, etc.
			initQuad();

			//initialize the text quad
			initTextQuad();
		}

		void initGround() {
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

		/*similar to bill board quad, keeping separate for ease in copy and paste between projects */
		void initTextQuad() {
			glGenVertexArrays(1, &TextVAO);
			glGenBuffers(1, &TextVBO);
			glBindVertexArray(TextVAO);
			glBindBuffer(GL_ARRAY_BUFFER, TextVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
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


		/* openGL initialization and shader loading and set up */
		void init(const std::string& resourceDirectory) {
			GLSL::checkVersion();

			g_phi = 0;
			g_theta = -90;

			// Set background color.
			glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
			// Enable z-buffer test.
			glEnable(GL_DEPTH_TEST);

			glEnable(GL_BLEND);
    		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

			textProg = make_shared<Program>();
			textProg->setVerbose(true);
			textProg->setShaderNames(resourceDirectory + "/textVert.glsl", resourceDirectory + "/textFrag.glsl");
			textProg->init();


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

			pupil = make_shared<Texture>();
			pupil->setFilename(resourceDirectory + "/black.jpg");
			pupil->init();
			pupil->setUnit(1);
			pupil->setWrapModes(GL_REPEAT, GL_REPEAT);


			eyeColor = make_shared<Texture>();
			eyeColor->setFilename(resourceDirectory + "/white.jpg");
			eyeColor->init();
			eyeColor->setUnit(1);
			eyeColor->setWrapModes(GL_REPEAT, GL_REPEAT);

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

			textProg->addAttribute("vertex");
			textProg->addUniform("projection");
			textProg->addUniform("textTex");
			textProg->addUniform("textColor");

			initShadow();
			int fError = initFont();
			cout << "Font error?: " << fError << endl;
		}

		/*initiatlization of the free type types in order to include text */
		int initFont() {

			if (FT_Init_FreeType(&ft)) {
				std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
				return -1;
			}

			FT_Face face;
			/*TODO you may need to change where this points - where is the arial file for you? */
			if (FT_New_Face(ft, "/usr/share/fonts/droid/DroidSans.ttf", 0, &face)) {
				std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
				return -1;
			}
			else {
	        // set size to load glyphs as
				FT_Set_Pixel_Sizes(face, 0, 18);

	        // disable byte-alignment restriction
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	        // load first 128 characters of ASCII set
				for (unsigned char c = 0; c < 128; c++)
				{
	            // Load character glyph 
					if (FT_Load_Char(face, c, FT_LOAD_RENDER))
					{
						std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
						continue;
					}
	            // generate texture
					unsigned int texture;
					glGenTextures(1, &texture);
					glBindTexture(GL_TEXTURE_2D, texture);
					glTexImage2D(
						GL_TEXTURE_2D,
						0,
						GL_RED,
						face->glyph->bitmap.width,
						face->glyph->bitmap.rows,
						0,
						GL_RED,
						GL_UNSIGNED_BYTE,
						face->glyph->bitmap.buffer
						);
	            // set texture options
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	            // now store character for later use
					Character character = {
						texture,
						glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
						glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
						static_cast<unsigned int>(face->glyph->advance.x)
					};
					Characters.insert(std::pair<char, Character>(c, character));
				}
				glBindTexture(GL_TEXTURE_2D, 0);
			}	

			return glGetError();
			//cout << "End of init font: " << glGetError() << endl;	
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

		/* set up the projection matrix for the font render */
		mat4 setTextProj(shared_ptr<Program> curShade)  {
			glm::mat4 proj = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
			glUniformMatrix4fv(curShade->getUniform("projection"), 1, GL_FALSE, value_ptr(proj));

		}

		/*
		Draw the scene (cubes, cats)
		Textures can be turned on an off (as shadow map depth drawing does not need textures)
		*/
		void drawScene(shared_ptr<Program> shader, GLint texID0, GLint texID1, bool TexOn, bool norMap) {

			//for cubes and ground plane attempt to normalmap
			if (TexOn) {
				glUniform1i(shader->getUniform("normMapFlag"), 1);
				texture0->bind(texID0);
				texture1->bind(texID1);
			}
			//draw the cube
			SetModel(vec3(1.5, -0.4, -7), gObjRot+1.2, 0, 1.3, shader);
			//draw with BTN data if final shading pass
			cube->draw(shader, true && norMap);

			SetModel(vec3(-2, -0.4, -7), gObjRot, 0, 1.3, shader);
			cube->draw(shader, false);

			if (TexOn) {
				texture0->bind(texID0);
				flowers->bind(texID1);
			}
			//draw the ground plane only if not shadow mapping
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

			//for fat kitty don't normal map
			if (TexOn) {
				glUniform1i(shader->getUniform("normMapFlag"), 0);
			}
			/* wanting more geometry int the scene */
			mat4 tranOrig = glm::translate(mat4(1.0), -1.0f * gD_trans);
			mat4 sc = glm::scale(mat4(1.0), vec3(gD_scale * 1.5));
			mat4 rot = glm::rotate(glm::mat4(1.0f), -radians(0.0f), vec3(1, 0, 0));
			mat4 rot2 = glm::rotate(glm::mat4(1.0f), -radians(90.0f), vec3(0, 1, 0));

			mat4 Dmod = sc*rot2*rot*tranOrig;
			mat4 ctm;
			float tx, tz, theta = 0.0;
			/* tail pivot transform */
			mat4 tailPivTO = translate(glm::mat4(1.0f),-1.0f*fatcatMesh[fatcatMesh.size()-3]->min);
			mat4 rotTail = rotate(glm::mat4(1.0f), (float)(1.0f+sin(glfwGetTime()))*-3.14f/3.0f, vec3(1, 0, 0));
			mat4 tailPiv = translate(glm::mat4(1.0f),fatcatMesh[fatcatMesh.size()-3]->min);

			/* draw ten cats waving their tails - note somewhat chaotic multi-shape coloring
			   resolved colors by hand */
			for (int i = 0; i < 10; i++) {
				tx = (6.0f) * sin(theta);
				tz = (6.0f) * cos(theta);
				mat4 trans;
				mat4 r1, r2;
				trans = translate(glm::mat4(1.0f), vec3(tx, -0.1f, tz));
				r2 = rotate(glm::mat4(1.0f), 0.0f + theta, vec3(0, 1, 0));
				ctm = trans*r2*Dmod;
				int id = shader->getUniform("M");
				glUniformMatrix4fv(shader->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
				//draw some body parts as
				//tabby aka wood texture
				if (TexOn) {
					texture0->bind(texID0);
					wood->bind(texID1);
				}
				fatcatMesh[0]->draw(shader, false);
				for (int j = 7; j <= 8; j++) {
					fatcatMesh[j]->draw(shader, false);
				}
				for (int j = 16; j < fatcatMesh.size()-3; j++) {
					fatcatMesh[j]->draw(shader, false);
				}
				fatcatMesh[fatcatMesh.size()-1]->draw(shader, false); //main body
				//animate the tail
				for (int j = fatcatMesh.size()-3; j < fatcatMesh.size()-1; j++) {
					glUniformMatrix4fv(shader->getUniform("M"), 1, GL_FALSE, value_ptr(ctm*tailPiv*rotTail*tailPivTO));
					fatcatMesh[j]->draw(shader, false);
				}
				//reset transform to normal for other body parts
				glUniformMatrix4fv(shader->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
				//draw eye balls as white
				if (TexOn) {
					texture0->bind(texID0);
					eyeColor->bind(texID1);
				}
				fatcatMesh[12]->draw(shader, false); //left eye
				fatcatMesh[13]->draw(shader, false); //right eye
				//draw the pupils and nose, whiskers black
				if (TexOn) {
					texture0->bind(texID0);
					pupil->bind(texID1);
				}
				fatcatMesh[10]->draw(shader, false);  //right pupil
				fatcatMesh[11]->draw(shader, false); //left pupil
				fatcatMesh[9]->draw(shader, false); //nose
				for (int j = 1; j <= 6; j++) { //whiskers
					fatcatMesh[j]->draw(shader, false);
				}
				for (int j = 14; j < 16; j++) { //nose
					fatcatMesh[j]->draw(shader, false);
				}
				theta += 6.28f / 10.f;
			}

		}

		/* helper function modified from learnOpenGL to fit with our programs */
		void RenderText(shared_ptr<Program> textProg, std::string text, float x, float y, float scale, glm::vec3 color){
	    	// activate corresponding render state	
			textProg->bind();

			//cout << "Text render error 1: " << glGetError() << endl;

			//set the projection matrix
			setTextProj(textProg);
			glUniform3f(textProg->getUniform("textColor"), color.x, color.y, color.z);

			//cout << "Text render error 2: " << glGetError() << endl;
			glActiveTexture(GL_TEXTURE0);
			glBindVertexArray(TextVAO);
			//cout << "Text render error 3: " << glGetError() << endl;
	    	// iterate through all characters
			std::string::const_iterator c;
			for (c = text.begin(); c != text.end(); c++) 
			{
				Character ch = Characters[*c];

				float xpos = x + ch.Bearing.x * scale;
				float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

				float w = ch.Size.x * scale;
				float h = ch.Size.y * scale;
	        	// update VBO for each character
				float vertices[6][4] = {
					{ xpos,     ypos + h,   0.0f, 0.0f },            
					{ xpos,     ypos,       0.0f, 1.0f },
					{ xpos + w, ypos,       1.0f, 1.0f },

					{ xpos,     ypos + h,   0.0f, 0.0f },
					{ xpos + w, ypos,       1.0f, 1.0f },
					{ xpos + w, ypos + h,   1.0f, 0.0f }           
				};
	        	// render glyph texture over quad
				glBindTexture(GL_TEXTURE_2D, ch.TextureID);
				//cout << "Text render error 4: " << glGetError() << endl;
	        	// update content of VBO memory
				glBindBuffer(GL_ARRAY_BUFFER, TextVBO);
				//cout << "Text render error 4.5: " << glGetError() << endl;
	        	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData
				//cout << "Text render error 5: " << glGetError() << endl;

	        	// render quad
	        	glDrawArrays(GL_TRIANGLES, 0, 6);

	        	glBindBuffer(GL_ARRAY_BUFFER, 0);
	        	// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
	        	x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	    	}
	    	//cout << "Text render error 6: " << glGetError() << endl;
	    	glBindVertexArray(0);
	    	glBindTexture(GL_TEXTURE_2D, 0);
	    	textProg->unbind();
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

		glDisable(GL_BLEND);
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

		glEnable(GL_BLEND);

		RenderText(textProg, "Cats are cute.  Cur time: " + to_string(glfwGetTime()), 10.0f, 265.0f, 1.0f, glm::vec3(0.9, 0.5f, 0.9f));
		RenderText(textProg, "Cats can be cute.  Cur time: " + to_string(glfwGetTime()), 10.0f, 205.0f, 1.0f, glm::vec3(0.9, 0.5f, 0.9f));
	}

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

