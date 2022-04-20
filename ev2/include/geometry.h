/**
 * @file geometry.h
 * @brief 
 * @date 2022-04-18
 * 
 */
#ifndef EV2_GEOMETRY_H
#define EV2_GEOMETRY_H

#include <optional>

#include <glm/glm.hpp>

namespace ev2 {

struct Sphere {
    glm::vec3   center;
    float       radius;
};

struct Plane {
    glm::vec4 p;

    inline void normalize() noexcept {
        p /= glm::length(glm::vec3(p));
    }

    /**
     * @brief Normalized plane required
     * 
     * @param point 
     * @return float 
     */
    inline float distanceFromPlane(glm::vec3 point) const noexcept {
        return p.x * point.x + p.y * point.y + p.z * point.z + p.w;
    }
};

struct Frustum {
    // 6 inward pointing planes
    Plane planes[6];

    Plane& left() noexcept  {return planes[0];}
    Plane& right() noexcept {return planes[1];}
    Plane& top() noexcept   {return planes[2];}
    Plane& bottom() noexcept{return planes[3];}
    Plane& near() noexcept  {return planes[4];}
    Plane& far() noexcept   {return planes[5];}

    const Plane& left() const noexcept  {return planes[0];}
    const Plane& right() const noexcept {return planes[1];}
    const Plane& top() const noexcept   {return planes[2];}
    const Plane& bottom() const noexcept{return planes[3];}
    const Plane& near() const noexcept  {return planes[4];}
    const Plane& far() const noexcept   {return planes[5];}

    void normalize() noexcept {
        for (int i = 0; i < 6; ++i) {
            planes[i].normalize();
        }
    }
};

/**
 * @brief check if a sphere intersects a frustum
 * 
 * @param f 
 * @param s 
 * @return bool
 */
bool intersect(const Frustum& f, const Sphere& s) noexcept {
    float dist = 0;
    for (int i = 0; i < 6; i++) {
        dist = f.planes[i].distanceFromPlane(s.center);
        //test against each plane
        if (dist < 0 && abs(dist) > s.radius) {
            return true;
        }
    }
    return false;
};

}

#endif // EV2_GEOMETRY_H