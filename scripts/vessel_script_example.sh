#!/bin/bash
#folder="/Users/marazulv/Data/example_tom" #Change the name accordingly here. A copy paste from the disk should do!
folder="/Users/tom/Medical Imaging Data/from Maria/example_tom" #Change the name accordingly here. A copy paste from the disk should do!

input=${folder}/placenta_2_1_sub_2.mha
mask=${folder}/placenta_test_tom_mask.mhd
vesselness=${folder}/"placenta_test_tom_vesselness.mhd"
vessel_mask=${folder}/"placenta_test_tom_vmasked.mhd"
vessel_bin=${folder}/"placenta_test_tom_filtered.mhd"

#1- Obtain a rough initial segmentation of the vessels. The results
# are mostly ok. However, there are some blood pools that are also segmented
# and should not be
cardiovasc_utils -i ${input} --otsu --inv -o ${mask}

#2- Use Sato's metric as an uncertainty estimation. Min and Max are the min and max
#scales to be used by the filter. The image that I am using as an example does not have
#the correct voxel spacing. This is why I am using those values which seem weird for
#something obtained from a micro-CT
#Note: I think the mask is not working (-b option) in my laptop's version. If it is
#working on the code I left there (i cannot access), you should use the option
#and as mask I recommend a dilated version of ${mask} (dilated for border effects)
#The vesselness output image can be used as an uncertainty measure of what is vessel
#and what is not
vessel_filter -i ${input} -o ${vesselness}  --min 1 --max 15

#3- Get rid of everything that Otsu did not pick up at first
# The first result is high in false positives but very low in false negatives.
# This step gets rid of everything that Otsu rejected.
# Also, if in 2 the mask is working it gets rid of border effects.
cardiovasc_utils -i ${vesselness} --mul ${mask} -o ${vessel_mask}

#4- Binarise the vesselness image and display the different connected components
vessel_binarise -i ${vessel_mask} -o ${vessel_bin} -t 1

#5- A final step could be to threshold the labels if you don't want to see all those
# colours. I am skipping it because it is straight forward with any tool.


