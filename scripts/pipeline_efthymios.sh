#!/bin/bash
#================================================================================
# Example script for generating a volume to be printed for Efthymios
# from pre-segmented data obtained from the micro-CT at GOSH.
# author: M.A. Zuluaga
#================================================================================

#1- Input and output folders
#Note to Tom D. - Ask for the original tif data to Efthymios as I have erased it 
#due to space problems
data="/home/mzuluaga/data/placenta/placenta_P16_AMed/P16_AMed_SurfaceDetermination"
out_data="/home/mzuluaga/data/placenta/placenta_out/P16_AMed_SurfaceDetermination"

#2- Input from the command line
#start_val - starting value of the input files
#end_val - end value of the input files
#ext - Extension of the input images (just to make it more generic but could be erased)
#threshold - Threshold to be applied in the thickness estimation
# An example: ./pipeline_efthymios.sh 71 1911 tif 254
# This means the first image follows a pattern starting in 71 and finishing in 1911
# The input images are tif and for thickness estimation a threshold of 254 should be
# used
start_val=$1
end_val=$2
ext=$3
threshold=$4


#3- Converts from RGB tif to mhd
#Note: I tend to use a lot omf mhd/raw data because ImageJ has troubles in some cases
#with the nifti format
program="/home/mzuluaga/bin/roz_tools/bin/extract_channels"

tmp_start=${start_val}
while [ ${start_val} -le ${end_val} ]
do
    img_name=""
    if [ ${start_val} -lt 10 ]
    then
        img_name=${img_name}"000"
    elif [ ${start_val} -lt 100 ]
    then
        img_name=${img_name}"00"
    elif [ ${start_val} -lt 1000 ]
    then
        img_name=${img_name}"0"
    fi
    in_img=${data}${img_name}${start_val}"."${ext}
    out_img=${out_data}${img_name}${start_val}".mhd"

    ${program} -i ${in_img} -o ${out_img} -c 1
    let start_val=${start_val}+1
done

#4- Concatenates a number of slices creating sub-volumes (could be also the full volume)
program="/home/mzuluaga/bin/roz_tools/bin/convert_slices_to_vol"
start_val=${tmp_start}
let num_slices=(${end_val}-${start_val})/10	#Estimation of the size of each subvol
img_name=${out_data}%04d.mhd			#Pattern of the input slices
out_img="/home/mzuluaga/data/placenta/placenta_out/full_volume"	#Name of the output image
count=0
while [ ${start_val} -le ${end_val} ]
do
    let tmp_end=${start_val}+${num_slices}
    if [ ${tmp_end} -gt ${end_val} ]
    then
        tmp_end=${end_val}
    fi

    ${program} -p ${img_name} -s ${start_val} -e ${tmp_end} -o ${out_img}_${count}.mhd
    let start_val=${tmp_end}+1
    let count=${count}+1
done
total_vols=${count}

#5- Run thickness from ImageJ
#IMPORTANT! It has to be run without headless mode. Reason for this is that the Thickness plugin crashes, at least for Linux
#WARNING! Currently, at least for linux, ImageJ fails to be launched from a script so, this bit of script just prints out
#the commands that should be executed. Once printed in the terminal, they should be copied and pasted in the command line
#for execution.  
program="/home/mzuluaga/bin/Fiji.app/ImageJ-linux64"
script="\"/home/mzuluaga/Code/source/roz_tools/ImageJ/ThicknessScript.bsh\""
count=0

while [ ${count} -lt ${total_vols} ]
do
    thick_img="/home/mzuluaga/data/placenta/placenta_out/thick_volume_"${count}".mhd"
    eval ${program} --ij2 --run  ${script} "'input_file=\""${out_img}_${count}.mhd"\", threshold="${threshold}, "output_file=\""${thick_img}"\"'"
    let count=${count}+1
done

#6- Run statistics
#Computes some basic statistics over the thickness image and displays them
#this should be useful to understand up to which level of thickness in the vessels you wanna
#keep 
program="/home/mzuluaga/bin/roz_tools/bin/compute_statistics"
change_type="/home/mzuluaga/bin/roz_tools/bin/cardiovasc_changetype"
count=0
while [ ${count} -lt ${total_vols} ]
do

    input=${out_img}_${count}.mhd
    mask=${out_img}_${count}_tmp.mhd
    thick_img="/home/mzuluaga/data/placenta/placenta_out/thick_volume_"${count}".mhd"
    cardiovasc_utils -i ${input} --ith 254 255 -o ${mask}
    ${change_type} -i ${mask} -o ${mask}

    ${program} -l ${mask} -i ${thick_img}
    rm -rf ${mask}
    rm -rf ${out_img}_${count}_tmp.raw
    let count=${count}+1
done

echo "***************************************** statistics are done ***************************************************"
echo "By now you should know what you wanna keep or not but, still the rest of the script will ran with some defaults"
echo "Re run with your specific values (might be differnt for each subvolume!)"

#7- Prune out smaller structures
count=0
out_dir="/home/mzuluaga/data/placenta/placenta_out/"
while [ ${count} -lt ${total_vols} ]
do
    input=${out_img}_${count}.mhd
    mask=${out_dir}/final_${count}.mhd    
    thick_img=${out_dir}"thick_volume_"${count}".mhd"	
    thick_bin=${out_dir}"thick_bin_"${count}".mhd"
    cardiovasc_utils -i ${thick_img} --ith 10 120 -o ${thick_bin} 
    cardiovasc_utils -i ${input} --mul ${thick_bin} -o ${mask}	
    ${change_type} -i ${mask} -o ${mask}	
    rm -rf ${out_dir}"thick_bin_"${count}.*
    let count=${count}+1
done

#8- Finally append the subvolumes. 
#Volume merging is done using the post-processed ones (after thickness)
# Alternatively, one can use dirctly those that were piled.
# Steps 6 and 7 can be repeated until a good balance on the prunning is met.
merge_program="/home/mzuluaga/bin/roz_tools/bin/append_volumes"
parts="0 2 3 4 5 6 7 8 9"
sp=1 #This is the spacing but it is just being ignored in this case
for p in ${parts}
do
    result=${out_dir}/result_segmented.mhd
    if [ ${p} == "0" ]
    then
        let p_next=${p}+1
        img_one=${out_dir}/final_${p}.mhd
        img_two=${out_dir}/final_${p_next}.mhd
    else
        let p_mod=${p}%2
        img_two=${out_dir}/final_${p}.mhd
        if [ ${p_mod} -eq 0 ]
        then
            img_one=${out_dir}/result_segmented.mhd
            result=${out_dir}/result_segmented2.mhd
        else
            img_one=${out_dir}/result_segmented2.mhd
        fi
    fi
    ${merge_program} -i1 ${img_one} -i2 ${img_two} -o ${result} -v $sp
done

rm -rf ${out_dir}/result_segmented2.mhd

