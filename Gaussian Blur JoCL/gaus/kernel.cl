__kernel void gausBlur( __global int *inPixels, __global float *matrix, __global int *outPixels,const int imageWidth, const int imageHeight, const int cols)
{
    //int ioffset = y*imageWidth;
    //int x = get_global_id(0);
    int y = get_global_id(0);
    int cols2 = cols/2;
    //printf("%d", cols2);
    int index = y;
    int ioffset = y*imageWidth;
    for (int x = 0; x < imageWidth; x++) {
        float r = 0, g = 0, b = 0, a = 0;
        for (int col = -cols2; col <= cols2; col++) {
            float f = matrix[cols2+col];

            if (f != 0) {
                int ix = x+col;
                if ( ix < 0 ) {
                    ix = 0;
                }
                if( ix >= imageWidth) {
                    ix = imageWidth-1;
                }
                int rgb = inPixels[ioffset+ix];
                a += f * ((rgb >> 24) & 0xff);
                r += f * ((rgb >> 16) & 0xff);
                g += f * ((rgb >> 8) & 0xff);
                b += f * (rgb & 0xff);
                //printf("%f \n", r);

             }
         }

        int ia = clampp((int)(a+0.5));
        int ir = clampp((int)(r+0.5));
        int ig = clampp((int)(g+0.5));
        int ib = clampp((int)(b+0.5));
        outPixels[index] = (ia << 24) | (ir << 16) | (ig << 8) | ib;
        index += imageHeight;

    }
}
int clampp(int c) {
		if (c < 0)
			return 0;
		if (c > 255)
			return 255;
		return c;
	}

    //printf("%d \n", get_global_size(1));

    //printf("inPixels[%d] %d \n",  gid, inPixels[gid]);
    //int gidX = get_global_id(0);
    //int gidY = get_global_id(1);
    //int2 pos = {gidX, gidY};
    //const int2 pos = {get_global_id(0), get_global_id(1)};
    //int4 color = read_imagei( image, samplerIn, pos);
    //printf("%s\n",get_image_channel_data_type(image));
    //printf("color4 = %v4i \n", color);
    //res_g[pos.x*pos.y] = colorcomputedFilter
    //write_imagei(targetImage, pos, color);

    //printf("computedFilter4 = %2.2v4f\n", computedFilter);
    //int4 icvt = convert_int4(computedFilter);

    //write_imagei(targetImage, (int2)(x, y), computedFilter);
