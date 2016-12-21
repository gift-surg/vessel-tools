/**
  * vessel_filter.cpp
  * Applies sato filter to an image
  * It was originally conceived to work within the brain and providing a mask
  * I strongly recommend the use of the mask as the results improve significantly.
  * How to get a mask? Do some Otsu thresholding. If no mask, you need to provide
  * a dummy image.
  * Note: Current help of this file (i.e. if -h is run) does not show all the options
  * as many are not relevant for the placenta problem or have been deprecated.
  * @author M.A. Zuluaga
  */
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include "itkMultiScaleVesselnessFilter.h"
#include "itkBrainMaskFromCTFilter.h"

void Usage(char *exec)
{
  std::cout << " " << std::endl;
  std::cout << "Applies Sato vesselness filter." << std::endl;
  std::cout << " " << std::endl;
  std::cout << " " << exec << " [-i inputFileName -o outputFileName -b mask]" << std::endl;
  std::cout << "**********************************************************" <<std::endl;
  std::cout << "Options:" <<std::endl;
  std::cout << "--min <float> \t Minimum scale value (default 1)" << std::endl;
  std::cout << "--max <float> \t Maximum scale value (default 3.09). Set min and max the same for single scale)" << std::endl;
  std::cout << "--aone <float> \t Alpha one of Sato filter (default 0.5)" << std::endl;
  std::cout << "--atwo <float> \t Alpha two of Sato filter (default 0.5)" << std::endl;
  std::cout << " " << std::endl;
  std::cout << " " << std::endl;
}

int main( int argc, char *argv[] )
{
  std::string inputImageName;
  std::string outputImageName;
  std::string brainImageName;
  unsigned int mod = 0;
  float max = 3.09375;
  float min = 1;
  float alphaone = 0.5;
  float alphatwo = 2.0;
  bool isCT = false;
  bool iscast = false;

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
    else if(strcmp(argv[i], "-b") == 0)
    {
      brainImageName=argv[++i];
      std::cout << "Set -b=" << brainImageName << std::endl;
    }
    else if(strcmp(argv[i], "--mod") == 0)
    {
      mod=atoi(argv[++i]);
      std::cout << "Set -mod=" << (mod) << std::endl;
    }
    else if(strcmp(argv[i], "--aone") == 0)
    {
      alphaone=atof(argv[++i]);
      std::cout << "Set -aone=" << (alphaone) << std::endl;
    }
    else if(strcmp(argv[i], "--atwo") == 0)
    {
      alphatwo=atof(argv[++i]);
      std::cout << "Set -atwo=" << (alphatwo) << std::endl;
    }
    else if(strcmp(argv[i], "--max") == 0)
    {
      max=atof(argv[++i]);
      std::cout << "Set -max=" << (max) << std::endl;
    }
    else if(strcmp(argv[i], "--min") == 0)
    {
      min=atof(argv[++i]);
      std::cout << "Set -min=" << (min) << std::endl;
    }
    else if(strcmp(argv[i], "--ct") == 0)
    {
      isCT=true;
      std::cout << "Set -ct=ON" << std::endl;
    }
    else if(strcmp(argv[i], "--cast") == 0)
    {
      iscast=true;
      std::cout << "Set -cast=ON" << std::endl;
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
  typedef short InputPixelType;
  typedef float InternalPixelType;

  typedef itk::Image< InputPixelType, Dimension > InputImageType;
  typedef itk::Image< InternalPixelType, Dimension > VesselImageType;
  typedef itk::MultiScaleVesselnessFilter< InputImageType, VesselImageType >  VesselnessFilterType;
  typedef itk::ImageFileReader< InputImageType > ReaderType;

  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( inputImageName );
  reader->Update();
  InputImageType::Pointer in_image = reader->GetOutput();
  InputImageType::SpacingType spacing = in_image->GetSpacing();
  InputImageType::SizeType size_in = in_image->GetLargestPossibleRegion().GetSize();

  ReaderType::Pointer mask_reader;
  InputImageType::Pointer mask_image;
  InputImageType::SizeType size_mask;
  bool useMask = false;
  if (brainImageName.length() > 0 )
  {
    mask_reader = ReaderType::New();
    mask_reader->SetFileName( brainImageName );
    mask_reader->Update();
    mask_image = mask_reader->GetOutput();
    useMask = true;
    size_mask = mask_image->GetLargestPossibleRegion().GetSize();

    if (size_mask[0] != size_in[0] || size_mask[1] != size_in[1] || size_mask[2] != size_in[2])
    {
      std::cout << "Warning: Mask and input image have different dimensions. Ignoring mask..." << std::endl;
      useMask = false;
    }
  }

  if (isCT)
    min = 0.775438;

  float min_spacing = static_cast<float>(spacing[0]);

  if (min < min_spacing)
    min = min_spacing;

  bool neg_img = false;
  if (isCT)
  {
    itk::ImageRegionIterator<InputImageType> inimageIterator(in_image,in_image->GetLargestPossibleRegion());
    while(!inimageIterator.IsAtEnd() && !neg_img) //Check if image comes in HU
    {
      if (inimageIterator.Get() < 0)
        neg_img = true;
      ++inimageIterator;
    }
  }

  //Create a skull mask from CTA
  if (isCT && !useMask)
  {
    typedef itk::BrainMaskFromCTFilter<InputImageType,InputImageType> MaskFilterType;
    MaskFilterType::Pointer maskfilter = MaskFilterType::New();
    maskfilter->SetInput(in_image);
    maskfilter->CheckHounsFieldUnitsOff();
    maskfilter->SetIsHU(neg_img);
    maskfilter->Update();
    mask_image = maskfilter->GetOutput();
    useMask = true;

  } //end skull mask

  if (useMask) // Erode for CT, dilate for other modalities!
  {
    typedef itk::BinaryBallStructuringElement<
        InputImageType::PixelType,Dimension>                  StructuringElementType;
    StructuringElementType structuringElement;
    if (isCT) {
      structuringElement.SetRadius(1);
      structuringElement.CreateStructuringElement();
      typedef itk::BinaryErodeImageFilter<InputImageType,InputImageType,StructuringElementType> ErodeFilter;
      ErodeFilter::Pointer erode = ErodeFilter::New();
      erode->SetInput( mask_reader->GetOutput() );
      erode->SetKernel(structuringElement);
      erode->SetErodeValue(1);
       erode->SetBackgroundValue(0);
      erode->Update();
      mask_image = erode->GetOutput();
    }
    else
    {
      structuringElement.SetRadius(8);
      structuringElement.CreateStructuringElement();
      typedef itk::BinaryDilateImageFilter<InputImageType,InputImageType,StructuringElementType> DilateFilter;
      DilateFilter::Pointer dilate = DilateFilter::New();
      dilate->SetInput( mask_reader->GetOutput() );
      dilate->SetKernel(structuringElement);
      dilate->SetDilateValue(1);
      dilate->SetBackgroundValue(0);
      dilate->Update();
      mask_image = dilate->GetOutput();
    }
    mask_image->DisconnectPipeline();
  }

  VesselnessFilterType::Pointer vesselnessFilter = VesselnessFilterType::New();
  vesselnessFilter->SetInput( in_image );
  vesselnessFilter->SetAlphaOne( alphaone );
  vesselnessFilter->SetAlphaTwo( alphatwo );
  vesselnessFilter->SetMinScale( min );
  vesselnessFilter->SetMaxScale( max );
  vesselnessFilter->SetScaleMode(static_cast<VesselnessFilterType::ScaleModeType>(mod));
  vesselnessFilter->Update();
  VesselImageType::Pointer maxImage = vesselnessFilter->GetOutput();
  maxImage->DisconnectPipeline();
  itk::ImageRegionIterator<VesselImageType> outimageIterator(maxImage,maxImage->GetLargestPossibleRegion());

  if (useMask && isCT)
  {
    itk::ImageRegionConstIterator<InputImageType> maskIterator(mask_image,maxImage->GetLargestPossibleRegion());
    itk::ImageRegionConstIterator<InputImageType> inimageIterator(in_image,maxImage->GetLargestPossibleRegion());
    outimageIterator.GoToBegin();
    InputPixelType thresh = 400;
    if (!neg_img)
      thresh = 1324;
    while(!outimageIterator.IsAtEnd()) //Apply brain mask
    {
      if (maskIterator.Get() == 0 || inimageIterator.Get() >= thresh)
        outimageIterator.Set(0);
      ++outimageIterator;
      ++maskIterator;
      ++inimageIterator;
    }
  }
  else if (useMask)
  {
    itk::ImageRegionConstIterator<InputImageType> maskIterator(mask_image,maxImage->GetLargestPossibleRegion());
    outimageIterator.GoToBegin();
    while(!outimageIterator.IsAtEnd()) //Apply brain mask
    {
      if (maskIterator.Get() == 0)
        outimageIterator.Set(0);
      ++outimageIterator;
      ++maskIterator;
    }
  }

  if (iscast)
  {
    typedef unsigned short OutputPixelType;
    typedef itk::Image< OutputPixelType, Dimension > OutputImageType;
    typedef itk::CastImageFilter< VesselImageType, OutputImageType > CastOutFilterType;
    typedef itk::ImageFileWriter< OutputImageType > WriterType; //TODO Change back
    WriterType::Pointer writer = WriterType::New();
    CastOutFilterType::Pointer caster = CastOutFilterType::New();
    caster->SetInput(maxImage);
    writer->SetInput(caster->GetOutput());
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
  }
  else
  {
    typedef itk::ImageFileWriter< VesselImageType > WriterType; //TODO Change back
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput(maxImage);
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
  }
  return EXIT_SUCCESS;
}

