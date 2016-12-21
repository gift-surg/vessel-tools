 add_executable(compute_statistics compute_statistics.cpp)
    target_link_libraries(compute_statistics ${ROZ_ITK_LIB})
 install_targets(/bin compute_statistics)
