/**
 * @file geometry.h
 * @brief 
 * @date 2022-04-18
 * 
 */
#ifndef EV2_GEOMETRY_H
#define EV2_GEOMETRY_H

#include <optional>
#include <limits>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <util.h>
#include <ev.h>
#include <reference_counted.h>

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
    Ref<Object> hit;

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

    Ray transform(const glm::mat4& tr) const {
        return {tr * glm::vec4{origin, 1.0f}, tr * glm::vec4{direction, .0f}};
    }

    glm::vec3 eval(float t) const {return origin + t * direction;}
};

struct Shape : public ReferenceCounted<Shape> {
    glm::mat4 transform = glm::identity<glm::mat4>();

    Shape() = default;
    virtual ~Shape() = default;

    virtual bool intersect(const Ray& ray, SurfaceInteraction& hit) {return false;}
};

struct Sphere : public Shape {
    glm::vec3   center;
    float       radius;

    Sphere(const glm::vec3& center, float radius) noexcept : center{center}, radius{radius} {}

    bool intersect(const Ray& ray, SurfaceInteraction& hit) override {
        using namespace glm;
        Ray local_ray = ray.transform(inverse(transform));

        vec3 e_c = (local_ray.origin - center);
        float a = dot(local_ray.direction, local_ray.direction);
        float b = 2 * dot(local_ray.direction, e_c);
        float c = dot(e_c,e_c) - radius*radius;
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
            normalize(hit_point - center),
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
};

struct Plane {
    glm::vec4 p = {0, 1, 0, 0};

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

    /**
     * @brief intersect ray and plane, ray direction and plane should be normalized
     * 
     * @param ray 
     * @param hit 
     * @return true 
     * @return false 
     */
    bool intersect(const Ray& ray, SurfaceInteraction& hit) {
        using namespace glm;
        const float denom = dot(ray.direction, vec3(p));
        if (denom == 0.0) // parallel
            return false;
        const float t = (p[3] - dot(ray.origin, vec3(p))) / denom;
        if (t >= 0.01f) {
            SurfaceInteraction h {
                vec3(p),
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
};

struct PlaneShape : public Shape {
    Plane plane{};

    PlaneShape() noexcept = default;
    PlaneShape(const glm::vec4& p) noexcept : plane{p} {
    }

    inline void normalize() noexcept {
        plane.normalize();
    }

    /**
     * @brief Normalized plane required
     * 
     * @param point 
     * @return float 
     */
    inline float distanceFromPlane(glm::vec3 point) const noexcept {
        glm::vec3 local_point = glm::inverse(transform) * glm::vec4{point, 1.0f};
        return plane.distanceFromPlane(local_point);
    }

    /**
     * @brief intersect ray and plane, ray direction and plane should be normalized
     * 
     * @param ray 
     * @param hit 
     * @return true 
     * @return false 
     */
    bool intersect(const Ray& ray, SurfaceInteraction& hit) override {
        using namespace glm;
        Ray local_ray = ray.transform(inverse(transform));

        return plane.intersect(local_ray, hit);
    }
};

/**
 * @brief axis aligned box
 * 
 * @tparam T 
 */
class Bounds3 {
public:
    const glm::vec3 pMin;
    const glm::vec3 pMax;

    Bounds3() : pMin{-INFINITY}, pMax{INFINITY} {}
    Bounds3(glm::vec3 pMin, glm::vec3 pMax) : pMin{pMin}, pMax{pMax} {}

    glm::vec3 diagonal() const noexcept {return pMax - pMin;}

    float volume() const noexcept {
        auto d = diagonal();
        return d.x * d.y * d.z;
    }


    template<typename... P>
    static Bounds3 FromPoints(P&&... args) {
        glm::vec3 minp{util::vmin(args.x...), util::vmin(args.y...), util::vmin(args.z...)};
        glm::vec3 maxp{util::vmax(args.x...), util::vmax(args.y...), util::vmax(args.z...)};

        return {minp, maxp};
    }

    /**
     * @brief intersect a ray with the axis aligned bounds
     * 
     * @param r ray
     * @param hit_t0 optional t0 close hit
     * @param hit_t1 optional t1 far hit
     * @return true got an intersection
     * @return false miss
     */
    bool intersect(const Ray& r, float* hit_t0 = nullptr, float* hit_t1 = nullptr) const {
        float t0 = 0;
        float t1 = INFINITY;
        // for each axis, check axis aligned plane t = (v[i] - o[i]) / d[i]
        for (int i = 0; i < 3; ++i) {
            float invRD = 1 / r.direction[i];
            float tn = (pMin[i] - r.origin[i]) * invRD;
            float tf = (pMax[i] - r.origin[i]) * invRD;
            if (tn > tf) // planes are ordered incorrectly, swap values
                std::swap(tn, tf);
            // 
            // keep the min far and max near t values
            t0 = std::max(tn, t0);
            t1 = std::min(tf, t1);
            if (t0 > t1) // shortcut check for overlap -> miss
                return false;
        }
        if (hit_t0)
            *hit_t0 = t0;
        if (hit_t1)
            *hit_t1 = t1;
        return true;
    }
};

struct Box {
    glm::mat4   transform;
    Bounds3     local_bounds;
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

}

#endif // EV2_GEOMETRY_H