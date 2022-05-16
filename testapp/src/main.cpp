/**
 * @file main.cpp
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-11-15
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include <iostream>
#include <memory>

#include <glm/gtc/matrix_transform.hpp>

#include <ssre.h>
#include <window.h>
#include <simpleShading.h>
#include <geometry.h>
#include <buffer.h>
#include <renderer.h>
#include <resource.h>
#include <vertex_buffer.h>
#include <scene.h>
#include <camera.h>
#include <skysphere.h>

using namespace ssre;

const uint32_t width = 1920, height = 1080;

int main() {
    try {
        SSRE_init();

        Window::ConstructStatic(width, height, "Something");
        Window::StaticInst().setMouseCursorVisible(true);
        Resource::ConstructStatic("./asset/");

        // move below to Window?
        {
            const GLubyte *renderer = glGetString( GL_RENDERER );
            const GLubyte *vendor = glGetString( GL_VENDOR );
            const GLubyte *version = glGetString( GL_VERSION );
            const GLubyte *glslVersion = glGetString( GL_SHADING_LANGUAGE_VERSION );

            GLint major, minor;
            glGetIntegerv(GL_MAJOR_VERSION, &major);
            glGetIntegerv(GL_MINOR_VERSION, &minor);
            
            printf("GL Vendor : %s\n", vendor);
            printf("GL Renderer : %s\n", renderer);
            printf("GL Version (string) : %s\n", version);
            printf("GL Version (integer) : %d.%d\n", major, minor);
            printf("GLSL Version : %s\n", glslVersion);
        }


        std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>();
        auto viewPortCallback = Window::FramebufferResizeCallback::create([renderer](uint32_t width, uint32_t height){
            renderer->setProjectionMatrix(glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f));
            renderer->setViewPortSize(width, height);
        });
        viewPortCallback(width, height);
        Window::StaticInst().SetRenderer(renderer);
        Window::StaticInst().SetFramebufferResizeCallback(viewPortCallback);


        std::shared_ptr<Program> skyprog = std::make_shared<SimpleShading>();
        skyprog->SetShaderPath(gl::GLSLShaderType::VERTEX_SHADER, "./asset/shader/sky.glsl.vert");
        skyprog->SetShaderPath(gl::GLSLShaderType::FRAGMENT_SHADER, "./asset/shader/sky.glsl.frag");
        skyprog->loadAndBuild();
        std::cout << *skyprog << std::endl;

        std::shared_ptr<Program> sp = std::make_shared<PBRShading>();
        sp->SetShaderPath(gl::GLSLShaderType::VERTEX_SHADER, "./asset/shader/pbr.glsl.vert");
        sp->SetShaderPath(gl::GLSLShaderType::FRAGMENT_SHADER, "./asset/shader/pbr.glsl.frag");
        sp->loadAndBuild();
        std::cout << *sp << std::endl;

        std::shared_ptr<Program> simpleDepth = std::make_shared<SimpleShading>();
        simpleDepth->SetShaderPath(gl::GLSLShaderType::VERTEX_SHADER, "./asset/shader/depth.glsl.vert");
        simpleDepth->SetShaderPath(gl::GLSLShaderType::FRAGMENT_SHADER, "./asset/shader/depth.glsl.frag");
        simpleDepth->SetShaderPath(gl::GLSLShaderType::GEOMETRY_SHADER, "./asset/shader/depth.glsl.geom");
        simpleDepth->loadAndBuild();
        std::cout << *simpleDepth << std::endl;

        //sp->setShaderParameter("a", 1.f);

        std::shared_ptr<Geometry> cubegeom = std::make_shared<Geometry>(3.f);//Resource::StaticInst().loadObj("cube.obj");
        MaterialInfo cubemat{};
        cubemat.diffuse_tex = Resource::StaticInst().load2DTexture("dry-dirt2-albedo.png");
        cubemat.metallic_tex = Resource::StaticInst().load2DTexture("dry-dirt2-metalness.png");
        cubemat.roughness_tex = Resource::StaticInst().load2DTexture("dry-dirt2-roughness.png");
        cubemat.normal_tex = Resource::StaticInst().load2DTexture("dry-dirt2-normal2.png");
        cubemat.ambient_tex = Resource::StaticInst().load2DTexture("dry-dirt2-ao.png");
        cubegeom->setMaterialId(0, cubegeom->addMaterial(cubemat));

        std::shared_ptr<Geometry> geom = Resource::StaticInst().loadObj("sphere.obj");
        MaterialInfo spmat{};
        spmat.diffuse_tex = Resource::StaticInst().load2DTexture("rustediron2_basecolor.png");
        spmat.metallic_tex = Resource::StaticInst().load2DTexture("rustediron2_metallic.png");
        spmat.roughness_tex = Resource::StaticInst().load2DTexture("rustediron2_roughness.png");
        spmat.normal_tex = Resource::StaticInst().load2DTexture("rustediron2_normal.png");
        spmat.ambient_tex = Resource::StaticInst().load2DTexture("rustediron2_ao.png");
        geom->setMaterialId(0, geom->addMaterial(spmat));

        std::shared_ptr<Geometry> geom0 = Resource::StaticInst().loadObj("Leather_Chair_OBJ.obj", "Leather_Chair_OBJ/");
        MaterialInfo mat0{};
        mat0.diffuse_tex = Resource::StaticInst().load2DTexture("Leather_Chair_OBJ/Maps/Low_for_Zb_Low_for_Zb_initialShadingGroup_baseColor.png");
        mat0.metallic_tex = Resource::StaticInst().load2DTexture("Leather_Chair_OBJ/Maps/Low_for_Zb_Low_for_Zb_initialShadingGroup_metallic.png");
        mat0.roughness_tex = Resource::StaticInst().load2DTexture("Leather_Chair_OBJ/Maps/Low_for_Zb_Low_for_Zb_initialShadingGroup_Roughness.png");
        mat0.normal_tex = Resource::StaticInst().load2DTexture("Leather_Chair_OBJ/Maps/Low_for_Zb_Low_for_Zb_initialShadingGroup_Normal.png");
        mat0.ambient_tex = Resource::StaticInst().load2DTexture("Leather_Chair_OBJ/Maps/Low_for_Zb_Low_for_Zb_initialShadingGroup_AmbientOcclusion.png");
        size_t mat0id = geom0->addMaterial(mat0);
        geom0->setMaterialId(0, mat0id);
        geom0->setMaterialId(1, mat0id);
        geom0->setMaterialId(2, mat0id);
        geom0->setMaterialId(3, mat0id);
        geom0->setMaterialId(4, mat0id);
        geom0->setMaterialId(5, mat0id);
        geom0->setMaterialId(6, mat0id);

        std::shared_ptr<Mesh> chair = std::make_shared<Mesh>("chair", geom0, sp, simpleDepth);
        chair->setPosition(glm::vec3{5, -0.8, -10});


        std::shared_ptr<Scene> world = std::make_shared<Scene>(std::make_shared<Node>("root"));
        renderer->setScene(world);

        std::shared_ptr<Mesh> drawable_test = std::make_shared<Mesh>("shpere0", geom, sp, simpleDepth);
        drawable_test->setPosition(glm::vec3{0, 0, -10.f});
        world->addNode(drawable_test);

        std::shared_ptr<Mesh> ground = std::make_shared<Mesh>("ground", cubegeom, sp, simpleDepth);
        ground->setPosition({0, -1.5, -10});
        ground->setScale({15, 1, 15});

        std::shared_ptr<Camera> camera = std::make_shared<Camera>("cam0");
        camera->setPosition(glm::vec3{0, 0, 0});
        world->setCamera(camera);

        std::shared_ptr<PointLight> pointlight = std::make_shared<PointLight>("light0");
        pointlight->setRadiantIntensity(glm::vec3{300, 300, 300});
        pointlight->setPosition(glm::vec3{0, 15, -10});
        world->addNode(pointlight);

        std::shared_ptr<PointLight> pointlight1 = std::make_shared<PointLight>("light1");
        pointlight1->setRadiantIntensity(glm::vec3{300, 300, 300});
        pointlight1->setPosition(glm::vec3{3, 3, -10});
        world->addNode(pointlight1);

        world->addNode(chair);
        world->addNode(ground);

        MaterialInfo skymat{};
        skymat.diffuse_tex = Resource::StaticInst().load2DTexture("sky.jpg");
        std::shared_ptr<SkySphere> sky = std::make_shared<SkySphere>(skyprog, skymat);
        sky->setModelMatrix(glm::rotate(glm::mat4{1.f}, glm::pi<float>() / 2.f, glm::vec3{1, 0, 0}));
        world->setSkysphere(sky);

        Window::StaticInst().Run();
    } catch(ssre_exception e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}