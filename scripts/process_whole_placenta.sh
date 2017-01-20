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
program="/home/mzuluaga/bin/Fiji.app/ImageJ-linux64 --ij2 --run /home/mzuluaga/Code/source/roz_tools/ImageJ/SkeletonScript.bsh" #ImageJ + options + script
for p in ${parts}
do
    input=${data}/part${p}_segmented.mhd            #Binarised vessels
    out_skeleton=${data}/part${p}_centerline.mhd    #Resulting centerline
    general_stats_file=${data}/part${p}_stats_one.xls   #Global statistics (see http://imagej.net/AnalyzeSkeleton#Table_of_results)
    detailed_stats_file=${data}/part${p}_stats_two.xls  #Detailed statistics (see http://imagej.net/AnalyzeSkeleton#Table_of_results)
    eval ${program} "'input_file=\""${input}"\", output_file=\""${out_skeleton}"\", output_statsOne=\""${general_stats_file}"\", output_statsTwo=\""${detailed_stats_file}"\"'"
done

#3- Thickness estimation
#Important: This ImageJ plugin is strongly connected to the GUI. It fails if ran in headless mode (gets a Java exception, headless exception)
program="/home/mzuluaga/bin/Fiji.app/ImageJ-linux64 --ij2 --run /home/mzuluaga/Code/source/roz_tools/ImageJ/ThicknessScript.bsh"
script="'/home/mzuluaga/Code/source/roz_tools/ImageJ/ThicknessScript.bsh'"
threshold=254 #This parameter could be also be given as an input
for p in ${parts}
do
    input=${data}/part${p}_segmented.mhd
    thick_img=${data}/part${p}_thickvolume.mhd
    eval ${program} "'input_file=\""${input}"\", threshold="${threshold}, "output_file=\""${thick_img}"\"'"

done

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

#5- Run statistics
#Computes some basic statistics over the thickness image and displays them
#this should be useful to understand up to which level of thickness in the vessels you wanna
#keep 
program="/home/mzuluaga/bin/roz_tools/bin/compute_statistics"
change_type="/home/mzuluaga/bin/roz_tools/bin/cardiovasc_changetype"
for p in ${parts}
do
    input=${data}/part${p}_segmented.mhd
    thick_img=${data}/part${p}_thickvolume.mhd
    mask=${data}/part${p}_tmp.mhd
  #  cardiovasc_utils -i ${input} --ith 254 255 -o ${mask}
   # ${change_type} -i ${mask} -o ${mask}

    #${program} -l ${mask} -i ${thick_img}
    #rm -rf ${mask}
    #rm -rf ${data}/part${p}_tmp.raw
done


echo "***************************************** statistics are done ***************************************************"
echo "By now you should know what you wanna keep or not but, still the rest of the script will ran with some defaults"
echo "Re run with your specific values (might be differnt for each subvolume!)"

#6- Prune out smaller structures
for p in ${parts}
do
    input=${data}/part${p}_segmented.mhd
    mask=${data}/final_${p}.mhd    
    thick_img=${data}/part${p}_thickvolume.mhd	
    thick_bin=${data}/part${p}_thickbin.mhd	
    cardiovasc_utils -i ${thick_img} --ith 4 100 -o ${thick_bin} 
    cardiovasc_utils -i ${input} --mul ${thick_bin} -o ${mask}	
    ${change_type} -i ${mask} -o ${mask}	
    rm -rf ${data}/part${p}_thickbin.*
    
done

#7- File merging - Note: Temporary file is removed manually at the moment
sp=$1    #This is the spacing of the image that can be overwritten as many ImageJ tools tend to put everything to 1.0
merge_program="/home/mzuluaga/bin/roz_tools/bin/append_volumes"
parts="0 2 3 4 5 6 7 8 9"
for p in ${parts}
do
    result=${data}/result_segmented_prune.mhd
    if [ ${p} == "0" ]
    then
        let p_next=${p}+1
        img_one=${data}/final_${p}.mhd
        img_two=${data}/final_${p_next}.mhd
    else
        let p_mod=${p}%2
        img_two=${data}/final_${p}.mhd
        if [ ${p_mod} -eq 0 ]
        then
            img_one=${data}/result_segmented_prune.mhd
            result=${data}/result_segmented2.mhd
        else
            img_one=${data}/result_segmented2.mhd
        fi
    fi
    ${merge_program} -i1 ${img_one} -i2 ${img_two} -o ${result} -v $sp
done

rm -rf ${data}/result_segmented2.*

