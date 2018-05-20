/*
 * This is OpenCL implementation of DFT without complex numbers, the original C++ implementation code is from:
 * https://www.nayuki.io/res/how-to-implement-the-discrete-fourier-transform/Dft.cpp
 */
#include <cmath>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <random>
#include <fstream>
#include <iomanip>
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
using namespace std::chrono;
using namespace std;

//number of elements
const int num = 7000;
//double vectors for two implementations
vector<double> vecOutReal(num);
vector<double> vecOutImag(num);
vector<double> vecOutRealNormal(num);
vector<double> vecOutImagNormal(num);

std::default_random_engine randGen((std::random_device())());
/*
 * Generates a random n numbers
 * @param: number of doubles to be generated
 * @return: vector of double numbers
 */
static vector<double> randomDouble(int n) {
    std::uniform_real_distribution<double> valueDist(1.0, 200.0);
    vector<double> result;
    for (int i = 0; i < n; i++)
        result.push_back(double(valueDist(randGen)));
    return result;
}
//considering same inputs for both implementations
vector<double> vecInreal = randomDouble(num);
vector<double> vecInimaginary = randomDouble(num);
/*
 * Computes the parallelized DFT using OpenCL on CPU or GPU
 * @param: outreal: Vector of double to store the real part of complex number
 * @param: outimage: Vector of ddouble to store the imaginary part of complex number
 * @param: runOnGPU: if set true, the program runs on the second platform and selects first GPU device,
 * if set false the program runs on the first platform and first CPU device.
 *
 */
void computeDftCL(vector<double> &outreal, vector<double> &outimag, bool runOnGPU = true) {

    try {
        // Get list of OpenCL platforms.
        vector<cl::Platform> platform;
        cl::Platform::get(&platform);

        if (platform.empty()) {
            cerr << "OpenCL platforms not found." << endl;
        }
        cl::Context context;
        vector<cl::Device> device;
        //Choosing if running on CPU or GPU and if GPU is selected, check if double precision is supported on it.
        if(!runOnGPU){
            platform[0].getDevices(CL_DEVICE_TYPE_CPU, &device);
        }else{
            platform[1].getDevices(CL_DEVICE_TYPE_GPU, &device);
            //Double precision check, source: https://gist.github.com/ddemidov/2925717
            for(auto p = platform.begin(); device.empty() && p != platform.end(); p++) {
                std::vector<cl::Device> pldev;

                try {
                    p->getDevices(CL_DEVICE_TYPE_GPU, &pldev);

                    for(auto d = pldev.begin(); device.empty() && d != pldev.end(); d++) {
                        if (!d->getInfo<CL_DEVICE_AVAILABLE>()) continue;

                        string ext = d->getInfo<CL_DEVICE_EXTENSIONS>();

                        if (
                                ext.find("cl_khr_fp64") == std::string::npos &&
                                ext.find("cl_amd_fp64") == std::string::npos
                                ) continue;

                        device.push_back(*d);
                        context = cl::Context(device);
                    }
                } catch(...) {
                    device.clear();
                }
            }

            if (device.empty()) {
                cerr << "GPUs with double precision not found." << endl;
            }

        }
        context = cl::Context(device[0]);
        // Creating command queue with event profiling enabled
        cl::CommandQueue queue(context, device[0], CL_QUEUE_PROFILING_ENABLE);
        //Getting the kernel as string, if it did not work, please inpu the full path of kernel
        ifstream ifs("dftKernel.cl");
        string content( (std::istreambuf_iterator<char>(ifs)),
                             (std::istreambuf_iterator<char>() ) );
        cl::Program program(context, cl::Program::Sources(
                1, std::make_pair(content.c_str(), content.length() +1)
        ));
        // compiling OpenCL program
        try {
            program.build(device);
        } catch (const cl::Error&) {
            cerr
                    << "OpenCL compilation error" << endl
                    << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device[0])
                    << endl;
        }
        cl::Kernel kern(program, "dftkernel");
        // Creating buffers
        cl::Buffer inRealBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                     vecInreal.size() * sizeof(double), vecInreal.data());

        cl::Buffer inImageBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                     vecInimaginary.size() * sizeof(double), vecInimaginary.data());

        cl::Buffer outRealBuffer(context, CL_MEM_READ_WRITE,
                     vecOutReal.size() * sizeof(double));

        cl::Buffer outImagBuffer(context, CL_MEM_READ_WRITE,
                     vecOutImag.size() * sizeof(double));

        // Setting kernel parameters

        kern.setArg(0, inRealBuffer);
        kern.setArg(1, inImageBuffer);
        kern.setArg(2, outRealBuffer);
        kern.setArg(3, outImagBuffer);
        kern.setArg(4, num);

        //Creating event for profiling
        cl::Event event;
        cl::Event eventBuf1;
        cl::Event eventBuf2;
        //Creating NDRangekernel, which its globalsize equals to size of output vector size
        queue.enqueueNDRangeKernel(kern, cl::NullRange, cl::NDRange(num), cl::NullRange, NULL, &event);
        event.wait();
        queue.enqueueReadBuffer(outRealBuffer, CL_TRUE, 0, outreal.size() * sizeof(double), outreal.data(), NULL, &eventBuf1);
        eventBuf1.wait();
        queue.enqueueReadBuffer(outImagBuffer, CL_TRUE, 0, outimag.size() * sizeof(double), outimag.data(), NULL, &eventBuf2);
        eventBuf2.wait();
        queue.finish();
        cl_ulong elapsedNdrange = (event.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
                            event.getProfilingInfo<CL_PROFILING_COMMAND_START>());

        cl_ulong elapsedBuf1 = (eventBuf1.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
                             eventBuf1.getProfilingInfo<CL_PROFILING_COMMAND_START>());

        cl_ulong elapsedBuf2 = (eventBuf2.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
                            eventBuf2.getProfilingInfo<CL_PROFILING_COMMAND_START>());

        //Calculating the overall elapsed time considering also the transfer time
        cl_ulong elapsed = elapsedNdrange + elapsedBuf1 + elapsedBuf2;
        cout << "Parallelized elapsed time in ms: " << elapsed / 1000000.0 << endl;

    } catch (const cl::Error &err) {
        cerr
                << "OpenCL error: "
                << err.what() << "(" << err.err() << ")"
                << endl;
    }

}
/*
 * Non parallelized implmentation of DFT
 * @param: outreal: Vector of doubles, to store the real part of complex number
 * @param: outimage: Vector of doubles, to store the imaginary part of complex number
 */
void computeDft(vector<double> &outreal, vector<double> &outimag) {

    size_t n = vecInreal.size();

    for (size_t k = 0; k < n; k++) {  // For each output element
        double sumreal = 0;
        double sumimag = 0;
        for (size_t t = 0; t < n; t++) {  // For each input element
            double angle = 2 * M_PI * t * k / n;
            sumreal +=  vecInreal[t] * cos(angle) + vecInimaginary[t] * sin(angle);
            sumimag += -vecInreal[t] * sin(angle) + vecInimaginary[t] * cos(angle);
        }
        outreal[k] = sumreal;
        outimag[k] = sumimag;
    }
}
int main(){
    //OpenCL implmentation
    computeDftCL(vecOutReal, vecOutImag, false);
    //Getting the duration of the non parallelized DFT
    high_resolution_clock::time_point t3 = high_resolution_clock::now();
    computeDft(vecOutRealNormal, vecOutImagNormal);
    high_resolution_clock::time_point t4 = high_resolution_clock::now();
    auto durationNormal = duration_cast<milliseconds>( t4 - t3 ).count();
    cout << "Non-Parallelized elapsed time in ms: " << durationNormal << endl;
    cout << endl;
    cout << "Testing 1000th element of both output array: " << endl;
    cout << "OpenCL Implementation: Vector real part: " << setprecision(8) << vecOutReal[1000] << " && imaginary part: " << vecOutImag[1000] << endl;
    cout << "Normal Implementation: Vector real part: " << setprecision(8) << vecOutRealNormal[1000] << " && imaginary part: " << vecOutImagNormal[1000] << endl;
}