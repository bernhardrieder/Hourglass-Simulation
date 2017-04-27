#pragma once

#include <CL/cl.hpp>
#include <SFML/Graphics.hpp>

class MargolusNeighborhoodSimulatorOpenCL
{
public:
	MargolusNeighborhoodSimulatorOpenCL() = delete;
	MargolusNeighborhoodSimulatorOpenCL(bool useGpu, int platformId, int deviceId);
	~MargolusNeighborhoodSimulatorOpenCL();

	void Initialize(const sf::Vector2u& imgSize, const char ruleLUT[16], const bool changesAvailableLUT[16], const sf::Color& particleColor, const sf::Color& obstacleColor, const sf::Color& idleColor);
	void ApplyMargolusRules(sf::Uint8* pixelptr, const sf::Vector2u& imgSize, const unsigned& pixelOffset);

private:
	const std::string m_sourceCodeFile = "kernel.cl";
	cl::Program m_program;
	cl::Platform m_usedPlatform;
	cl::Device m_device;
	cl::Context m_context;
	cl::Kernel m_kernelSimpleGeneration;
	cl::CommandQueue m_queue;
	cl::Buffer m_bufferTempData;
	cl::Buffer m_bufferData;
	cl::Buffer m_bufferDimensionX;
	cl::Buffer m_bufferDimensionY;
	cl::Buffer m_bufferPixelOffset;
	cl::Buffer m_bufferRulesLUT;
	cl::Buffer m_bufferChangesAvailableLUT;
	cl::Buffer m_bufferParticleColor;
	cl::Buffer m_bufferObstacleColor;
	cl::Buffer m_bufferIdleColor;
	cl::Buffer m_bufferRandomNumbers;
	int m_deviceMaxWorkGroupSize;
	cl::NDRange m_globalRange;
	cl::NDRange m_localRange;

	sf::Vector2u m_dataSize;
	int m_sizeOfImage;
	int m_randomNumbers[2];

	void createKernel(const sf::Vector2u& imgSize, const char ruleLUT[16], const bool changesAvailableLUT[16], const sf::Color& particleColor, const sf::Color& obstacleColor, const sf::Color& idleColor);
	std::vector<cl::Platform> getPlatforms() const;
	void getDevice(bool useGpu, int platformId, int deviceId);
	void createContext(const cl::Device& device);
	void createProgram(const cl::Context& context, const std::string& sourceCodeFile);
	void createCommandQueue(const cl::Context& context, const cl::Device& device);
	cl::Device getDevice(const std::vector<cl::Platform>& platforms, int platformId, int deviceId);
	cl::Device getDevice(const std::vector<cl::Platform>& platforms, int cl_device_type);
	static void debugDeviceOutput(const cl::Device& device);
	cl::Kernel createKernel(const std::string& functionName) const;

	static std::string cl_errorstring(cl_int err);
	static void handle_clerror(cl_int err);
};

