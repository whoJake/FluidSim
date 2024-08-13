#include "MeshSource.h"

namespace fw
{
namespace ecs
{

MeshSource::MeshSource(const mtl::hash_string& source) :
    m_source(source)
{ }

const mtl::hash_string& MeshSource::get_source() const
{
    return m_source;
}

} // ecs
} // fw