#include "MargolusNeighborhoodSimulatorOpenCL.h"
#include <iostream>
#include <fstream>

inline int randomNumber(int low, int high)
{
	return low + rand() / (RAND_MAX / (high - low));
}

MargolusNeighborhoodSimulatorOpenCL::MargolusNeighborhoodSimulatorOpenCL(bool useGpu, int platformId, int deviceId)
{
	srand(static_cast<unsigned int>(time(nullptr)));

	getDevice(useGpu, platformId, deviceId);
	createContext(m_device);
	createProgram(m_context, m_sourceCodeFile);
	createCommandQueue(m_context, m_device);
}

MargolusNeighborhoodSimulatorOpenCL::~MargolusNeighborhoodSimulatorOpenCL()
{
}

void MargolusNeighborhoodSimulatorOpenCL::Initialize(const sf::Vector2u& imgSize, const char ruleLUT[16], const bool changesAvailableLUT[16], const sf::Color& particleColor, const sf::Color& obstacleColor, const sf::Color& idleColor)
{
	m_dataSize = imgSize;
	m_sizeOfImage = imgSize.x * imgSize.y * sizeof(sf::Color);
	createKernel(imgSize, ruleLUT, changesAvailableLUT, particleColor, obstacleColor, idleColor);
}


void MargolusNeighborhoodSimulatorOpenCL::ApplyMargolusRules(sf::Uint8* pixelptr, const sf::Vector2u& imgSize, const unsigned& pixelOffset, const bool& refreshImageBuffer)
{
	if (imgSize != m_dataSize)
	{
		std::cerr << "YOU SHOULD NOT CHANGE THE IMAGE SIZE - BREAKS THE CODE!\n OPENCL WILL NOT PERFORM!";
		return;
	}

	if (refreshImageBuffer)
		handle_clerror(m_queue.enqueueWriteBuffer(
			m_bufferData, // which buffer to write to
			CL_TRUE, // block until command is complete
			0, // offset
			m_sizeOfImage, // size of write 
			pixelptr)); // pointer to input

	handle_clerror(m_queue.enqueueWriteBuffer(m_bufferPixelOffset, CL_TRUE, 0, sizeof(unsigned), &pixelOffset));

	for(int i = 0; i < 2; ++i)
		m_randomNumbers[i] = randomNumber(0, 10000);

	handle_clerror(m_queue.enqueueWriteBuffer(m_bufferRandomNumbers, CL_TRUE, 0, sizeof(int)* 2, m_randomNumbers));

	cl::Event event;
	handle_clerror(m_queue.enqueueNDRangeKernel(m_kernelSimpleGeneration, cl::NullRange, m_globalRange, m_localRange, NULL, &event));
	event.wait();

	handle_clerror(m_queue.enqueueReadBuffer(m_bufferData, CL_TRUE, 0, m_sizeOfImage, pixelptr, NULL, &event));
	event.wait();
}

void MargolusNeighborhoodSimulatorOpenCL::createKernel(const sf::Vector2u& imgSize, const char ruleLUT[16], const bool changesAvailableLUT[16], const sf::Color& particleColor, const sf::Color& obstacleColor, const sf::Color& idleColor)
{
	m_globalRange = cl::NDRange(imgSize.x/2);
	m_localRange = cl::NDRange(imgSize.x / m_deviceMaxWorkGroupSize + 1);

	m_bufferData = cl::Buffer(m_context, CL_MEM_READ_WRITE, m_sizeOfImage * sizeof(unsigned char));
	m_bufferDimensionX = cl::Buffer(m_context, CL_MEM_READ_ONLY, sizeof(int));
	m_bufferDimensionY = cl::Buffer(m_context, CL_MEM_READ_ONLY, sizeof(int));
	m_bufferPixelOffset = cl::Buffer(m_context, CL_MEM_READ_ONLY, sizeof(unsigned int));
	m_bufferParticleColor = cl::Buffer(m_context, CL_MEM_READ_ONLY, sizeof(unsigned char) * 4);
	m_bufferObstacleColor = cl::Buffer(m_context, CL_MEM_READ_ONLY, sizeof(unsigned char) * 4);
	m_bufferIdleColor = cl::Buffer(m_context, CL_MEM_READ_ONLY, sizeof(unsigned char) * 4);
	m_bufferRulesLUT = cl::Buffer(m_context, CL_MEM_READ_ONLY, sizeof(char) * 16);
	m_bufferChangesAvailableLUT = cl::Buffer(m_context, CL_MEM_READ_ONLY, sizeof(bool) * 16);
	m_bufferRandomNumbers = cl::Buffer(m_context, CL_MEM_READ_ONLY, sizeof(int) * 2);

	handle_clerror(m_queue.enqueueWriteBuffer(m_bufferDimensionX, CL_TRUE, 0, sizeof(int), &m_dataSize.x));
	handle_clerror(m_queue.enqueueWriteBuffer(m_bufferDimensionY, CL_TRUE, 0, sizeof(int), &m_dataSize.y));
	handle_clerror(m_queue.enqueueWriteBuffer(m_bufferParticleColor, CL_TRUE, 0, sizeof(sf::Color), &particleColor));
	handle_clerror(m_queue.enqueueWriteBuffer(m_bufferObstacleColor, CL_TRUE, 0, sizeof(sf::Color), &obstacleColor));
	handle_clerror(m_queue.enqueueWriteBuffer(m_bufferIdleColor, CL_TRUE, 0, sizeof(sf::Color), &idleColor));
	handle_clerror(m_queue.enqueueWriteBuffer(m_bufferRulesLUT, CL_TRUE, 0, sizeof(char) * 16, ruleLUT));
	handle_clerror(m_queue.enqueueWriteBuffer(m_bufferChangesAvailableLUT, CL_TRUE, 0, sizeof(bool) * 16, changesAvailableLUT));

	m_kernelSimpleGeneration = createKernel("simple_iteration");
	handle_clerror(m_kernelSimpleGeneration.setArg(0, m_bufferData));
	handle_clerror(m_kernelSimpleGeneration.setArg(1, m_bufferPixelOffset));
	handle_clerror(m_kernelSimpleGeneration.setArg(2, m_bufferDimensionX));
	handle_clerror(m_kernelSimpleGeneration.setArg(3, m_bufferDimensionY));
	handle_clerror(m_kernelSimpleGeneration.setArg(4, m_bufferParticleColor));
	handle_clerror(m_kernelSimpleGeneration.setArg(5, m_bufferObstacleColor));
	handle_clerror(m_kernelSimpleGeneration.setArg(6, m_bufferIdleColor));
	handle_clerror(m_kernelSimpleGeneration.setArg(7, m_bufferRulesLUT));
	handle_clerror(m_kernelSimpleGeneration.setArg(8, m_bufferChangesAvailableLUT));
	handle_clerror(m_kernelSimpleGeneration.setArg(9, m_bufferRandomNumbers));
}

std::vector<cl::Platform> MargolusNeighborhoodSimulatorOpenCL::getPlatforms() const
{
	std::vector<cl::Platform> platforms;
	handle_clerror(cl::Platform::get(&platforms));
	return platforms;
}

void MargolusNeighborhoodSimulatorOpenCL::getDevice(bool useGpu, int platformId, int deviceId)
{
	if (platformId == -1 || deviceId == -1)
		m_device = getDevice(getPlatforms(), useGpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU);
	else if (platformId != -1 && deviceId != -1)
		m_device = getDevice(getPlatforms(), platformId, deviceId);
	else
		handle_clerror(CL_INVALID_DEVICE);

	//debugDeviceOutput(m_device);
	m_deviceMaxWorkGroupSize = m_device.getInfo <CL_DEVICE_MAX_WORK_GROUP_SIZE>();
}

void MargolusNeighborhoodSimulatorOpenCL::createContext(const cl::Device& device)
{
	//cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)(m_usedPlatform)(), 0 };
	//m_context = cl::Context(CL_DEVICE_TYPE_GPU, properties);
	m_context = cl::Context(device);
}

void MargolusNeighborhoodSimulatorOpenCL::createProgram(const cl::Context& context, const std::string& sourceCodeFile)
{
	std::ifstream sourceFile(sourceCodeFile);
	if (!sourceFile)
	{
		std::cerr << "MargolusNeighborhoodSimulatorOpenCL Error: kernel source file " << sourceCodeFile << " not found!" << std::endl;
		handle_clerror(CL_INVALID_KERNEL);
	}
	std::string sourceCode(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));
	cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));
	m_program = cl::Program(context, source);
	handle_clerror(m_program.build(context.getInfo<CL_CONTEXT_DEVICES>()));
}

void MargolusNeighborhoodSimulatorOpenCL::createCommandQueue(const cl::Context& context, const cl::Device& device)
{
	int err;
	m_queue = cl::CommandQueue(context, device, 0, &err);
	handle_clerror(err);
}

cl::Device MargolusNeighborhoodSimulatorOpenCL::getDevice(const std::vector<cl::Platform>& platforms, int platformId, int deviceId)
{
	if (platformId >= platforms.size())
		handle_clerror(CL_INVALID_PLATFORM);

	auto platform = platforms[platformId];
	std::vector<cl::Device> devices;
	handle_clerror(platform.getDevices(CL_DEVICE_TYPE_ALL, &devices));

	if (deviceId >= devices.size())
		handle_clerror(CL_INVALID_DEVICE);

	m_usedPlatform = platform;
	return devices[deviceId];
}

cl::Device MargolusNeighborhoodSimulatorOpenCL::getDevice(const std::vector<cl::Platform>& platforms, int cl_device_type)
{
	for (auto platform : platforms)
	{
		int err;
		std::vector<cl::Device> devices;
		err = platform.getDevices(cl_device_type, &devices);

		if (devices.size() == 0) continue;
		handle_clerror(err);

		m_usedPlatform = platform;
		return devices[0];
	}
	handle_clerror(CL_INVALID_DEVICE);
	return{};
}

void MargolusNeighborhoodSimulatorOpenCL::debugDeviceOutput(const cl::Device& device)
{
	int err;
	auto vendor = device.getInfo<CL_DEVICE_VENDOR>(&err);
	handle_clerror(err);
	auto version = device.getInfo<CL_DEVICE_VERSION>(&err);
	handle_clerror(err);
	auto deviceName = device.getInfo<CL_DEVICE_NAME>(&err);
	handle_clerror(err);

	std::cout << "---- DEVICE ----" << std::endl;
	std::cout << "Vendor: " << vendor << std::endl;
	std::cout << "Version: " << version << std::endl;
	std::cout << "Name: " << deviceName << std::endl;
}

cl::Kernel MargolusNeighborhoodSimulatorOpenCL::createKernel(const std::string& functionName) const
{
	int err;
	cl::Kernel kernel = cl::Kernel(m_program, functionName.c_str(), &err);
	handle_clerror(err);
	return kernel;
}

std::string MargolusNeighborhoodSimulatorOpenCL::cl_errorstring(cl_int err)
{
	switch (err) {
	case CL_SUCCESS:                          return std::string("Success");
	case CL_DEVICE_NOT_FOUND:                 return std::string("Device not found");
	case CL_DEVICE_NOT_AVAILABLE:             return std::string("Device not available");
	case CL_COMPILER_NOT_AVAILABLE:           return std::string("Compiler not available");
	case CL_MEM_OBJECT_ALLOCATION_FAILURE:    return std::string("Memory object allocation failure");
	case CL_OUT_OF_RESOURCES:                 return std::string("Out of resources");
	case CL_OUT_OF_HOST_MEMORY:               return std::string("Out of host memory");
	case CL_PROFILING_INFO_NOT_AVAILABLE:     return std::string("Profiling information not available");
	case CL_MEM_COPY_OVERLAP:                 return std::string("Memory copy overlap");
	case CL_IMAGE_FORMAT_MISMATCH:            return std::string("Image format mismatch");
	case CL_IMAGE_FORMAT_NOT_SUPPORTED:       return std::string("Image format not supported");
	case CL_BUILD_PROGRAM_FAILURE:            return std::string("Program build failure");
	case CL_MAP_FAILURE:                      return std::string("Map failure");
	case CL_MISALIGNED_SUB_BUFFER_OFFSET:     return std::string("Misaligned sub buffer offset");
	case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: return std::string("Exec status error for events in wait list");
	case CL_INVALID_VALUE:                    return std::string("Invalid value");
	case CL_INVALID_DEVICE_TYPE:              return std::string("Invalid device type");
	case CL_INVALID_PLATFORM:                 return std::string("Invalid platform");
	case CL_INVALID_DEVICE:                   return std::string("Invalid device");
	case CL_INVALID_CONTEXT:                  return std::string("Invalid context");
	case CL_INVALID_QUEUE_PROPERTIES:         return std::string("Invalid queue properties");
	case CL_INVALID_COMMAND_QUEUE:            return std::string("Invalid command queue");
	case CL_INVALID_HOST_PTR:                 return std::string("Invalid host pointer");
	case CL_INVALID_MEM_OBJECT:               return std::string("Invalid memory object");
	case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:  return std::string("Invalid image format descriptor");
	case CL_INVALID_IMAGE_SIZE:               return std::string("Invalid image size");
	case CL_INVALID_SAMPLER:                  return std::string("Invalid sampler");
	case CL_INVALID_BINARY:                   return std::string("Invalid binary");
	case CL_INVALID_BUILD_OPTIONS:            return std::string("Invalid build options");
	case CL_INVALID_PROGRAM:                  return std::string("Invalid program");
	case CL_INVALID_PROGRAM_EXECUTABLE:       return std::string("Invalid program executable");
	case CL_INVALID_KERNEL_NAME:              return std::string("Invalid kernel name");
	case CL_INVALID_KERNEL_DEFINITION:        return std::string("Invalid kernel definition");
	case CL_INVALID_KERNEL:                   return std::string("Invalid kernel");
	case CL_INVALID_ARG_INDEX:                return std::string("Invalid argument index");
	case CL_INVALID_ARG_VALUE:                return std::string("Invalid argument value");
	case CL_INVALID_ARG_SIZE:                 return std::string("Invalid argument size");
	case CL_INVALID_KERNEL_ARGS:              return std::string("Invalid kernel arguments");
	case CL_INVALID_WORK_DIMENSION:           return std::string("Invalid work dimension");
	case CL_INVALID_WORK_GROUP_SIZE:          return std::string("Invalid work group size");
	case CL_INVALID_WORK_ITEM_SIZE:           return std::string("Invalid work item size");
	case CL_INVALID_GLOBAL_OFFSET:            return std::string("Invalid global offset");
	case CL_INVALID_EVENT_WAIT_LIST:          return std::string("Invalid event wait list");
	case CL_INVALID_EVENT:                    return std::string("Invalid event");
	case CL_INVALID_OPERATION:                return std::string("Invalid operation");
	case CL_INVALID_GL_OBJECT:                return std::string("Invalid OpenGL object");
	case CL_INVALID_BUFFER_SIZE:              return std::string("Invalid buffer size");
	case CL_INVALID_MIP_LEVEL:                return std::string("Invalid mip-map level");
	case CL_INVALID_GLOBAL_WORK_SIZE:         return std::string("Invalid gloal work size");
	case CL_INVALID_PROPERTY:                 return std::string("Invalid property");
	default:                                  return std::string("Unknown error code");
	}
}

void MargolusNeighborhoodSimulatorOpenCL::handle_clerror(cl_int err)
{
	if (err != CL_SUCCESS) {
		std::cerr << "MargolusNeighborhoodSimulatorOpenCL Error: " << cl_errorstring(err) << std::string(".") << std::endl;
		getchar();
		exit(EXIT_FAILURE);
	}
}
