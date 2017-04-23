#include <iostream>
#include <SFML/Graphics.hpp>

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

int main(int argc, char* argv[])
{
	switch (commandLineHandles::requestCmdLine(argc, argv))
	{
		case commandLineHandles::Error: system("PAUSE"); return 0;
		case commandLineHandles::CPU_Usage: break;
		case commandLineHandles::GPU_Usage: break;
	}
	

	sf::Vector2u hourGlassDimensions = { 300, 1000 };
	sf::Vector2u windowDimensions = { 1000, 1000 };
	
	sf::ContextSettings settings;
	settings.antialiasingLevel = 0;
	
	sf::RenderWindow window(sf::VideoMode(windowDimensions.x, windowDimensions.y), "SFML works!", sf::Style::Default, settings);

	sf::RectangleShape rect(sf::Vector2f(300, 1000));
	rect.setFillColor(sf::Color::Black);
	rect.setPosition(350, 0);

	// create an array of 3 vertices that define a triangle primitive
	sf::VertexArray triangle(sf::Triangles, 6);

	// define the position of the triangle's points
	triangle[0].position = sf::Vector2f(windowDimensions.x/2.f-hourGlassDimensions.x/2.f, 0);
	triangle[1].position = { windowDimensions.x / 2.f, windowDimensions.y / 2.f + 10};
	triangle[2].position = { triangle[0].position.x+ hourGlassDimensions.x, triangle[0].position.y };

	triangle[3].position = { triangle[0].position.x, triangle[0].position.y + hourGlassDimensions.y };
	triangle[4].position = triangle[1].position;
	triangle[4].position.y -= 20;
	triangle[5].position = { triangle[2].position.x, triangle[2].position.y + hourGlassDimensions.y };


	// define the color of the triangle's points
	triangle[0].color = sf::Color::White;
	triangle[1].color = sf::Color::White;
	triangle[2].color = sf::Color::White;
	triangle[3].color = sf::Color::White;
	triangle[4].color = sf::Color::White;
	triangle[5].color = sf::Color::White;


	// no texture coordinates here, we'll see that later

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
				if (event.key.code == sf::Keyboard::Add || event.key.code == sf::Keyboard::Equal) //US/UK layout -> Equal == +
				{
					std::cout << "increase delete brush radius" << std::endl;
				}
				else if (event.key.code == sf::Keyboard::Subtract || event.key.code == sf::Keyboard::Dash) //US/UK layout -> Dash == -
				{
					std::cout << "decrease delete brush radius" << std::endl;
				}
			}
		}

		window.clear();
		window.draw(rect);
		window.draw(triangle);
		window.display();
	}

	return 0;
}
