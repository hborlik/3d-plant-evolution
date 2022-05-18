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

#if 0
static Ref<Node> loadNode() {

}

std::unique_ptr<GLTFScene> ResourceManager::loadGLTF(const std::filesystem::path& filename, const std::filesystem::path& base_dir, bool normalize) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;
    std::string input_filename = (asset_path / filename).generic_string();
    std::string ext = filename.extension();
    std::unique_ptr<GLTFScene> scene = std::make_unique<GLTFScene>(filename);

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

    VertexBuffer state = VertexBuffer::vbInitDefault();
    state.bind();

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
            state.add_buffer(i, Buffer{
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

            state.add_buffer(i, Buffer{
                (gl::BindingTarget)bufferView.target,
                gl::Usage::STATIC_DRAW,
                bufferView.byteLength,
                tmp_buffer}
            );
            delete[] tmp_buffer;
        }
        glBindBuffer(bufferView.target, 0);
    }

    for (size_t i_node = 0; i_node < model.nodes.size(); i_node++) {
        if (model.nodes[i_node].mesh != -1) { // this node has a mesh
            tinygltf::Mesh& mesh = model.meshes[model.nodes[i_node].mesh];
            for (size_t i = 0; i < mesh.primitives.size(); i++)
            {
                const tinygltf::Primitive &primitive = mesh.primitives[i];

                if (primitive.indices < 0)
                    continue;

                // Assume TEXTURE_2D target for the texture object.
                // glBindTexture(GL_TEXTURE_2D, gMeshState[mesh.name].diffuseTex[i]);

                std::map<std::string, int>::const_iterator it(primitive.attributes.begin());
                std::map<std::string, int>::const_iterator itEnd(
                    primitive.attributes.end());

                for (; it != itEnd; it++)
                {
                    assert(it->second >= 0);
                    const tinygltf::Accessor &accessor = model.accessors[it->second];
                    state.get_buffer(accessor.bufferView).Bind();
                    int size = 1;
                    if (accessor.type == TINYGLTF_TYPE_SCALAR)
                    {
                        size = 1;
                    }
                    else if (accessor.type == TINYGLTF_TYPE_VEC2)
                    {
                        size = 2;
                    }
                    else if (accessor.type == TINYGLTF_TYPE_VEC3)
                    {
                        size = 3;
                    }
                    else if (accessor.type == TINYGLTF_TYPE_VEC4)
                    {
                        size = 4;
                    }
                    else
                    {
                        assert(0);
                    }
                    // it->first would be "POSITION", "NORMAL", "TEXCOORD_0", ...
                    if ((it->first.compare("POSITION") == 0) ||
                        (it->first.compare("NORMAL") == 0) ||
                        (it->first.compare("TEXCOORD_0") == 0))
                    {

                        // Compute byteStride from Accessor + BufferView combination.
                        int byteStride =
                            accessor.ByteStride(model.bufferViews[accessor.bufferView]);
                        assert(byteStride != -1);
                        const uint32_t location = gl::glBindingLocation(it->first);
                        glVertexAttribPointer(location, size,
                                                accessor.componentType,
                                                accessor.normalized ? GL_TRUE : GL_FALSE,
                                                byteStride, BUFFER_OFFSET(accessor.byteOffset));
                        CheckErrors("vertex attrib pointer");
                        glEnableVertexAttribArray(location);
                        CheckErrors("enable vertex attrib array");
                    }
                }

                const tinygltf::Accessor &indexAccessor =
                    model.accessors[primitive.indices];
                state.get_buffer(indexAccessor.bufferView).Bind();
                int mode = -1;
                if (primitive.mode == TINYGLTF_MODE_TRIANGLES)
                {
                    mode = GL_TRIANGLES;
                }
                else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_STRIP)
                {
                    mode = GL_TRIANGLE_STRIP;
                }
                else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_FAN)
                {
                    mode = GL_TRIANGLE_FAN;
                }
                else if (primitive.mode == TINYGLTF_MODE_POINTS)
                {
                    mode = GL_POINTS;
                }
                else if (primitive.mode == TINYGLTF_MODE_LINE)
                {
                    mode = GL_LINES;
                }
                else if (primitive.mode == TINYGLTF_MODE_LINE_LOOP)
                {
                    mode = GL_LINE_LOOP;
                }
                else
                {
                    assert(0);
                }
                glDrawElements(mode, indexAccessor.count, indexAccessor.componentType,
                            BUFFER_OFFSET(indexAccessor.byteOffset));
                CheckErrors("draw elements");
            }
        }
    }

    scene->vertex_buffer = ev2::Renderer::get_singleton().create_vertex_buffer(std::move(state));

    return {};
}
#endif

}