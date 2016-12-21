/**
  * cardiovasc_utils.cpp
  * Utility file that I use for different purposes
  * This has been developed through the years so, it might be that
  * some options that appear within the help that are not working
  * @author M.A. Zuluaga
  */
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkOtsuThresholdImageFilter.h>
#include <itkInvertIntensityImageFilter.h>
#include <itkNumericTraits.h>
#include <itkSmoothingRecursiveGaussianImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkLabelShapeKeepNObjectsImageFilter.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryDilateImageFilter.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkBinaryImageToLabelMapFilter.h>
#include <itkLabelMapToLabelImageFilter.h>
#include <itkLabelOverlayImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkBinaryFillholeImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>
#if ITK_VERSION_MAJOR >= 4
#include <itkMultiplyImageFilter.h>
#else
#include <itkMultiplyByConstantImageFilter.h>
#endif
#include <map>

#define OTSU 1  //Otsu thresholding
#define INV 2   //Invert binary image
#define SMO 3   //Smooth image
#define DIL 4   //Dilate image
#define CON 5   //Largest Connected component
#define LTH 6   //Lower thresholding
#define UTH 7   //Upper thresholding
#define ITH 8   //Interval thresholding
#define OVL 9   //Overlay image
#define ERO 10  //Erode image
#define HOL 11  //Hole filling
#define MUL 12  //Multiply images

#define IN_VALUE 1
#define OUT_VALUE 0

void Usage(char *exec)
{
  std::cout << " " << std::endl;
  std::cout << "Image processing utilities" << std::endl;
  std::cout << " " << std::endl;
  std::cout << " " << exec << " [-i inputimage -o outputimage <options>  ]" << std::endl;
  std::cout << "**********************************************************" <<std::endl;
  std::cout << "Options:" <<std::endl;
  std::cout << "--mask <file> \t\t Mask image to be potentially used for some operations" <<std::endl;
  std::cout << "** Thresholding operations **" <<std::endl;
  std::cout << "--otsu \t\t\t Otsu thresholding" << std::endl;
  std::cout << "--lth <val> \t\t Threshold the image below <val>" <<std::endl;
  std::cout << "--uth <val> \t\t Threshold the image above <val>" <<std::endl;
  std::cout << "--ith <val1> <val2> \t Interval thresholding the image between <val1>-<val2>" <<std::endl;
  std::cout << " " << std::endl;
  std::cout << "** Morphological operations **" <<std::endl;
  std::cout << "--dil <val> \t\t Dilates the image using a spherical kernel of radius <val>" << std::endl;
  std::cout << "--ero <val> \t\t Erodes the image using a spherical kernel of radius <val>" << std::endl;
  std::cout << "--holes \t\t Hole filling operation" << std::endl;
  std::cout << "--lconcom \t\t Keeps the largest connected component " << std::endl;
  std::cout << " " << std::endl;
  std::cout << "** Arithmetical operations **" <<std::endl;
  std::cout  << "--inv \t\t\t Inverts the image by doing max(I) - I(x)" << std::endl;
  std::cout  << "--mul <val/file> \t Multiplies image by <val> if <val> is a number or by another"
             << " image if <val/file> is a valid filename" << std::endl;
  std::cout  << "--mul <val/file> \t Multiplies image by <val> if <val> is a number or by another"
             << " image if <val/file> is a valid filename" << std::endl;
  std::cout << " " << std::endl;
  std::cout << "** Filtering operations **" <<std::endl;
  std::cout << "--smo <val> \t\t Smooth the image with a Gaussian kernel of radius <val>" << std::endl;
  std::cout << " " << std::endl;
  std::cout << "** Misc operations **" <<std::endl;
  std::cout << "--overlay <file> \t Overlays the working image with binary image provided in <file>." << std::endl;
  std::cout << "\t\t\t WARN: This operation cannot be chained" << std::endl;
  std::cout << " " << std::endl;
}

//template<int Dim>
//void UtilsProcessingFunction<Dim>( int argc, char *argv[] );

template<int Dim, typename PixelType >
int UtilsProcessingFunction( int argc, char *argv[] )
{
  std::string inputImageNameOne;
  std::string inputImageNameTwo;
  std::string maskImageName;
  bool isMask = false;
  std::string outputImageName;
  std::vector<float> operations;

  /**
   * @brief aux_operations
   * Holds extra variables for different operations. Key is
   * the operation id (as in the defines) and it maps to a vector of
   * strings.
   * Validation of correctly parsed arguments can be achieved by counting
   * the elements in the vector.
   */
  std::vector<std::vector<std::string> > aux_operations;

  /**
   * @brief dummy Dummy variable to keep order when an operation does
   * not require any parameters.
   */
  std::vector<std::string> dummy;
  for(int i=1; i < argc; i++)
  {
    if(strcmp(argv[i], "-help")==0 || strcmp(argv[i], "-Help")==0 || strcmp(argv[i], "-HELP")==0 || strcmp(argv[i], "-h")==0 || strcmp(argv[i], "--h")==0)
    {
      Usage(argv[0]);
      return EXIT_FAILURE;
    }
    else if(strcmp(argv[i], "-i") == 0)
    {
      inputImageNameOne=argv[++i];
      std::cout << "Set --i=" << inputImageNameOne << std::endl;
    }
    else if(strcmp(argv[i], "-o") == 0)
    {
      outputImageName=argv[++i];
      aux_operations.push_back(dummy);
      std::cout << "Set -o=" << outputImageName << std::endl;
    }
    else if(strcmp(argv[i], "--mask") == 0)
    {
      maskImageName=argv[++i];
      isMask = true;
      aux_operations.push_back(dummy);
      std::cout << "Set --mask=" << maskImageName << std::endl;
    }
    else if(strcmp(argv[i], "--otsu") == 0)
    {
      operations.push_back(OTSU);
      aux_operations.push_back(dummy);
      std::cout << "Set -otsu=ON" << std::endl;
    }
    else if(strcmp(argv[i], "--inv") == 0)
    {
      operations.push_back(INV);
      aux_operations.push_back(dummy);
      std::cout << "Set -inv=ON" << std::endl;
    }
    else if(strcmp(argv[i], "--lconcom") == 0)
    {
      operations.push_back(CON);
      aux_operations.push_back(dummy);
      std::cout << "Set -lconcomp=ON" << std::endl;
    }
    else if(strcmp(argv[i], "--holes") == 0)
    {
      operations.push_back(HOL);
      aux_operations.push_back(dummy);
      std::cout << "Set -holes=ON" << std::endl;
    }
    else if(strcmp(argv[i], "--smo") == 0)
    {
      operations.push_back(SMO);
      std::vector<std::string> tmp_map;

      tmp_map.push_back(std::string(argv[++i]));
      aux_operations.push_back(tmp_map);
      std::cout << "Set -smo=ON" << std::endl;
    }
    else if(strcmp(argv[i], "--dil") == 0)
    {
      operations.push_back(DIL);
      std::vector<std::string> tmp_map;

      tmp_map.push_back(std::string(argv[++i]));
      aux_operations.push_back(tmp_map);
      std::cout << "Set -dil=ON" << std::endl;
    }
    else if(strcmp(argv[i], "--ero") == 0)
    {
      operations.push_back(ERO);
      std::vector<std::string> tmp_map;

      tmp_map.push_back(std::string(argv[++i]));
      aux_operations.push_back(tmp_map);
      std::cout << "Set -ero=ON" << std::endl;
    }
    else if(strcmp(argv[i], "--ith") == 0)
    {
      operations.push_back(ITH);
      std::vector<std::string> tmp_map;
      std::string low_th(argv[++i]); //lower threshold
      std::string up_th(argv[++i]); //upper threshold

      tmp_map.push_back(low_th);
      tmp_map.push_back(up_th);
      aux_operations.push_back(tmp_map);
      std::cout << "Set -intervalthresh [" << low_th << " " << up_th << "]" << std::endl;
    }
    else if(strcmp(argv[i], "--overlay") == 0)
    {
      operations.push_back(OVL);
      std::vector<std::string> tmp_map;
      std::string overlay_file(argv[++i]);

      tmp_map.push_back(overlay_file);
      aux_operations.push_back(tmp_map);
      std::cout << "Set -overlay with: " << overlay_file << std::endl;
    }
    else if(strcmp(argv[i], "--mul") == 0)
    {
      operations.push_back(MUL);
      std::vector<std::string> tmp_map;
      std::string mult_param(argv[++i]);

      tmp_map.push_back(mult_param);
      aux_operations.push_back(tmp_map);
      std::cout << "Set -mul with: " << mult_param << std::endl;
    }
  }

  if (outputImageName == "")
  {
    std::cerr << "Missing arguments: Exit" << std::endl;
    return EXIT_FAILURE;
  }

  typedef itk::Image<PixelType,Dim> ImageType;
  typedef itk::Image<unsigned short,Dim> BinImageType;
  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  typedef itk::ImageFileWriter<ImageType> WriterType;

  typename ImageReaderType::Pointer reader = ImageReaderType::New();
  reader->SetFileName(inputImageNameOne);
  reader->Update();
  typename ImageType::Pointer in_img = reader->GetOutput();
  typename ImageType::Pointer out_img = reader->GetOutput();
  typename ImageType::Pointer mask_img;

  if (isMask == true)
  {
      typename ImageReaderType::Pointer reader_mask = ImageReaderType::New();
      reader_mask->SetFileName(maskImageName);
      reader_mask->Update();
      mask_img = reader_mask->GetOutput();
  }

  for (size_t i = 0; i < operations.size(); ++i)
  {
    unsigned int option = static_cast<unsigned int>(operations[i]);
    //std::cout << "here " << op << std::endl;
    switch(option)
    {
    case OTSU:
    {
      typedef itk::OtsuThresholdImageFilter<ImageType,
          ImageType> OtsuFilterType;
      typename OtsuFilterType::Pointer otsu = OtsuFilterType::New();
      otsu->SetInput( in_img );
      otsu->SetInsideValue(IN_VALUE);
      otsu->SetOutsideValue(OUT_VALUE);
      if (isMask == true)
      {
          otsu->SetMaskImage( mask_img );
          otsu->SetMaskValue(IN_VALUE);
      }
      otsu->Update();
      in_img = otsu->GetOutput();
      out_img = otsu->GetOutput();
      in_img->DisconnectPipeline();

      break;
    }
    case INV:
    {
      typedef itk::MinimumMaximumImageCalculator <ImageType>
          ImageCalculatorFilterType;
      typedef itk::InvertIntensityImageFilter <ImageType>
          InvertIntensityImageFilterType;

      typename ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
      imageCalculatorFilter->SetImage(in_img);
      imageCalculatorFilter->ComputeMaximum();

      typename InvertIntensityImageFilterType::Pointer invertIntensityFilter
          = InvertIntensityImageFilterType::New();
      invertIntensityFilter->SetInput(in_img);
      invertIntensityFilter->SetMaximum(imageCalculatorFilter->GetMaximum());
      invertIntensityFilter->Update();
      in_img = invertIntensityFilter->GetOutput();
      out_img = invertIntensityFilter->GetOutput();
      in_img->DisconnectPipeline();
      break;
    }
    case SMO:
    {
      PixelType sigma = 1;
      try
      {
        std::vector<std::string> params = aux_operations[i];
        sigma = static_cast<PixelType>(atof(params[0].c_str()));
      } catch (...)
      {
        std::cout << "Invalid kernel size" << std::endl;
        return EXIT_FAILURE;
      }
      typedef itk::SmoothingRecursiveGaussianImageFilter<
          ImageType, ImageType >  GaussFilterType;

      typename GaussFilterType::Pointer gauss = GaussFilterType::New();
      gauss->SetInput(in_img);
      gauss->SetSigma(sigma*(in_img->GetSpacing())[0]);
      gauss->Update();
      in_img = gauss->GetOutput();
      out_img = gauss->GetOutput();
      in_img->DisconnectPipeline();

      break;
    }
    case DIL:
    {
      typedef itk::BinaryBallStructuringElement<
          unsigned short,Dim>   StructuringElementType;
      typedef itk::CastImageFilter<ImageType,
          BinImageType> CastToBinFilter;
      typedef itk::BinaryDilateImageFilter <BinImageType, ImageType,
          StructuringElementType> BinaryDilateImageFilterType;
      unsigned int radius = 0;
      try
      {
        std::vector<std::string> params = aux_operations[i];
        radius = static_cast<int>(atoi(params[0].c_str()));
      } catch (...)
      {
        std::cout << "Invalid kernel size" << std::endl;
        return EXIT_FAILURE;
      }

      StructuringElementType structuringElement;
      structuringElement.SetRadius(radius);
      structuringElement.CreateStructuringElement();

      typename CastToBinFilter::Pointer casttobin = CastToBinFilter::New();
      casttobin->SetInput(in_img);
      casttobin->Update();

      typename BinaryDilateImageFilterType::Pointer dilateFilter
          = BinaryDilateImageFilterType::New();
      dilateFilter->SetInput(casttobin->GetOutput());
      dilateFilter->SetKernel(structuringElement);
      dilateFilter->SetDilateValue(IN_VALUE);
      dilateFilter->Update();

      out_img = dilateFilter->GetOutput();
      in_img = dilateFilter->GetOutput();
      in_img->DisconnectPipeline();
      break;
    }
    case ERO:
    {
      typedef itk::BinaryBallStructuringElement<
          unsigned short,Dim>   StructuringElementType;
      typedef itk::CastImageFilter<ImageType,
          BinImageType> CastToBinFilter;
      typedef itk::BinaryErodeImageFilter <BinImageType, ImageType,
          StructuringElementType> BinaryErodeImageFilterType;
      unsigned int radius = 0;
      try
      {
        std::vector<std::string> params = aux_operations[i];
        radius = static_cast<int>(atoi(params[0].c_str()));
      } catch (...)
      {
        std::cout << "Invalid kernel size" << std::endl;
        return EXIT_FAILURE;
      }

      StructuringElementType structuringElement;
      structuringElement.SetRadius(radius);
      structuringElement.CreateStructuringElement();

      typename CastToBinFilter::Pointer casttobin = CastToBinFilter::New();
      casttobin->SetInput(in_img);
      casttobin->Update();

      typename BinaryErodeImageFilterType::Pointer erodeFilter
          = BinaryErodeImageFilterType::New();
      erodeFilter->SetInput(casttobin->GetOutput());
      erodeFilter->SetKernel(structuringElement);
      erodeFilter->SetErodeValue(IN_VALUE);
      erodeFilter->Update();

      out_img = erodeFilter->GetOutput();
      in_img = erodeFilter->GetOutput();
      in_img->DisconnectPipeline();

      break;
    }
    case CON:
    {
      typedef itk::CastImageFilter<ImageType,BinImageType> CastToBinFilter;
      typedef itk::ConnectedComponentImageFilter<BinImageType,
          BinImageType> ConnectFilterType;
      typedef itk::LabelShapeKeepNObjectsImageFilter< BinImageType >
          LabelShapeKeepNObjectsImageFilterType;

      typename CastToBinFilter::Pointer casttobin = CastToBinFilter::New();
      casttobin->SetInput(in_img);
      casttobin->Update();
      typename ConnectFilterType::Pointer connectfilter = ConnectFilterType::New();
      connectfilter->SetInput(casttobin->GetOutput());
      connectfilter->SetBackgroundValue(0);
      connectfilter->FullyConnectedOn();
      typename LabelShapeKeepNObjectsImageFilterType::Pointer labelfilter =
          LabelShapeKeepNObjectsImageFilterType::New();
      labelfilter->SetInput(connectfilter->GetOutput());
      labelfilter->SetBackgroundValue( 0 );
      labelfilter->SetNumberOfObjects( 1 );
      labelfilter->SetAttribute(LabelShapeKeepNObjectsImageFilterType::LabelObjectType::NUMBER_OF_PIXELS);
      labelfilter->Update();

      typedef itk::RescaleIntensityImageFilter< BinImageType, ImageType > RescaleFilterType;
      typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
      rescaleFilter->SetInput(labelfilter->GetOutput());
      rescaleFilter->SetOutputMinimum(OUT_VALUE);
      rescaleFilter->SetOutputMaximum(IN_VALUE);
      rescaleFilter->Update();
      in_img = rescaleFilter->GetOutput();
      out_img = rescaleFilter->GetOutput();
      in_img->DisconnectPipeline();
      break;
    }
    case ITH:
    {

      PixelType lowerThreshold = 0;
      PixelType upperThreshold = 0;
      try
      {
        std::vector<std::string> params = aux_operations[i];
        lowerThreshold = static_cast<PixelType>(atof(params[0].c_str()));
        upperThreshold = static_cast<PixelType>(atof(params[1].c_str()));
      } catch (...)
      {
        std::cout << "Invalid threshold parameters" << std::endl;
        return EXIT_FAILURE;
      }
      typedef itk::BinaryThresholdImageFilter<ImageType, ImageType>
          BinaryThresholdImageFilterType;
      typename BinaryThresholdImageFilterType::Pointer thresholdFilter
          = BinaryThresholdImageFilterType::New();
      thresholdFilter->SetInput(in_img);

      thresholdFilter->SetLowerThreshold(lowerThreshold);
      thresholdFilter->SetUpperThreshold(upperThreshold);
      thresholdFilter->SetInsideValue(IN_VALUE);
      thresholdFilter->SetOutsideValue(OUT_VALUE);
      thresholdFilter->Update();
      in_img = thresholdFilter->GetOutput();
      out_img = thresholdFilter->GetOutput();
      in_img->DisconnectPipeline();
      break;
    }
    case HOL:
    {
      typedef itk::BinaryFillholeImageFilter<ImageType> BinaryFillholeImageFilterType;

      typename BinaryFillholeImageFilterType::Pointer binholefilter;
      binholefilter = BinaryFillholeImageFilterType::New();
      binholefilter->SetInput(in_img);
      binholefilter->SetForegroundValue(IN_VALUE);
      binholefilter->FullyConnectedOn();
      binholefilter->Update();

      in_img = binholefilter->GetOutput();
      out_img = binholefilter->GetOutput();
      in_img->DisconnectPipeline();
      break;

    }
    case MUL:
    {
      std::string mul_param = (aux_operations[i])[0];
      float factor = 0;
      bool mul_factor = true;

      if (isdigit(mul_param.c_str()[0]))
      {
        try
        {
          std::vector<std::string> params = aux_operations[i];
          factor = static_cast<float>(atof(mul_param.c_str()));
          std::cout << factor << std::endl;
        } catch (...)
        {
          std::cout << "Invalid parameters" << std::endl;
          return EXIT_FAILURE;
        }
      }
      else
      {
        mul_factor = false;
      }

      typedef itk::MultiplyImageFilter <ImageType, ImageType >
          MultiplyImageFilterType;
      typename MultiplyImageFilterType::Pointer multiplyFilter
          = MultiplyImageFilterType::New ();

      if (mul_factor)
      {
        multiplyFilter->SetInput(in_img);
        multiplyFilter->SetConstant(factor);
      }
      else
      {
        typename ImageReaderType::Pointer mul_reader = ImageReaderType::New();
        mul_reader->SetFileName(mul_param);
        mul_reader->Update();
        multiplyFilter->SetInput1(in_img);
        multiplyFilter->SetInput2(mul_reader->GetOutput());
      }

      multiplyFilter->Update();
      in_img = multiplyFilter->GetOutput();
      out_img = multiplyFilter->GetOutput();
      in_img->DisconnectPipeline();
      break;
    }
    case OVL:
    {
      std::string overlay = (aux_operations[i])[0];

      typedef itk::RGBPixel<unsigned char> RGBPixelType;
      typedef itk::Image<RGBPixelType,Dim> RGBImageType;
      typedef itk::ImageFileReader<BinImageType> BinImageReaderType;
      typedef itk::LabelOverlayImageFilter<BinImageType, BinImageType, RGBImageType>
          LabelOverlayImageFilterType;
      typedef itk::ImageFileWriter<RGBImageType> RGBWriterType;


      typedef itk::RescaleIntensityImageFilter< ImageType, BinImageType > RescaleFilterType;
      typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
      rescaleFilter->SetInput(in_img);
      rescaleFilter->SetOutputMinimum(0);
      rescaleFilter->SetOutputMaximum(255);

      typename BinImageReaderType::Pointer binreader = BinImageReaderType::New();
      binreader->SetFileName(overlay);

      typename LabelOverlayImageFilterType::Pointer labelOverlayImageFilter = LabelOverlayImageFilterType::New();
      labelOverlayImageFilter->SetInput(rescaleFilter->GetOutput());
      labelOverlayImageFilter->SetLabelImage(binreader->GetOutput());
      labelOverlayImageFilter->SetOpacity(0.4);
      labelOverlayImageFilter->SetBackgroundValue( 0 );
      // labelOverlayImageFilter->AddColor(0,255,0);
      labelOverlayImageFilter->Update();

      typename RGBWriterType::Pointer writer = RGBWriterType::New();
      writer->SetInput(labelOverlayImageFilter->GetOutput());
      writer->SetFileName(outputImageName);

      try
      {
        writer->Update();
      }
      catch(itk::ExceptionObject & err)
      {
        std::cerr << "Something went wrong " << err.GetDescription() << std::endl;
        return EXIT_FAILURE;
      }

      return EXIT_SUCCESS;

    }

    }
  }
  typename WriterType::Pointer writer = WriterType::New();
  writer->SetInput(out_img);
  writer->SetFileName(outputImageName);

  try
  {
    writer->Update();
  }
  catch(itk::ExceptionObject & err)
  {
    std::cerr << "Something went wrong " << err.GetDescription() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int main( int argc, char *argv[] )
{
  std::string inputImageNameOne;

  for(int i=1; i < argc; i++)
  {
    if(strcmp(argv[i], "-help")==0 || strcmp(argv[i], "-Help")==0 || strcmp(argv[i], "-HELP")==0 || strcmp(argv[i], "-h")==0 || strcmp(argv[i], "--h")==0)
    {
      Usage(argv[0]);
      return EXIT_FAILURE;
    }
    else if(strcmp(argv[i], "-i") == 0)
    {
      inputImageNameOne=argv[++i];
      // std::cout << "Set --i=" << inputImageNameOne << std::endl;
      break;
    }
  }

  if (inputImageNameOne == "")
  {
    std::cerr << "Missing arguments: Exit" << std::endl;
    return EXIT_FAILURE;
  }

  // Get the image dimension
  itk::ImageIOBase::Pointer imageIO;
  try
  {
    imageIO = itk::ImageIOFactory::CreateImageIO(
          inputImageNameOne.c_str(), itk::ImageIOFactory::ReadMode);
    if ( imageIO )
    {
      imageIO->SetFileName(inputImageNameOne);
      imageIO->ReadImageInformation();
    }
    else
    {
      std::cout << "Could not read the fixed image information." << std::endl;
      exit( EXIT_FAILURE );
    }
  }
  catch( itk::ExceptionObject& err )
  {
    std::cout << "Could not read the fixed image information." << std::endl;
    std::cout << err << std::endl;
    exit( EXIT_FAILURE );
  }

  int resp = 0;
  switch ( imageIO->GetNumberOfDimensions() )
  {
  case 2:
    resp = UtilsProcessingFunction<2, float>(argc,argv);
    break;
  case 3:
    resp = UtilsProcessingFunction<3, float>(argc,argv);
    break;
  default:
    std::cout << "Unsuported dimension" << std::endl;
    exit( EXIT_FAILURE );
  }

  return resp;

}






