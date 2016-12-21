/**
  * extract_channels.cpp
  * Extracts a colour image channel. Default is the first one
  * Useful for converting the thresholded images from the micro-CT
  * to a grayscale binary format.
  * @author M.A. Zuluga
  */
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"

void Usage(char *exec)
{
    std::cout << " " << std::endl;
    std::cout << "Extracts channels from an RGB image" << std::endl;
    std::cout << " " << std::endl;
    std::cout << " " << exec << " [-i input file -o output file <options>  ]" << std::endl;
    std::cout << "**********************************************************" <<std::endl;
    std::cout << "Options:" <<std::endl;
    std::cout << "-c <int> \t Channel to extract (default 1, max 3)" << std::endl;
}

int main(int argc, char **argv)
{
    std::string inputFileName = "";
    std::string outputFileName = "";
    unsigned int channels = 1;

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
        else if(strcmp(argv[i], "-c") == 0)
        {
            channels=atoi(argv[++i]);
            std::cout << "Set --c=" << channels << std::endl;
        }
        else
        {
            std::cerr << argv[0] << ":\tParameter " << argv[i] << " unknown."
                                 << std::endl;
            return EXIT_FAILURE;
        }
    }

    //Validate command options
    if (inputFileName.length() == 0 && outputFileName.length() == 0)
    {
        Usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (channels < 1)
    {
        std::cout << "Number of channels cannot be less than one. Setting it to one to make "
                     << "your life simpler (my guess)" << std::endl;
        channels = 1;
    }
    if (channels > 3)
    {
        std::cout << "Number of channels cannot be > than 3. Setting it to one to make "
                     << "my life simpler" << std::endl;
        channels = 3;
    }
    channels--;

    const unsigned int Dimension = 2;

    typedef unsigned char                           ComponentType;
    typedef itk::RGBPixel< ComponentType >          InputPixelType;
    typedef itk::Image< InputPixelType, Dimension > InputImageType;

    typedef unsigned char                            OutputPixelType;
    typedef itk::Image< OutputPixelType, Dimension >  OutputImageType;
    typedef itk::ImageFileReader< InputImageType >   ReaderType;
    typedef itk::ImageFileWriter< OutputImageType >   WriterType;

    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName( inputFileName );
    reader->Update( );
    InputImageType::Pointer img = reader->GetOutput();
    OutputImageType::Pointer out_img = OutputImageType::New();
    OutputImageType::IndexType start;
    start.Fill(0);

    OutputImageType::SizeType size = img->GetLargestPossibleRegion().GetSize();
    OutputImageType::RegionType region(start, size);
    out_img->SetRegions(region);
    out_img->Allocate();
    out_img->FillBuffer(0);


    typedef itk::ImageRegionIterator< InputImageType > IteratorType;
    typedef itk::ImageRegionIterator< OutputImageType > OutIteratorType;
    IteratorType it ( img, img->GetLargestPossibleRegion() );
    OutIteratorType an_it( out_img,
                            out_img->GetLargestPossibleRegion() );

    while (!it.IsAtEnd())
    {
        InputPixelType p_rgb = it.Get();
        an_it.Set( p_rgb[channels] );
        ++an_it;
        ++it;
    }


    WriterType::Pointer writer = WriterType::New();
    try {
        writer->SetInput( out_img );
        writer->SetFileName(outputFileName);
        writer->Update();
    }
    catch( itk::ExceptionObject & excp ) {
        std::cerr << "Error: " << excp << std::endl;
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;


}
