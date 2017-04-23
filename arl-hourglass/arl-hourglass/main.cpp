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
	case commandLineHandles::Error: getchar(); return 0;
	case commandLineHandles::CPU_Usage: break;
	case commandLineHandles::GPU_Usage: break;
	}


	sf::Vector2u hourGlassDimensions = {300, 1000};
	sf::Vector2u windowDimensions = {1000, 1000};

	sf::ContextSettings settings;
	settings.antialiasingLevel = 0;
	sf::RenderWindow window(sf::VideoMode(windowDimensions.x, windowDimensions.y), "'Hourglass Simulation' by Bernhard Rieder", sf::Style::Default, settings);


	/********************************************** CREATE HOURGLASS ***********************************************/
	sf::RenderTexture hourGlassRenderTexture;
	if (!hourGlassRenderTexture.create(hourGlassDimensions.x, hourGlassDimensions.y))
	{
		std::cerr << "cant create render texture";
		//error
	}

	// create an array of 3 vertices that define a triangle primitive
	sf::VertexArray hourGlassWalls(sf::Triangles, 6);

	signed hourGlassFlowWidth = 8;
	// define the position of the triangle's points
	//left wall
	hourGlassWalls[0].position = {0,0};
	hourGlassWalls[1].position = {(hourGlassDimensions.x / 2.f) - hourGlassFlowWidth / 2, hourGlassDimensions.y / 2.f};
	hourGlassWalls[2].position = {0, static_cast<float>(hourGlassDimensions.y)};
	//right wall
	hourGlassWalls[3].position = {static_cast<float>(hourGlassDimensions.x), 0};
	hourGlassWalls[4].position = {(hourGlassDimensions.x / 2.f) + hourGlassFlowWidth / 2, hourGlassDimensions.y / 2.f};
	hourGlassWalls[5].position = {static_cast<float>(hourGlassDimensions.x), static_cast<float>(hourGlassDimensions.y)};

	// define the color of the triangle's points
	hourGlassWalls[0].color = sf::Color::Black;
	hourGlassWalls[1].color = sf::Color::Black;
	hourGlassWalls[2].color = sf::Color::Black;
	hourGlassWalls[3].color = sf::Color::Black;
	hourGlassWalls[4].color = sf::Color::Black;
	hourGlassWalls[5].color = sf::Color::Black;

	//create sand
	sf::Color sandColor = sf::Color(230, 197, 92, 255);
	float emptyPercentage = 0.01f;
	sf::VertexArray hourGlassSand(sf::Quads);
	hourGlassSand.append(sf::Vertex(sf::Vector2f(static_cast<float>(hourGlassDimensions.x), hourGlassDimensions.y * emptyPercentage), sandColor)); // upper right
	hourGlassSand.append(sf::Vertex(sf::Vector2f(0, hourGlassDimensions.y * emptyPercentage), sandColor)); // upper left
	hourGlassSand.append(sf::Vertex(sf::Vector2f(0, hourGlassDimensions.y / 2.f), sandColor)); // lower left
	hourGlassSand.append(sf::Vertex(sf::Vector2f(static_cast<float>(hourGlassDimensions.x), hourGlassDimensions.y / 2.f), sandColor)); // lower right

	//render hourglass into rendertexture!
	hourGlassRenderTexture.clear(sf::Color::White);
	hourGlassRenderTexture.draw(hourGlassSand);
	hourGlassRenderTexture.draw(hourGlassWalls);
	hourGlassRenderTexture.display();

	// get the target texture (where the stuff has been drawn)
	sf::Texture& texture = const_cast<sf::Texture&>(hourGlassRenderTexture.getTexture());
	//sf::Color col = sf::Color::Green;
	//sf::Uint8 pixel[4] = { col.r,col.g,col.b,col.a };
	//texture.update(pixel, 1, 1, 500, 500);

	//pixel manipulation
	sf::Image img = hourGlassRenderTexture.getTexture().copyToImage();
	//img.setPixel(149, 499, sf::Color::Red);
	auto imgSize = img.getSize();
	sf::Uint8* pixelptr = const_cast<sf::Uint8*>(img.getPixelsPtr());
	for (int x = 0; x < imgSize.x; x += 2)
	{
		for (int y = 0; y < imgSize.y; y += 2)
		{
			size_t index = 4 * (y * imgSize.x + x);
			//pixelptr[index + 0] = 255; //r
			//pixelptr[index + 1] = 0; //g
			//pixelptr[index + 2] = 0; //b
			//pixelptr[index + 3] = 255; //a
		}
	}

	//use manipulated image and create renderable hourglass sprite
	sf::Texture modifiedTex;
	if (!modifiedTex.loadFromImage(img))
	{
		getchar();
		return 0;
	}
	sf::Sprite hourglassSprite(modifiedTex);
	hourglassSprite.setOrigin(hourGlassDimensions.x / 2.f, hourGlassDimensions.y / 2.f);
	hourglassSprite.setPosition(windowDimensions.x / 2.f, windowDimensions.y / 2.f);

	//test sand placed by mouse button
	sf::VertexArray placedSand(sf::Points);

	sf::Clock clock;

	unsigned sandTeleportBrushRadius = 10;
	sf::CircleShape sandTeleportBrush = sf::CircleShape(sandTeleportBrushRadius);
	sf::CircleShape sandTeleportBrushMarker = sf::CircleShape(sandTeleportBrushRadius);
	sandTeleportBrush.setFillColor(sf::Color::Yellow);
	sandTeleportBrushMarker.setFillColor(sf::Color(255,0,0,50));

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
				if (event.key.code == sf::Keyboard::Add || event.key.code == sf::Keyboard::Equal) //US/UK layout -> Equal == +
				{
					++sandTeleportBrushRadius;
					sandTeleportBrush.setRadius(sandTeleportBrushRadius);
					sandTeleportBrushMarker.setRadius(sandTeleportBrushRadius);
				}
				else if (event.key.code == sf::Keyboard::Subtract || event.key.code == sf::Keyboard::Dash) //US/UK layout -> Dash == -
				{
					if(sandTeleportBrushRadius > 0)
						--sandTeleportBrushRadius;
					sandTeleportBrush.setRadius(sandTeleportBrushRadius);
					sandTeleportBrushMarker.setRadius(sandTeleportBrushRadius);
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

		window.clear(sf::Color(100, 100, 100, 100));
		window.draw(hourglassSprite);
		window.draw(placedSand);
		window.draw(sandTeleportBrushMarker);
		window.display();
	}

	return 0;
}
