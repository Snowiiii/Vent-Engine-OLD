#include "mesh/Vulkan_Mesh.hpp"

#include <fstream>

vk::ShaderModule VKBase::load_shader_module(const std::string_view &filename)
{
	std::ifstream file(filename.data(), std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		SDL_LogWarn(SDL_LOG_CATEGORY_SYSTEM, "Failed to open Shader file!, %s", filename.data());
	}
	const auto fileSize = file.tellg();
	file.seekg(0);
	std::vector<char> buffer(fileSize);
	file.read(buffer.data(), fileSize);
	file.close();

	// vulkan shader module info
	vk::ShaderModuleCreateInfo createInfo{};
	createInfo.codeSize = fileSize;
	createInfo.pCode = reinterpret_cast<uint32_t *>(buffer.data());

	return context->device.createShaderModule(createInfo);
}

void VKBase::createPipeline(const std::string_view &vertexShaderFilename, const std::string_view &fragmentShaderFilename)
{
	auto bindingDescriptions = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();
	vk::PipelineVertexInputStateCreateInfo vertex_input({}, bindingDescriptions, attributeDescriptions);

	const vk::PipelineInputAssemblyStateCreateInfo input_assembly({}, vk::PrimitiveTopology::eTriangleList);

	vk::PipelineRasterizationStateCreateInfo raster;
	raster.polygonMode = vk::PolygonMode::eFill;
	raster.cullMode = vk::CullModeFlagBits::eNone;
	raster.frontFace = vk::FrontFace::eClockwise;
	raster.lineWidth = 1.0f;

	// vulkan pipeline color blend state
	vk::PipelineColorBlendAttachmentState blend_attachment;
	blend_attachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

	vk::PipelineViewportStateCreateInfo viewport({}, 1, nullptr, 1, nullptr);

	// Disable all depth testing.
	vk::PipelineDepthStencilStateCreateInfo depth_stencil;
	depth_stencil.depthTestEnable = true;
	depth_stencil.depthWriteEnable = true;
	depth_stencil.depthCompareOp = vk::CompareOp::eGreater;

	vk::PipelineMultisampleStateCreateInfo multisample({}, vk::SampleCountFlagBits::e1);

	// vulkan pipeline color blend state
	vk::PipelineColorBlendStateCreateInfo blend({}, {}, {}, blend_attachment);

	// vulkan pipeline dynamic state
	const std::array<vk::DynamicState, 2> dynamics{vk::DynamicState::eViewport, vk::DynamicState::eScissor};

	const vk::PipelineDynamicStateCreateInfo dynamic({}, dynamics);

	const std::array<vk::PipelineShaderStageCreateInfo, 2> shader_stages{
		vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, load_shader_module(vertexShaderFilename), "main"),
		vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, load_shader_module(fragmentShaderFilename), "main")};

	vk::GraphicsPipelineCreateInfo pipe({}, shader_stages);
	pipe.pVertexInputState = &vertex_input;
	pipe.pInputAssemblyState = &input_assembly;
	pipe.pRasterizationState = &raster;
	pipe.pColorBlendState = &blend;
	pipe.pMultisampleState = &multisample;
	pipe.pViewportState = &viewport;
	pipe.pDepthStencilState = &depth_stencil;
	pipe.pDynamicState = &dynamic;

	// We need to specify the pipeline layout and the render pass description up front as well.
	pipe.renderPass = context->render_pass;
	pipe.layout = context->pipeline_layout;

	context->pipeline = context->device.createGraphicsPipeline(nullptr, pipe).value;

	// Pipeline is baked, we can delete the shader modules now.
	context->device.destroyShaderModule(shader_stages[0].module);
	context->device.destroyShaderModule(shader_stages[1].module);

}

void VKBase::destroyPipeline()
{
	if (context->pipeline)
	{
		context->device.destroyPipeline(context->pipeline);
		context->pipeline = nullptr;
	}
}