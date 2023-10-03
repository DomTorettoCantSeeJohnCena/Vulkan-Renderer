#include "GameLayer.h"
#include "VulkanRenderer.h"

GameLayer::GameLayer()
	: Layer()
{

}

GameLayer::~GameLayer()
{
	VulkanRenderer::Cleanup();
}

void GameLayer::OnAttach()
{
	VulkanRenderer::VulkanInit();
}

void GameLayer::OnUpdate()
{
	VulkanRenderer::OnUpdate();
}