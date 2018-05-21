/*
* Incomplete implementation of Gaussian Blur in OpenCL via JoCL binder
 */

import org.jocl.*;

import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.awt.image.Kernel;
import java.io.*;

import static org.jocl.CL.*;

/**
 * A filter which applies Gaussian blur to an image. This is a subclass of ConvolveFilter
 * which simply creates a kernel with a Gaussian distribution for blurring.
 * @author Jerry Huxtable
 */
public class GaussianCL extends ConvolveFilter {

    public static void defaultInitialization()
    {
    }

    static final long serialVersionUID = 5377089073023183684L;

    protected float radius;
    protected Kernel kernel;

    /**
     * Construct a Gaussian filter
     */
    public GaussianCL() {
        this(2);
    }

    /**
     * Construct a Gaussian filter
     * @param radius blur radius in pixels
     */
    public GaussianCL(float radius) {
        setRadius(radius);
    }

    /**
     * Set the radius of the kernel, and hence the amount of blur. The bigger the radius, the longer this filter will take.
     * @param radius the radius of the blur in pixels.
     */
    public void setRadius(float radius) {
        defaultInitialization();
        this.radius = radius;
        kernel = makeKernel(radius);
    }

    /**
     * Get the radius of the kernel.
     * @return the radius
     */
    public float getRadius() {
        return radius;
    }


    public BufferedImage filter( BufferedImage src, BufferedImage dst ) {
        int width = src.getWidth();
        int height = src.getHeight();

        if ( dst == null )
            dst = createCompatibleDestImage( src, null );

        int[] inPixels = new int[width*height];
        int[] outPixels = new int[width*height];
        src.getRGB( 0, 0, width, height, inPixels, 0, width );

         //int[] outPixels = convolveAndTranspose(width, height, inPixels);
        convolveAndTranspose(kernel, outPixels, inPixels, width, height);

        dst.setRGB( 0, 0, width, height, outPixels, 0, width );
        return dst;
    }



    //int r = (int)Math.ceil(getRadius());
    static int r = 2;
    static String programSource = readKernel("kernel.cl");
    //Kernel kernel, int[] inPixels, int[] outPixels,
    /**
     * The height of the image
     */
    public int height;
    BufferedImage inputImage;

    /**
     * The output image
     */
    BufferedImage outputImage;

    public  void convolveAndTranspose(Kernel kernel, int[] inPixels, int[] outPixels, int width, int height ) {
        width  = 10;
        height = 9;
        inputImage = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
        File f = new File("F:\\GausImage\\smal.jpg"); //image file path
        try {
            inputImage = ImageIO.read(f);
        }catch (IOException e){
            e.printStackTrace();
        }

        inputImage.getRGB( 0, 0, width, height, inPixels, 0, width );

        int n = width * height;

        // The platform, device type and device number
        // that will be used
        final int platformIndex = 1;
        final long deviceType = CL_DEVICE_TYPE_GPU;
        final int deviceIndex = 0;

        // Enable exceptions and subsequently omit error checks in this sample
        CL.setExceptionsEnabled(true);

        // Obtain the number of platforms
        int numPlatformsArray[] = new int[1];
        clGetPlatformIDs(0, null, numPlatformsArray);
        int numPlatforms = numPlatformsArray[0];

        // Obtain a platform ID
        cl_platform_id platforms[] = new cl_platform_id[numPlatforms];
        clGetPlatformIDs(platforms.length, platforms, null);
        cl_platform_id platform = platforms[platformIndex];

        // Initialize the context properties
        cl_context_properties contextProperties = new cl_context_properties();
        contextProperties.addProperty(CL_CONTEXT_PLATFORM, platform);

        // Obtain the number of devices for the platform
        int numDevicesArray[] = new int[1];
        clGetDeviceIDs(platform, deviceType, 0, null, numDevicesArray);
        int numDevices = numDevicesArray[0];

        // Obtain a device ID
        cl_device_id devices[] = new cl_device_id[numDevices];
        clGetDeviceIDs(platform, deviceType, numDevices, devices, null);
        cl_device_id device = devices[deviceIndex];

        // Create a context for the selected device
        cl_context context = clCreateContext(
                contextProperties, 1, new cl_device_id[]{device},
                null, null, null);
        cl_image_format format = new cl_image_format();
        format.image_channel_order = CL_ARGB;
        format.image_channel_data_type = CL_SIGNED_INT8;
        cl_image_desc image_desc = new cl_image_desc();
        image_desc.image_type = CL_MEM_OBJECT_IMAGE1D;
        image_desc.image_width = width;
        image_desc.image_height = height;
        image_desc.image_array_size = 1;
        image_desc.image_row_pitch = 0;
        image_desc.image_slice_pitch = 0;
        image_desc.num_mip_levels = 0;
        image_desc.num_samples = 0;
        image_desc.buffer = null;
        cl_mem img = clCreateImage(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,format, image_desc, Pointer.to(inPixels), null);
        // Create a command-queue for the selected device
        cl_command_queue commandQueue =
                clCreateCommandQueue(context, device, 0, null);
        float[] matrixFilter = kernel.getKernelData(null);
        int MatrixFilterSize = Sizeof.cl_float * (r*2+1);
        cl_mem matrixFilterBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,MatrixFilterSize, Pointer.to(matrixFilter), null);
        int outputSize = Sizeof.cl_int * width * height;;
        cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, outputSize, null, null);
        cl_mem inPixelsBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,outputSize, Pointer.to(inPixels), null);



        //cl_mem outPutIntBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, outputIntLen, Pointer.to(outputInt), ciErrNum);
        // Create the program from the source code
        cl_program program = clCreateProgramWithSource(context,
                1, new String[]{ programSource }, null, null);

        // Build the program
        clBuildProgram(program, 0, null, null, null, null);

        // Create the kernel
        cl_kernel kern = clCreateKernel(program, "gausBlur", null);
        //clSetKernelArg(kern, 0, Sizeof.cl_mem, Pointer.to(img));
        clSetKernelArg(kern, 0, Sizeof.cl_mem, Pointer.to(inPixelsBuffer));
        clSetKernelArg(kern, 1, Sizeof.cl_mem, Pointer.to(matrixFilterBuffer));
        clSetKernelArg(kern, 2, Sizeof.cl_mem, Pointer.to(outputBuffer));
        clSetKernelArg(kern, 3, Sizeof.cl_int, Pointer.to(new int[]{width}));
        clSetKernelArg(kern, 4, Sizeof.cl_int, Pointer.to(new int[]{height}));
        clSetKernelArg(kern, 5, Sizeof.cl_int, Pointer.to(new int[]{2}));

        // Set the work-item dimensions
        long[] global_work_size = new long[1];
        global_work_size[0] = height;
        long local_work_size[] = new long[2];
        local_work_size[0] = 1;
        local_work_size[1] = 1;

        // Execute the kernel
        clEnqueueNDRangeKernel(commandQueue, kern, 1, null,
          global_work_size, local_work_size, 0, null, null);
        clEnqueueReadBuffer(commandQueue, outputBuffer, CL_TRUE, 0,
                Sizeof.cl_int * width * height, Pointer.to(outPixels), 0, null, null);

       /*
        clEnqueueReadImage(
                commandQueue, outputImageMem, true, new long[3],
                new long[]{width, height, 1},
                width * Sizeof.cl_int, 0,
                Pointer.to(outPixels), 0, null, null);
        if ( outputImage == null )
            outputImage = createCompatibleDestImage( inputImage, null );
        outputImage.setRGB( 0, 0, width, height, outPixels, 0, width );
        try {
            f = new File("F:\\GausImage\\Output10.jpg");  //output file path
            ImageIO.write(outputImage, "png", f);
        }catch (IOException e){
            e.printStackTrace();
        }
        if(Arrays.equals(outPixels, inPixels)){
            System.out.println("same");
        }
        */
        //Graphics g1 = outputImage.createGraphics();
        //g1.drawImage(outputImage, 0, 0, null);
        //g1.dispose();
        /*outputImage.setRGB(0,0,width, height, inPixels, 0, width);
        outputImage.setRGB( 0, 0, width, height, outPixels, 0, width );
        try {
            File f1 = new File("F:\\GausImage\\Output2.jpg");  //output file path
            ImageIO.write(outputImage, "jpg", f1);

        }catch (IOException e){
            e.printStackTrace();
        }
        */

        //clEnqueueReadBuffer(commandQueue, memObjects[1], CL_TRUE, 0,
          //      Sizeof.cl_int * width * height, dst, 0, null, null);
    }

    public static Kernel makeKernel(float radius) {
        int r = (int)Math.ceil(radius);
        int rows = r*2+1;
        float[] matrix = new float[rows*rows];
        float sigma = radius/3;
        //Pointer.to
        float sigma22 = 2*sigma*sigma;
        float sigmaPi2 = 2*ImageMath.PI*sigma;
        float sqrtSigmaPi2 = (float)Math.sqrt(sigmaPi2);
        float radius2 = radius*radius;
        float total = 0;
        int index = 0;
        for (int row = -r; row < r+1; row++) {
            for(int height = -r; height< r+1; height++){
                float distance = row*row;
                if (distance > radius2)
                    matrix[index] = 0;
                else
                    matrix[row + r + (height+r) * rows] = (float)Math.exp(-(distance)/sigma22) / sqrtSigmaPi2;
                    total += matrix[row + r + (height+r) * rows];
            }
        }
        for (int i = 0; i < rows; i++)
            matrix[i] /= total;

        return new Kernel(rows, rows, matrix);
    }

    public String toString() {
        return "Blur/Gaussian Blur...";
    }
    public static String readKernel(String kernelname)
    {
        try
        {
            InputStream kernelinPackage = GaussianCL.class.getResourceAsStream(kernelname);
            BufferedReader br = new BufferedReader(new InputStreamReader(kernelinPackage));
            StringBuilder sb = new StringBuilder();
            String line = null;
            while (true)
            {
                line = br.readLine();
                if (line == null)
                {
                    break;
                }
                sb.append(line+"\n");
            }
            return sb.toString();
        }
        catch (IOException e)
        {
            e.printStackTrace();
            return "";
        }
    }


}
