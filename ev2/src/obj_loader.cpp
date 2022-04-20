
#include <map>
#include <unordered_map>
#include <algorithm>
#include <iostream>

#include <glm/glm.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <buffer.h>
#include <resource.h>

struct DrawObject {
    size_t start;
    size_t numTriangles;
    size_t material_id;
};

namespace {

glm::vec3 CalcNormal(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2)
{
    glm::vec3 v10 = v1 - v0;
    glm::vec3 v20 = v2 - v0;

    glm::vec3 N{};
    N.x = v10.y * v20.z - v10.z * v20.y;
    N.y = v10.z * v20.x - v10.x * v20.z;
    N.z = v10.x * v20.y - v10.y * v20.x;

    float len2 = N.x * N.x + N.y * N.y + N.z * N.z;
    if (len2 > 0.0f)
    {
        float len = sqrtf(len2);
        N.x /= len;
        N.y /= len;
        N.z /= len;
    }
    return N;
}

bool hasSmoothingGroup(const tinyobj::shape_t &shape)
{
    for (size_t i = 0; i < shape.mesh.smoothing_group_ids.size(); i++)
    {
        if (shape.mesh.smoothing_group_ids[i] > 0)
        {
            return true;
        }
    }
    return false;
}

void computeSmoothingNormals(const tinyobj::attrib_t &attrib, const tinyobj::shape_t &shape,
                             std::map<int, glm::vec3> &smoothVertexNormals)
{
    smoothVertexNormals.clear();
    std::map<int, glm::vec3>::iterator iter;

    for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++)
    {
        // Get the three indexes of the face (all faces are triangular)
        tinyobj::index_t idx0 = shape.mesh.indices[3 * f + 0];
        tinyobj::index_t idx1 = shape.mesh.indices[3 * f + 1];
        tinyobj::index_t idx2 = shape.mesh.indices[3 * f + 2];

        // Get the three vertex indexes and coordinates
        int vi[3];      // indexes
        glm::vec3 v[3]; // coordinates

        for (int k = 0; k < 3; k++)
        {
            vi[0] = idx0.vertex_index;
            vi[1] = idx1.vertex_index;
            vi[2] = idx2.vertex_index;
            assert(vi[0] >= 0);
            assert(vi[1] >= 0);
            assert(vi[2] >= 0);

            v[0][k] = attrib.vertices[3 * vi[0] + k];
            v[1][k] = attrib.vertices[3 * vi[1] + k];
            v[2][k] = attrib.vertices[3 * vi[2] + k];
        }

        // Compute the normal of the face
        glm::vec3 normal = CalcNormal(v[0], v[1], v[2]);

        // Add the normal to the three vertexes
        for (size_t i = 0; i < 3; ++i)
        {
            iter = smoothVertexNormals.find(vi[i]);
            if (iter != smoothVertexNormals.end())
            {
                // add
                iter->second.x += normal[0];
                iter->second.y += normal[1];
                iter->second.z += normal[2];
            }
            else
            {
                smoothVertexNormals[vi[i]].x = normal[0];
                smoothVertexNormals[vi[i]].y = normal[1];
                smoothVertexNormals[vi[i]].z = normal[2];
            }
        }

    } // f

    // Normalize the normals, that is, make them unit vectors
    for (iter = smoothVertexNormals.begin(); iter != smoothVertexNormals.end();
         iter++)
    {
        iter->second = glm::normalize(iter->second);
    }

} // computeSmoothingNormals

void computeAllSmoothingNormals(tinyobj::attrib_t &attrib,
                                       std::vector<tinyobj::shape_t> &shapes)
{
    glm::vec3 p[3];
    for (size_t s = 0, slen = shapes.size(); s < slen; ++s)
    {
        const tinyobj::shape_t &shape(shapes[s]);
        size_t facecount = shape.mesh.num_face_vertices.size();
        assert(shape.mesh.smoothing_group_ids.size());

        for (size_t f = 0, flen = facecount; f < flen; ++f)
        {
            for (unsigned int v = 0; v < 3; ++v)
            {
                tinyobj::index_t idx = shape.mesh.indices[3 * f + v];
                assert(idx.vertex_index != -1);
                p[v].x = attrib.vertices[3 * idx.vertex_index];
                p[v].y = attrib.vertices[3 * idx.vertex_index + 1];
                p[v].z = attrib.vertices[3 * idx.vertex_index + 2];
            }

            // cross(p[1] - p[0], p[2] - p[0])
            float nx = (p[1][1] - p[0][1]) * (p[2][2] - p[0][2]) -
                       (p[1][2] - p[0][2]) * (p[2][1] - p[0][1]);
            float ny = (p[1][2] - p[0][2]) * (p[2][0] - p[0][0]) -
                       (p[1][0] - p[0][0]) * (p[2][2] - p[0][2]);
            float nz = (p[1][0] - p[0][0]) * (p[2][1] - p[0][1]) -
                       (p[1][1] - p[0][1]) * (p[2][0] - p[0][0]);

            // Don't normalize here.
            for (unsigned int v = 0; v < 3; ++v)
            {
                tinyobj::index_t idx = shape.mesh.indices[3 * f + v];
                attrib.normals[3 * idx.normal_index] += nx;
                attrib.normals[3 * idx.normal_index + 1] += ny;
                attrib.normals[3 * idx.normal_index + 2] += nz;
            }
        }
    }

    assert(attrib.normals.size() % 3 == 0);
    for (size_t i = 0, nlen = attrib.normals.size() / 3; i < nlen; ++i)
    {
        tinyobj::real_t &nx = attrib.normals[3 * i];
        tinyobj::real_t &ny = attrib.normals[3 * i + 1];
        tinyobj::real_t &nz = attrib.normals[3 * i + 2];
        tinyobj::real_t len = sqrtf(nx * nx + ny * ny + nz * nz);
        tinyobj::real_t scale = len == 0 ? 0 : 1 / len;
        nx *= scale;
        ny *= scale;
        nz *= scale;
    }
}

void computeSmoothingShape(tinyobj::attrib_t &inattrib, tinyobj::shape_t &inshape,
                                  std::vector<std::pair<unsigned int, unsigned int>> &sortedids,
                                  unsigned int idbegin, unsigned int idend,
                                  std::vector<tinyobj::shape_t> &outshapes,
                                  tinyobj::attrib_t &outattrib)
{
    unsigned int sgroupid = sortedids[idbegin].first;
    bool hasmaterials = inshape.mesh.material_ids.size();
    // Make a new shape from the set of faces in the range [idbegin, idend).
    outshapes.emplace_back();
    tinyobj::shape_t &outshape = outshapes.back();
    outshape.name = inshape.name;
    // Skip lines and points.

    std::unordered_map<unsigned int, unsigned int> remap;
    for (unsigned int id = idbegin; id < idend; ++id)
    {
        unsigned int face = sortedids[id].second;

        outshape.mesh.num_face_vertices.push_back(3); // always triangles
        if (hasmaterials)
            outshape.mesh.material_ids.push_back(inshape.mesh.material_ids[face]);
        outshape.mesh.smoothing_group_ids.push_back(sgroupid);
        // Skip tags.

        for (unsigned int v = 0; v < 3; ++v)
        {
            tinyobj::index_t inidx = inshape.mesh.indices[3 * face + v], outidx;
            assert(inidx.vertex_index != -1);
            auto iter = remap.find(inidx.vertex_index);
            // Smooth group 0 disables smoothing so no shared vertices in that case.
            if (sgroupid && iter != remap.end())
            {
                outidx.vertex_index = (*iter).second;
                outidx.normal_index = outidx.vertex_index;
                outidx.texcoord_index = (inidx.texcoord_index == -1) ? -1 : outidx.vertex_index;
            }
            else
            {
                assert(outattrib.vertices.size() % 3 == 0);
                unsigned int offset = static_cast<unsigned int>(outattrib.vertices.size() / 3);
                outidx.vertex_index = outidx.normal_index = offset;
                outidx.texcoord_index = (inidx.texcoord_index == -1) ? -1 : offset;
                outattrib.vertices.push_back(inattrib.vertices[3 * inidx.vertex_index]);
                outattrib.vertices.push_back(inattrib.vertices[3 * inidx.vertex_index + 1]);
                outattrib.vertices.push_back(inattrib.vertices[3 * inidx.vertex_index + 2]);
                outattrib.normals.push_back(0.0f);
                outattrib.normals.push_back(0.0f);
                outattrib.normals.push_back(0.0f);
                if (inidx.texcoord_index != -1)
                {
                    outattrib.texcoords.push_back(inattrib.texcoords[2 * inidx.texcoord_index]);
                    outattrib.texcoords.push_back(inattrib.texcoords[2 * inidx.texcoord_index + 1]);
                }
                remap[inidx.vertex_index] = offset;
            }
            outshape.mesh.indices.push_back(outidx);
        }
    }
}

void computeSmoothingShapes(tinyobj::attrib_t &inattrib,
                                   std::vector<tinyobj::shape_t> &inshapes,
                                   std::vector<tinyobj::shape_t> &outshapes,
                                   tinyobj::attrib_t &outattrib)
{
    for (size_t s = 0, slen = inshapes.size(); s < slen; ++s)
    {
        tinyobj::shape_t &inshape = inshapes[s];

        unsigned int numfaces = static_cast<unsigned int>(inshape.mesh.smoothing_group_ids.size());
        assert(numfaces);
        std::vector<std::pair<unsigned int, unsigned int>> sortedids(numfaces);
        for (unsigned int i = 0; i < numfaces; ++i)
            sortedids[i] = std::make_pair(inshape.mesh.smoothing_group_ids[i], i);
        sort(sortedids.begin(), sortedids.end());

        unsigned int activeid = sortedids[0].first;
        unsigned int id = activeid, idbegin = 0, idend = 0;
        // Faces are now bundled by smoothing group id, create shapes from these.
        while (idbegin < numfaces)
        {
            while (activeid == id && ++idend < numfaces)
                id = sortedids[idend].first;
            computeSmoothingShape(inattrib, inshape, sortedids, idbegin, idend,
                                  outshapes, outattrib);
            activeid = id;
            idbegin = idend;
        }
    }
}

} // namespace

static bool LoadObjAndConvert(glm::vec3 &bmin, glm::vec3 &bmax,
                        std::vector<DrawObject> *drawObjects,
                        std::vector<tinyobj::material_t> &materials,
                    //    std::map<std::string, GLuint> &textures,
                        std::vector<float> &buffer,
                        const std::string &filename,
                        std::string base_dir)
{
    tinyobj::attrib_t inattrib;
    std::vector<tinyobj::shape_t> inshapes;

    if (base_dir.empty())
    {
        base_dir = ".";
    }
#ifdef _WIN32
    base_dir += "\\";
#else
    base_dir += "/";
#endif

    std::string warn;
    std::string err;
    bool ret = tinyobj::LoadObj(&inattrib, &inshapes, &materials, &warn, &err, filename.data(),
                                base_dir.c_str());
    if (!warn.empty())
    {
        std::cout << "WARN: " << warn << std::endl;
    }
    if (!err.empty())
    {
        std::cerr << err << std::endl;
    }

    if (!ret)
    {
        std::cerr << "Failed to load " << filename << std::endl;
        return false;
    }

    printf("# of vertices  = %d\n", (int)(inattrib.vertices.size()) / 3);
    printf("# of normals   = %d\n", (int)(inattrib.normals.size()) / 3);
    printf("# of texcoords = %d\n", (int)(inattrib.texcoords.size()) / 2);
    printf("# of materials = %d\n", (int)materials.size());
    printf("# of shapes    = %d\n", (int)inshapes.size());

    // Append `default` material
    materials.push_back(tinyobj::material_t());

    for (size_t i = 0; i < materials.size(); i++)
    {
        printf("material[%d].diffuse_texname = %s\n", int(i),
               materials[i].diffuse_texname.c_str());
    }

    // Load diffuse textures
    // {
    //     for (size_t m = 0; m < materials.size(); m++)
    //     {
    //         tinyobj::material_t *mp = &materials[m];

    //         if (mp->diffuse_texname.length() > 0)
    //         {
    //             // Only load the texture if it is not already loaded
    //             if (textures.find(mp->diffuse_texname) == textures.end())
    //             {
    //                 GLuint texture_id;
    //                 int w, h;
    //                 int comp;

    //                 std::string texture_filename = mp->diffuse_texname;
    //                 if (!FileExists(texture_filename))
    //                 {
    //                     // Append base dir.
    //                     texture_filename = base_dir + mp->diffuse_texname;
    //                     if (!FileExists(texture_filename))
    //                     {
    //                         std::cerr << "Unable to find file: " << mp->diffuse_texname
    //                                   << std::endl;
    //                         exit(1);
    //                     }
    //                 }

    //                 unsigned char *image =
    //                     stbi_load(texture_filename.c_str(), &w, &h, &comp, STBI_default);
    //                 if (!image)
    //                 {
    //                     std::cerr << "Unable to load texture: " << texture_filename
    //                               << std::endl;
    //                     exit(1);
    //                 }
    //                 std::cout << "Loaded texture: " << texture_filename << ", w = " << w
    //                           << ", h = " << h << ", comp = " << comp << std::endl;

    //                 glGenTextures(1, &texture_id);
    //                 glBindTexture(GL_TEXTURE_2D, texture_id);
    //                 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //                 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //                 if (comp == 3)
    //                 {
    //                     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB,
    //                                  GL_UNSIGNED_BYTE, image);
    //                 }
    //                 else if (comp == 4)
    //                 {
    //                     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
    //                                  GL_UNSIGNED_BYTE, image);
    //                 }
    //                 else
    //                 {
    //                     assert(0); // TODO
    //                 }
    //                 glBindTexture(GL_TEXTURE_2D, 0);
    //                 stbi_image_free(image);
    //                 textures.insert(std::make_pair(mp->diffuse_texname, texture_id));
    //             }
    //         }
    //     }
    // }

    bmin[0] = bmin[1] = bmin[2] = std::numeric_limits<float>::max();
    bmax[0] = bmax[1] = bmax[2] = -std::numeric_limits<float>::max();

    bool regen_all_normals = inattrib.normals.size() == 0;
    tinyobj::attrib_t outattrib;
    std::vector<tinyobj::shape_t> outshapes;
    if (regen_all_normals)
    {
        computeSmoothingShapes(inattrib, inshapes, outshapes, outattrib);
        computeAllSmoothingNormals(outattrib, outshapes);
    }

    std::vector<tinyobj::shape_t> &shapes = regen_all_normals ? outshapes : inshapes;
    tinyobj::attrib_t &attrib = regen_all_normals ? outattrib : inattrib;

    {
        //std::vector<float> buffer; // pos(3float), normal(3float), color(3float), texcoord(2float)
        for (size_t s = 0; s < shapes.size(); s++)
        {
            size_t buffer_start = buffer.size();
            // Check for smoothing group and compute smoothing normals
            std::map<int, glm::vec3> smoothVertexNormals;
            if (!regen_all_normals && (hasSmoothingGroup(shapes[s]) > 0))
            {
                std::cout << "Compute smoothingNormal for shape [" << s << "]" << std::endl;
                computeSmoothingNormals(attrib, shapes[s], smoothVertexNormals);
            }

            for (size_t f = 0; f < shapes[s].mesh.indices.size() / 3; f++)
            {
                tinyobj::index_t idx0 = shapes[s].mesh.indices[3 * f + 0];
                tinyobj::index_t idx1 = shapes[s].mesh.indices[3 * f + 1];
                tinyobj::index_t idx2 = shapes[s].mesh.indices[3 * f + 2];

                int current_material_id = shapes[s].mesh.material_ids[f];

                if ((current_material_id < 0) ||
                    (current_material_id >= static_cast<int>(materials.size())))
                {
                    // Invaid material ID. Use default material.
                    current_material_id =
                        materials.size() -
                        1; // Default material is added to the last item in `materials`.
                }
                // if (current_material_id >= materials.size()) {
                //    std::cerr << "Invalid material index: " << current_material_id <<
                //    std::endl;
                //}
                //
                float diffuse[3];
                for (size_t i = 0; i < 3; i++)
                {
                    diffuse[i] = materials[current_material_id].diffuse[i];
                }
                float tc[3][2];
                if (attrib.texcoords.size() > 0)
                {
                    if ((idx0.texcoord_index < 0) || (idx1.texcoord_index < 0) ||
                        (idx2.texcoord_index < 0))
                    {
                        // face does not contain valid uv index.
                        tc[0][0] = 0.0f;
                        tc[0][1] = 0.0f;
                        tc[1][0] = 0.0f;
                        tc[1][1] = 0.0f;
                        tc[2][0] = 0.0f;
                        tc[2][1] = 0.0f;
                    }
                    else
                    {
                        assert(attrib.texcoords.size() >
                               size_t(2 * idx0.texcoord_index + 1));
                        assert(attrib.texcoords.size() >
                               size_t(2 * idx1.texcoord_index + 1));
                        assert(attrib.texcoords.size() >
                               size_t(2 * idx2.texcoord_index + 1));

                        // Flip Y coord.
                        tc[0][0] = attrib.texcoords[2 * idx0.texcoord_index];
                        tc[0][1] = 1.0f - attrib.texcoords[2 * idx0.texcoord_index + 1];
                        tc[1][0] = attrib.texcoords[2 * idx1.texcoord_index];
                        tc[1][1] = 1.0f - attrib.texcoords[2 * idx1.texcoord_index + 1];
                        tc[2][0] = attrib.texcoords[2 * idx2.texcoord_index];
                        tc[2][1] = 1.0f - attrib.texcoords[2 * idx2.texcoord_index + 1];
                    }
                }
                else
                {
                    tc[0][0] = 0.0f;
                    tc[0][1] = 0.0f;
                    tc[1][0] = 0.0f;
                    tc[1][1] = 0.0f;
                    tc[2][0] = 0.0f;
                    tc[2][1] = 0.0f;
                }

                glm::vec3 v[3];
                for (int k = 0; k < 3; k++)
                {
                    int f0 = idx0.vertex_index;
                    int f1 = idx1.vertex_index;
                    int f2 = idx2.vertex_index;
                    assert(f0 >= 0);
                    assert(f1 >= 0);
                    assert(f2 >= 0);

                    v[0][k] = attrib.vertices[3 * f0 + k];
                    v[1][k] = attrib.vertices[3 * f1 + k];
                    v[2][k] = attrib.vertices[3 * f2 + k];
                    bmin[k] = std::min(v[0][k], bmin[k]);
                    bmin[k] = std::min(v[1][k], bmin[k]);
                    bmin[k] = std::min(v[2][k], bmin[k]);
                    bmax[k] = std::max(v[0][k], bmax[k]);
                    bmax[k] = std::max(v[1][k], bmax[k]);
                    bmax[k] = std::max(v[2][k], bmax[k]);
                }

                glm::vec3 n[3];
                {
                    bool invalid_normal_index = false;
                    if (attrib.normals.size() > 0)
                    {
                        int nf0 = idx0.normal_index;
                        int nf1 = idx1.normal_index;
                        int nf2 = idx2.normal_index;

                        if ((nf0 < 0) || (nf1 < 0) || (nf2 < 0))
                        {
                            // normal index is missing from this face.
                            invalid_normal_index = true;
                        }
                        else
                        {
                            for (int k = 0; k < 3; k++)
                            {
                                assert(size_t(3 * nf0 + k) < attrib.normals.size());
                                assert(size_t(3 * nf1 + k) < attrib.normals.size());
                                assert(size_t(3 * nf2 + k) < attrib.normals.size());
                                n[0][k] = attrib.normals[3 * nf0 + k];
                                n[1][k] = attrib.normals[3 * nf1 + k];
                                n[2][k] = attrib.normals[3 * nf2 + k];
                            }
                        }
                    }
                    else
                    {
                        invalid_normal_index = true;
                    }

                    if (invalid_normal_index && !smoothVertexNormals.empty())
                    {
                        // Use smoothing normals
                        int f0 = idx0.vertex_index;
                        int f1 = idx1.vertex_index;
                        int f2 = idx2.vertex_index;

                        if (f0 >= 0 && f1 >= 0 && f2 >= 0)
                        {
                            n[0] = smoothVertexNormals[f0];
                            n[1] = smoothVertexNormals[f1];
                            n[2] = smoothVertexNormals[f2];

                            invalid_normal_index = false;
                        }
                    }

                    if (invalid_normal_index)
                    {
                        // compute geometric normal
                        n[0] = CalcNormal(v[0], v[1], v[2]);
                        n[1] = n[0];
                        n[2] = n[0];
                    }
                }

                for (int k = 0; k < 3; k++)
                {
                    buffer.push_back(v[k][0]);
                    buffer.push_back(v[k][1]);
                    buffer.push_back(v[k][2]);
                    buffer.push_back(n[k][0]);
                    buffer.push_back(n[k][1]);
                    buffer.push_back(n[k][2]);
                    // Combine normal and diffuse to get color.
                    float normal_factor = 0.2;
                    float diffuse_factor = 1 - normal_factor;
                    float c[3] = {n[k][0] * normal_factor + diffuse[0] * diffuse_factor,
                                  n[k][1] * normal_factor + diffuse[1] * diffuse_factor,
                                  n[k][2] * normal_factor + diffuse[2] * diffuse_factor};
                    float len2 = c[0] * c[0] + c[1] * c[1] + c[2] * c[2];
                    if (len2 > 0.0f)
                    {
                        float len = sqrtf(len2);

                        c[0] /= len;
                        c[1] /= len;
                        c[2] /= len;
                    }
                    buffer.push_back(c[0] * 0.5 + 0.5);
                    buffer.push_back(c[1] * 0.5 + 0.5);
                    buffer.push_back(c[2] * 0.5 + 0.5);

                    buffer.push_back(tc[k][0]);
                    buffer.push_back(tc[k][1]);
                }
            }

            if (buffer.size() - buffer_start > 0)
            {
                DrawObject o{0, 0, 0};
                o.start = (buffer_start) / (3 + 3 + 3 + 2) /
                                 3;
                o.numTriangles = (buffer.size() - buffer_start) / (3 + 3 + 3 + 2) /
                                 3; // 3:vtx, 3:normal, 3:col, 2:texcoord

                printf("shape[%d] # of triangles = %d\n", static_cast<int>(s),
                       o.numTriangles);

                // OpenGL viewer does not support texturing with per-face material.
                if (shapes[s].mesh.material_ids.size() > 0 &&
                    shapes[s].mesh.material_ids.size() > s)
                {
                    o.material_id = shapes[s].mesh.material_ids[0]; // use the material ID
                                                                    // of the first face.
                    if (int(o.material_id) == -1)
                        o.material_id = 0; // default material
                }
                else
                {
                    o.material_id = materials.size() - 1; // = ID for default material.
                }
                printf("shape[%d] material_id %d\n", int(s), int(o.material_id));

                drawObjects->push_back(o);
            }

        }

    }

    printf("bmin = %f, %f, %f\n", bmin[0], bmin[1], bmin[2]);
    printf("bmax = %f, %f, %f\n", bmax[0], bmax[1], bmax[2]);

    return true;
}

namespace ev2 {

std::unique_ptr<Model> loadObj(const std::filesystem::path& filename, const std::filesystem::path& base_dir) {
    glm::vec3 bmin, bmax;
    std::vector<DrawObject> drawObjects;
    std::vector<tinyobj::material_t> materials;
    std::vector<float> buffer;
    std::cout << base_dir / filename << std::endl;
    bool success = LoadObjAndConvert(bmin, bmax, &drawObjects, materials, buffer, (base_dir / filename).generic_string(), base_dir.generic_string());
    if (success) {
        std::vector<Material> ev_materials(materials.size());
        std::vector<Mesh> ev_meshs(drawObjects.size());

        size_t i = 0;
        for (auto& m : materials) {
            // copy materials
            ev_materials[i++] = Material {
                m.name
                ,glm::vec3{m.ambient[0], m.ambient[1], m.ambient[2]}
                ,glm::vec3{m.diffuse[0], m.diffuse[1], m.diffuse[2]}
                ,glm::vec3{m.specular[0], m.specular[1], m.specular[2]}
                ,glm::vec3{m.transmittance[0], m.transmittance[1], m.transmittance[2]}
                ,glm::vec3{m.emission[0], m.emission[1], m.emission[2]}
                ,m.shininess
                ,m.ior
                ,m.dissolve
                ,m.ambient_texname
                ,m.diffuse_texname
                ,m.specular_texname
                ,m.specular_highlight_texname
                ,m.bump_texname
                ,m.displacement_texname
                ,m.alpha_texname
                ,m.reflection_texname
            };
        }

        if (ev_materials.size() > 1)
            ev_materials[0] = Material{}; // default material

        // copy index information
        i = 0;
        for (auto& dObj : drawObjects) {
            ev_meshs[i++] = Mesh {
                dObj.start * 3,
                dObj.numTriangles * 3,
                dObj.material_id
            };
        }

        return std::make_unique<Model>(
            std::move(ev_meshs),
            std::move(ev_materials),
            bmin,
            bmax,
            VertexBuffer::vbInitArrayVertexData(buffer)
        );

    }
    return {};
}

} // namespace ev2