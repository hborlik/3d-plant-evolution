/**
 * @file visual_instance.h
 * @brief 
 * @date 2022-04-27
 * 
 */
#ifndef EV2_VISUAL_INSTANCE_H
#define EV2_VISUAL_INSTANCE_H

#include <node.h>
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

    void on_ready() override;
    void on_process(float dt) override;
    void on_destroy() override;

    void set_active();

    void set_fov(float fov) noexcept {this->fov = fov; update_internal();}
    float get_fov() noexcept {return fov;}

    void set_near_clip(float near) noexcept {this->near = near; update_internal();}
    float get_near_clip() noexcept {return near;}

    void set_far_clip(float far) noexcept {this->far = far;update_internal();}
    float get_far_clip() noexcept {return far;}

    const Camera& get_camera() const noexcept {return camera;}


private:
    void update_internal();

    Camera camera{};
    float fov = 60.0f;
    float near = 0.1f, far = 500.0f;
    float aspect = 1.0f;
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