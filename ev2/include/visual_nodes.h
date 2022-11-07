/**
 * @file visual_instance.h
 * @brief 
 * @date 2022-04-27
 * 
 */
#ifndef EV2_VISUAL_INSTANCE_H
#define EV2_VISUAL_INSTANCE_H

#include <node.h>
#include <renderer/model.h>
#include <renderer/buffer.h>
#include <renderer/renderer.h>

namespace ev2 {

class VisualInstance : public Node {
public:
    explicit VisualInstance(const std::string &name) : Node{name} {}

    void on_init() override;
    void on_ready() override;
    void on_process(float delta) override;
    void on_destroy() override;

    void pre_render() override;

    void set_model(std::shared_ptr<renderer::Drawable> model);
    void set_material_override(renderer::Material* material_override);

private:
    renderer::ModelInstance* iid{};
};

class InstancedGeometry : public Node {
public:
    explicit InstancedGeometry(const std::string &name) : Node{name} {}

    void on_init() override;
    void on_destroy() override;
    void pre_render() override;

    void set_material_override(renderer::Material* material_override);

    std::vector<glm::mat4> instance_transforms{};

protected:
    std::shared_ptr<renderer::Drawable> geometry = {};
    renderer::InstancedDrawable* instance = nullptr;
};

class CameraNode : public Node {
public:
    CameraNode(const std::string &name) : Node{name} {}

    void on_ready() override;
    void on_process(float dt) override;
    void on_destroy() override;

    void pre_render() override;

    void set_fov(float fov) noexcept {this->fov = fov;}
    float get_fov() noexcept {return fov;}

    void set_near_clip(float m_near) noexcept {this->m_near = m_near;}
    float get_near_clip() noexcept {return m_near;}

    void set_far_clip(float m_far) noexcept {this->m_far = m_far;}
    float get_far_clip() noexcept {return m_far;}

    const Camera& get_camera() noexcept {
        update_internal();
        return camera;
    }


private:
    void update_internal();

    Camera camera{};
    float fov = 60.0f;
    float m_near = 0.1f, m_far = 500.0f;
    float aspect = 1.0f;
};


class PointLightNode : public Node {
public:
    PointLightNode(const std::string &name) : Node{name} {}

    void on_init() override;
    void on_ready() override;
    void on_process(float delta) override;
    void on_destroy() override;

    void pre_render() override;

    void set_color(const glm::vec3& color);

private:
    renderer::LID lid{};
};

class DirectionalLightNode : public Node {
public:
    DirectionalLightNode(const std::string &name) : Node{name} {}

    void on_init() override;
    void on_ready() override;
    void on_process(float delta) override;
    void on_destroy() override;

    void pre_render() override;

    void set_color(const glm::vec3& color);
    void set_ambient(const glm::vec3& color);

private:
    renderer::LID lid{};
};

}

#endif // EV2_VISUAL_INSTANCE_H