#include <iostream>
#include "openCL/OpenCLBuilder.h"
#include <vector>

int main(int argc, char** argv)
{
	std::cerr << "start" << std::endl;

    const unsigned int count = 1024;
    float data[count];              // original data set given to device
    for (unsigned int i = 0; i < count; i++)
        data[i] = rand() / (float)RAND_MAX;
    

    float results[count];           // results returned from device


	OpenCLBuilder::printDevices();

	OpenCLBuilder cpu(DEVICE_TYPE::CPU, true);

	cl_kernel square_kernel = cpu.buildKernel("square.cl", "square");
        
    cl_command_queue queue = cpu.createCommandQueue();
    
    // Create the input and output arrays in device memory for our calculation
    cl_mem input = cpu.createBuffer<float>(count, CL_MEM_READ_ONLY);
    cl_mem output = cpu.createBuffer<float>(count, CL_MEM_WRITE_ONLY);

    // Write our data set into the input array in device memory 
    //
    cpu.writeData<float>(queue, input, count, data);
    
    cpu.execute_kernel(queue, square_kernel, { count }, { count },
        input, output, LocalMemory<float>(count), count);
    
       
    cpu.readData<float>(queue, output, count, results);


    cpu.get_profile_log();

    // Validate our results
    //
    int correct = 0;
    for (unsigned int i = 0; i < count; i++)
    {
        if (results[i] == data[i] * data[i])
            correct++;
    }

    // Print a brief summary detailing the results
    //
    printf("Computed '%d/%d' correct values!\n", correct, count);



}