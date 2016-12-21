/**
  * convert_slices_to_vol.cpp
  * Stacks a series of slices into a single volume
  * The name of the slices should follow a pattern that is provided by the user
  * (see script for an example)
  * @author M.A. Zuluaga
  */
#include "itkImage.h"
#include "itkImageSeriesReader.h"
#include "itkImageFileWriter.h"
#include "itkNumericSeriesFileNames.h"


void Usage(char *exec)
{
    std::cout << " " << std::endl;
    std::cout << "Generates volume from a set of slices" << std::endl;
    std::cout << " " << std::endl;
    std::cout << " " << exec << " [-p pattern -s first slice value -e last slice value -o outputimage -v verbose ON]" << std::endl;
    std::cout << "**********************************************************" <<std::endl;
}

int main( int argc, char * argv[] )
{
    std::string pattern;
    unsigned int firstslice = 0;
    unsigned int lastslice = 0;
    bool verbose = false;
    std::string outputFileName;

    for(int i=1; i < argc; i++)
    {
        if(strcmp(argv[i], "-help")==0 || strcmp(argv[i], "-Help")==0
                || strcmp(argv[i], "-HELP")==0 || strcmp(argv[i], "-h")==0
                || strcmp(argv[i], "--h")==0)
        {
            Usage(argv[0]);
            return EXIT_FAILURE;
        }
        else if(strcmp(argv[i], "-p") == 0)
        {
            pattern=argv[++i];
            std::cout << "Set --p=" << pattern << std::endl;
        }
        else if(strcmp(argv[i], "-s") == 0)
        {
            firstslice=atoi(argv[++i]);
            std::cout << "Set --i2=" << firstslice << std::endl;
        }
        else if(strcmp(argv[i], "-e") == 0)
        {
            lastslice=atoi(argv[++i]);
            std::cout << "Set --e=" << lastslice << std::endl;
        }
        else if(strcmp(argv[i], "-o") == 0)
        {
            outputFileName=argv[++i];
            std::cout << "Set --o=" << outputFileName << std::endl;
        }
        else if(strcmp(argv[i], "-v") == 0)
        {
            ++i;
            verbose = true;
            std::cout << "Set --verbose=ON" << std::endl;
        }
        else
        {
            std::cout << "Error in arguments" << std::endl;
            Usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    //Validate input files
    if (pattern == "" || outputFileName == "")
    {
        std::cout << "Missing arguments" << std::endl;
        Usage(argv[0]);
        return EXIT_FAILURE;
    }

    typedef unsigned char   PixelType;
    const unsigned int Dimension = 3;

    typedef itk::Image< PixelType, Dimension >  ImageType;
    typedef itk::ImageSeriesReader< ImageType > ReaderType;
    typedef itk::ImageFileWriter<   ImageType > WriterType;

    ReaderType::Pointer reader = ReaderType::New();
    WriterType::Pointer writer = WriterType::New();


    typedef itk::NumericSeriesFileNames    NameGeneratorType;

    NameGeneratorType::Pointer nameGenerator = NameGeneratorType::New();

    nameGenerator->SetSeriesFormat( pattern );

    nameGenerator->SetStartIndex( firstslice );
    nameGenerator->SetEndIndex( lastslice );
    nameGenerator->SetIncrementIndex( 1 );
    std::vector<std::string> names = nameGenerator->GetFileNames();

    // List the files for visual check - Make verbose OFF if annoying
    if (verbose)
    {
        std::vector<std::string>::iterator nit;
        for (nit = names.begin(); nit != names.end(); nit++)
            std::cout << "File: " << (*nit).c_str() << std::endl;
    }

    reader->SetFileNames( names  );

     writer->SetFileName( outputFileName );
     writer->SetInput( reader->GetOutput() );
     try
     {
         writer->Update();
     }
     catch( itk::ExceptionObject & err )
     {
         std::cerr << "ExceptionObject caught !" << std::endl;
         std::cerr << err << std::endl;
         return EXIT_FAILURE;
     }
    return EXIT_SUCCESS;
}
