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

struct SurfaceInteraction
{
    double t = 0;
    glm::vec3 point;
    glm::vec3 incoming;
    glm::vec3 normal;
    glm::vec3 tan;
    glm::vec3 bi;
    glm::vec2 uv;

    SurfaceInteraction() = default;
    SurfaceInteraction(glm::vec3 normal,
                       glm::vec3 tan,
                       glm::vec3 bi,
                       glm::vec2 uv,
                       float t,
                       glm::vec3 point,
                       glm::vec3 incoming)
        : t{t}, point{point}, incoming{incoming},
          normal{normal},
          tan{tan},
          bi{bi},
          uv{uv}
    {
    }
};

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;

    Ray(const glm::vec3& origin, const glm::vec3& direction) : origin{origin}, direction{glm::normalize(direction)} {}

    glm::vec3 eval(float t) const {return origin + t * direction;}
};

struct Sphere {
    glm::vec3   center;
    float       radius;
};

struct Plane {
    glm::vec4 p;

    Plane() noexcept = default;
    Plane(const glm::vec4& p) noexcept : p{p} {
        normalize();
    }

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
inline bool intersect(const Frustum& f, const Sphere& s) noexcept {
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

inline bool intersect(const Ray& ray, const Sphere& sph, SurfaceInteraction& hit) {
    using namespace glm;
    vec3 e_c = (ray.origin - sph.center);
    float a = dot(ray.direction, ray.direction);
    float b = 2 * dot(ray.direction, e_c);
    float c = dot(e_c,e_c) - sph.radius*sph.radius;
    float det_sq = b*b - 4*a*c;
    if(det_sq <= 0)
        return false;
    det_sq = glm::sqrt(det_sq);
    float t = -b - det_sq;
    t /= 2*a;
    if(t < 0)
        return false;

    auto hit_point = ray.eval(t);
    // hit info into surface interaction
    SurfaceInteraction h {
        normalize(hit_point - sph.center),
        {},
        {},
        {},
        t,
        hit_point,
        ray.direction
    };
    hit = h;
    return true;
}

/**
 * @brief intersect ray and plane, ray direction and plane should be normalized
 * 
 * @param ray 
 * @param p 
 * @param hit 
 * @return true 
 * @return false 
 */
inline bool intersect(const Ray& ray, const Plane& p, SurfaceInteraction& hit) {
    using namespace glm;
    const float denom = dot(ray.direction, vec3(p.p));
    if (denom == 0.0) // parallel
        return false;
    const float t = (p.p[3] - dot(ray.origin, vec3(p.p))) / denom;
    if (t >= 0.01f) {
        SurfaceInteraction h {
            vec3(p.p),
            {},
            {},
            {},
            t,
            ray.eval(t),
            ray.direction
        };
        hit = h;
        return true;
    }
    return false;
}

}

#endif // EV2_GEOMETRY_H