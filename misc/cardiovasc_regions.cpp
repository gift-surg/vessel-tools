/**
  * cardiovasc_regions.cpp
  * This processes files from Leuven (lungs) and it can create
  * overlapping subvolumes. It is targeted specificaly to the
  * images provided by Leuven so, it might need some work before
  * it can be used in other images (e.g not Leuven lungs).
  * @author M.A. Zuluaga
  */
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkJoinSeriesImageFilter.h>
#include <itkChangeInformationImageFilter.h>
#include <itkExtractImageFilter.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

const std::string sep_tag = "=";
const std::string start_tag = "FirstFile";
const std::string end_tag = "LastFile";

//typedef's required for functions
typedef itk::Image<unsigned char,3> OutImageType;
typedef itk::ImageFileWriter< OutImageType > WriterType;
typedef itk::ChangeInformationImageFilter< OutImageType > FilterType;

void Usage(char *exec)
{
    std::cout << " " << std::endl;
    std::cout << "Piles jpg images into an mhd file" << std::endl;
    std::cout << " " << std::endl;
    std::cout << " " << exec << " [-i inputConfigFile -o outputimage -d directory with jpg's <options>  ]" << std::endl;
    std::cout << "**********************************************************" <<std::endl;
    std::cout << "Options:" <<std::endl;
    std::cout << "-bz <uint> \t Number of blocks in z (default 5)" << std::endl;
    std::cout << "-by <uint> \t Number of slices per block in y (default 5)" << std::endl;
    std::cout << "-bx <uint> \t Number of slices per block in x (default 5)" << std::endl;
    std::cout << "-m <uint> \t Overlap margin in all directions (default 0)" << std::endl;
    std::cout << "-p <str> \t File pattern on jpg files" << std::endl;
    std::cout << "-v <float> \t Isotropic voxel spacing (default 1.0)" << std::endl;
    std::cout << "-e <str> \t Image extension (default .mhd)" << std::endl;
}

/**
 * @brief get_number_from_text Parses string from config file and
 *        converts into a number. Used to obtain number of files
 * @param s String to parse
 * @return a number
 */
int get_number_from_text(std::string s, std::string pattern)
{
    int num;
    size_t text_before;

    size_t text_jpg = s.find(".jpg");

    if (pattern == "_")
    {
        text_before = s.find_last_of(pattern) +1;
        std::cout << text_before << std::endl;
    }
    else
    {
        text_before = s.find(pattern) + pattern.length();
        std::cout << text_before << std::endl;
    }

    std::string str_number =s.substr(text_before,text_jpg-text_before);
    std::cout  << str_number << std::endl;
    std::istringstream(str_number) >> num;
    std::cout << num << std::endl;
    return num;
}

/**
 * @brief get_filename_from_text Parses a string to obtain the basename of
 *        the jpg files
 * @param s String to parse
 * @return the basename
 */
std::string get_filename_from_text(std::string s, std::string pattern)
{
    size_t text_after;
    if (pattern == "_")
        text_after = s.find_last_of("_");
    else
        text_after = s.find(pattern);
    size_t text_before = s.find_last_of("\\") +1;
    std::string str_file =s.substr(text_before,text_after-text_before +1);
    return str_file;
}

/**
 * @brief padnumber Converts an int into a 4-digit string. Pads it with zero's
 *        if required
 * @param i int to convert
 * @return String with a 4-digit number
 */
std::string padnumber(unsigned int i)
{
    std::stringstream ss;
    ss << i;
    std::string str_i = ss.str();
    if (i < 10)
    {
        str_i = "000" + str_i;
    }
    else if (i < 100)
    {
        str_i = "00" + str_i;
    }
    else if (i < 1000)
    {
        str_i = "0" + str_i;
    }

    return str_i;
}

/**
 * @brief parse_params Processes configuration file and puts it into a map
 * @param param_filename Filename to parse
 * @return map with all the config file parameters
 */
std::map<std::string,std::string> parse_params(std::string param_filename)
{
    std::ifstream param_file;
    std::map<std::string,std::string> params;

    param_file.open(param_filename.c_str());

    if (param_file.is_open())
    {
        std::string line;
        getline(param_file,line);

        while ( getline(param_file,line) )
        {
            size_t separator = line.find(sep_tag);
            std::string part_one = line.substr(0,separator);
            std::string part_two = line.substr(separator+1);
            params.insert(make_pair(part_one,part_two));
        }
        param_file.close();
    }

    return params;

}

int crop_block(OutImageType::Pointer image, unsigned int x_dim, unsigned int y_dim,
                unsigned int z_dim, unsigned int margin, std::string base, std::string ext)
{
     typedef itk::ExtractImageFilter< OutImageType, OutImageType > ExtractFilterType;

    OutImageType::IndexType desiredStart;
    desiredStart.Fill(0);

    OutImageType::SizeType desiredSize;
    desiredSize[0] = x_dim;
    desiredSize[1] = y_dim;
    desiredSize[2] = z_dim;

    OutImageType::SizeType realSize = image->GetLargestPossibleRegion().GetSize();

    unsigned int num_blocks_x = ceil(realSize[0] )/ (x_dim);
    unsigned int num_blocks_y = ceil(realSize[1] )/ ( y_dim);
    std::cout << num_blocks_x << std::endl;
     std::cout << num_blocks_y << std::endl;

    unsigned int x = 0;
    unsigned int block_counter = 0;
    while (x < num_blocks_x)
    {
        unsigned int y = 0;
         std::cout <<"X: " << x << " " << desiredStart[0] << " " << desiredSize[0] << " " << realSize[0] << std::endl;
        while (y < num_blocks_y)
        {
            OutImageType::RegionType desiredRegion(desiredStart, desiredSize);
            ExtractFilterType::Pointer filter = ExtractFilterType::New();
            filter->SetExtractionRegion(desiredRegion);
            filter->SetInput(image);
         //
             std::cout << y << " " << desiredStart[1] << " " << desiredSize[1] << " " << realSize[1] << std::endl;
#if ITK_VERSION_MAJOR >= 4
            filter->SetDirectionCollapseToIdentity(); // This is required.
#endif
            filter->Update();

            std::string final_out = base + "_" + padnumber(block_counter) + ext;

            WriterType::Pointer writer = WriterType::New();
            writer->SetFileName( final_out );
            writer->SetInput( filter->GetOutput() );

            try
            {
                writer->Update();
            }
            catch( itk::ExceptionObject & error )
            {
                std::cerr << "Error: " << error << std::endl;
                return EXIT_FAILURE;
            }
            y++;

            unsigned int tmp_start = desiredSize[1] + (desiredSize[1] - margin);
            if (tmp_start + desiredSize[1] > realSize[1] && desiredSize[1] == y_dim)
            {
                std::cout << "The problem: " << (desiredStart[1] + desiredSize[1]) << " " <<
                                        realSize[1]<<    " " << desiredStart[1] <<      std::endl;
                desiredSize[1] =  realSize[1] - desiredStart[1] - 1;
                // desiredStart[1] += (desiredSize[1] -margin);
                 std::cout << "fixed? " << y << " " << desiredStart[1] << " " << desiredSize[1] << " " << realSize[1] << std::endl;
            }
            else if (desiredSize[1] < y_dim)
            {
                y = num_blocks_y;
            }

            desiredStart[1] += (desiredSize[1] - margin);
            block_counter++;

        }
        desiredStart[1] = 0;
        desiredSize[1] = y_dim;
        x++;
        unsigned int tmp_start = desiredSize[0] + (desiredSize[0] - margin);

        if (tmp_start + desiredSize[0] > realSize[0] && desiredSize[0] == x_dim)
        {
            std::cout << "The problem: " << (desiredStart[0] + desiredSize[0]) << " " <<
                                    realSize[0]<<    " " << desiredStart[0] <<      std::endl;
            desiredSize[0] =  realSize[0] - desiredStart[0] - 1;
        }
//        else if (desiredSize[0] < x_dim)
//        {
//            x = num_blocks_x;
//        }
        //block_counter++;
        desiredStart[0] += (desiredSize[0] - margin);
    }
    return 0;
}


int main(int argc, char *argv[] )
{

    std::string inputFolderName;
    std::string inputFileName;
    std::string outputFileName;
    std::string ext = ".mhd";
    std::string pattern= "_";
    unsigned int num_blocks = 5;
    unsigned int x_dim = 5;
    unsigned int y_dim = 5;
    unsigned int margin = 0;
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
        else if(strcmp(argv[i], "-bz") == 0)
        {
            num_blocks=(unsigned int) atoi(argv[++i]);
            std::cout << "Set --bz=" << num_blocks << std::endl;
        }
        else if(strcmp(argv[i], "-bx") == 0)
        {
            x_dim=(unsigned int) atoi(argv[++i]);
            std::cout << "Set --bx=" << x_dim << std::endl;
        }
        else if(strcmp(argv[i], "-by") == 0)
        {
            y_dim=(unsigned int) atoi(argv[++i]);
            std::cout << "Set --by=" << y_dim << std::endl;
        }
        else if(strcmp(argv[i], "-m") == 0)
        {
            margin=(unsigned int) atoi(argv[++i]);
            std::cout << "Set --m=" << margin << std::endl;
        }
        else if(strcmp(argv[i], "-p") == 0)
        {
            pattern=argv[++i];
            std::cout << "Set --p=" << pattern << std::endl;
        }
        else if(strcmp(argv[i], "-v") == 0)
        {
            scalingFactor=atof(argv[++i]);
            change_spacing = true;
            std::cout << "Set --v=" << scalingFactor << std::endl;
        }
        else if(strcmp(argv[i], "-e") == 0)
        {
            ext=argv[++i];
            std::cout << "Set --e=" << ext << std::endl;
        }
    }

    // Validate command line args
    if (inputFileName.length() == 0 || outputFileName.length() == 0
            || inputFolderName.length() == 0)
    {
        std::cout << "Missing required files" << std::endl;
        Usage(argv[0]);
        return EXIT_FAILURE;
    }

    std::map<std::string,std::string> params = parse_params(inputFileName);

    //Get slice numbers
    int start_slice = -1, end_slice = -1;
    if (params.find(start_tag)!= params.end())
        start_slice = get_number_from_text(params[start_tag],pattern);

    if (params.find(end_tag)!= params.end())
        end_slice = get_number_from_text(params[end_tag],pattern);

    if (start_slice == -1 || end_slice == -1)
    {
        std::cout << "Error in config file: No slice information. Program will exit."
                  << std::endl;
        return EXIT_FAILURE;
    }

    //get base-filename
    std::string basename = "";
    if (params.find(start_tag)!= params.end())
    {
        if (pattern == "_")
            basename = get_filename_from_text(params[start_tag], pattern);
        else
            basename = pattern;
        std::cout << basename << std::endl;
    }
    else
    {
        std::cout << "Error in config file: No filename information. Program will exit."
                  << std::endl;
        return EXIT_FAILURE;
    }

    //Basic template definition
    typedef itk::Image<unsigned char,2> ImageType;
    typedef itk::ImageFileReader<ImageType> ImageReaderType;
    typedef itk::JoinSeriesImageFilter<ImageType, OutImageType>
            JoinSeriesFilterType;

    ImageReaderType::Pointer reader = ImageReaderType::New();
    JoinSeriesFilterType::Pointer joinSeries = JoinSeriesFilterType::New();
    FilterType::Pointer changeInfo = FilterType::New();

    //Check if spacing info needs to be updated
    if (change_spacing)
    {
        OutImageType::SpacingType spacing = ( scalingFactor );
        changeInfo->SetOutputSpacing( spacing );
        changeInfo->ChangeSpacingOn();
    }

    OutImageType::PointType::VectorType translation;
    translation[0] = 0;
    translation[1] = 0;
    //Compute number of blocks in z
    unsigned int num_slices = end_slice - start_slice + 1;
    unsigned int size_block = num_slices / num_blocks;

    //Compute sizes of y an x
    std::string one_file = inputFolderName + "/" + basename +
            padnumber(0) + ".jpg";
    reader->SetFileName(one_file);
    reader->Update();
//    ImageType::SizeType xy_size = (reader->GetOutput())->
//            GetLargestPossibleRegion().GetSize(); //gets z too but I don't care


    //Pad dimensions if margin is required
    if (margin != 0)
    {
        size_block += 2*margin;
        y_dim += 2*margin;
        x_dim += 2* margin;
    }


    unsigned int slice_counter = 0;
    unsigned int block_counter = 0;
    ImageType::Pointer img;
    unsigned int i = 0;
    while (i < num_slices)
    {
        //Overlap is desired so the blocks need to be overlapping
        std::string final_file = inputFolderName + "/" + basename +
                padnumber(i) + ".jpg";
        std::cout << final_file << std::endl;

        reader->SetFileName(final_file);
        reader->Update();
        img = reader->GetOutput();
        img->DisconnectPipeline();
        joinSeries->SetInput( slice_counter, img );

        if (slice_counter == size_block && block_counter < num_blocks - 1)
        {
            joinSeries->Update();
            std::string final_out = outputFileName + padnumber(block_counter) + ext;
            if (block_counter != 0 || change_spacing)
            {
                translation[2] = block_counter * slice_counter * scalingFactor;
                OutImageType::PointType origin = joinSeries->GetOutput()->GetOrigin();
                origin += translation;
                changeInfo->SetOutputOrigin( origin );
                changeInfo->ChangeOriginOn();
                changeInfo->SetInput( joinSeries->GetOutput() );
                changeInfo->UpdateOutputInformation();
                if (margin != 0)
                {
                    crop_block(changeInfo->GetOutput(), x_dim, y_dim, slice_counter,
                               margin, outputFileName + padnumber(block_counter) , ext);
                }
                else
                {
                    WriterType::Pointer writer = WriterType::New();
                    writer->SetFileName( final_out );

                    writer->SetInput( changeInfo->GetOutput() );
                    try
                    {
                        writer->Update();

                    }
                    catch( itk::ExceptionObject & error )
                    {
                        std::cerr << "Error: " << error << std::endl;
                        return EXIT_FAILURE;
                    }
                }
            }
            else
            {
                if (margin != 0)
                {
                    crop_block(joinSeries->GetOutput(), x_dim, y_dim, slice_counter,
                               margin, outputFileName + padnumber(block_counter) , ext);
                }
                else
                {
                    WriterType::Pointer writer = WriterType::New();
                                        writer->SetFileName( final_out );
                    writer->SetInput( joinSeries->GetOutput() );
                    try
                    {
                        writer->Update();

                    }
                    catch( itk::ExceptionObject & error )
                    {
                        std::cerr << "Error: " << error << std::endl;
                        return EXIT_FAILURE;
                    }
                }
            }



            block_counter++;
            slice_counter = 0;
            joinSeries = JoinSeriesFilterType::New();

            if (margin != 0)
            {
                i = i - margin + 1;
            }
        }
        else if (block_counter == num_blocks - 1 && i == num_slices -1)
        {
            joinSeries->Update();
            std::string final_out = outputFileName + padnumber(block_counter) + ext;


            if (block_counter != 0 || change_spacing)
            {
                translation[2] = block_counter * i * scalingFactor;
                OutImageType::PointType origin = joinSeries->GetOutput()->GetOrigin();
                origin += translation;
                changeInfo->SetOutputOrigin( origin );
                changeInfo->ChangeOriginOn();
                changeInfo->SetInput( joinSeries->GetOutput() );
                changeInfo->UpdateOutputInformation();
                if (margin != 0)
                {
                    crop_block(changeInfo->GetOutput(), x_dim, y_dim, slice_counter,
                               margin, outputFileName + padnumber(block_counter) , ext);
                }
                else
                {
                    WriterType::Pointer writer = WriterType::New();
                    writer->SetFileName( final_out );
                    writer->SetInput( changeInfo->GetOutput() );
                    try
                    {
                        writer->Update();
                    }
                    catch( itk::ExceptionObject & error )
                    {
                        std::cerr << "Error: " << error << std::endl;
                        return EXIT_FAILURE;
                    }
                }
            }
            else
            {
                if (margin != 0)
                {
                    crop_block(joinSeries->GetOutput(), x_dim, y_dim, slice_counter,
                               margin, outputFileName + padnumber(block_counter) , ext);
                }
                else
                {
                    WriterType::Pointer writer = WriterType::New();
                    writer->SetFileName( final_out );
                    writer->SetInput( joinSeries->GetOutput() );
                    try
                    {
                        writer->Update();
                    }
                    catch( itk::ExceptionObject & error )
                    {
                        std::cerr << "Error: " << error << std::endl;
                        return EXIT_FAILURE;
                    }
                }
            }


        }
        else
        {
            slice_counter++;
            i++;
        }

    }




    return EXIT_SUCCESS;



}

