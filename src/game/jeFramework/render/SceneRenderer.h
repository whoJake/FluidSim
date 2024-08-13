#pragma once

#include "data/hash_string.h"
#include "../scene/spatial/Scene.h"
#include "../scene/spatial/components/RenderableMesh.h"

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
	void register_material(const gfx::MaterialDefinition& definition);

	void pre_render(glm::vec3 position, glm::vec3 rotation);
	void render();

	void add_scene(Scene* scene);
private:
	void setup_renderpass();
	void load_mesh(RenderableMeshComponent* meshComponent);

	void set_global_buffer(GlobalSetData* data);
private:
	vk::RenderContext& m_context;
	std::vector<Scene*> m_scenes;
	RenderState m_state;

	std::unordered_map<mtl::hash_string, std::unique_ptr<gfx::Shader>> m_shaderStore;
	std::unordered_map<mtl::hash_string, std::unique_ptr<gfx::Material>> m_materialStore;

	struct MeshDrawData
	{
		mtl::hash_string material;
		std::unique_ptr<vk::Buffer> vertexBuffer;
		u32 vertexCount;
	};

	std::unordered_map<mtl::hash_string, MeshDrawData> m_loadedMeshes;
};

} // fw