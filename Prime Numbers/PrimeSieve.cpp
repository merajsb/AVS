/*
 * This is the sieve of aktin implementation in OpenCL. It is used to print the prime numbers given a limit.
 * The original code is from:
 * https://www.geeksforgeeks.org/sieve-of-atkin/
 * Printing is commented out, the OpenCL implementation is not faster than the normal one.
 * @author: Merajuddin Sobhani
 */
#include <iostream>
#include <vector>
#include <string>
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <fstream>
#include <chrono>

using namespace std::chrono;
int limit = 10000;
int sieve()
{
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    // 2 and 3 are known to be prime
    if (limit > 2){
        //std::cout << 2 << " ";
    }
    if (limit > 3){
        //std::cout << 3 << " ";
    }

    // Initialise the sieve array with false values
    bool sieve[limit];
    for (int i = 0; i < limit; i++)
        sieve[i] = false;

    /* Mark siev[n] is true if one
       of the following is true:
    a) n = (4*x*x)+(y*y) has odd number of
       solutions, i.e., there exist
       odd number of distinct pairs (x, y)
       that satisfy the equation and
        n % 12 = 1 or n % 12 = 5.
    b) n = (3*x*x)+(y*y) has odd number of
       solutions and n % 12 = 7
    c) n = (3*x*x)-(y*y) has odd number of
       solutions, x > y and n % 12 = 11 */
    for (int x = 1; x * x < limit; x++) {
        for (int y = 1; y * y < limit; y++) {

            // Main part of Sieve of Atkin
            int n = (4 * x * x) + (y * y);
            if (n <= limit && (n % 12 == 1 || n % 12 == 5))
                sieve[n] ^= true;

            n = (3 * x * x) + (y * y);
            if (n <= limit && n % 12 == 7)
                sieve[n] ^= true;

            n = (3 * x * x) - (y * y);
            if (x > y && n <= limit && n % 12 == 11)
                sieve[n] ^= true;
        }
    }

    // Mark all multiples of squares as non-prime
    for (int r = 5; r * r < limit; r++) {
        if (sieve[r]) {
            for (int i = r * r; i < limit; i += r * r)
                sieve[i] = false;
        }
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>( t2 - t1 ).count();
    std::cout << "Normal implementation: elapsed time in ms is: " << duration << std::endl;

    for (int a = 5; a < limit; a++)
        if (sieve[a]){
            //std::cout << a << " ";
        }
}

void sieveCl(){
    try {
        // Get list of OpenCL platforms.
        std::vector<cl::Platform> platform;
        cl::Platform::get(&platform);

        if (platform.empty()) {
            std::cerr << "OpenCL platforms not found." << std::endl;
        }
        cl::Context context;
        std::vector<cl::Device> device;
        platform[1].getDevices(CL_DEVICE_TYPE_GPU, &device);
        context = cl::Context(device[0]);
        // Create command queue.
        cl::CommandQueue queue(context, device[0], CL_QUEUE_PROFILING_ENABLE);
        std::ifstream ifs("kernelData.cl");
        std::string content( (std::istreambuf_iterator<char>(ifs)),
                             (std::istreambuf_iterator<char>() ) );
        // Compile OpenCL program for found device.
        cl::Program program(context, cl::Program::Sources(
                1, std::make_pair(content.c_str(), content.length() +1)
        ));

        try {
            program.build(device);
        } catch (const cl::Error&) {
            std::cerr
                    << "OpenCL compilation error" << std::endl
                    << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device[0])
                    << std::endl;
        }

        cl::Kernel add(program, "sieveofAktin");
        bool sieve[limit];
        cl::Buffer resultBuffer(context, CL_MEM_WRITE_ONLY,
                     sizeof(sieve) * sizeof(bool));

        // Kernel parameters.
        add.setArg(0, limit);
        add.setArg(1, resultBuffer);
        cl::Event event;
        //int groupSize = ((limit -1) /4 ) * 4;
        queue.enqueueNDRangeKernel(add, cl::NDRange(1,1), cl::NDRange(limit), cl::NullRange,NULL, &event);
        event.wait();
        queue.finish();
        cl_ulong elapsed = (event.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
                event.getProfilingInfo<CL_PROFILING_COMMAND_START>());

        queue.enqueueReadBuffer(resultBuffer, CL_TRUE, 0, sizeof(sieve) * sizeof(bool), sieve);
        std::cout << "OpenCL implementation: elapsed time in ms is: " << elapsed  << std::endl;
        if (limit > 2){
            //std::cout << 2 << " ";
        }
        if (limit > 3){
            //std::cout << 3 << " ";
        }
        for (int a = 5; a < limit; a++)
            if (sieve[a]){
                //std::cout << a << " ";
            }
    } catch (const cl::Error &err) {
        std::cerr
                << "OpenCL error: "
                << err.what() << "(" << err.err() << ")"
                << std::endl;
    }
}
int main(){
    //executing non parallelized Sieve
    sieve();
    //executing parallelized Sieve
    sieveCl();
}