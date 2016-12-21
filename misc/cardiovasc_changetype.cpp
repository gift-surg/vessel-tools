/**
  * change_datatype.cpp
  * Rescales and casts an image to be
  * unsigned char.
  * @author M.A. Zuluaga
  */
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkCastImageFilter.h"

void Usage(char *exec)
{
  std::cout << " " << std::endl;
  std::cout << "Changes from float to unsigned char" << std::endl;
  std::cout << "TODO: Make it generic (data and dimension)" << std::endl;
  std::cout << " " << std::endl;
  std::cout << " " << exec << " [-i inputimage -o outputimage  ]" << std::endl;
  std::cout << "**********************************************************" <<std::endl;

}

int main(int argc, char *argv[] )
{
  std::string inputImageName;
  std::string outputImageName;

  for(int i=1; i < argc; i++)
  {
    if(strcmp(argv[i], "-help")==0 || strcmp(argv[i], "-Help")==0 ||
       strcmp(argv[i], "-HELP")==0 || strcmp(argv[i], "-h")==0 ||
       strcmp(argv[i], "--h")==0)
    {
      Usage(argv[0]);
      return -1;
    }
    else if(strcmp(argv[i], "-i") == 0)
    {
      inputImageName=argv[++i];
      std::cout << "Set -i=" << inputImageName << std::endl;
    }
    else if(strcmp(argv[i], "-o") == 0)
    {
      outputImageName=argv[++i];
      std::cout << "Set -o=" << outputImageName << std::endl;
    }
    else
    {
      std::cerr << argv[0] << ":\tParameter " << argv[i] << " unknown."
                           << std::endl;
      Usage(argv[0]);
      return EXIT_FAILURE;
    }
  }

  // Validate command line args
  if (inputImageName.length() == 0 || outputImageName.length() == 0)
  {
    Usage(argv[0]);
    return EXIT_FAILURE;
  }

  const unsigned int Dimension = 3;
  typedef float                                     InputPixelType;
  typedef unsigned char                             OutputPixelType;
  typedef itk::Image< InputPixelType, Dimension >   InputImageType;
  typedef itk::Image< OutputPixelType, Dimension >  OutputImageType;

  typedef itk::ImageFileReader< InputImageType >  ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( inputImageName );

  typedef itk::RescaleIntensityImageFilter< InputImageType, InputImageType >
      RescaleType;
  RescaleType::Pointer rescale = RescaleType::New();
  rescale->SetInput( reader->GetOutput() );
  rescale->SetOutputMinimum( 0 );
  rescale->SetOutputMaximum( itk::NumericTraits< OutputPixelType >::max() );

  typedef itk::CastImageFilter< InputImageType, OutputImageType > FilterType;
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput( rescale->GetOutput() );

  typedef itk::ImageFileWriter< OutputImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( outputImageName );
  writer->SetInput( filter->GetOutput() );

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


