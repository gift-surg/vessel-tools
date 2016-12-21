 add_executable(convert_tiff convert_tiff.cpp)
    target_link_libraries(convert_tiff ${ROZ_ITK_LIB})
 install_targets(/bin convert_tiff)

 add_executable(extract_channels extract_channels.cpp)
    target_link_libraries(extract_channels ${ROZ_ITK_LIB})
 install_targets(/bin extract_channels)

 add_executable(append_volumes append_volumes.cpp)
    target_link_libraries(append_volumes ${ROZ_ITK_LIB})
 install_targets(/bin append_volumes)

 add_executable(convert_slices_to_vol convert_slices_to_vol.cpp)
    target_link_libraries(convert_slices_to_vol ${ROZ_ITK_LIB})
 install_targets(/bin convert_slices_to_vol)

