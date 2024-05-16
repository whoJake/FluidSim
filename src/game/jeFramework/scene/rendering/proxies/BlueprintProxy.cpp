#include "BlueprintProxy.h"

BlueprintProxy::BlueprintProxy(vk::RenderContext* context, Blueprint* blueprint) :
    m_blueprint(blueprint),
    m_materialProxy(0),
    m_meshProxy(context, &blueprint->mesh())
{ }

void BlueprintProxy::sync()
{
    m_meshProxy.sync();
}

bpid_t BlueprintProxy::get_id() const
{
    return m_blueprint->get_id();
}

const MeshProxy& BlueprintProxy::get_mesh_proxy() const
{
    return m_meshProxy;
}