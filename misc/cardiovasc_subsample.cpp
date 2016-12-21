/**
  * cardiovasc_subsample.cpp
  * Subsamples an image by a factor given by the user
  * By default reduction is isotropic but if
  * factors are given for all it can be anisotropic too
  * IMPORTANT! It accepts streaming
  * @author Maria A. Zuluaga
  */
#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkImageFileReader.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkShrinkImageFilter.h"
#include "itkImageIOBase.h"


void Usage(char *exec)
{
    std::cout << " " << std::endl;
    std::cout << "Subsamples an image by a iven factor" << std::endl;
    std::cout << " " << exec << " [-i input image -o output image ]" << std::endl;
    std::cout << "**********************************************************" <<std::endl;
    std::cout << "Options:" <<std::endl;
    std::cout << "--fx <file> \t Factor in x (default 2)" << std::endl;
    std::cout << "--fy <int> \t Factor in y (default 2)" << std::endl;
    std::cout << "--fz <int> \t Factor in z (default 2)" << std::endl;
    std::cout << std::endl;
}

template< class TImage >
int shrinkImage( std::string inputFilename,
                 std::string outputFilename,
                 unsigned int fx, unsigned int fy,
                 unsigned int fz )
{
    typedef TImage                            ImageType;
    typedef itk::ImageFileReader< ImageType > ImageReaderType;

    typename ImageReaderType::Pointer reader = ImageReaderType::New();
    reader->SetFileName( inputFilename );
    reader->Update();
    typename ImageType::Pointer image = reader->GetOutput();

    std::cout << "Original size: " << image->GetLargestPossibleRegion().GetSize()
              << std::endl;

    typedef itk::ShrinkImageFilter<ImageType, ImageType> ShrinkImageFilterType;

    typename ShrinkImageFilterType::Pointer shrinkFilter = ShrinkImageFilterType::New();
    shrinkFilter->SetInput(image);
    shrinkFilter->SetShrinkFactor(0, fx);
    shrinkFilter->SetShrinkFactor(1, fy);
    shrinkFilter->SetShrinkFactor(2, fz);
    shrinkFilter->Update();

    std::cout << "New size: "
              << shrinkFilter->GetOutput()->GetLargestPossibleRegion().GetSize()
              << std::endl;

    typedef itk::ImageFileWriter< ImageType > WriterType;
    typename WriterType::Pointer writer = WriterType::New();
    writer->SetNumberOfStreamDivisions( 200 ); //This is for the streaming!
    writer->SetFileName( outputFilename );
    writer->SetInput( shrinkFilter->GetOutput() );

    try
    {
      writer->Update();
    }
    catch( itk::ExceptionObject & e )
    {
      std::cerr << "Error: " << e << std::endl;
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


int main( int argc, char * argv[] )
{
    std::string inputFileName;
    std::string outputFileName;
    unsigned int fx = 2;
    unsigned int fy = 2;
    unsigned int fz = 2;
    unsigned int count_factors = 0;

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
            inputFileName=argv[++i];
            std::cout << "Set --i=" << inputFileName << std::endl;
        }
        else if(strcmp(argv[i], "-o") == 0)
        {
            outputFileName=argv[++i];
            std::cout << "Set --o=" << outputFileName << std::endl;
        }
        else if(strcmp(argv[i], "--fx") == 0)
        {
            fx=atoi(argv[++i]);
            count_factors++;
            std::cout << "Set --fx=" << fx << std::endl;
        }
        else if(strcmp(argv[i], "--fy") == 0)
        {
            fy=atoi(argv[++i]);
            count_factors++;
            std::cout << "Set --fy=" << fy << std::endl;
        }
        else if(strcmp(argv[i], "--fz") == 0)
        {
            fz=atoi(argv[++i]);
            count_factors++;
            std::cout << "Set --fz=" << fz << std::endl;
        }
        else
        {
            std::cout << "Error in arguments" << std::endl;
            Usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    // Validate command line args
    if (inputFileName.length() == 0 || outputFileName.length() == 0)
    {
      Usage(argv[0]);
      return EXIT_FAILURE;
    }

    if (count_factors == 1) //Only one factor given so assume desired isotropy.
    {
      if (fx != 2)
      {
        fy = fx;
        fz = fx;
      }
      else if (fy != 2)
      {
        fx = fy;
        fz = fy;
      }
      else if (fz != 2) //not needed in principle
      {
        fx = fz;
        fy = fz;
      }
    }

    //Now reading any 3D image
    const unsigned int Dimension = 3;
    itk::ImageIOBase::Pointer imageIO = itk::ImageIOFactory::CreateImageIO(
          inputFileName.c_str(), itk::ImageIOFactory::ReadMode );

    imageIO->SetFileName( inputFileName );
    imageIO->ReadImageInformation();

    typedef itk::ImageIOBase::IOComponentType IOComponentType;
    const IOComponentType componentType = imageIO->GetComponentType();

    switch( componentType )
    {
        default:
        case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
          std::cerr << "Unknown and unsupported component type!" << std::endl;
          return EXIT_FAILURE;

        case itk::ImageIOBase::UCHAR:
        {
          typedef unsigned char PixelType;
          typedef itk::Image< PixelType, Dimension > ImageType;

          return shrinkImage< ImageType >( inputFileName, outputFileName, fx, fy, fz );

        }
        case itk::ImageIOBase::CHAR:
        {
          typedef char PixelType;
          typedef itk::Image< PixelType, Dimension > ImageType;

          return shrinkImage< ImageType >( inputFileName, outputFileName, fx, fy, fz );
        }
        case itk::ImageIOBase::USHORT:
        {
          typedef unsigned short PixelType;
          typedef itk::Image< PixelType, Dimension > ImageType;

          return shrinkImage< ImageType >( inputFileName, outputFileName, fx, fy, fz );
        }
        case itk::ImageIOBase::SHORT:
        {
          typedef short PixelType;
          typedef itk::Image< PixelType, Dimension > ImageType;

          return shrinkImage< ImageType >( inputFileName, outputFileName, fx, fy, fz );
        }
        case itk::ImageIOBase::UINT:
        {
          typedef unsigned int PixelType;
          typedef itk::Image< PixelType, Dimension > ImageType;

          return shrinkImage< ImageType >( inputFileName, outputFileName, fx, fy, fz );
        }
        case itk::ImageIOBase::INT:
        {
          typedef int PixelType;
          typedef itk::Image< PixelType, Dimension > ImageType;

          return shrinkImage< ImageType >( inputFileName, outputFileName, fx, fy, fz );
        }
        case itk::ImageIOBase::ULONG:
        {
          typedef unsigned long PixelType;
          typedef itk::Image< PixelType, Dimension > ImageType;

          return shrinkImage< ImageType >( inputFileName, outputFileName, fx, fy, fz );
        }
        case itk::ImageIOBase::LONG:
        {
          typedef long PixelType;
          typedef itk::Image< PixelType, Dimension > ImageType;

          return shrinkImage< ImageType >( inputFileName, outputFileName, fx, fy, fz );
        }
        case itk::ImageIOBase::FLOAT:
        {
          typedef float PixelType;
          typedef itk::Image< PixelType, Dimension > ImageType;

          return shrinkImage< ImageType >( inputFileName, outputFileName, fx, fy, fz );
        }
        case itk::ImageIOBase::DOUBLE:
        {
          typedef double PixelType;
          typedef itk::Image< PixelType, Dimension > ImageType;

          return shrinkImage< ImageType >( inputFileName, outputFileName, fx, fy, fz );
        }
    }

    return EXIT_FAILURE; //but it should never get here
}
