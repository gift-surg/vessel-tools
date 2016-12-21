 add_executable(seg_withhisto segment_withhistogram.cxx)
    target_link_libraries(seg_withhisto ${ROZ_ITK_LIB})
 install_targets(/bin seg_withhisto)

 add_executable(vessel_filter vessel_filter.cpp)
    target_link_libraries(vessel_filter ${ROZ_ITK_LIB})
 install_targets(/bin vessel_filter)

 add_executable(vessel_binarise vessel_binarise.cpp)
    target_link_libraries(vessel_binarise ${ROZ_ITK_LIB})
 install_targets(/bin vessel_binarise)

