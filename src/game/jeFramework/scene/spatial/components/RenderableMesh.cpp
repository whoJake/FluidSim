#include "RenderableMesh.h"
#include "loaders/obj_waveform.h"

namespace fw
{

RenderableMeshComponent::RenderableMeshComponent(std::string source) :
    m_hash(source),
    m_source(source)
{ }

void RenderableMeshComponent::load()
{
    if( m_mesh )
    {
        return;
    }

	fiDevice file;
	file.open(m_source.c_str());

	obj::file source;
	source.parse(file);

	m_mesh = std::make_unique<mtl::mesh>();
	m_mesh->add_submesh();
	mtl::submesh& submesh = m_mesh->get_submesh(0);

	submesh.add_channel(); // vertex
	submesh.add_channel(); // normal

	submesh.set_material_name(mtl::hash_string("material1"));

	const auto& objects = source.get_objects();
	const auto& vertices = source.get_vertices();
	const auto& normals = source.get_normals();

	for( const auto& obj : objects )
	{
		for( const auto& tri : obj.get_triangles() )
		{
			glm::vec3 v0 = vertices[tri.vertices[0].position];
			glm::vec3 v1 = vertices[tri.vertices[1].position];
			glm::vec3 v2 = vertices[tri.vertices[2].position];

			glm::vec3 n0 = normals[tri.vertices[0].normal];
			glm::vec3 n1 = normals[tri.vertices[1].normal];
			glm::vec3 n2 = normals[tri.vertices[2].normal];

			submesh.get_channel(0).push_back(glm::vec4(v0, 1.f));
			submesh.get_channel(1).push_back(glm::vec4(n0, 1.f));

			submesh.get_channel(0).push_back(glm::vec4(v1, 1.f));
			submesh.get_channel(1).push_back(glm::vec4(n1, 1.f));

			submesh.get_channel(0).push_back(glm::vec4(v2, 1.f));
			submesh.get_channel(1).push_back(glm::vec4(n2, 1.f));
		}
	}
}

void RenderableMeshComponent::unload()
{
    m_mesh.reset();
}

const mtl::mesh& RenderableMeshComponent::get_mesh() const
{
    return *m_mesh;
}

const mtl::hash_string& RenderableMeshComponent::get_hash_string() const
{
    return m_hash;
}

} // fw