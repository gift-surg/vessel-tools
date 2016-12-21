/**
  * cardiovasc_pile.cpp
  * This processes files from Leuven and piles them using info
  * from a configuration file. The code is very target specific so,
  * it might need some work before it can be used for a more generic
  * situation.
  * @author M.A. Zuluaga
  */
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkJoinSeriesImageFilter.h>
#include <itkChangeInformationImageFilter.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

const std::string sep_tag = "=";
const std::string start_tag = "FirstFile";
const std::string end_tag = "LastFile";

void Usage(char *exec)
{
    std::cout << " " << std::endl;
    std::cout << "Piles jpg images into an mhd file" << std::endl;
    std::cout << " " << std::endl;
    std::cout << " " << exec << " [-i inputConfigFile -o outputimage -d directory with jpg's <options>  ]" << std::endl;
    std::cout << "**********************************************************" <<std::endl;
    std::cout << "Options:" <<std::endl;
    std::cout << "-b <uint> \t Number of blocks (default 5)" << std::endl;
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


int main(int argc, char *argv[] )
{

    std::string inputFolderName;
    std::string inputFileName;
    std::string outputFileName;
    std::string ext = ".mhd";
    std::string pattern= "_";
    unsigned int num_blocks = 0;
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
        else if(strcmp(argv[i], "-b") == 0)
        {
            num_blocks=(unsigned int) atoi(argv[++i]);
            std::cout << "Set --b=" << num_blocks << std::endl;
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
    typedef itk::Image<unsigned char,3> OutImageType;
    typedef itk::ImageFileReader<ImageType> ImageReaderType;
    typedef itk::ImageFileWriter<OutImageType> WriterType;
    typedef itk::JoinSeriesImageFilter<ImageType, OutImageType>
            JoinSeriesFilterType;
    typedef itk::ChangeInformationImageFilter< OutImageType > FilterType;

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
    unsigned int num_slices = end_slice - start_slice + 1;
    unsigned int size_block = num_slices / num_blocks;


    unsigned int slice_counter = 0;
    unsigned int block_counter = 0;
    ImageType::Pointer img;
    for (unsigned int i = 0; i < num_slices; ++i)
    {
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
            typedef itk::ImageFileWriter< OutImageType > WriterType;
            WriterType::Pointer writer = WriterType::New();
            writer->SetFileName( final_out );

            if (block_counter != 0 || change_spacing)
            {
                translation[2] = block_counter * slice_counter * scalingFactor;
                OutImageType::PointType origin = joinSeries->GetOutput()->GetOrigin();
                origin += translation;
                changeInfo->SetOutputOrigin( origin );
                changeInfo->ChangeOriginOn();
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
            block_counter++;
            slice_counter = 0;
            joinSeries = JoinSeriesFilterType::New();
        }
        else if (block_counter == num_blocks - 1 && i == num_slices -1)
        {
            joinSeries->Update();
            std::string final_out = outputFileName + padnumber(block_counter) + ext;
            typedef itk::ImageFileWriter< OutImageType > WriterType;
            WriterType::Pointer writer = WriterType::New();
            writer->SetFileName( final_out );

            if (block_counter != 0 || change_spacing)
            {
                translation[2] = block_counter * i * scalingFactor;
                OutImageType::PointType origin = joinSeries->GetOutput()->GetOrigin();
                origin += translation;
                changeInfo->SetOutputOrigin( origin );
                changeInfo->ChangeOriginOn();
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
        }
        else
        {
            slice_counter++;
        }
    }




    return EXIT_SUCCESS;



}
