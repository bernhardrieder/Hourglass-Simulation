#include <iostream>
#include <SFML/Graphics.hpp>
#include "MargolusNeighborhoodSimulator.h"
#include "Hourglass.h"

namespace commandLineHandles
{
	void showUsage(const char* name)
	{
		std::cerr << "Usage: " << name << " <option(s)> multithreading parameters\n\n"
			<< "Options:\n"
			<< "\t-h,--help\tShow this help message\n"
			<< "\t-cpu\t\tHourglass Simulation with on CPU via OpenMP.\n"
			<< "\t-gpu\t\tHourglass Simulation with on GPU via OpenCL.\n"
			<< std::endl;
	}

	enum AppInputResult
	{
		Error = 0,
		CPU_Usage,
		GPU_Usage
	};

	AppInputResult requestCmdLine(int argc, char* argv[])
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
}

/*
 * Turn the contents with your magenta shape into an Image and apply mask
 * img = renderTex.getTexture().copyToImage();
 * img.createMaskFromColor(sf::Color::Magenta);
 */

int main(int argc, char* argv[])
{
	sf::Color sandColor = sf::Color(230, 197, 92, 255);
	sf::Color wallColor = sf::Color::Black;
	sf::Color idleColor = sf::Color::White;

	MargolusNeighborhoodSimulator margolusSimulator(MargolusNeighborhood::Sand::RulesLUT, MargolusNeighborhood::Sand::ChangesAvailableLUT, sandColor, wallColor, idleColor);

	switch (commandLineHandles::requestCmdLine(argc, argv))
	{
		case commandLineHandles::Error: getchar(); return 0;
		case commandLineHandles::CPU_Usage: margolusSimulator.ActivateOpenMP(); break;
		case commandLineHandles::GPU_Usage: margolusSimulator.ActivateOpenCL(); break;
	}

	Hourglass hourglass({ 300, 1000 }, 8, 0.90f, wallColor, sandColor, idleColor);
	sf::Vector2u windowDimensions = {1000, 1000};

	//sf::ContextSettings settings;
	//settings.antialiasingLevel = 4;
	sf::RenderWindow window(sf::VideoMode(windowDimensions.x, windowDimensions.y), "'Hourglass Simulation' by Bernhard Rieder"/*, sf::Style::Default, settings*/);
	
	//test sand placed by mouse button
	sf::VertexArray placedSand(sf::Points);

	sf::Clock clock;

	unsigned sandTeleportBrushRadius = 10;
	sf::CircleShape sandTeleportBrush = sf::CircleShape(sandTeleportBrushRadius);
	sf::CircleShape sandTeleportBrushMarker = sf::CircleShape(sandTeleportBrushRadius);
	sandTeleportBrush.setFillColor(sf::Color::Yellow);
	sandTeleportBrushMarker.setFillColor(sf::Color(255, 0, 0, 50));

	sf::Texture windowSizedTextureWithHourglass;
	{
		//create window sized texture with hourglass in it
		sf::RenderTexture rtWithHourglassInside;
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
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Escape)
					window.close();
				else if (event.key.code == sf::Keyboard::Add || event.key.code == sf::Keyboard::Equal) //US/UK layout -> Equal == +
				{
					//increase hourglass sand teleporter brush size
					++sandTeleportBrushRadius;
					sandTeleportBrush.setRadius(sandTeleportBrushRadius);
					sandTeleportBrushMarker.setRadius(sandTeleportBrushRadius);
				}
				else if (event.key.code == sf::Keyboard::Subtract || event.key.code == sf::Keyboard::Dash) //US/UK layout -> Dash == -
				{
					//decrease hourglass sand teleporter brush size
					if (sandTeleportBrushRadius > 0)
						--sandTeleportBrushRadius;
					sandTeleportBrush.setRadius(sandTeleportBrushRadius);
					sandTeleportBrushMarker.setRadius(sandTeleportBrushRadius);
				}
				else if (event.key.code == sf::Keyboard::Left)
				{
					//hourglass.GetSpriteCenteredTo(sf::Vector2u(windowDimensions.x / 2, windowDimensions.y / 2)).rotate(-90);
					//rotate hourglass by 45 degree to the left
				}
				else if (event.key.code == sf::Keyboard::Right)
				{
					//hourglass.GetSpriteCenteredTo(sf::Vector2u(windowDimensions.x / 2, windowDimensions.y / 2)).rotate(+90);
					//rotate hourglass by 45 degree to the right
				}
			}
		}
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			using namespace sf;
			placedSand.append(Vertex(static_cast<Vector2f>(Mouse::getPosition(window)), Color::Yellow));
		}
		sandTeleportBrushMarker.setOrigin(sandTeleportBrushRadius, sandTeleportBrushRadius);
		sandTeleportBrushMarker.setPosition(static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)));

		sf::Time elapsed = clock.restart();
		//hourglassSprite.rotate(10*elapsed.asSeconds());

		if (true)
		{
			sf::Image img = windowSizedTextureWithHourglass.copyToImage();
			margolusSimulator.ApplyMargolusRules(img);
			windowSizedTextureWithHourglass.loadFromImage(img);
		}

		sf::Sprite sprite(windowSizedTextureWithHourglass);
		//sprite.setOrigin(1000, 1000);
		//sprite.setPosition(1000, 1000);

		window.clear(sf::Color(100, 100, 100, 100));
		window.draw(sprite);
		window.draw(placedSand);
		window.draw(sandTeleportBrushMarker);
		window.display();
	}

	return 0;
}