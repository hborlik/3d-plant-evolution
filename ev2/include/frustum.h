/**
 * @file frustum.h
 * @brief 
 * @date 2022-04-15
 * 
 */
#ifndef EV_FRUSTUM_H
#define EV_FRUSTUM_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ev {

void ExtractVFPlanes(mat4 P, mat4 V) {
    /* composite matrix */
    mat4 comp = P*V;
    vec3 n; //use to pull out normal
    float l; //length of normal for plane normalization

    Left.x = comp[0][3] + comp[0][0]; 
    Left.y = comp[1][3] + comp[1][0]; 
    Left.z = comp[2][3] + comp[2][0]; 
    Left.w = comp[3][3] + comp[3][0];
    Left /= length(vec3{Left});
    planes[0] = Left;
    //   cout << "Left' " << Left.x << " " << Left.y << " " <<Left.z << " " << Left.w << endl;

    Right.x = comp[0][3] - comp[0][0];
    Right.y = comp[1][3] - comp[1][0];
    Right.z = comp[2][3] - comp[2][0];
    Right.w = comp[3][3] - comp[3][0];
    Right /= length(vec3{Right});
    planes[1] = Right;
    //   cout << "Right " << Right.x << " " << Right.y << " " <<Right.z << " " << Right.w << endl;

    Bottom.x = comp[0][3] + comp[0][1];
    Bottom.y = comp[1][3] + comp[1][1];
    Bottom.z = comp[2][3] + comp[2][1];
    Bottom.w = comp[3][3] + comp[3][1];
    Bottom /= length(vec3{Bottom});
    planes[2] = Bottom;
    //   cout << "Bottom " << Bottom.x << " " << Bottom.y << " " <<Bottom.z << " " << Bottom.w << endl;

    Top.x = comp[0][3] - comp[0][1];
    Top.y = comp[1][3] - comp[1][1];
    Top.z = comp[2][3] - comp[2][1];
    Top.w = comp[3][3] - comp[3][1];
    Top /= length(vec3{Top});
    planes[3] = Top;
    //   cout << "Top " << Top.x << " " << Top.y << " " <<Top.z << " " << Top.w << endl;

    Near.x = comp[0][3] + comp[0][2];
    Near.y = comp[1][3] + comp[1][2];
    Near.z = comp[2][3] + comp[2][2];
    Near.w = comp[3][3] + comp[3][2];
    Near /= length(vec3{Near});
    planes[4] = Near;
    //   cout << "Near " << Near.x << " " << Near.y << " " <<Near.z << " " << Near.w << endl;

    Far.x = comp[0][3] - comp[0][2];
    Far.y = comp[1][3] - comp[1][2];
    Far.z = comp[2][3] - comp[2][2];
    Far.w = comp[3][3] - comp[3][2];
    Far /= length(vec3{Far});
    planes[5] = Far;
    //   cout << "Far " << Far.x << " " << Far.y << " " <<Far.z << " " << Far.w << endl;
}

} // namespace ev

#endif // EV_FRUSTUM_H