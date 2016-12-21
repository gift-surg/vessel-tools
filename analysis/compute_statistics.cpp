/**
  * compute_statistics.cpp
  * Computes some basic statistics over an image.
  * The -f flag should be activated so that different regions can be evaluated (by providing different
  * values). At the moment, anything that is not background is considered a label.
  * Optionally writes these to a file
  * @author M.A. Zuluaga
  */
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>


#include <iostream>
#include <fstream>

void Usage(char *exec)
{
    std::cout << " " << std::endl;
    std::cout << "Computes some basic statistics over the regions of an image. Regions are define via a label map." << std::endl;
    std::cout << " " << exec << " [-i image -l label map ]" << std::endl;
    std::cout << "**********************************************************" <<std::endl;
    std::cout << "Options:" <<std::endl;
    std::cout << "-o <file> \t Output file with results (printed on screen default)" << std::endl;
    std::cout << "-b <int> \t Background label" << std::endl;
    std::cout << "-f <int> \t Foreground label from where to do statistics (default is 1 but not used at the moment)" << std::endl;
}

int main( int argc, char * argv[] )
{
    std::string image;
    std::string map;
    std::string outputFileName;
    unsigned int bck = 0;
    unsigned int fore = 1;

    for(int i=1; i < argc; i++)
    {
        if(strcmp(argv[i], "-help")==0 || strcmp(argv[i], "-Help")==0
                || strcmp(argv[i], "-HELP")==0 || strcmp(argv[i], "-h")==0
                || strcmp(argv[i], "--h")==0)
        {
            Usage(argv[0]);
            return EXIT_FAILURE;
        }
        else if(strcmp(argv[i], "-i") == 0)
        {
            image=argv[++i];
            std::cout << "Set --i=" << image << std::endl;
        }
        else if(strcmp(argv[i], "-l") == 0)
        {
            map=argv[++i];
            std::cout << "Set --l=" << map << std::endl;
        }
        else if(strcmp(argv[i], "-o") == 0)
        {
            outputFileName=argv[++i];
            std::cout << "Set --o=" << outputFileName << std::endl;
        }
        else if(strcmp(argv[i], "-b") == 0)
        {
            bck=atoi(argv[++i]);
            std::cout << "Set --b=" << outputFileName << std::endl;
        }
        else if(strcmp(argv[i], "-f") == 0)
        {
            fore=atoi(argv[++i]);
            std::cout << "Set --f=" << outputFileName << std::endl;
        }
        else
        {
            std::cout << "Error in arguments" << std::endl;
            Usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    typedef itk::Image<unsigned char,3> LabelImageType;
    typedef itk::Image<float,3> ImageType;
    typedef itk::ImageFileReader<ImageType> ImageReaderType;
    typedef itk::ImageFileReader<LabelImageType> LabelReaderType;

    LabelReaderType::Pointer labelreader = LabelReaderType::New();
    labelreader->SetFileName( map );
    ImageReaderType::Pointer imagereader = ImageReaderType::New();
    imagereader->SetFileName ( image );
    labelreader->Update();
    imagereader->Update();

    LabelImageType::Pointer mask_image = labelreader->GetOutput();
    ImageType::Pointer in_img = imagereader->GetOutput();

    itk::ImageRegionConstIterator<LabelImageType> maskIterator(mask_image,
                                                            in_img->GetLargestPossibleRegion());
    itk::ImageRegionIterator<ImageType> inimageIterator(in_img,
                                                          in_img->GetLargestPossibleRegion());

    ImageType::PixelType min_val = itk::NumericTraits< ImageType::PixelType >::max() ;
    ImageType::PixelType max_val = itk::NumericTraits< ImageType::PixelType >::min() ;
    float mean = 0;
    float variance = 0;
    unsigned long volume = 0;
    while(!inimageIterator.IsAtEnd())
    {
        if (maskIterator.Get() != bck)
        {
            if (inimageIterator.Get() < min_val)
                min_val = inimageIterator.Get();
            if (inimageIterator.Get() > max_val)
                max_val = inimageIterator.Get();
            mean += inimageIterator.Get();
            volume++;
        }
        ++maskIterator;
        ++inimageIterator;
    }
    mean /= volume;

    inimageIterator.GoToBegin();
    maskIterator.GoToBegin();
    while(!inimageIterator.IsAtEnd())
    {
        if (maskIterator.Get() != bck)
            variance += ((inimageIterator.Get() - mean) *  (inimageIterator.Get() - mean));
        ++maskIterator;
        ++inimageIterator;
    }
    variance /= (volume -1);
    float std = sqrt(variance);

    if (outputFileName == "")
    {
      std::cout << "min: " << min_val << std::endl;
      std::cout << "max: " << max_val << std::endl;
      std::cout << "mean: " << mean << std::endl;
      std::cout << "sigma: " << std << std::endl;
      std::cout << "variance: " << variance<< std::endl;
      std::cout << "volume: " << volume << std::endl; //Not sure it is working properly

      std::cout << std::endl;
    }
    else
    {
        std::ofstream a_file;
        a_file.open (outputFileName.c_str());
        a_file << "min; " << min_val<< std::endl;
        a_file << "max; " << max_val << std::endl;
        a_file << "mean; " << mean << std::endl;
        a_file << "sigma; " << std << std::endl;
        a_file << "variance; " << variance << std::endl;
        a_file << "volume; " << volume << std::endl; //Not sure it is working properly

        a_file.close();
    }



    return EXIT_SUCCESS;
}
