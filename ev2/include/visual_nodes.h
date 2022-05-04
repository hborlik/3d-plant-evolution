/**
 * @file visual_instance.h
 * @brief 
 * @date 2022-04-27
 * 
 */
#ifndef EV2_VISUAL_INSTANCE_H
#define EV2_VISUAL_INSTANCE_H

#include <scene.h>
#include <mesh.h>
#include <renderer.h>

namespace ev2 {

class VisualInstance : public Node {
public:
    explicit VisualInstance(const std::string &name) : Node{name} {}

    void on_init() override;
    void on_ready() override;
    void on_process(float delta) override;
    void on_destroy() override;

    void set_model(MID model);
    void set_material_override(int32_t material_override);

private:
    IID iid{};
};

class CameraNode : public Node {
public:
    CameraNode(const std::string &name) : Node{name} {}

    void on_init() override;
    void on_ready() override;
    void on_process(float delta) override;
    void on_destroy() override;

    void set_fov(const glm::vec3& color);

private:
    Camera camera{};
    float fov = 60.0f;
};

class LightInstance : public Node {
public:
    LightInstance(const std::string &name) : Node{name} {}

    void on_init() override;
    void on_ready() override;
    void on_process(float delta) override;
    void on_destroy() override;

    void set_color(const glm::vec3& color);

private:
    LID lid{};
};

}

#endif // EV2_VISUAL_INSTANCE_H