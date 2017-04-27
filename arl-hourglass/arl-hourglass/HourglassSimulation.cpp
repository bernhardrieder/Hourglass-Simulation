#include <iostream>
#include <SFML/Graphics.hpp>
#include "MargolusNeighborhoodSimulator.h"
#include "Hourglass.h"
#include "LookUpTables.h"
#include "HourglassSimulation.h"

HourglassSimulation::HourglassSimulation()
{
}


HourglassSimulation::~HourglassSimulation()
{
}

int HourglassSimulation::Execute(int argc, char* argv[])
{
	/********************************************** INITIALIZE ***********************************************/
	sf::Color sandColor = sf::Color(230, 197, 92, 255);
	sf::Color wallColor = sf::Color::Black;
	sf::Color idleColor = sf::Color::White;

	MargolusNeighborhoodSimulator margolusSimulator(MargolusNeighborhood::Sand::RulesLUT, MargolusNeighborhood::Sand::ChangesAvailableLUT, sandColor, wallColor, idleColor);
	sf::Vector2u windowDimensions = { 1000, 1000 };

	switch (parseCmdLine(argc, argv))
	{
	case Error: getchar();
		return 0;
	case CPU_Usage: margolusSimulator.ActivateOpenMP();
		break;
	case GPU_Usage: margolusSimulator.ActivateOpenCL(windowDimensions, sandColor, wallColor, idleColor);
		break;
	}

	Hourglass hourglass({ 300, 1000 }, 8, 0.10f, wallColor, sandColor, idleColor);
	

	sf::RenderWindow window(sf::VideoMode(windowDimensions.x, windowDimensions.y), "'Hourglass Simulation' by Bernhard Rieder", sf::Style::Titlebar | sf::Style::Close);

	// create sand teleport brush for delete and adding sand
	unsigned sandTeleportBrushRadius = 10;
	sf::CircleShape sandTeleportBrush = sf::CircleShape(sandTeleportBrushRadius);
	sandTeleportBrush.setFillColor(sf::Color(255, 0, 0, 50));

	//create window sized texture with hourglass in it
	sf::Texture windowSizedTextureWithHourglass;
	sf::RenderTexture rtWithHourglassInside;
	{
		if (!rtWithHourglassInside.create(windowDimensions.x, windowDimensions.y))
		{
			std::cerr << "Cannot create rendertexture - File " << __FILE__ << ", Line " << __LINE__;
			getchar();
			return 0;
		}
		rtWithHourglassInside.clear(wallColor);
		rtWithHourglassInside.draw(hourglass.GetSpriteCenteredTo(sf::Vector2u(windowDimensions.x / 2, windowDimensions.y / 2)));
		rtWithHourglassInside.display();
		windowSizedTextureWithHourglass.loadFromImage(rtWithHourglassInside.getTexture().copyToImage());
	}

	/********************************************** RENDER ***********************************************/
	sf::Clock clock;
	while (window.isOpen())
	{
		sandTeleportBrush.setOrigin(sandTeleportBrushRadius, sandTeleportBrushRadius);
		sandTeleportBrush.setPosition(static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)));

		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (window.hasFocus() && event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Escape)
					window.close();
				else if (event.key.code == sf::Keyboard::Add || event.key.code == sf::Keyboard::Equal) //US/UK layout -> Equal == +
				{
					//increase hourglass sand teleporter brush size
					++sandTeleportBrushRadius;
					sandTeleportBrush.setRadius(sandTeleportBrushRadius);
				}
				else if (event.key.code == sf::Keyboard::Subtract || event.key.code == sf::Keyboard::Dash) //US/UK layout -> Dash == -
				{
					//decrease hourglass sand teleporter brush size
					if (sandTeleportBrushRadius > 0)
						--sandTeleportBrushRadius;
					sandTeleportBrush.setRadius(sandTeleportBrushRadius);
				}
				else if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Right)
				{
					sf::Sprite sprite(windowSizedTextureWithHourglass);
					sprite.setOrigin(500, 500);
					sprite.setPosition(500, 500);

					sprite.rotate(event.key.code == sf::Keyboard::Left ? -45 : 45);

					rtWithHourglassInside.clear(wallColor);
					rtWithHourglassInside.draw(sprite);
					rtWithHourglassInside.display();

					windowSizedTextureWithHourglass.loadFromImage(rtWithHourglassInside.getTexture().copyToImage());
				}
			}
		}

		// brush input
		if (window.hasFocus() && (sf::Mouse::isButtonPressed(sf::Mouse::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Right)))
		{
			sf::Image img = windowSizedTextureWithHourglass.copyToImage();
			colorizePixelAtPosition(img, sf::Mouse::getPosition(window), sandTeleportBrush.getRadius(), sf::Mouse::isButtonPressed(sf::Mouse::Left) ? idleColor : sandColor, wallColor, windowDimensions);
			windowSizedTextureWithHourglass.loadFromImage(img);
		}

		sf::Time elapsed = clock.restart();
		//hourglassSprite.rotate(10*elapsed.asSeconds());

		if (true)
		{
			sf::Image img = windowSizedTextureWithHourglass.copyToImage();
			margolusSimulator.ApplyMargolusRules(img);
			windowSizedTextureWithHourglass.loadFromImage(img);
		}

		window.clear(wallColor);
		window.draw(sf::Sprite(windowSizedTextureWithHourglass));
		window.draw(sandTeleportBrush);
		window.display();
	}

	return 0;
}

HourglassSimulation::AppInputResult HourglassSimulation::parseCmdLine(int argc, char* argv[])
{
	if (argc != 2)
	{
		showUsage(argv[0]);
		return Error;
	}
	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if ((arg == "-h") || (arg == "--help"))
		{
			showUsage(argv[0]);
			return Error;
		}
		if (arg == "-cpu")
			return CPU_Usage;
		if (arg == "-gpu")
			return GPU_Usage;
	}
	return Error;
}

void HourglassSimulation::showUsage(const char* name)
{
	std::cerr << "Usage: " << name << " <option(s)> multithreading parameters\n\n"
		<< "Options:\n"
		<< "\t-h,--help\tShow this help message\n"
		<< "\t-cpu\t\tHourglass Simulation with on CPU via OpenMP.\n"
		<< "\t-gpu\t\tHourglass Simulation with on GPU via OpenCL.\n"
		<< std::endl;
}

void HourglassSimulation::colorizePixelAtPosition(sf::Image& inOutImage, const sf::Vector2i& position, const float& radius, const sf::Color& newColor, const sf::Color& restrictedColor, const sf::Vector2u& windowDimensions) const
{
	for (int x = -radius; x <= radius; ++x)
	{
		for (int y = -radius; y <= radius; ++y)
		{
			sf::Vector2i newPos = position + sf::Vector2i(x, y);
			if (newPos.x < 0 || newPos.y < 0 || newPos.x >= windowDimensions.x || newPos.y >= windowDimensions.y)
				continue;

			float distance = std::sqrt(std::pow(newPos.x - position.x, 2) + std::pow(newPos.y - position.y, 2));
			if (distance > radius)
				continue;

			auto pixelColor = inOutImage.getPixel(newPos.x, newPos.y);
			if (pixelColor != restrictedColor)
				inOutImage.setPixel(newPos.x, newPos.y, newColor);
		}
	}
}
