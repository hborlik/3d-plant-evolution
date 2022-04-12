/**
 * @file simpleShading.h
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-11-13
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#pragma once
#ifndef SSRE_SIMPLE_SHADING_H
#define SSRE_SIMPLE_SHADING_H

#include <shader.h>

namespace ssre {

class SimpleShading : public Program {
public:
    SimpleShading() : Program{"simple_shading"} {
        
    }
    virtual ~SimpleShading() = default;

    std::unique_ptr<Material> createMaterial(const MaterialInfo& mcfg) const override;

protected:
    void onBuilt() override;
};

class PBRShading : public Program {
public:
    PBRShading() : Program{"pbr_shading"} {}
    virtual ~PBRShading() = default;

    std::unique_ptr<Material> createMaterial(const MaterialInfo& mcfg) const override;

protected:
    void onBuilt() override;
};

}

#endif // SSRE_SIMPLE_SHADING_H