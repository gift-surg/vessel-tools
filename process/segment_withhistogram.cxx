/**
  * segment_withhistogram.cxx
  * Identifies peak at the histogram and keeps all values
  * greater than the max and once the frequency drops to < 10% of
  * the max.
  * This is the simplified version of the code finding the two lobes
  * as I am getting rid of the main one through the mask.
  * @author Maria A. Zuluaga
  */
#include <itkImage.h>
#include <itkImageToHistogramFilter.h>
#include <itkImageRandomIteratorWithIndex.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageRegionIterator.h>
#include <itkBinaryThresholdImageFilter.h>

void Usage(char *exec)
{
    std::cout << " " << std::endl;
    std::cout << "Segmentation using the image histogram." << std::endl;
    std::cout << " " << std::endl;
    std::cout << " " << exec << " [-i inputimage -o outputimage -m maskimage <options>  ]" << std::endl;
    std::cout << "**********************************************************" <<std::endl;

}

int main(int argc, char *argv[] )
{
    std::string inputImageName;
    std::string outputImageName;
    std::string maskImageName;
    float ratio_thresh = 0.070;

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
        else if(strcmp(argv[i], "-m") == 0)
        {
            maskImageName=argv[++i];
            std::cout << "Set -m=" << maskImageName << std::endl;
        }
        else
        {
            std::cerr << argv[0] << ":\tParameter " << argv[i] << " unknown."
                                 << std::endl;
            return -1;
        }
    }

    // Validate command line args
    if (inputImageName.length() == 0 || outputImageName.length() == 0)
    {
        //Usage(argv[0]);
        return EXIT_FAILURE;
    }

    const unsigned int Dimension = 3;
    typedef short PixelType;
    typedef unsigned char FinalPixelType;
    typedef float OutPixelType;
    typedef itk::Image<OutPixelType, Dimension> ImageType;
    typedef itk::Image<FinalPixelType, Dimension> SegImageType;
    typedef itk::Image<PixelType,Dimension> Image3DType;
    typedef itk::ImageFileReader< Image3DType > Reader3DType;

    Reader3DType::Pointer reader = Reader3DType::New();
    reader->SetFileName( inputImageName );
    reader->Update();
    Image3DType::Pointer in_img = reader->GetOutput();
    Image3DType::SizeType size_img = in_img->GetLargestPossibleRegion().GetSize();

    Reader3DType::Pointer mask_reader = Reader3DType::New();
    mask_reader->SetFileName( maskImageName );
    mask_reader->Update();
    Image3DType::Pointer mask_image = mask_reader->GetOutput();
    Image3DType::SizeType size_mask = mask_image->GetLargestPossibleRegion().GetSize();

    if (size_mask[0] != size_img[0] || size_mask[1] != size_img[1] || size_mask[2] != size_img[2])
    {
        std::cout << "Warning: Mask and input image have different dimensions. Exiting..." << std::endl;
        return EXIT_FAILURE;
    }

    itk::ImageRegionConstIterator<Image3DType> maskIterator(mask_image,
                                                            in_img->GetLargestPossibleRegion());
    itk::ImageRegionIterator<Image3DType> inimageIterator(in_img,
                                                          in_img->GetLargestPossibleRegion());

    inimageIterator.GoToBegin();
    maskIterator.GoToBegin();
    OutPixelType min_val = itk::NumericTraits< OutPixelType >::max() ;
    while(!inimageIterator.IsAtEnd())
    {
        if (maskIterator.Get() != 0)
        {
            if (inimageIterator.Get() < min_val)
                min_val = inimageIterator.Get();
        }
        ++maskIterator;
        ++inimageIterator;
    }

    const unsigned int MeasurementVectorSize = 1; // Grayscale
    const unsigned int binsPerDimension = 30;

    typedef itk::Statistics::ImageToHistogramFilter< Image3DType > ImageToHistogramFilterType;

    ImageToHistogramFilterType::HistogramType::MeasurementVectorType lowerBound(binsPerDimension);
    lowerBound.Fill(0);
    std::cout << min_val << std::endl;

    ImageToHistogramFilterType::HistogramType::MeasurementVectorType upperBound(binsPerDimension);
    upperBound.Fill(255) ;

    ImageToHistogramFilterType::HistogramType::SizeType size(MeasurementVectorSize);
    size.Fill(binsPerDimension);

    ImageToHistogramFilterType::Pointer imageToHistogramFilter = ImageToHistogramFilterType::New();
    imageToHistogramFilter->SetInput(in_img);
    imageToHistogramFilter->SetHistogramBinMinimum(lowerBound);
    imageToHistogramFilter->SetHistogramBinMaximum(upperBound);
    imageToHistogramFilter->SetHistogramSize(size);
    imageToHistogramFilter->Update();
    ImageToHistogramFilterType::HistogramType* histogram = imageToHistogramFilter->GetOutput();

    std::cout << "Frequency = ";
    for(unsigned int i = 0; i < histogram->GetSize()[0]; ++i)
    {
        std::cout << histogram->GetFrequency(i) << ": " <<   histogram->GetBinMin(0,i) << " ";
    }
    std::cout << std::endl;

    unsigned int reject_index = 0;
    for(unsigned int i = 0; i < histogram->GetSize()[0]; ++i)
    {
        if  (histogram->GetBinMax(0,i) > min_val)
        {
            reject_index = i;
            break;
        }
    }

    unsigned int max_freq = 0;
    unsigned int max_freq_index = 0;
    for(unsigned int i = reject_index; i < histogram->GetSize()[0]; ++i)
    {
        if  (histogram->GetFrequency(i) > max_freq)
        {
            max_freq = histogram->GetFrequency(i);
            max_freq_index = i;
        }
    }

    unsigned int thresh_index = max_freq;
    for(unsigned int i = max_freq_index + 1; i < histogram->GetSize()[0]; ++i)
    {
        float ratio = (float) histogram->GetFrequency(i) / (float) max_freq;
        if  (ratio < ratio_thresh)
        {
            thresh_index = i;
            break;
        }
    }

    typedef itk::BinaryThresholdImageFilter <Image3DType, SegImageType>
            BinaryThresholdImageFilterType;

    BinaryThresholdImageFilterType::Pointer thresholdFilter
            = BinaryThresholdImageFilterType::New();
    thresholdFilter->SetInput(in_img);
    thresholdFilter->SetLowerThreshold(histogram->GetBinMax(0,thresh_index));
    thresholdFilter->SetUpperThreshold(histogram->GetBinMax(0,histogram->GetSize()[0]-1));
    thresholdFilter->SetInsideValue(254);
    thresholdFilter->SetOutsideValue(0);

    typedef itk::ImageFileWriter< SegImageType > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName( outputImageName );
    writer->SetInput( thresholdFilter->GetOutput() );
    try
    {
        writer->Update();
    }
    catch( itk::ExceptionObject & error )
    {
        std::cerr << "Error: " << error << std::endl;
        return EXIT_FAILURE;
    }


    std::cout << "Cutting index is: " << thresh_index << " with frequency " << histogram->GetFrequency(thresh_index) << std::endl;

    return EXIT_SUCCESS;
}
