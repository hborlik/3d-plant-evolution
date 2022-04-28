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

namespace ev2 {

class VisualInstance : public Node {
public:
    VisualInstance(const std::string &name) : Node{name} {}

    void on_init() override;
    void on_ready() override;
    void on_process(float delta) override;
    void on_destroy() override;

    ModelInstance *model;
};

}

#endif // EV2_VISUAL_INSTANCE_H