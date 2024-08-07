#pragma once

#include "data/hash_string.h"

namespace vk
{

class Buffer;
class DescriptorSet;
class DescriptorSetLayout;
class RenderContext;
class RenderPass;

} // vk

namespace fw
{

class Scene;
class Blueprint;
class Entity;

namespace gfx
{

class Shader;
struct ShaderDefinition;
class Material;
struct MaterialDefinition;

} // gfx

struct GlobalSetData
{
	glm::mat4 projection;
	glm::mat4 view;

	// temp
	glm::mat4 model;
	glm::mat4 mvp;
};

struct ModelSetData
{
	glm::mat4 model;
	glm::mat4 mvp;
};

struct RenderState
{
	std::vector<Entity*> visibleEntities;

	std::unique_ptr<vk::RenderPass> renderPass;
	std::unique_ptr<vk::Buffer> globalBuffer;
};

class SceneRenderer
{
public:
	SceneRenderer(vk::RenderContext& context);
	~SceneRenderer() = default;

	void initialise_shaders(const std::vector<gfx::ShaderDefinition>& shaders);

	void pre_render(glm::vec3 position);
	void render();

	void add_scene(Scene* scene);
private:
	void setup_renderpass();
	void register_material(const gfx::MaterialDefinition& definition);
	void load_blueprint(Blueprint* blueprint);

	void set_global_buffer(GlobalSetData* data);
private:
	vk::RenderContext& m_context;
	std::vector<Scene*> m_scenes;
	RenderState m_state;

	std::unordered_map<mtl::hash_string, std::unique_ptr<gfx::Shader>> m_shaderStore;
	std::unordered_map<mtl::hash_string, std::unique_ptr<gfx::Material>> m_materialStore;

	struct BlueprintDrawData
	{
		std::unique_ptr<vk::Buffer> vertexBuffer;
	};

	std::unordered_map<mtl::hash_string, BlueprintDrawData> m_loadedBlueprints;
};

} // fw