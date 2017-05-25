#!/bin/bash
#================================================================================
# Example script for processing a whole raw placenta volume
# Authors: Tom Doel
#          M.A. Zuluaga
# Copyright UCL 2017
#================================================================================
#Example on how to call this: ./process_whole_placenta.sh 0.088767678

imagej_bin="/home/mzuluaga/bin/Fiji.app/ImageJ-linux64" #ImageJ + options + script
repo_dir="/home/mzuluaga/Code/source/roz_tools"
tools_dir="/home/mzuluaga/bin/roz_tools"
input_dir="/home/mzuluaga/data/placenta"
output_dir="/home/mzuluaga/data/placenta_processed"

split_data_dir="${output_dir}/input_split"
seg_with_histogram_bin="${tools_dir}/bin/seg_withhisto"
stats_bin="${tools_dir}/bin/compute_statistics"
change_type_bin="${tools_dir}/cardiovasc_changetype"

# ImageJ scripts
skeleton_script="${repo_dir}/ImageJ/SkeletonScript.bsh"
thickness_script="${repo_dir}/ImageJ/ThicknessScript.bsh"

# Set up output folders
mask_folder=${output_dir}/mask
segmented_folder=${output_dir}/segmented
centerline_folder=${output_dir}/centerline

# Ensures folders exist
mkdir -p $mask_folder
mkdir -p $segmented_folder
mkdir -p $centerline_folder

shopt -s nullglob # Ensures the for loop does not process if there are no files
for input_filename in ${split_data_dir}/*.mhd
do
    echo Processing ${input_filename}
    base_filename=$(basename ${input_filename} .mhd)

    # Create mask
    mask_filename=${mask_folder}/${base_filename}_mask.mhd
    cardiovasc_utils -i ${input_filename} --otsu --inv --lconcom -o ${mask_filename}

    # Segmentation with histogram
    segmented_filename=${segmented_folder}/${base_filename}_segmented.mhd
    ${seg_with_histogram_bin}  -i  ${input_filename} -o ${segmented_filename} -m  ${mask_filename}

    # Extract centerline and get statistics (see http://imagej.net/AnalyzeSkeleton#Table_of_results)
    # Note: This ImageJ plugin is strongly connected to the GUI. It will return a Java Headless Exception if run in headless mode
    centerline_filename=${centerline_folder}/${base_filename}_centerline.mhd
    general_stats_filename=${centerline_folder}/${base_filename}_stats_one.xls
    details_stats_filename=${centerline_folder}/${base_filename}_stats_two.xls
    centerline_filename=${centerline_folder}/${base_filename}_centerline.mhd
    eval ${imagej_bin} --ij2 --run ${skeleton_script} \'input_file=\"${segmented_filename}\", output_file=\"${centerline_filename}\", output_statsOne=\"${general_stats_filename}\", output_statsTwo=\"${detailed_stats_filename}\"\'

    # Thickness estimation
    # Note: This ImageJ plugin is strongly connected to the GUI. It will return a Java Headless Exception if run in headless mode
    threshold=254 #This parameter could be also be given as an input
    thickness_filename=${centerline_folder}/${base_filename}_thickvolume.mhd
    eval ${imagej_bin} --ij2 --run ${thickness_script} \'input_file=\"${segmented_filename}\", threshold=\"${threshold}\", output_file=\"${thickness_filename}\"\'

    # Run statistics
    #   Computes some basic statistics over the thickness image and displays them.
    #   This should be useful to understand up to which level of thickness in the vessels you want to keep.
    statsmask_filename=${centerline_folder}/${base_filename}_statsmask.mhd
    cardiovasc_utils -i ${segmented_filename} --ith 254 255 -o statsmask_filename
    ${change_type_bin} -i ${statsmask_filename} -o ${statsmask_filename}
    ${stats_bin} -l ${statsmask_filename} -i ${thickness_filename}

    # Prune out smaller structures
    thicknessbin_filename=${centerline_folder}/${base_filename}_thickbin.mhd
    cardiovasc_utils -i ${thickness_filename} --ith 4 100 -o ${thicknessbin_filename}
    thickmask_filename=${centerline_folder}/${base_filename}_thickmask.mhd
    cardiovasc_utils -i ${segmented_filename} --mul ${thicknessbin_filename} -o ${thickmask_filename}
    ${change_type_bin} -i ${thickmask_filename} -o ${thickmask_filename}
done
