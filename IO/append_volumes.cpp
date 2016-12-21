/**
  * append_volumes.cpp
  * Merges two volumes into one. Volumes are
  * connected via the 3rd dimension.
  * Final vol size is max(x1,x2),max(y1,y2), z1+z2.
  * @author M.A. Zuluga
  */

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkTileImageFilter.h>
#include <itkChangeInformationImageFilter.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>



void Usage(char *exec)
{
    std::cout << " " << std::endl;
    std::cout << "Piles two volumes" << std::endl;
    std::cout << " " << std::endl;
    std::cout << " " << exec << " [-i1 file 1 -i2 file 2 -o outputimage <options>  ]" << std::endl;
    std::cout << "**********************************************************" <<std::endl;
    std::cout << "Options:" <<std::endl;
    std::cout << "-v <float> \t Isotropic voxel spacing (default 1.0)" << std::endl;
}


int main(int argc, char *argv[] )
{
    std::string inputFileName1;
    std::string inputFileName2;
    std::string outputFileName;
    float scalingFactor = 1.0;
    bool change_spacing = false;

    for(int i=1; i < argc; i++)
    {
        if(strcmp(argv[i], "-help")==0 || strcmp(argv[i], "-Help")==0
                || strcmp(argv[i], "-HELP")==0 || strcmp(argv[i], "-h")==0
                || strcmp(argv[i], "--h")==0)
        {
            Usage(argv[0]);
            return EXIT_FAILURE;
        }
        else if(strcmp(argv[i], "-i1") == 0)
        {
            inputFileName1=argv[++i];
            std::cout << "Set --i1=" << inputFileName1 << std::endl;
        }
        else if(strcmp(argv[i], "-i2") == 0)
        {
            inputFileName2=argv[++i];
            std::cout << "Set --i2=" << inputFileName2 << std::endl;
        }
        else if(strcmp(argv[i], "-o") == 0)
        {
            outputFileName=argv[++i];
            std::cout << "Set --o=" << outputFileName << std::endl;
        }
        else if(strcmp(argv[i], "-v") == 0)
        {
            scalingFactor=atof(argv[++i]);
            change_spacing = true;
            std::cout << "Set --v=" << scalingFactor << std::endl;
        }
        else
        {
            std::cout << "Error in arguments" << std::endl;
            Usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    // Validate command line args
    if (inputFileName1.length() == 0 || inputFileName2.length() == 0 ||
            outputFileName.length() == 0)
    {
        std::cout << "Missing required files" << std::endl;
        Usage(argv[0]);
        return EXIT_FAILURE;
    }

    //Two image types even if the same, justin case there is need for changing
    typedef itk::Image<unsigned char,3> ImageType;
    typedef itk::Image<unsigned char,3> OutImageType;
    typedef itk::ImageFileReader<ImageType> ImageReaderType;
    typedef itk::ImageFileWriter<OutImageType> WriterType;
    typedef itk::TileImageFilter<ImageType, OutImageType>
            JoinSeriesFilterType;
    typedef itk::ChangeInformationImageFilter< OutImageType > FilterType;

    ImageReaderType::Pointer reader1 = ImageReaderType::New();
    ImageReaderType::Pointer reader2 = ImageReaderType::New();
    JoinSeriesFilterType::Pointer joinSeries = JoinSeriesFilterType::New();
    FilterType::Pointer changeInfo = FilterType::New();

    itk::FixedArray< unsigned int, 3> layout;
    layout[0] = 1;
    layout[1] = 1;
    layout[2] = 0;

    reader1->SetFileName(inputFileName1);
    reader2->SetFileName(inputFileName2);
    reader1->Update();
    reader2->Update();

    joinSeries->SetInput( 0, reader1->GetOutput());
    joinSeries->SetInput( 1, reader2->GetOutput());
    joinSeries->SetLayout( layout );
    joinSeries->Update();

    typedef itk::ImageFileWriter< OutImageType > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName( outputFileName );

    //Check if spacing info needs to be updated
    if (change_spacing == true)
    {
        OutImageType::SpacingType spacing = ( scalingFactor );
        changeInfo->SetOutputSpacing( spacing );
        changeInfo->ChangeSpacingOn();
        changeInfo->SetInput( joinSeries->GetOutput() );
        changeInfo->UpdateOutputInformation();
        writer->SetInput( changeInfo->GetOutput() );
    }
    else
    {
        writer->SetInput( joinSeries->GetOutput() );
    }

    try
    {
        writer->Update();
    }
    catch( itk::ExceptionObject & error )
    {
        std::cerr << "Error: " << error << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
