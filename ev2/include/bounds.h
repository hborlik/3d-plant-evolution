/**
 * @file box.h
 * @brief 
 * @version 0.2
 * @date 2020-10-12
 * 
 * [2022-05-11] : integration ev2
 */
#ifndef EV2_BOUNDS_H
#define EV2_BOUNDS_H

#include <limits>

#include <util.h>
#include <geometry.h>

namespace ev2 {

template<typename T>
class Bounds3 {
public:
    const point3<T> pMin;
    const point3<T> pMax;

    Bounds3() : pMin{std::numeric_limits<T>().lowest()}, pMax{std::numeric_limits<T>().max()} {}
    Bounds3(point3<T> pMin, point3<T> pMax) : pMin{pMin}, pMax{pMax} {}

    vec3<T> diagonal() const noexcept {return pMax - pMin;}

    T volume() const noexcept {
        auto d = diagonal();
        return d.x() * d.y() * d.z();
    }


    template<typename... P>
    static Bounds3<T> FromPoints(vec3<P>... args) {
        vec3<T> minp{util::vmin(args.x()...), util::vmin(args.y()...), util::vmin(args.z()...)};
        vec3<T> maxp{util::vmax(args.x()...), util::vmax(args.y()...), util::vmax(args.z()...)};

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
            float invRD = 1 / r.d[i];
            float tn = (pMin[i] - r.o[i]) * invRD;
            float tf = (pMax[i] - r.o[i]) * invRD;
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

using Bounds3f = Bounds3<float>;
using Bounds3d = Bounds3<double>;

} // namespace ev2

#endif // EV2_BOUNDS_H