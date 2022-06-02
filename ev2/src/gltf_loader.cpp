#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include <resource.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#define CheckGLErrors(desc)                                                   \
  {                                                                           \
    GLenum e = glGetError();                                                  \
    if (e != GL_NO_ERROR) {                                                   \
      printf("OpenGL error in \"%s\": %d (%d) %s:%d\n", desc, e, e, __FILE__, \
             __LINE__);                                                       \
      exit(20);                                                               \
    }                                                                         \
  }

void CheckErrors(std::string desc) {
  GLenum e = glGetError();
  if (e != GL_NO_ERROR) {
    fprintf(stderr, "OpenGL error in \"%s\": %d (%d)\n", desc.c_str(), e, e);
    exit(20);
  }
}

namespace ev2 {

static size_t ComponentTypeByteSize(int type) {
    switch (type) {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
        case TINYGLTF_COMPONENT_TYPE_BYTE:
            return sizeof(char);
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        case TINYGLTF_COMPONENT_TYPE_SHORT:
            return sizeof(short);
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        case TINYGLTF_COMPONENT_TYPE_INT:
            return sizeof(int);
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            return sizeof(float);
        case TINYGLTF_COMPONENT_TYPE_DOUBLE:
            return sizeof(double);
        default:
            return 0;
    }
}

void decompose(glm::mat4 mat, glm::vec3 &pos, glm::quat &quat, glm::vec3 &scale)
{
    // extract position
    pos = mat[3];
    mat[3] = { 0,0,0,1 };

    // extract the scales
    for (int i = 0; i < 3; ++i)
    {
        scale[i] = glm::length(mat[i]);
        mat[i] /= scale[i];
    }

    // extract the remaining rotation
    quat = glm::quat_cast(mat);
}


static Ref<Node> loadNode(Ref<GLTFScene> scene, Ref<Node> parent, const tinygltf::Model& model, const tinygltf::Node& node) {
    glm::vec3 pos;
    glm::quat quat;
    glm::vec3 scale;
    Ref<Node> ev_node;
    if (node.matrix.size() == 16) {
        // Use `matrix' attribute
        glm::mat4 mat{
            node.matrix[0], node.matrix[1], node.matrix[2], node.matrix[3],
            node.matrix[4], node.matrix[5], node.matrix[6], node.matrix[7],
            node.matrix[8], node.matrix[9], node.matrix[10], node.matrix[11],
            node.matrix[12], node.matrix[13], node.matrix[14], node.matrix[15]
        };

        decompose(mat, pos, quat, scale);
    } else {
        pos = glm::vec3{node.translation[0], node.translation[1], node.translation[2]};
        quat = glm::quat{node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]};
        scale = glm::vec3{node.scale[0], node.scale[1], node.scale[2]};
    }

    if (node.mesh > 0) {
        ev_node = scene->create_node<GLTFNode>(node.name);
        // TODO create mesh instance
    } else {
        // regular node
        ev_node = scene->create_node<Node>(node.name);
    }
    ev_node->transform.position = pos;
    ev_node->transform.rotation = quat;
    ev_node->transform.scale = scale;
    return ev_node;
}

Ref<GLTFScene> ResourceManager::loadGLTF(const std::filesystem::path& filename, bool normalize) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;
    std::string input_filename = (asset_path / filename).generic_string();
    std::string ext = filename.extension().generic_string();

    bool ret = false;
    if (ext.compare("glb") == 0)
    {
        // assume binary glTF.
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, input_filename);
    }
    else
    {
        // assume ascii glTF.
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, input_filename);
    }

    if (!ret) {
        std::cerr << "Failed to load " << input_filename << std::endl;
        return {};
    }

    Ref<GLTFScene> scene = make_referenced<GLTFScene>(filename.generic_string());
    VertexBuffer* state = scene->get_vertex_buffer();
    renderer::VBID vertex_buffer_id = scene->get_vertex_buffer_id();    

    for (std::size_t i = 0; i < model.bufferViews.size(); i++)
    {
        const tinygltf::BufferView &bufferView = model.bufferViews[i];
        if (bufferView.target == 0)
        {
            std::cout << "WARN: bufferView.target is zero" << std::endl;
            continue; // Unsupported bufferView.
        }

        int sparse_accessor = -1;
        for (size_t a_i = 0; a_i < model.accessors.size(); ++a_i)
        {
            const auto &accessor = model.accessors[a_i];
            if (accessor.bufferView == i)
            {
                std::cout << i << " is used by accessor " << a_i << std::endl;
                if (accessor.sparse.isSparse)
                {
                    std::cout
                        << "WARN: this bufferView has at least one sparse accessor to "
                           "it. We are going to load the data as patched by this "
                           "sparse accessor, not the original data"
                        << std::endl;
                    sparse_accessor = a_i;
                    break;
                }
            }
        }

        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

        std::cout << "buffer.size= " << buffer.data.size()
                  << ", byteOffset = " << bufferView.byteOffset << std::endl;

        if (sparse_accessor < 0) {
            state->add_buffer(i, Buffer{
                (gl::BindingTarget)bufferView.target,
                gl::Usage::STATIC_DRAW,
                bufferView.byteLength,
                &buffer.data.at(0) + bufferView.byteOffset}
            );
        } else
        {
            const auto accessor = model.accessors[sparse_accessor];
            // copy the buffer to a temporary one for sparse patching
            unsigned char *tmp_buffer = new unsigned char[bufferView.byteLength];
            memcpy(tmp_buffer, buffer.data.data() + bufferView.byteOffset,
                   bufferView.byteLength);

            const size_t size_of_object_in_buffer =
                ComponentTypeByteSize(accessor.componentType);
            const size_t size_of_sparse_indices =
                ComponentTypeByteSize(accessor.sparse.indices.componentType);

            const auto &indices_buffer_view =
                model.bufferViews[accessor.sparse.indices.bufferView];
            const auto &indices_buffer = model.buffers[indices_buffer_view.buffer];

            const auto &values_buffer_view =
                model.bufferViews[accessor.sparse.values.bufferView];
            const auto &values_buffer = model.buffers[values_buffer_view.buffer];

            for (size_t sparse_index = 0; sparse_index < accessor.sparse.count; ++sparse_index)
            {
                int index = 0;
                // std::cout << "accessor.sparse.indices.componentType = " <<
                // accessor.sparse.indices.componentType << std::endl;
                switch (accessor.sparse.indices.componentType)
                {
                case TINYGLTF_COMPONENT_TYPE_BYTE:
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                    index = (int)*(
                        unsigned char *)(indices_buffer.data.data() +
                                         indices_buffer_view.byteOffset +
                                         accessor.sparse.indices.byteOffset +
                                         (sparse_index * size_of_sparse_indices));
                    break;
                case TINYGLTF_COMPONENT_TYPE_SHORT:
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                    index = (int)*(
                        unsigned short *)(indices_buffer.data.data() +
                                          indices_buffer_view.byteOffset +
                                          accessor.sparse.indices.byteOffset +
                                          (sparse_index * size_of_sparse_indices));
                    break;
                case TINYGLTF_COMPONENT_TYPE_INT:
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                    index = (int)*(
                        unsigned int *)(indices_buffer.data.data() +
                                        indices_buffer_view.byteOffset +
                                        accessor.sparse.indices.byteOffset +
                                        (sparse_index * size_of_sparse_indices));
                    break;
                }
                std::cout << "updating sparse data at index  : " << index
                          << std::endl;
                // index is now the target of the sparse index to patch in
                const unsigned char *read_from =
                    values_buffer.data.data() +
                    (values_buffer_view.byteOffset +
                     accessor.sparse.values.byteOffset) +
                    (sparse_index * (size_of_object_in_buffer * accessor.type));

                /*
                std::cout << ((float*)read_from)[0] << "\n";
                std::cout << ((float*)read_from)[1] << "\n";
                std::cout << ((float*)read_from)[2] << "\n";
                */

                unsigned char *write_to =
                    tmp_buffer + index * (size_of_object_in_buffer * accessor.type);

                memcpy(write_to, read_from, size_of_object_in_buffer * accessor.type);
            }

            // debug:
            /*for(size_t p = 0; p < bufferView.byteLength/sizeof(float); p++)
            {
            float* b = (float*)tmp_buffer;
            std::cout << "modified_buffer [" << p << "] = " << b[p] << '\n';
            }*/

            state->add_buffer(i, Buffer{
                (gl::BindingTarget)bufferView.target,
                gl::Usage::STATIC_DRAW,
                bufferView.byteLength,
                tmp_buffer}
            );
            delete[] tmp_buffer;
        }
        glBindBuffer(bufferView.target, 0);
    }

    // copy out all accessors
    for (size_t i_accessor = 0; i_accessor < model.accessors.size(); i_accessor++) {
        tinygltf::Accessor& accessor = model.accessors[i_accessor];
        int byteStride =
            accessor.ByteStride(model.bufferViews[accessor.bufferView]);
        assert(byteStride != -1);
        int size = 1;
        if (accessor.type == TINYGLTF_TYPE_SCALAR) {
            size = 1;
        } else if (accessor.type == TINYGLTF_TYPE_VEC2) {
            size = 2;
        } else if (accessor.type == TINYGLTF_TYPE_VEC3) {
            size = 3;
        } else if (accessor.type == TINYGLTF_TYPE_VEC4) {
            size = 4;
        } else {
            assert(0);
        }

        state->add_accessor(i_accessor, accessor.bufferView,
            accessor.byteOffset,
            accessor.normalized,
            (gl::DataType)accessor.componentType,
            size, byteStride);
    }

    for (size_t i_mesh = 0; i_mesh < model.meshes.size(); i_mesh++) {
        renderer::MSID msid = ev2::renderer::Renderer::get_singleton().create_mesh();

        std::vector<renderer::MeshPrimitive> mesh_primitives{};

        tinygltf::Mesh& mesh = model.meshes[i_mesh];
        for (size_t i = 0; i < mesh.primitives.size(); i++)
        {
            const tinygltf::Primitive &primitive = mesh.primitives[i];

            if (primitive.indices < 0)
                continue;

            renderer::MeshPrimitive mesh_primitive{};
            mesh_primitive.vbid = vertex_buffer_id;
            mesh_primitive.indices = primitive.indices;
            mesh_primitive.material_id = primitive.material;

            // Assume TEXTURE_2D target for the texture object.
            // glBindTexture(GL_TEXTURE_2D, gMeshState[mesh.name].diffuseTex[i]);
            for (auto it = primitive.attributes.begin(); it != primitive.attributes.end(); it++)
            {
                assert(it->second >= 0);
                const uint32_t location = gl::glBindingLocation(it->first);
                mesh_primitive.attributes.insert_or_assign(location, it->second);
            }

            const tinygltf::Accessor &indexAccessor =
                model.accessors[primitive.indices];

            mesh_primitives.push_back(mesh_primitive);
        }

        ev2::renderer::Renderer::get_singleton().set_mesh_primitives(msid, mesh_primitives);

        scene->add_mesh(msid);
    }

    assert(model.scenes.size() > 0);
    int scene_to_display = model.defaultScene > -1 ? model.defaultScene : 0;
    const tinygltf::Scene &loaded_scene = model.scenes[scene_to_display];
    for (size_t i = 0; i < loaded_scene.nodes.size(); i++) {
        loadNode(scene, scene, model, model.nodes[loaded_scene.nodes[i]]);
    }

    return scene;
}

}