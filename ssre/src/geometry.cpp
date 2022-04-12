/**
 * @file geometry.cpp
 * @author Hunter Borlik 
 * @brief Geometry implementation. Connects attributes and shader program
 * @version 0.1
 * @date 2019-10-09
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <cassert>
#include <algorithm>

#include <geometry.h>
#include <material.h>
#include <renderer.h>
#include <uniform.h>
#include <resource.h>


using namespace ssre;

Geometry::Geometry(std::vector<DrawObject> dobjs, const std::vector<GLfloat>& data) : 
    buffer{std::make_unique<Buffer>(gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW)},
    drawObjects{std::move(dobjs)} {

    std::vector<GLfloat> scaledData{data};

    float max_x, min_x, max_y, min_y, max_z, min_z;
    max_x = max_y = max_z = std::numeric_limits<float>::min();
    min_x = min_y = min_x = std::numeric_limits<float>::max();

    for(size_t i = 0; i < data.size(); i+=GeomSizeAndStride) {
        max_x = std::max(max_x, data[i]);
        max_y = std::max(max_y, data[i+1]);
        max_z = std::max(max_z, data[i+2]);

        min_x = std::min(min_x, data[i]);
        min_y = std::min(min_y, data[i+1]);
        min_z = std::min(min_z, data[i+2]);
    }

    float x_extent, y_extent, z_extent;
    x_extent = max_x - min_x;
    y_extent = max_y - min_y;
    z_extent = max_z - min_z;
    float max_extent = std::max(x_extent, std::max(y_extent, z_extent));
    float scale = 2.f / max_extent;
    
    float offset_x = min_x + x_extent / 2.f;
    float offset_y = min_y + y_extent / 2.f;
    float offset_z = min_z + z_extent / 2.f;
    
    // linearly transform all points
    for(size_t i = 0; i < data.size(); i+=GeomSizeAndStride) {
        scaledData[i] = scale * (data[i] - offset_x);
        scaledData[i+1] = scale * (data[i+1] - offset_y);
        scaledData[i+2] = scale * (data[i+2] - offset_z);

        glm::vec3 tangent{
            scaledData[i+3],
            scaledData[i+4],
            scaledData[i+5]
        };
        tangent = glm::normalize(tangent);
        scaledData[i+3] = tangent[0];
        scaledData[i+4] = tangent[1];
        scaledData[i+5] = tangent[2];

        glm::vec3 bitangent{
            scaledData[i+6],
            scaledData[i+7],
            scaledData[i+8]
        };
        bitangent = glm::normalize(bitangent);
        scaledData[i+6] = bitangent[0];
        scaledData[i+7] = bitangent[1];
        scaledData[i+8] = bitangent[2];

    }

    buffer->CopyData(scaledData);
}

Geometry::Geometry(float uvextent) : 
    buffer{std::make_unique<Buffer>(gl::BindingTarget::ARRAY, gl::Usage::STATIC_DRAW)} {
    
    drawObjects.push_back(DrawObject{});
    drawObjects[0].numElements = 6;
    drawObjects[0].element_buffer->CopyData(std::vector<GLuint>{0, 3, 1, 0, 2, 3});

    std::vector<float> data;
    // point 1
    data.push_back(0.5);
    data.push_back(0);
    data.push_back(-0.5);

    data.push_back(1);
    data.push_back(0);
    data.push_back(0);

    data.push_back(0);
    data.push_back(0);
    data.push_back(-1);

    data.push_back(0);
    data.push_back(0);

    // point 2
    data.push_back(0.5);
    data.push_back(0);
    data.push_back(0.5);

    data.push_back(1);
    data.push_back(0);
    data.push_back(0);

    data.push_back(0);
    data.push_back(0);
    data.push_back(-1);

    data.push_back(0);
    data.push_back(1 * uvextent);

    // point 3
    data.push_back(-0.5);
    data.push_back(0);
    data.push_back(-0.5);

    data.push_back(1);
    data.push_back(0);
    data.push_back(0);

    data.push_back(0);
    data.push_back(0);
    data.push_back(-1);

    data.push_back(1 * uvextent);
    data.push_back(0);

    // point 4
    data.push_back(-0.5);
    data.push_back(0);
    data.push_back(0.5);

    data.push_back(1);
    data.push_back(0);
    data.push_back(0);

    data.push_back(0);
    data.push_back(0);
    data.push_back(-1);

    data.push_back(1 * uvextent);
    data.push_back(1 * uvextent);

    buffer->CopyData(data);
}

void Geometry::configVaoAttribPtrs(uint32_t shape, GLuint vao_id, GLint vert_loc, GLint tan_loc, GLint bitan_loc, GLint texc_loc) {
    assert(shape < drawObjects.size());
    glBindVertexArray(vao_id);
    buffer->Bind();

    // bind element buffer
    drawObjects[shape].element_buffer->Bind();

    if(vert_loc != -1) {
        //glEnableVertexAttribArray(vert_loc);
        // specify stride in bytes
        //glBindVertexBuffer(0, geometry->getVertexBufferHandle(), 0, sizeof(GLfloat)*3);
        // specify binding and number of float elements per value
        //glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
        //glVertexAttribBinding(0, vert_loc);
        glEnableVertexAttribArray(vert_loc);
        glVertexAttribPointer(vert_loc, 3, GL_FLOAT, GL_FALSE, GeomSizeAndStrideBytes, (void*)GeomVertexOffsetBytes);
    }

    if(tan_loc != -1) {
        glEnableVertexAttribArray(tan_loc);
        glVertexAttribPointer(tan_loc, 3, GL_FLOAT, GL_FALSE, GeomSizeAndStrideBytes, (void*)(GeomTangentOffsetBytes));
    }

    if(bitan_loc != -1) {
        glEnableVertexAttribArray(bitan_loc);
        glVertexAttribPointer(bitan_loc, 3, GL_FLOAT, GL_FALSE, GeomSizeAndStrideBytes, (void*)(GeomBitangentOffset));
    }

    if(texc_loc != -1) {
        //glEnableVertexAttribArray(tex_loc);
        // specify stride in bytes
        //glBindVertexBuffer(2, geometry->getTexBufferHandle(), 0, sizeof(GLfloat)*2);
        // specify binding and number of float elements per value
        /* glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, 0);
        glVertexAttribBinding(2, tex_loc); */
        glEnableVertexAttribArray(texc_loc);
        glVertexAttribPointer(texc_loc, 2, GL_FLOAT, GL_FALSE, GeomSizeAndStrideBytes, (void*)(GeomTexCoordOffsetBytes));
    }

    buffer->Unbind();
}