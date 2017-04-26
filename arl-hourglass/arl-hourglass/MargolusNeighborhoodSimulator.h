#pragma once

#include <SFML/Graphics.hpp>
#include "LookUpTables.h"

class MargolusNeighborhoodSimulator
{
public:
	explicit MargolusNeighborhoodSimulator(const char ruleLUT[16], const bool changesAvailableLUT[16], const sf::Color& particleColor, const sf::Color& obstacleColor, const sf::Color& idleColor);
	MargolusNeighborhoodSimulator(MargolusNeighborhoodSimulator&& other) = delete; // move constructor
	MargolusNeighborhoodSimulator(const MargolusNeighborhoodSimulator& other) = delete; // copy constructor
	MargolusNeighborhoodSimulator& operator=(const MargolusNeighborhoodSimulator& other) = delete; // copy assignment
	MargolusNeighborhoodSimulator& operator=(MargolusNeighborhoodSimulator&& other) = delete; // move assignment
	~MargolusNeighborhoodSimulator();

	void ApplyMargolusRules(sf::Image& inOutImage);
	void ActivateOpenMP();
	void ActivateOpenCL();

private:
	static bool isBitSet(const char& bits, const char& desiredBit);
	static bool hasPixelDesiredColor(const sf::Uint8* const inputPixel, const sf::Color& desiredColor);
	static void applyColorToPixel(sf::Uint8* inOutPixel, const sf::Color& color);

	const char* m_rulesLUT;
	const bool* m_changesAvailableLUT;
	sf::Color m_particleColor;
	sf::Color m_obstacleColor;
	sf::Color m_idleColor;

	unsigned m_pixelOffset = 1;
};

