#if defined(cl_khr_fp64)
    #   pragma OPENCL EXTENSION cl_khr_fp64: enable
#elif defined(cl_amd_fp64)
    #  pragma OPENCL EXTENSION cl_amd_fp64: enable
#else
    #  error double precision is not supported
#endif
#define  PI 3.14159265358979323846
__kernel void dftkernel(__global const double *inreal, __global const double *inimag, __global double *outreal, __global double *outimag, const int n)
{
        int k = get_global_id(0);
        double sumreal = 0;
        double sumimag = 0;
        for (size_t t = 0; t < n; t++) {
            double angle = 2 * PI * t * k / n;
            sumreal +=  inreal[t] * cos(angle) + inimag[t] * sin(angle);
            sumimag += -inreal[t] * sin(angle) + inimag[t] * cos(angle);
        }
        outreal[k] = sumreal;
        outimag[k] = sumimag;
}