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

/*
 * Turn the contents with your magenta shape into an Image and apply mask
 * img = renderTex.getTexture().copyToImage();
 * img.createMaskFromColor(sf::Color::Magenta);
 */

bool applyMargolusRules(const unsigned& pixelOffset, sf::Image& img, const sf::Color& sandColor, const sf::Color& obstacleColor, const sf::Color& freeColor);

int main(int argc, char* argv[])
{
	switch (commandLineHandles::requestCmdLine(argc, argv))
	{
	case commandLineHandles::Error: getchar();
		return 0;
	case commandLineHandles::CPU_Usage: break;
	case commandLineHandles::GPU_Usage: break;
	}


	sf::Vector2u hourGlassDimensions = {300, 1000};
	sf::Vector2u windowDimensions = {1000, 1000};

	//sf::ContextSettings settings;
	//settings.antialiasingLevel = 4;
	sf::RenderWindow window(sf::VideoMode(windowDimensions.x, windowDimensions.y), "'Hourglass Simulation' by Bernhard Rieder"/*, sf::Style::Default, settings*/);


	/********************************************** CREATE HOURGLASS ***********************************************/
	sf::RenderTexture hourGlassRenderTexture;
	if (!hourGlassRenderTexture.create(hourGlassDimensions.x, hourGlassDimensions.y))
	{
		std::cerr << "cant create render texture";
		//error
	}

	// create an array of 3 vertices that define a triangle primitive
	sf::VertexArray hourGlassWalls(sf::Triangles, 6);

	signed hourGlassFlowWidth = 6;
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
	float emptyPercentage = 0.8f;
	sf::VertexArray hourGlassSand(sf::Quads);
	hourGlassSand.append(sf::Vertex(sf::Vector2f(static_cast<float>(hourGlassDimensions.x), hourGlassDimensions.y * (emptyPercentage/2.f)), sandColor)); // upper right
	hourGlassSand.append(sf::Vertex(sf::Vector2f(0, hourGlassDimensions.y * (emptyPercentage/2.f)), sandColor)); // upper left
	hourGlassSand.append(sf::Vertex(sf::Vector2f(0, hourGlassDimensions.y / 2.f), sandColor)); // lower left
	hourGlassSand.append(sf::Vertex(sf::Vector2f(static_cast<float>(hourGlassDimensions.x), hourGlassDimensions.y / 2.f), sandColor)); // lower right

	//render hourglass into rendertexture!
	hourGlassRenderTexture.clear(sf::Color::White);
	hourGlassRenderTexture.draw(hourGlassSand);
	hourGlassRenderTexture.draw(hourGlassWalls);
	hourGlassRenderTexture.display();

	// get the target texture (where the stuff has been drawn)
	//sf::Texture& texture = const_cast<sf::Texture&>(hourGlassRenderTexture.getTexture());
	//sf::Color col = sf::Color::Green;
	//sf::Uint8 pixel[4] = { col.r,col.g,col.b,col.a };
	//texture.update(pixel, 1, 1, 500, 500);

	//pixel manipulation
	sf::Image img = hourGlassRenderTexture.getTexture().copyToImage();
	////img.setPixel(149, 499, sf::Color::Red);
	//auto imgSize = img.getSize();
	//sf::Uint8* pixelptr = const_cast<sf::Uint8*>(img.getPixelsPtr());

	//for (int x = 0; x < imgSize.x; x += 3)
	//{
	//	for (int y = 0; y < imgSize.y / 2; y += 3)
	//	{
	//		size_t index = 4 * (y * imgSize.x + x);
	//		pixelptr[index + 0] = 255; //r
	//		pixelptr[index + 1] = 0; //g
	//		pixelptr[index + 2] = 0; //b
	//		pixelptr[index + 3] = 255; //a
	//	}
	//}

	//use manipulated image and create renderable hourglass sprite
	sf::Texture modifiedTex;
	if (!modifiedTex.loadFromImage(img))
	{
		std::cerr << "fucking load from image error";
		getchar();
		return 0;
	}
	sf::Sprite hourglassSprite(modifiedTex);
	hourglassSprite.setOrigin(hourGlassDimensions.x / 2.f, hourGlassDimensions.y / 2.f);
	hourglassSprite.setPosition(windowDimensions.x / 2.f, windowDimensions.y / 2.f);
	//hourglassSprite.rotate(45);
	
	//test sand placed by mouse button
	sf::VertexArray placedSand(sf::Points);

	sf::Clock clock;

	unsigned sandTeleportBrushRadius = 10;
	sf::CircleShape sandTeleportBrush = sf::CircleShape(sandTeleportBrushRadius);
	sf::CircleShape sandTeleportBrushMarker = sf::CircleShape(sandTeleportBrushRadius);
	sandTeleportBrush.setFillColor(sf::Color::Yellow);
	sandTeleportBrushMarker.setFillColor(sf::Color(255, 0, 0, 50));

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
					//rotate hourglass by 45 degree to the left
				}
				else if (event.key.code == sf::Keyboard::Right)
				{
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
			static unsigned offset = 0;
			//do margolus
			sf::Image newImg = modifiedTex.copyToImage();
			if(applyMargolusRules(offset, newImg, sandColor, sf::Color::Black, sf::Color::White))
			{
				if (!modifiedTex.loadFromImage(newImg))
				{
					std::cerr << "fucking load from image error";
					getchar();
					return 0;
				}

				hourglassSprite.setTexture(modifiedTex);
			}
			offset += 1;
			offset %= 2;
		}

		window.clear(sf::Color(100, 100, 100, 100));
		window.draw(hourglassSprite);
		window.draw(placedSand);
		window.draw(sandTeleportBrushMarker);
		window.display();
	}

	return 0;
}

#include "LookUpTables.h"

bool hasPixelDesiredColor(const sf::Uint8* const inputPixel, const sf::Color& desiredColor);
bool isBitSet(const char& bits, const char& desiredBit);
void applyColor(sf::Uint8* inOutPixel, const sf::Color& color);

bool applyMargolusRules(const unsigned& pixelOffset, sf::Image& img, const sf::Color& sandColor, const sf::Color& obstacleColor, const sf::Color& freeColor)
{
	bool somethingApplied = false;
	sf::Uint8* pixelptr = const_cast<sf::Uint8*>(img.getPixelsPtr());
	auto imgSize = img.getSize();

	for (int x = pixelOffset; x < imgSize.x - pixelOffset; x += 2)
	{
		for (int y = pixelOffset; y < imgSize.y - pixelOffset; y += 2)
		{
			long row1 = 4 * (y * imgSize.x + x);
			long row2 = 4 * ((y + 1) * imgSize.x + x);
			long pixelPositions[4] = {row1 , row1 + 4, row2, row2 + 4};
			/******************* CHECK SAND CONSTELLATION AND WRITE BITS *******************/
			char sandConstellation = 0;
			sandConstellation |= hasPixelDesiredColor(&pixelptr[pixelPositions[0]], sandColor) ? 1 : 0; //upper left
			sandConstellation |= hasPixelDesiredColor(&pixelptr[pixelPositions[1]], sandColor) ? 2 : 0; //upper right
			sandConstellation |= hasPixelDesiredColor(&pixelptr[pixelPositions[2]], sandColor) ? 4 : 0; //lower left
			sandConstellation |= hasPixelDesiredColor(&pixelptr[pixelPositions[3]], sandColor) ? 8 : 0; //lower right

			/******************* GET MARGULOS NEIGHBORHOOD *******************/

			if (sandConstellation == 0 || sandConstellation == 4 || sandConstellation == 8 || sandConstellation == 12 || sandConstellation == 13 || sandConstellation == 14 || sandConstellation == 15)
				continue;
			else
			{
				//std::cout << "margolus!";
			}

			char margolusResult = MargolusNeighborhoodRules[sandConstellation];
			if (sandConstellation == 3)
			{
				//determine random if it remains 3 or will be 12
				margolusResult = 12;
			}

			/******************* CHECK OBSTACLES AND WRITE BITS *******************/
			char wallBits = 0;
			wallBits |= hasPixelDesiredColor(&pixelptr[pixelPositions[0]], obstacleColor) ? 1 : 0;
			wallBits |= hasPixelDesiredColor(&pixelptr[pixelPositions[1]], obstacleColor) ? 2 : 0;
			wallBits |= hasPixelDesiredColor(&pixelptr[pixelPositions[2]], obstacleColor) ? 4 : 0;
			wallBits |= hasPixelDesiredColor(&pixelptr[pixelPositions[3]], obstacleColor) ? 8 : 0;

			/******************* CHECK IF USABLE *******************/
			bool resultUsable = true;
			char mustCheckIfUsable = ~sandConstellation & margolusResult;
			if (isBitSet(mustCheckIfUsable, 0))
				resultUsable &= !isBitSet(wallBits, 0);
			if (isBitSet(mustCheckIfUsable, 1))
				resultUsable &= !isBitSet(wallBits, 1);
			if (isBitSet(mustCheckIfUsable, 2))
				resultUsable &= !isBitSet(wallBits, 2);
			if (isBitSet(mustCheckIfUsable, 3))
				resultUsable &= !isBitSet(wallBits, 3);

			if (!resultUsable)
				continue;


			/******************* APPLY NEW COLORS/ MARGOLUS RULES *******************/
			if (!isBitSet(wallBits, 0))
				applyColor(&pixelptr[pixelPositions[0]], isBitSet(margolusResult, 0) ? sandColor : freeColor);
			if (!isBitSet(wallBits, 1))
				applyColor(&pixelptr[pixelPositions[1]], isBitSet(margolusResult, 1) ? sandColor : freeColor);
			if (!isBitSet(wallBits, 2))
				applyColor(&pixelptr[pixelPositions[2]], isBitSet(margolusResult, 2) ? sandColor : freeColor);
			if (!isBitSet(wallBits, 3))
				applyColor(&pixelptr[pixelPositions[3]], isBitSet(margolusResult, 3) ? sandColor : freeColor);

			somethingApplied |= true;
		}
	}
	return somethingApplied;
}

bool isBitSet(const char& bits, const char& desiredBit)
{
	return bits & (1 << desiredBit);
}

bool hasPixelDesiredColor(const sf::Uint8* const inputPixel, const sf::Color& desiredColor)
{
	return *(reinterpret_cast<const sf::Color* const>(inputPixel)) == desiredColor;
	//return inputPixel[0] == desiredColor.r &&
	//	inputPixel[1] == desiredColor.g &&
	//	inputPixel[2] == desiredColor.b &&
	//	inputPixel[3] == desiredColor.a;
}

void applyColor(sf::Uint8* inOutPixel, const sf::Color& color)
{
	*(reinterpret_cast<sf::Color*>(inOutPixel)) = color;
	//inOutPixel[0] = color.r;
	//inOutPixel[1] = color.g;
	//inOutPixel[2] = color.b;
	//inOutPixel[3] = color.a;
}
