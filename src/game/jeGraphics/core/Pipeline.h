#pragma once

#include "vkcommon.h"
#include "Resource.h"
#include "PipelineState.h"

namespace vk
{

class Pipeline : public Resource<VkPipeline>
{
public:
    Pipeline(Device& device, PipelineState state);
    virtual ~Pipeline();

    Pipeline(Pipeline&& other);
    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(Pipeline&&) = delete;
    Pipeline& operator=(const Pipeline&&) = delete;

    const PipelineState& get_state() const;
protected:
    PipelineState m_state;
};

class GraphicsPipeline : public Pipeline
{
public:
    GraphicsPipeline(Device&         device,
                     VkPipelineCache cache,
                     PipelineState   state);
    virtual ~GraphicsPipeline() = default;

    GraphicsPipeline(GraphicsPipeline&&) = default;
};

} // vk