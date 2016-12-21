/**
  * convert_tiff.cpp
  * Reads a single tiff or a set of files and converts them to
  * a specified format.
  * Most of this code is taken from the ReadUknownImageType
  * example from the ITK documentation.
  * @author M.A. Zuluga
  */
#include <itkImage.h>
#include <itkImageFileWriter.h>
#include <itkImageFileReader.h>
#include <itkTIFFImageIO.h>
#include <itkImageIOBase.h>
#include <itkCastImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>

void Usage(char *exec)
{
    std::cout << " " << std::endl;
    std::cout << "Converts tiff files into a different format" << std::endl;
    std::cout << " " << std::endl;
    std::cout << " " << exec << " [-i input file -d input directory (for several files) <options>  ]" << std::endl;
    std::cout << "**********************************************************" <<std::endl;
    std::cout << "Options:" <<std::endl;
    std::cout << "-o <file> \t Outputfile name" << std::endl;
    std::cout << "-e <str> \t Output extension (ignored if -o is provided with extension)" << std::endl;
    std::cout << "--3d \t Convert images to 3D. Ignored if no -d option" << std::endl;
}


template< class TImage >
int ReadImage( const char* fileName,
               typename TImage::Pointer image )
{
    typedef TImage                            ImageType;
    typedef itk::ImageFileReader< ImageType > ImageReaderType;

    typename ImageReaderType::Pointer reader = ImageReaderType::New();
    reader->SetFileName( fileName );

    try
    {
        reader->Update();
    }
    catch( itk::ExceptionObject& e )
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    image->Graft( reader->GetOutput() );

    return EXIT_SUCCESS;
}

template< class TImage >
int WriteImage( const char* fileName,
               typename TImage::Pointer image )
{
    typedef TImage                            ImageType;
    typedef itk::ImageFileWriter< ImageType > ImageWriterType;

    typename ImageWriterType::Pointer writer = ImageWriterType::New();
    writer->SetFileName( fileName );
    writer->SetInput( image );

    try
    {
        writer->Update();
    }
    catch( itk::ExceptionObject& e )
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    //himage->Graft( reader->GetOutput() );

    return EXIT_SUCCESS;
}

/**
 * @brief Reads an image based on the component image type provided
 * @param inputFileName - Image to process
 * @param componentType - Type of the voxel/pixel
 * @return success/failure flag
 */
template< unsigned int VDimension >
int ReadScalarImage( const char* inputFileName, const char* outputFileName,
                     const itk::ImageIOBase::IOComponentType componentType,
                     bool isJPG)
{
    typedef itk::Image< unsigned char , VDimension > JPGImageType;

    switch( componentType )
    {
        default:
        case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
            std::cerr << "Unknown and unsupported component type!" << std::endl;
            return EXIT_FAILURE;

        case itk::ImageIOBase::UCHAR:
        {
            typedef unsigned char PixelType;
            typedef itk::Image< PixelType, VDimension > ImageType;

            typename ImageType::Pointer image = ImageType::New();

            if( ReadImage< ImageType >( inputFileName, image ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }
            if( WriteImage< ImageType >( outputFileName, image ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }

//            std::cout << image << std::endl;
            break;
        }
        case itk::ImageIOBase::CHAR:
        {
            typedef char PixelType;
            typedef itk::Image< PixelType, VDimension > ImageType;

            typename ImageType::Pointer image = ImageType::New();

            if( ReadImage< ImageType >( inputFileName, image ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }
            if (isJPG)
            {
                typedef itk::RescaleIntensityImageFilter<ImageType> RescaleFilterType;
                typedef itk::CastImageFilter<ImageType, JPGImageType> CastFilterType;
                typename CastFilterType::Pointer castFilter = CastFilterType::New();
                typename RescaleFilterType::Pointer rescalefilter = RescaleFilterType::New();
                rescalefilter->SetInput( image );
                rescalefilter->SetOutputMinimum( 0 );
                rescalefilter->SetOutputMaximum( 255 );
                castFilter->SetInput( rescalefilter->GetOutput() );
                castFilter->Update();
                if( WriteImage< JPGImageType >( outputFileName, castFilter->GetOutput()  ) == EXIT_FAILURE )
                {
                    return EXIT_FAILURE;
                }
            }
            else if( WriteImage< ImageType >( outputFileName, image  ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }

//            std::cout << image << std::endl;
            break;
        }
        case itk::ImageIOBase::USHORT:
        {
            typedef unsigned short PixelType;
            typedef itk::Image< PixelType, VDimension > ImageType;

            typename ImageType::Pointer image = ImageType::New();

            if( ReadImage< ImageType >( inputFileName, image ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }
            if (isJPG)
            {
                typedef itk::CastImageFilter<ImageType, JPGImageType> CastFilterType;
                typename CastFilterType::Pointer castFilter = CastFilterType::New();
                castFilter->SetInput( image );
                castFilter->Update();
                if( WriteImage< JPGImageType >( outputFileName, castFilter->GetOutput()  ) == EXIT_FAILURE )
                {
                    return EXIT_FAILURE;
                }
            }
            else if( WriteImage< ImageType >( outputFileName, image  ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }

//            std::cout << image << std::endl;
            break;
        }
        case itk::ImageIOBase::SHORT:
        {
            typedef short PixelType;
            typedef itk::Image< PixelType, VDimension > ImageType;

            typename ImageType::Pointer image = ImageType::New();

            if( ReadImage< ImageType >( inputFileName, image ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }
            if (isJPG)
            {
                typedef itk::RescaleIntensityImageFilter<ImageType> RescaleFilterType;
                typedef itk::CastImageFilter<ImageType, JPGImageType> CastFilterType;
                typename CastFilterType::Pointer castFilter = CastFilterType::New();
                typename RescaleFilterType::Pointer rescalefilter = RescaleFilterType::New();
                rescalefilter->SetInput( image );
                rescalefilter->SetOutputMinimum( 0 );
                rescalefilter->SetOutputMaximum( 255 );
                castFilter->SetInput( rescalefilter->GetOutput() );
                castFilter->Update();
                if( WriteImage< JPGImageType >( outputFileName, castFilter->GetOutput()  ) == EXIT_FAILURE )
                {
                    return EXIT_FAILURE;
                }
            }
            else if( WriteImage< ImageType >( outputFileName, image  ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }

//            std::cout << image << std::endl;
            break;
        }
        case itk::ImageIOBase::UINT:
        {
            typedef unsigned int PixelType;
            typedef itk::Image< PixelType, VDimension > ImageType;

            typename ImageType::Pointer image = ImageType::New();

            if( ReadImage< ImageType >( inputFileName, image ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }
            if (isJPG)
            {
                typedef itk::RescaleIntensityImageFilter<ImageType> RescaleFilterType;
                typedef itk::CastImageFilter<ImageType, JPGImageType> CastFilterType;
                typename CastFilterType::Pointer castFilter = CastFilterType::New();
                typename RescaleFilterType::Pointer rescalefilter = RescaleFilterType::New();
                rescalefilter->SetInput( image );
                rescalefilter->SetOutputMinimum( 0 );
                rescalefilter->SetOutputMaximum( 255 );
                castFilter->SetInput( rescalefilter->GetOutput() );
                castFilter->Update();
                if( WriteImage< JPGImageType >( outputFileName, castFilter->GetOutput()  ) == EXIT_FAILURE )
                {
                    return EXIT_FAILURE;
                }
            }
            else if( WriteImage< ImageType >( outputFileName, image  ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }

//            std::cout << image << std::endl;
            break;
        }
        case itk::ImageIOBase::INT:
        {
            typedef int PixelType;
            typedef itk::Image< PixelType, VDimension > ImageType;

            typename ImageType::Pointer image = ImageType::New();

            if( ReadImage< ImageType >( inputFileName, image ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }
            if (isJPG)
            {
                typedef itk::RescaleIntensityImageFilter<ImageType> RescaleFilterType;
                typedef itk::CastImageFilter<ImageType, JPGImageType> CastFilterType;
                typename CastFilterType::Pointer castFilter = CastFilterType::New();
                typename RescaleFilterType::Pointer rescalefilter = RescaleFilterType::New();
                rescalefilter->SetInput( image );
                rescalefilter->SetOutputMinimum( 0 );
                rescalefilter->SetOutputMaximum( 255 );
                castFilter->SetInput( rescalefilter->GetOutput() );
                castFilter->Update();
                if( WriteImage< JPGImageType >( outputFileName, castFilter->GetOutput()  ) == EXIT_FAILURE )
                {
                    return EXIT_FAILURE;
                }
            }
            else if( WriteImage< ImageType >( outputFileName, image  ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }

//            std::cout << image << std::endl;
            break;
        }
        case itk::ImageIOBase::ULONG:
        {
            typedef unsigned long PixelType;
            typedef itk::Image< PixelType, VDimension > ImageType;

            typename ImageType::Pointer image = ImageType::New();

            if( ReadImage< ImageType >( inputFileName, image ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }
            if (isJPG)
            {
                typedef itk::RescaleIntensityImageFilter<ImageType> RescaleFilterType;
                typedef itk::CastImageFilter<ImageType, JPGImageType> CastFilterType;
                typename CastFilterType::Pointer castFilter = CastFilterType::New();
                typename RescaleFilterType::Pointer rescalefilter = RescaleFilterType::New();
                rescalefilter->SetInput( image );
                rescalefilter->SetOutputMinimum( 0 );
                rescalefilter->SetOutputMaximum( 255 );
                castFilter->SetInput( rescalefilter->GetOutput() );
                castFilter->Update();
                if( WriteImage< JPGImageType >( outputFileName, castFilter->GetOutput()  ) == EXIT_FAILURE )
                {
                    return EXIT_FAILURE;
                }
            }
            else if( WriteImage< ImageType >( outputFileName, image  ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }

//            std::cout << image << std::endl;
            break;
        }
        case itk::ImageIOBase::LONG:
        {
            typedef long PixelType;
            typedef itk::Image< PixelType, VDimension > ImageType;

            typename ImageType::Pointer image = ImageType::New();

            if( ReadImage< ImageType >( inputFileName, image ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }
            if (isJPG)
            {
                typedef itk::RescaleIntensityImageFilter<ImageType> RescaleFilterType;
                typedef itk::CastImageFilter<ImageType, JPGImageType> CastFilterType;
                typename CastFilterType::Pointer castFilter = CastFilterType::New();
                typename RescaleFilterType::Pointer rescalefilter = RescaleFilterType::New();
                rescalefilter->SetInput( image );
                rescalefilter->SetOutputMinimum( 0 );
                rescalefilter->SetOutputMaximum( 255 );
                castFilter->SetInput( rescalefilter->GetOutput() );
                castFilter->Update();
                if( WriteImage< JPGImageType >( outputFileName, castFilter->GetOutput()  ) == EXIT_FAILURE )
                {
                    return EXIT_FAILURE;
                }
            }
            else if( WriteImage< ImageType >( outputFileName, image  ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }
//            std::cout << image << std::endl;
            break;
        }
        case itk::ImageIOBase::FLOAT:
        {
            typedef float PixelType;
            typedef itk::Image< PixelType, VDimension > ImageType;

            typename ImageType::Pointer image = ImageType::New();

            if( ReadImage< ImageType >( inputFileName, image ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }
            if (isJPG)
            {
                typedef itk::RescaleIntensityImageFilter<ImageType> RescaleFilterType;
                typedef itk::CastImageFilter<ImageType, JPGImageType> CastFilterType;
                typename CastFilterType::Pointer castFilter = CastFilterType::New();
                typename RescaleFilterType::Pointer rescalefilter = RescaleFilterType::New();
                rescalefilter->SetInput( image );
                rescalefilter->SetOutputMinimum( 0 );
                rescalefilter->SetOutputMaximum( 255 );
                castFilter->SetInput( rescalefilter->GetOutput() );
                castFilter->Update();
                if( WriteImage< JPGImageType >( outputFileName, castFilter->GetOutput()  ) == EXIT_FAILURE )
                {
                    return EXIT_FAILURE;
                }
            }
            else if( WriteImage< ImageType >( outputFileName, image  ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }

//            std::cout << image << std::endl;
            break;
        }
        case itk::ImageIOBase::DOUBLE:
        {
            typedef double PixelType;
            typedef itk::Image< PixelType, VDimension > ImageType;

            typename ImageType::Pointer image = ImageType::New();

            if( ReadImage< ImageType >( inputFileName, image ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }
            if (isJPG)
            {
                typedef itk::RescaleIntensityImageFilter<ImageType> RescaleFilterType;
                typedef itk::CastImageFilter<ImageType, JPGImageType> CastFilterType;
                typename CastFilterType::Pointer castFilter = CastFilterType::New();
                typename RescaleFilterType::Pointer rescalefilter = RescaleFilterType::New();
                rescalefilter->SetInput( image );
                rescalefilter->SetOutputMinimum( 0 );
                rescalefilter->SetOutputMaximum( 255 );
                castFilter->SetInput( rescalefilter->GetOutput() );
                castFilter->Update();
                if( WriteImage< JPGImageType >( outputFileName, castFilter->GetOutput()  ) == EXIT_FAILURE )
                {
                    return EXIT_FAILURE;
                }
            }
            else if( WriteImage< ImageType >( outputFileName, image  ) == EXIT_FAILURE )
            {
                return EXIT_FAILURE;
            }

//            std::cout << image << std::endl;
            break;
        }
    }
    return EXIT_SUCCESS;
}

int processFile(std::string inputFileName, std::string outputFileName,
                bool isJPG)
{
    itk::ImageIOBase::Pointer imageIO =
            itk::ImageIOFactory::CreateImageIO(
                inputFileName.c_str(),
                itk::ImageIOFactory::ReadMode );

    imageIO->SetFileName( inputFileName );
    imageIO->ReadImageInformation();

    typedef itk::ImageIOBase::IOPixelType     IOPixelType;
    const IOPixelType pixelType = imageIO->GetPixelType();

    std::cout << "Pixel Type is "
              << itk::ImageIOBase::GetPixelTypeAsString( pixelType )
              << std::endl;

    typedef itk::ImageIOBase::IOComponentType IOComponentType;
    const IOComponentType componentType = imageIO->GetComponentType();

    std::cout << "Component Type is "
              << imageIO->GetComponentTypeAsString( componentType )
              << std::endl;

    const unsigned int imageDimension = imageIO->GetNumberOfDimensions();

    std::cout << "Image Dimension is " << imageDimension << std::endl;

    switch( pixelType )
    {
    case itk::ImageIOBase::SCALAR:
    {
        if( imageDimension == 2 )
        {
            return ReadScalarImage< 2 >( inputFileName.c_str(),
                                         outputFileName.c_str(), componentType,
                                         isJPG );
        }
        else if( imageDimension == 3 )
        {
            return ReadScalarImage< 3 >( inputFileName.c_str(),
                                         outputFileName.c_str(), componentType,
                                         isJPG );
        }
        else if( imageDimension == 4 )
        {
            return ReadScalarImage< 4 >( inputFileName.c_str(),
                                         outputFileName.c_str(), componentType,
                                         isJPG );
        }
    }

    default:
        std::cerr << "not implemented yet!" << std::endl;
        return EXIT_FAILURE;
    }
}

int main(int argc, char *argv[] )
{
    std::string inputFileName = "";
    std::string inputFolderName = "";
    std::string outputFileName = "";
    std::string ext = "";
    bool is3D = false;

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
        else if(strcmp(argv[i], "-d") == 0)
        {
            inputFolderName=argv[++i];
            std::cout << "Set --d=" << inputFolderName << std::endl;
        }
        else if(strcmp(argv[i], "-o") == 0)
        {
            outputFileName=argv[++i];
            std::cout << "Set --o=" << outputFileName << std::endl;
        }
        else if(strcmp(argv[i], "-e") == 0)
        {
            ext=argv[++i];
            std::cout << "Set --e=" << ext << std::endl;
        }
        else if(strcmp(argv[i], "--3d") == 0)
        {
            ++i;
            is3D = true;
            std::cout << "Set --is3D=ON" << std::endl;
        }
    }

    //Validate command options
    if (inputFileName.length() == 0 && inputFolderName.length() == 0)
    {
        Usage(argv[0]);
        return EXIT_FAILURE;
    }

    // If by mistake two options are provided, the multiple file one
    //is ignored
    if (inputFileName.length() > 0 && inputFolderName.length() > 0) {
        inputFolderName = "";
    }

    //Validate output options
    bool isJPG = false;
    if (outputFileName.length() > 0){
         std::string::size_type idx = outputFileName.rfind(".");
         if (idx == std::string::npos && ext.length() ==0)
         {
            outputFileName = outputFileName + ".nii.gz";
            std::cout << "Extension of output file has been set to nifti" << std::endl;
         }
         else if (ext.length() > 0)
         {
             outputFileName = outputFileName + "." + ext;
             if (ext == "jpg" || ext == "JPG" || ext == "jpeg" || ext == "JPEG")
                 isJPG = true;
         }
         else if (idx != std::string::npos)
         {
             std::string tmp_ext = outputFileName.substr(idx+1);
             if (tmp_ext == "jpg" || tmp_ext == "JPG" || tmp_ext == "jpeg" || tmp_ext == "JPEG")
                 isJPG = true;
         }
    }

    if (outputFileName.length() == 0 && ext.length() == 0)
    {
        std::cout << "Using input as output filename and nifti as format. 3D OFF" << std::endl;
        if (inputFileName.length() >0){
            std::string::size_type idx = inputFileName.rfind(".");
            if (idx != std::string::npos)
            {
                outputFileName = inputFileName.substr(0,idx+1) + "nii.gz";
            }
            else {
                outputFileName = inputFolderName + ".nii.gz";
            }
        }
    }
    else if (outputFileName.length() == 0)
    {
        std::cout << "Using input as output filename. 3D OFF" << std::endl;
        if (inputFileName.length() >0){
            std::string::size_type idx = inputFileName.rfind(".");
            if (idx != std::string::npos)
            {
                outputFileName = inputFileName.substr(0,idx+1) + ext;
            }
            else {
                outputFileName = inputFolderName + ext;
            }
        }
    }

    if ((outputFileName.length() >0 && outputFileName.rfind(".")
         != std::string::npos) && ext.length() >0) {    //Ignore extension if output has it
        ext = "";
    }

    if (isJPG)
        std::cout << "WARNING: The image quality might be severely affected. DO CHECK THE RESULTS" << std::endl;

    //Which kind of input there?
    if (inputFileName.length() > 0)
    {
        processFile(inputFileName, outputFileName, isJPG); //I know this function is kind of useless but, was conceived when i thought to do the --d option
    }
    else    //If not then is a directory because I validated it
    {
        std::cout << "Directory scheme not yet supported through C++" << std::endl;
    }


    return EXIT_SUCCESS;

}
