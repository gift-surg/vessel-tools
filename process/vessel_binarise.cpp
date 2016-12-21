/**
  * vessel_binarise.cpp
  * Given a vessel map, it binarises it using a threshold.
  * @author M.A. Zuluaga
  */
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

#include "itkBinariseVesselResponseFilter.h"


void Usage(char *exec)
{
  std::cout << " " << std::endl;
  std::cout << "Binarises vesselness response." << std::endl;
  std::cout << " " << std::endl;
  std::cout << " " << exec << " [-i inputFileName -o outputFileName -t threshold (4 default)]" << std::endl;
  std::cout << " " << std::endl;
}

int main( int argc, char *argv[] )
{
  std::string inputImageName;
  std::string outputImageName;
  float thresh=4;

  for(int i=1; i < argc; i++)
  {
    if(strcmp(argv[i], "-help")==0 || strcmp(argv[i], "-Help")==0 || strcmp(argv[i], "-HELP")==0 || strcmp(argv[i], "-h")==0 || strcmp(argv[i], "--h")==0)
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
    else if(strcmp(argv[i], "-t") == 0)
    {
      thresh=atof(argv[++i]);
      std::cout << "Set -t=" << (thresh) << std::endl;
    }
  }

  // Validate command line args
  if (inputImageName.length() == 0 || outputImageName.length() == 0)
  {
    Usage(argv[0]);
    return EXIT_FAILURE;
  }

  //Check for the extension
  std::size_t found_nii = outputImageName.rfind(".nii");
  std::size_t found_mhd = outputImageName.rfind(".mhd");
  if ((found_nii == std::string::npos) && (found_mhd == std::string::npos))
  {
    outputImageName += ".nii";
  }

  const unsigned int Dimension = 3;
  typedef float InputPixelType;
  typedef unsigned int OutputPixelType;

  typedef itk::Image< InputPixelType, Dimension > InputImageType;
  typedef itk::Image< OutputPixelType, Dimension > OutputImageType;
  typedef itk::ImageFileReader< InputImageType > ReaderType;
  typedef itk::ImageFileWriter< OutputImageType > WriterType; //TODO Change back

  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( inputImageName );
  reader->Update();

  typedef itk::BinariseVesselResponseFilter<InputImageType,OutputImageType> BinariseFilter;
  BinariseFilter::Pointer binarise = BinariseFilter::New();
  binarise->SetInput( reader->GetOutput() );
  binarise->SetLowThreshold(thresh);

  WriterType::Pointer writer = WriterType::New();
  writer->SetInput( binarise->GetOutput() );
  writer->SetFileName( outputImageName );

  try
  {
    writer->Update();
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "Failed: " << err << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
