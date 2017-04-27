#pragma once
#include <SFML/Graphics.hpp>

class HourglassSimulation
{
public:
	HourglassSimulation();
	~HourglassSimulation();

	int Execute(int argc, char* argv[]);

private:
	enum AppInputResult
	{
		Error = 0,
		CPU_Usage,
		GPU_Usage
	};

	static AppInputResult parseCmdLine(int argc, char* argv[]);
	static void showUsage(const char* name);

	void colorizePixelAtPosition(sf::Image& inOutImage, const sf::Vector2i& position, const float& radius, const sf::Color& newColor, const sf::Color& restrictedColor, const sf::Vector2u& windowDimensions) const;
};

