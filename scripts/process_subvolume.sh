#!/bin/bash

#Script to process a subvolume using the vesselness filter - (as in big arteries)
datapath=/home/mzuluaga/tmp
image=$1
min=$2
max=$3
result=$4

cardiovasc_utils -i ${image} --otsu --inv --ero 2 --lconcom --ero 1 --lconcom --dil 7 -o ${datapath}/mask.mhd
vessel_filter -i ${image} -o ${datapath}/vesselness.mhd --min ${min} --max ${max} -b ${datapath}/mask.mhd
cardiovasc_utils -i ${datapath}/vesselness.mhd --mul ${datapath}/mask.mhd -o ${datapath}/vessel_map.nii.gz
vessel_binarise -i ${datapath}/vessel_map.mhd -o ${datapath}/vessel_bin.mhd -t 4
cardiovasc_utils -i ${datapath}/vessel_bin.mhd --ith 1 6 -o ${result}

rm -rf ${datapath}/*
