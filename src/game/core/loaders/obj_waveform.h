#pragma once

#include "system/device.h"
#include "data/fixed_vector.h"
#include "obj_waveform/obj_object.h"

namespace obj
{

class file
{
public:
    file();
    ~file();

    bool parse(sys::fi_device& device);

    const mtl::fixed_vector<glm::vec3>& get_vertices() const;

    const mtl::fixed_vector<glm::vec3>& get_normals() const;

    const mtl::fixed_vector<object>& get_objects() const;
private:
    void calculate_buffer_sizes(sys::fi_device& device);

    void parse_vertex(size_t context, const std::string& line);

    void parse_normal(size_t context, const std::string& line);

    void parse_face(size_t context, const std::string& line);
private:
    mtl::fixed_vector<glm::vec3> m_vertices;
    mtl::fixed_vector<glm::vec3> m_normals;

    mtl::fixed_vector<object> m_objects;
};

} // obj