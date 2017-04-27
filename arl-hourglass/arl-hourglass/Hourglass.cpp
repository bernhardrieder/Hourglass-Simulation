#include "Hourglass.h"
#include <iostream>

Hourglass::Hourglass(sf::Vector2u dimensions, unsigned sandFlowWidth, float emptyPercentage, sf::Color wallColor, sf::Color sandColor, sf::Color unusedColor)
	: m_dimensions(dimensions), m_sandColor(sandColor), m_wallColor(wallColor), m_unusedColor(unusedColor)
{
	if(!create(sandFlowWidth, emptyPercentage))
	{
		std::cerr << "error during hourglass creation!";
	}
}

Hourglass::~Hourglass()
{
}

sf::Sprite& Hourglass::GetSpriteCenteredTo(sf::Vector2u centerPosition)
{
	m_sprite.setTexture(m_texture, true);
	m_sprite.setOrigin(m_dimensions.x / 2.f, m_dimensions.y / 2.f);
	m_sprite.setPosition(centerPosition.x, centerPosition.y);
	return m_sprite;
}

bool Hourglass::create(unsigned sandFlowWidth, float emptyPercentage)
{
	/********************************************** CREATE HOURGLASS ***********************************************/
	if (!m_renderTexture.create(m_dimensions.x, m_dimensions.y))
		return false;

	// create an array of 3 vertices that define a triangle primitive
	sf::VertexArray hourGlassWalls(sf::Triangles, 6);

	// define the position of the triangle's points
	//left wall
	hourGlassWalls[0].position = { 0,0 };
	hourGlassWalls[1].position = { (m_dimensions.x / 2.f) - sandFlowWidth / 2, m_dimensions.y / 2.f };
	hourGlassWalls[2].position = { 0, static_cast<float>(m_dimensions.y) };
	//right wall
	hourGlassWalls[3].position = { static_cast<float>(m_dimensions.x), 0 };
	hourGlassWalls[4].position = { (m_dimensions.x / 2.f) + sandFlowWidth / 2, m_dimensions.y / 2.f };
	hourGlassWalls[5].position = { static_cast<float>(m_dimensions.x), static_cast<float>(m_dimensions.y) };

	// define the color of the triangle's points
	hourGlassWalls[0].color = m_wallColor;
	hourGlassWalls[1].color = m_wallColor;
	hourGlassWalls[2].color = m_wallColor;
	hourGlassWalls[3].color = m_wallColor;
	hourGlassWalls[4].color = m_wallColor;
	hourGlassWalls[5].color = m_wallColor;

	//create sand
	//sf::Color sandColor = sf::Color(230, 197, 92, 255);
	//float emptyPercentage = 0.8f;
	sf::VertexArray hourGlassSand(sf::Quads);
	hourGlassSand.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_dimensions.x), m_dimensions.y * (emptyPercentage / 2.f)), m_sandColor)); // upper right
	hourGlassSand.append(sf::Vertex(sf::Vector2f(0, m_dimensions.y * (emptyPercentage / 2.f)),	m_sandColor)); // upper left
	hourGlassSand.append(sf::Vertex(sf::Vector2f(0, m_dimensions.y / 2.f), m_sandColor)); // lower left
	hourGlassSand.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_dimensions.x), m_dimensions.y / 2.f), m_sandColor)); // lower right

	//render hourglass into rendertexture!
	m_renderTexture.clear(m_unusedColor);
	m_renderTexture.draw(hourGlassSand);
	m_renderTexture.draw(hourGlassWalls);
	m_renderTexture.display();

	m_image = m_renderTexture.getTexture().copyToImage();
	m_texture.loadFromImage(m_image);

	return true;
}
