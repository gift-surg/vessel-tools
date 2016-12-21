#!/bin/bash
#================================================================================
# Example script for processing a whole raw placenta volume provided that
# the volume has already been divided into parts.
# pre: the volume has been splitted into parts
# author: M.A. Zuluaga
#================================================================================
#Example on how to call this: ./process_whole_placenta.sh 0.088767678
#1- Setting up folders
data="/home/mzuluaga/data/placenta"
parts="0 1 2 3 4 5 6 7 8 9"

#2- Segmentation of each part using segmentation with histogram
program="/home/mzuluaga/bin/roz_tools/bin/seg_withhisto"
for p in ${parts}
do
    input=${data}/part${p}.mhd
    mask=${data}/part${p}_mask.mhd
    cardiovasc_utils -i ${input} --otsu --inv --lconcom -o ${mask}
    ${program}  -i  ${input} -o ${data}/part${p}_segmented.mhd -m  ${mask}
done

#2- Extraction of the centerline (per subvolume)
#Important: This ImageJ plugin is strongly connected to the GUI. It fails if ran in headless mode (gets a Java exception, headless exception)
#WARNING! Currently, at least for linux, ImageJ fails to be launched from a script so, this bit of script just prints out
#the commands that should be executed. Once printed in the terminal, they should be copied and pasted in the command line
#for execution.  
program="/home/mzuluaga/bin/Fiji.app/ImageJ-linux64 --ij2 --run /home/mzuluaga/Code/source/roz_tools/ImageJ/SkeletonScript.bsh" #ImageJ + options + script
for p in ${parts}
do
    input=${data}/part${p}_segmented.mhd            #Binarised vessels
    out_skeleton=${data}/part${p}_centerline.mhd    #Resulting centerline
    general_stats_file=${data}/part${p}_stats_one.xls   #Global statistics (see http://imagej.net/AnalyzeSkeleton#Table_of_results)
    detailed_stats_file=${data}/part${p}_stats_two.xls  #Detailed statistics (see http://imagej.net/AnalyzeSkeleton#Table_of_results)
    echo ${program} "'input_file=\""${input}"\", output_file=\""${out_skeleton}"\", output_statsOne=\""${general_stats_file}"\", output_statsTwo=\""${detailed_stats_file}"\"'"
done

#3- Thickness estimation
#Important: This ImageJ plugin is strongly connected to the GUI. It fails if ran in headless mode (gets a Java exception, headless exception)
#WARNING! Currently, at least for linux, ImageJ fails to be launched from a script so, this bit of script just prints out
#the commands that should be executed. Once printed in the terminal, they should be copied and pasted in the command line
#for execution.  
program="/home/mzuluaga/bin/Fiji.app/ImageJ-linux64 --ij2 --run /home/mzuluaga/Code/source/roz_tools/ImageJ/ThicknessScript.bsh"
script="'/home/mzuluaga/Code/source/roz_tools/ImageJ/ThicknessScript.bsh'"
threshold=254 #This parameter could be also be given as an input
for p in ${parts}
do
    input=${data}/part${p}_segmented.mhd
    thick_img=${data}/part${p}_thickvolume.mhd
    echo ${program} "'input_file=\""${input}"\", threshold="${threshold}, "output_file=\""${thick_img}"\"'"

done
echo "****************************** ImageJ commands finish here ****************************************"
echo "This script can not go further from here if you don't copy-paste those commands in the command line. So, I am exiting here!"
echo "Run process_whole_placenta_p2 part 2 for that matter"



#4- File merging - Note: Temporary file is removed manually at the moment
sp=$1    #This is the spacing of the image that can be overwritten as many ImageJ tools tend to put everything to 1.0
merge_program="/home/mzuluaga/bin/roz_tools/bin/append_volumes"
parts="0 2 3 4 5 6 7 8 9"
for p in ${parts}
do
    result=${data}/result_segmented.mhd
    if [ ${p} == "0" ]
    then
        let p_next=${p}+1
        img_one=${data}/part${p}_segmented.mhd
        img_two=${data}/part${p_next}_segmented.mhd
    else
        let p_mod=${p}%2
        img_two=${data}/part${p}_segmented.mhd
        if [ ${p_mod} -eq 0 ]
        then
            img_one=${data}/result_segmented.mhd
            result=${data}/result_segmented2.mhd
        else
            img_one=${data}/result_segmented2.mhd
        fi
    fi
    ${merge_program} -i1 ${img_one} -i2 ${img_two} -o ${result} -v $sp
done

rm -rf ${data}/result_segmented2.*
echo "Fine, I am exiting right after merging the original files. For refining based on thickness you really need to do ImageJ"
echo "Afterwards process_whole_placenta_p2"
exit 


