#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <CL/cl.h>
#include <vector>

enum DEVICE_TYPE {
    CPU,
    INTEL_GPU,
    GPU
};

// helper struct to allocate local memory, use it when setting the kernel arguments
template <typename T>
struct LocalMemory {

    LocalMemory(unsigned int _nof_elements) {
        nof_elements = _nof_elements;
    }
    
    size_t getSize() {
        return nof_elements * sizeof(T);
    }

    unsigned int nof_elements;
};

class OpenCLBuilder {

    cl_device_id device_id;             // compute device id 
    cl_context context;                 // compute context

    bool profile = false;
    std::vector<std::pair<std::string,cl_event>> event_list;
public:

    OpenCLBuilder(DEVICE_TYPE type = DEVICE_TYPE::CPU, bool _profile = false) : profile {_profile} {
        int err;                            // error code returned from api calls

        cl_uint platformCount;
        cl_platform_id* platforms;
        clGetPlatformIDs(0, NULL, &platformCount);
        std::cerr << platformCount << std::endl;

        platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * platformCount);
        clGetPlatformIDs(platformCount, platforms, NULL);

        err = clGetDeviceIDs(platforms[2], CL_DEVICE_TYPE_ALL, 1, &device_id, NULL);
        if (err != CL_SUCCESS)
        {
            printf("Error: Failed to create a device group!\n");
       
        }

        // Create a compute context 
        context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
        if (!context)
        {
            printf("Error: Failed to create a compute context!\n");
        }

    }

    cl_command_queue createCommandQueue() {

        int err;                            // error code returned from api calls

        cl_command_queue command_queue;          // compute command queue


        if (!context)
        {
            printf("Error: Failed to create a compute context!\n");
            return nullptr;
        }

        // Create a command commands
        //
        command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &err);
        if (!command_queue)
        {
            printf("Error: Failed to create a command commands!\n");
            return nullptr;
        }
        return command_queue;
    }

    void get_profile_log() {
        cl_kernel kernel;

        //char kernelName[32];
        //cl_uint argCnt;
        //clGetKernelInfo(kernel,
         //   CL_KERNEL_FUNCTION_NAME,
        //    sizeof(kernelName),
        //    kernelName, NULL);

        for (auto& event_entry : event_list) {
            double elapsed = 0;
            cl_ulong time_start, time_end;          
            clWaitForEvents(1, &event_entry.second);
            clGetEventProfilingInfo(event_entry.second, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
            clGetEventProfilingInfo(event_entry.second, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
            elapsed += (time_end - time_start);
            std::cerr << event_entry.first << " " << time_start << " " << elapsed << std::endl;
            
        }

        // TODO
        // https://stackoverflow.com/questions/23550912/measuring-execution-time-of-opencl-kernels
    }

    cl_kernel buildKernel(std::string fileName, std::string kernel_name) {

        cl_program program;
        cl_kernel kernel;                   // compute kernel
        int err;                            // error code returned from api calls

        std::string basePath = "C:\\Users\\kalve\\Source\\Repos\\OpenCLPlayGround\\kernels\\";

        // Create the compute program from the source buffer



        FILE* programHandle;
        size_t programSize, kernelSourceSize;
        char* programBuffer, * kernelSource;

        std::string complete_filename = basePath + fileName;

        std::ifstream ifs(complete_filename);
        std::string programString((std::istreambuf_iterator<char>(ifs)),
            (std::istreambuf_iterator<char>()));

        // create program from buffer
        const size_t size = programString.size();
        const char* source = programString.c_str();
        program = clCreateProgramWithSource(context, 1,
            &source, &size, NULL);
           
        // Build the program executable
        //
        err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  
        size_t len;
        char buffer[2048];

        printf("Error: Failed to build program executable!\n");
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);

        if (err != CL_SUCCESS)
        {
            printf("Could not compile code\n");
            exit(1);
        }

        // Create the compute kernel in the program we wish to run
        //
        kernel = clCreateKernel(program, "square", &err);
        if (!kernel || err != CL_SUCCESS)
        {
            printf("Error: Failed to create compute kernel!\n");
            exit(1);
        }
        return kernel;
    }

	static void printDevices() {
        // Connect to a compute device
        cl_uint platformCount;
        cl_platform_id* platforms;
        clGetPlatformIDs(0, NULL, &platformCount);

        platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * platformCount);
        clGetPlatformIDs(platformCount, platforms, NULL);     

        char* value;
        size_t valueSize;
        cl_uint deviceCount;
        cl_device_id* devices;
        cl_uint maxComputeUnits;
        for (int i = 0; i < platformCount; i++) {

            // get all devices
            clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &deviceCount);
            devices = (cl_device_id*)malloc(sizeof(cl_device_id) * deviceCount);
            clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, deviceCount, devices, NULL);

            // for each device print critical attributes
            for (int j = 0; j < deviceCount; j++) {

                // print device name
                clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, NULL, &valueSize);
                value = (char*)malloc(valueSize);
                clGetDeviceInfo(devices[j], CL_DEVICE_NAME, valueSize, value, NULL);
                printf("%d. Device: %s\n", j + 1, value);
                free(value);

                // print hardware device version
                clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, 0, NULL, &valueSize);
                value = (char*)malloc(valueSize);
                clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, valueSize, value, NULL);
                printf(" %d.%d Hardware version: %s\n", j + 1, 1, value);
                free(value);

                // print software driver version
                clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, 0, NULL, &valueSize);
                value = (char*)malloc(valueSize);
                clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, valueSize, value, NULL);
                printf(" %d.%d Software version: %s\n", j + 1, 2, value);
                free(value);

                // print c version supported by compiler for device
                clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &valueSize);
                value = (char*)malloc(valueSize);
                clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, valueSize, value, NULL);
                printf(" %d.%d OpenCL C version: %s\n", j + 1, 3, value);
                free(value);

                // print parallel compute units
                clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS,
                    sizeof(maxComputeUnits), &maxComputeUnits, NULL);
                printf(" %d.%d Parallel compute units: %d\n", j + 1, 4, maxComputeUnits);

            }

            free(devices);
        }
	}

    template <typename T>
    cl_mem createBuffer(unsigned int nof_elements, unsigned int read_write_flag) {
        cl_mem buffer = clCreateBuffer(context, read_write_flag, sizeof(T)* nof_elements, NULL, NULL);

        if (!buffer)
        {
            printf("Error: Failed to allocate device memory!\n");
            exit(1);
        }
        return buffer;
    }

    template <typename T>
    void writeData(cl_command_queue queue, cl_mem buffer, unsigned int nof_elements, void* data) {
        //
        int err = clEnqueueWriteBuffer(queue, buffer, CL_TRUE, 0, sizeof(T) * nof_elements, data, 0, NULL, NULL);
        if (err != CL_SUCCESS)
        {
            printf("Error: Failed to write to source array!\n");
            exit(1);
        }
    }


    template <typename T>
    void readData(cl_command_queue queue, cl_mem buffer,unsigned int nof_elements, void* data, bool wait_for_kernel_execution = true) {
        if (wait_for_kernel_execution) {
            // Wait for the command commands to get serviced before reading back results
            clFinish(queue);
        }

        // Read back the results from the device to verify the output
        int err = clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, sizeof(T) * nof_elements, data, 0, NULL, NULL);
        if (err != CL_SUCCESS)
        {
            printf("Error: Failed to read output array! %d\n", err);
            exit(1);
        }
    }

    // Get the maximum work group size for executing the kernel on the device
//
    //err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
    //if (err != CL_SUCCESS)
    //{
    //    printf("Error: Failed to retrieve kernel work group info! %d\n", err);
    //    exit(1);
   // }


    template <typename ...ArgsT>
    void execute_kernel(cl_command_queue queue, cl_kernel kernel,
        std::vector<size_t> global, std::vector<size_t> local, ArgsT... kernel_arguments) {
        _set_kernel_argument(kernel, 0, kernel_arguments...);

        cl_event ev;
        
        int err = clEnqueueNDRangeKernel(queue, kernel, global.size(), NULL, global.data(), local.data(), 0, NULL, &ev);
        if (err)
        {
            printf("Error: Failed to execute kernel! %d\ n", err);
            exit(1);
        }
        else {
            char kernelName[32];
            cl_uint argCnt;
            clGetKernelInfo(kernel,
               CL_KERNEL_FUNCTION_NAME,
               sizeof(kernelName),
               kernelName, NULL);
            event_list.emplace_back(kernelName,ev);
        }


    }

    template <typename ...ArgsT>
    void set_kernel_argument(cl_kernel kernel, ArgsT... rest) {
        _set_kernel_argument(kernel, 0, rest...);
    }

    private:


        void _set_kernel_argument(cl_kernel kernel, int index) {
            std::cerr << "done setting arguments" << index << std::endl;
        }

        template <typename T, typename ...ArgsT>
        void _set_kernel_argument(cl_kernel kernel, int index, LocalMemory<T> data, ArgsT... rest) {

            int err = clSetKernelArg(kernel, index, data.getSize(), NULL);
            _set_kernel_argument(kernel, ++index, rest...);
        }

        template <typename T, typename ...ArgsT>
        void _set_kernel_argument(cl_kernel kernel, int index, T data, ArgsT... rest) {
            
            int err = clSetKernelArg(kernel, index, sizeof(T), &data);
            _set_kernel_argument(kernel, ++index, rest...);
        }


};