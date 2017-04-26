#include "MargolusNeighborhoodSimulator.h"
#include <omp.h>

MargolusNeighborhoodSimulator::MargolusNeighborhoodSimulator(const char ruleLUT[16], const bool changesAvailableLUT[16], const sf::Color& particleColor, const sf::Color& obstacleColor, const sf::Color& idleColor)
	: m_rulesLUT{ruleLUT}, m_changesAvailableLUT(changesAvailableLUT), m_particleColor(particleColor), m_obstacleColor(obstacleColor), m_idleColor(idleColor)
{
}

MargolusNeighborhoodSimulator::~MargolusNeighborhoodSimulator()
{
	if (m_ocl != nullptr)
		delete m_ocl;
}

void MargolusNeighborhoodSimulator::ApplyMargolusRules(sf::Image& inOutImage)
{
	++m_pixelOffset %= 2;
	sf::Uint8* pixelptr = const_cast<sf::Uint8*>(inOutImage.getPixelsPtr());
	m_concreteApplyRulesFunction(pixelptr, inOutImage.getSize());
}

void MargolusNeighborhoodSimulator::ActivateOpenMP()
{
	omp_set_num_threads(omp_get_max_threads()); 
	m_concreteApplyRulesFunction = [&](sf::Uint8* pixelptr, sf::Vector2u imgSize) {applyRulesOpenMP(pixelptr, imgSize); };
}

void MargolusNeighborhoodSimulator::ActivateOpenCL(const sf::Vector2u& windowSize, const sf::Color& particleColor, const sf::Color& obstacleColor, const sf::Color& idleColor)
{
	int platformId = 0;
	int deviceId = 0;
	bool useGPU = true;

	m_ocl = new MargolusNeighborhoodSimulatorOpenCL(useGPU, platformId, deviceId);
	m_ocl->Initialize(windowSize, m_rulesLUT, m_changesAvailableLUT, particleColor, obstacleColor, idleColor);
	m_concreteApplyRulesFunction = [&](sf::Uint8* pixelptr, sf::Vector2u imgSize) {applyRulesOpenCL(pixelptr, imgSize); };
}

bool MargolusNeighborhoodSimulator::isBitSet(const char& bits, const char& desiredBit)
{
	return bits & (1 << desiredBit);
}

bool MargolusNeighborhoodSimulator::hasPixelDesiredColor(const sf::Uint8* const inputPixel, const sf::Color& desiredColor)
{
	return *(reinterpret_cast<const sf::Color* const>(inputPixel)) == desiredColor;
}

void MargolusNeighborhoodSimulator::applyColorToPixel(sf::Uint8* inOutPixel, const sf::Color& color)
{
	*(reinterpret_cast<sf::Color*>(inOutPixel)) = color;
}

void MargolusNeighborhoodSimulator::applyRulesOpenMP(sf::Uint8* pixelptr, const sf::Vector2u& imgSize)
{
	int row1 = -1, row2 = -1, pixelPositions[4];
	char particleBits = 0, ruleBits = 0, obstacleBits = 0;

	#pragma omp parallel for private(row1, row2, pixelPositions, particleBits, ruleBits, obstacleBits)
	for (int x = m_pixelOffset; x < imgSize.x - m_pixelOffset; x += 2)
	{
		for (int y = m_pixelOffset; y < imgSize.y - m_pixelOffset; y += 2)
		{
			row1 = 4 * (y * imgSize.x + x);
			row2 = row1 + (4 * imgSize.x);
			pixelPositions[0] = row1;
			pixelPositions[1] = row1 + 4;
			pixelPositions[2] = row2;
			pixelPositions[3] = row2 + 4;

			/******************* CHECK SAND CONSTELLATION AND WRITE BITS *******************/
			/*
			* bits represent:
			* 0 = upper left
			* 1 = upper right
			* 2 = lower left
			* 3 = lower right
			*/
			particleBits = 0;
			for (char bitIndex = 0; bitIndex < 4; ++bitIndex)
				particleBits |= hasPixelDesiredColor(&pixelptr[pixelPositions[bitIndex]], m_particleColor) ? 1 << bitIndex : 0;


			/******************* GET MARGULOS NEIGHBORHOOD *******************/
			if (!m_changesAvailableLUT[particleBits])
				continue;

			ruleBits = m_rulesLUT[particleBits];
			if (particleBits == 3)
			{
				//determine random if it remains 3 or will be 12
				ruleBits = 12;
			}

			/******************* CHECK FOR OBSTACLES AND WRITE BITS *******************/
			obstacleBits = 0;
			for (char bitIndex = 0; bitIndex < 4; ++bitIndex)
			{
				if (!isBitSet(particleBits, bitIndex))
					obstacleBits |= hasPixelDesiredColor(&pixelptr[pixelPositions[bitIndex]], m_obstacleColor) ? 1 << bitIndex : 0;
			}

			/******************* CHECK IF NEW CONSTELLATION IS DOABLE *******************/
			// ~wallBits -> 0 = wall, 1 = usable
			if ((~obstacleBits & ruleBits) != ruleBits)
				continue;

			/******************* APPLY NEW COLORS/ MARGOLUS RULES *******************/
			for (char bitIndex = 0; bitIndex < 4; ++bitIndex)
			{
				//if current bit isn't wall then go on
				if (!isBitSet(obstacleBits, bitIndex))
					applyColorToPixel(&pixelptr[pixelPositions[bitIndex]], isBitSet(ruleBits, bitIndex) ? m_particleColor : m_idleColor);
			}
		}
	}
}

void MargolusNeighborhoodSimulator::applyRulesOpenCL(sf::Uint8* pixelptr, const sf::Vector2u& imgSize)
{
	m_ocl->ApplyMargolusRules(pixelptr, imgSize, m_pixelOffset);
}
