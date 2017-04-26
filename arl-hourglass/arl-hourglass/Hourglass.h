#pragma once
#include <SFML/Graphics.hpp>

class Hourglass
{
public:
	Hourglass(sf::Vector2u dimensions, unsigned sandFlowWidth, float emptyPercentage, sf::Color wallColor, sf::Color sandColor, sf::Color unusedColor);
	Hourglass(Hourglass&& other) = delete; // move constructor
	Hourglass(const Hourglass& other) = delete; // copy constructor
	Hourglass& operator=(const Hourglass& other) = delete; // copy assignment
	Hourglass& operator=(Hourglass&& other) = delete; // move assignment
	~Hourglass();

	//sf::RenderTexture& GetRenderTexture();
	const sf::Texture& GetTexture();
	sf::Image& GetImage();
	void RefreshTexture();
	sf::Sprite& GetSpriteCenteredTo(sf::Vector2u centerToPosition);

private:
	bool create(unsigned sandFlowWidth, float emptyPercentage);

	sf::RenderTexture m_renderTexture;
	sf::Vector2u m_dimensions;
	sf::Color m_sandColor;
	sf::Color m_wallColor;
	sf::Color m_unusedColor;
	sf::Sprite m_sprite;
	sf::Image m_image;
	sf::Texture m_texture;
};

