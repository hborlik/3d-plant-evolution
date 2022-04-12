/**
 * @file resource.cpp
 * @author Hunter Borlik 
 * @brief 
 * @version 0.1
 * @date 2019-11-06
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <iostream>
#include <vector>

#include <resource.h>
#include <texture.h>
#include <geometry.h>
#include <material.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <tiny_obj_wrapper.h>

#include <glm/gtx/string_cast.hpp>

using namespace ssre;

Resource::Resource(std::string assetPath) : assetPath{std::move(assetPath)} {
    stbi_set_flip_vertically_on_load(true);
}

Resource::~Resource() {

}

std::shared_ptr<Texture> Resource::getTexture(const std::string& name) {
    auto itr = textures.find(name);
    if(itr != textures.end()) {
        return itr->second;
    }
    return {};
}

std::shared_ptr<Texture> Resource::load2DTexture(const std::string& path) {
    std::shared_ptr<Texture> out;
    bool error = false;
    if(!path.empty()) {
        out = getTexture(path);
        if(!out) {
            std::string file = assetPath + path;
            int width, height, nrChannels;
            unsigned char *image = stbi_load(file.c_str(), &width, &height, &nrChannels, 0);
            if(image) {
                std::cout << "Loaded texture data: " << file << ", w = " << width
                    << ", h = " << height << ", channels = " << nrChannels << std::endl;                

                std::shared_ptr<Texture2D> tex = std::make_shared<Texture2D>(path);
                if(nrChannels == 1) {
                    tex->setData(image, gl::PixelFormat::RED, gl::PixelType::UNSIGNED_BYTE, gl::TextureInternalFormat::RED, width, height);
                } else if(nrChannels == 3) { // rgb
                    tex->setData(image, gl::PixelFormat::RGB, gl::PixelType::UNSIGNED_BYTE, gl::TextureInternalFormat::RGB, width, height);
                } else if(nrChannels == 4) { // rgba
                    tex->setData(image, gl::PixelFormat::RGBA, gl::PixelType::UNSIGNED_BYTE, gl::TextureInternalFormat::RGBA, width, height);
                } else {
                    error = true;
                    std::cerr << "unable to load unsupported texture format." + file + " Channels:" + std::to_string(nrChannels) << std::endl;
                }
                stbi_image_free(image);

                out = tex;
                textures.insert(std::make_pair(out->getName(), out));
            } else {
                std::cerr << "Unable to load texture: " + file << std::endl;
                error = true;
            }
        }
    } else
        error = true;

    if(error) {
        const std::string error_tex_name = "ERROR_TEXTURE";
        out = getTexture(error_tex_name);
        if(!out) {
            std::cout << "Creating error texture" << std::endl;
            std::shared_ptr<Texture2D> tex = std::make_shared<Texture2D>(error_tex_name);
            std::vector<unsigned char> err_data;
            const uint32_t width = 100, height = 100;
            for(uint32_t i = 0; i < 100; ++i) {
                for(uint32_t j = 0; j < 100; ++j) { // make error color
                    err_data.push_back(244);
                    err_data.push_back(31);
                    err_data.push_back(237);
                }
            }
            tex->setData(err_data.data(), gl::PixelFormat::RGB, gl::PixelType::UNSIGNED_BYTE, gl::TextureInternalFormat::RGB, width, height);
            out = tex;
            textures.insert(std::make_pair(error_tex_name, out));
        }
    }
    out->generateMipMap();
    return out;
}

std::shared_ptr<Geometry> Resource::loadObj(const std::string& file, const std::string& baseDir) {
    const float FaceDPCutoff = 0.7f; // prevent tangents from varying too much
    std::string fullName = baseDir + file;
    auto geom = geometries.find(fullName);
    if(geom == geometries.end()) {
        std::cout << "Resource: load " << fullName << std::endl;
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> mat_array;
        std::string warn;
        std::string err;
        bool rc = tinyobj::LoadObj(&attrib, &shapes, &mat_array, &warn, &err, (assetPath + fullName).c_str(), (assetPath + baseDir).c_str());

        struct Vertex {
            glm::vec3 position{};
            glm::vec3 tangent{};
            glm::vec3 bitangent{};
            glm::vec3 normal{};
            glm::vec2 texpos{-1, -1};
        };
        std::vector<Vertex> vertices(attrib.vertices.size() / 3);

        // each vertex has GeomSizeAndStride values associated with it
        //std::vector<GLfloat> buffer_data((attrib.vertices.size() / 3) * GeomSizeAndStride);
        std::vector<GLfloat> buffer_data;

        SSRE_CHECK_THROW(rc, err);

        if (shapes.size() > 0) {

            std::vector<MaterialInfo> materials;
            std::vector<DrawObject> objects;

            // load materials
            materials.push_back(MaterialInfo{}); // default material at mat_id 0
            for(auto& mat : mat_array) {
                materials.push_back(MaterialInfo{tinyobj_wrapper::material_t{mat}, baseDir});
            }

            // Loop over shapes
            for (size_t s = 0; s < shapes.size(); ++s) {
                // Loop over faces(polygon)
                const std::size_t num_faces = shapes[s].mesh.num_face_vertices.size();

                size_t index_offset = 0;
                // num face vertices array size is number of faces in shape
                std::vector<GLuint> element_data;
                for (size_t f = 0; f < num_faces; f++) {
                    // get all face info
                    glm::vec3 verts[3] = {};
                    bool normalsValid = false;
                    glm::vec3 normals[3] = {};
                    bool coordsValid = false;
                    glm::vec2 coords[3] = {};
                    glm::vec3 tans[3] = {};
                    glm::vec3 bitans[3] = {};
                    tinyobj::index_t indices[3] = {};
                    for (size_t v = 0; v < 3; v++) {                    
                        indices[v] = shapes[s].mesh.indices[index_offset + v];

                        uint32_t vertex_index = 3*indices[v].vertex_index;
                        uint32_t normal_index = 3*indices[v].normal_index;
                        uint32_t tex_index = 2*indices[v].texcoord_index;

                        assert(indices[v].vertex_index >= 0);

                        verts[v] = {
                            attrib.vertices[vertex_index+0],
                            attrib.vertices[vertex_index+1],
                            attrib.vertices[vertex_index+2]
                        };

                        if(attrib.normals.size() > 0) {
                            if(normal_index >= 0) {
                                normals[v] = {
                                    attrib.normals[normal_index+0],
                                    attrib.normals[normal_index+1],
                                    attrib.normals[normal_index+2]
                                };
                                normals[v] = glm::normalize(normals[v]);
                                normalsValid = true;
                            } else {
                                std::cerr << "Warn: vertex has no normal index" << std::endl;
                            }
                        } else {
                            std::cout << "Warm: " << fullName << " has no normals" << std::endl;
                        }

                        if(attrib.texcoords.size() > 0) {
                            if(tex_index >= 0) {
                                coords[v] = {
                                    attrib.texcoords[tex_index+0],
                                    - attrib.texcoords[tex_index+1]
                                };
                                coordsValid = true;
                            }
                        } else {
                            std::cout << "Warm: " << fullName << " has no texture coordinates" << std::endl;
                        }
                    }

                    bool forceNewVert[3] = {};
                    if(coordsValid) {
                        glm::vec3 dp0 = verts[1] - verts[0];
                        glm::vec3 dp1 = verts[2] - verts[0];

                        glm::vec2 duv1 = coords[1] - coords[0];
                        glm::vec2 duv0 = coords[2] - coords[0];

                        float r = 1.f / (duv0.x*duv1.y - duv0.y*duv1.x);
                        glm::vec3 ftangent = glm::normalize((dp0*duv1.y - dp1*duv0.y)*r);
                        glm::vec3 fbitangent = glm::normalize((dp1*duv0.x - dp0*duv1.x)*r);

                        // create normal corrected tangents and bitangents
                        float n0dp = glm::dot(ftangent, normals[0]);
                        float n1dp = glm::dot(ftangent, normals[1]);
                        float n2dp = glm::dot(ftangent, normals[2]);

                        glm::vec3 tangent[3];
                        tangent[0] = glm::normalize(ftangent - n0dp * normals[0]);
                        tangent[1] = glm::normalize(ftangent - n1dp * normals[1]);
                        tangent[2] = glm::normalize(ftangent - n2dp * normals[2]);

                        n0dp = glm::dot(fbitangent, normals[0]);
                        n1dp = glm::dot(fbitangent, normals[1]);
                        n2dp = glm::dot(fbitangent, normals[2]);

                        glm::vec3 bitangent[3];
                        // bitangent[0] = fbitangent - n0dp * normals[0];
                        // bitangent[1] = fbitangent - n1dp * normals[1];
                        // bitangent[2] = fbitangent - n2dp * normals[2];
                        bitangent[0] = glm::normalize(glm::cross(normals[0], tangent[0]));
                        bitangent[1] = glm::normalize(glm::cross(normals[1], tangent[1]));
                        bitangent[2] = glm::normalize(glm::cross(normals[2], tangent[2]));

                        // assign normal corrected tangents and bitangents
                        tans[0] = tangent[0];
                        tans[1] = tangent[1];
                        tans[2] = tangent[2];
                        bitans[0] = bitangent[0];
                        bitans[1] = bitangent[1];
                        bitans[2] = bitangent[2];
                        // tans[0] = ftangent;
                        // tans[1] = ftangent;
                        // tans[2] = ftangent;
                        // bitans[0] = fbitangent;
                        // bitans[1] = fbitangent;
                        // bitans[2] = fbitangent;

                        glm::vec3 avgN = normals[0] + normals[1] + normals[2];
                        avgN = glm::normalize(avgN);
                        float avdp = glm::dot(glm::normalize(glm::cross(ftangent, fbitangent)), avgN);
                        if(avdp < 0) {
                            std::swap(tans[0], bitans[0]);
                            std::swap(tans[1], bitans[1]);
                            std::swap(tans[2], bitans[2]);
                            std::swap(ftangent, fbitangent);
                        }

                        for (uint32_t v = 0; v < 3; v++){
                            // mesh normal
                            glm::vec3 n = glm::normalize(normals[v]);
                            // existing normal
                            glm::vec3 en = vertices[indices[v].vertex_index].normal;
                            if(en != glm::vec3{0, 0, 0}) {
                                float dp = 1;
                                dp = glm::dot(en, n);
                                // check if faces are different
                                if(dp < 0.8) {
                                    forceNewVert[v] = true;
                                    continue;
                                }
                            }
                            
                            // check existing against texture coordinates
                            if(vertices[indices[v].vertex_index].texpos != glm::vec2{-1, -1}) {
                                // check if vertex texture coords differ from those at assigned index
                                if(glm::length(vertices[indices[v].vertex_index].texpos - coords[v]) > 0.001) {
                                    forceNewVert[v] = true;
                                    continue;
                                }
                            }
                        }

                        

                        // std::cout << "n0: " << glm::to_string(normals[0]) << std::endl;
                        // std::cout << "n1: " << glm::to_string(normals[1]) << std::endl;
                        // std::cout << "n2: " << glm::to_string(normals[2]) << std::endl;

                        // std::cout << "fndp: " << glm::dot(glm::normalize(normals[0]), glm::normalize(glm::cross(tangent0, bitangent0))) << std::endl;
                        // std::cout << "fndp: " << glm::dot(glm::normalize(normals[1]), glm::normalize(glm::cross(tangent1, bitangent1))) << std::endl;
                        // std::cout << "fndp: " << glm::dot(glm::normalize(normals[2]), glm::normalize(glm::cross(tangent2, bitangent2))) << std::endl;

                    } else {
                        std::cout << "Warm: " << fullName << " is missing texture coordinates, cannot find tangent and bitangent" << std::endl;
                    }

                    // save data
                    for(uint32_t v = 0; v < 3; ++v) {
                        uint32_t elem_ind = indices[v].vertex_index;

                        if(forceNewVert[v]) {
                            elem_ind = vertices.size();
                            vertices.push_back({
                                verts[v],
                                tans[v],
                                bitans[v],
                                normals[v],
                                coords[v]
                            });
                        } else {
                            vertices[elem_ind] = {
                                verts[v],
                                vertices[elem_ind].tangent + tans[v],
                                vertices[elem_ind].bitangent + bitans[v],
                                normals[v],
                                coords[v]
                            };
                        }

                        element_data.push_back(elem_ind);
                    }
                    index_offset += 3;
                }

                // create buffer data
                uint32_t nElems = element_data.size();
                std::vector<GLuint> newElements;
                for(uint32_t i = 0; i < nElems; i++) {
                    uint32_t ind = element_data[i];

                    uint32_t dind = ind * GeomSizeAndStride;

                    // if(ind == tind) {
                        // buffer_data[dind] = vertices[ind][0];
                        // buffer_data[dind+1] = vertices[ind][1];
                        // buffer_data[dind+2] = vertices[ind][2];

                        // buffer_data[dind+3] = tangents[ind][0];
                        // buffer_data[dind+4] = tangents[ind][1];
                        // buffer_data[dind+5] = tangents[ind][2];

                        // buffer_data[dind+6] = bitangents[ind][0];
                        // buffer_data[dind+7] = bitangents[ind][1];
                        // buffer_data[dind+8] = bitangents[ind][2];

                        // buffer_data[dind+9] = texcoords[tind][0];
                        // buffer_data[dind+10] = texcoords[tind][1];
                    // } else {
                        newElements.push_back(buffer_data.size() / GeomSizeAndStride);

                        buffer_data.push_back(vertices[ind].position[0]);
                        buffer_data.push_back(vertices[ind].position[1]);
                        buffer_data.push_back(vertices[ind].position[2]);

                        buffer_data.push_back(vertices[ind].tangent[0]);
                        buffer_data.push_back(vertices[ind].tangent[1]);
                        buffer_data.push_back(vertices[ind].tangent[2]);

                        buffer_data.push_back(vertices[ind].bitangent[0]);
                        buffer_data.push_back(vertices[ind].bitangent[1]);
                        buffer_data.push_back(vertices[ind].bitangent[2]);

                        buffer_data.push_back(vertices[ind].texpos[0]);
                        buffer_data.push_back(vertices[ind].texpos[1]);
                    // }

                }

                // create new draw object
                DrawObject dobj{};
                dobj.numElements = newElements.size();
                // the first material for the shape. id is increased by 1 for default material id being -1
                dobj.material_id = shapes[s].mesh.material_ids[0] + 1;
                dobj.element_buffer->CopyData<GLuint>(newElements);

                objects.push_back(std::move(dobj));
            }

            // make new geometry
            std::shared_ptr<Geometry> newGeom = std::make_shared<Geometry>(std::move(objects), buffer_data);
            newGeom->setMaterials(materials);

            geometries.insert(std::make_pair(fullName, newGeom));
            return newGeom;
        }
        return {};
    }
    return geom->second;
}