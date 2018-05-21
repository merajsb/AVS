package avs.gaus;

import java.io.File;
import java.io.IOException;
import java.awt.image.BufferedImage;
import javax.imageio.ImageIO;

public class Main {
    public static void main(String[] args){
        GaussianCL gaus = new GaussianCL();
        int width = 10;    //width of the image
        int height = 9;   //height of the image
        BufferedImage imageSrc = null;
        BufferedImage imageDest = null;
        File f = null;
        long startTime = 0;
        //read image
        try{


            f = new File("F:\\GausImage\\smal.jpg"); //image file path
            imageSrc = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
            imageSrc = ImageIO.read(f);
            //imageDest= gausJ.filter(imageSrc, imageDest);
            //startTime = System.nanoTime();
            imageDest =  gaus.filter(imageSrc, imageDest);
        }catch (Exception e){
            System.out.println(e);
        }

        try{
            f = new File("F:\\GausImage\\Output1.jpg");  //output file path
            ImageIO.write(imageDest, "jpg", f);
            System.out.println("Writing complete.");
            long endTime   = System.nanoTime();
            long totalTime = (endTime - startTime) / 1000000000;
            //System.out.println(totalTime);
        }catch(IOException e){
            System.out.println("Error: "+e);
        }


    }

}