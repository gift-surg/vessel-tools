    ADD_EXECUTABLE(cardiovasc_utils cardiovasc_utils.cpp)
        TARGET_LINK_LIBRARIES(cardiovasc_utils ${ROZ_ITK_LIB})
    INSTALL_TARGETS(/bin cardiovasc_utils)

    ADD_EXECUTABLE(cardiovasc_pile cardiovasc_pile.cpp)
        TARGET_LINK_LIBRARIES(cardiovasc_pile ${ROZ_ITK_LIB})
    INSTALL_TARGETS(/bin cardiovasc_pile)

    ADD_EXECUTABLE(cardiovasc_regions cardiovasc_regions.cpp)
        TARGET_LINK_LIBRARIES(cardiovasc_regions ${ROZ_ITK_LIB})
    INSTALL_TARGETS(/bin cardiovasc_regions)

    ADD_EXECUTABLE(cardiovasc_changetype cardiovasc_changetype.cpp)
        TARGET_LINK_LIBRARIES(cardiovasc_changetype ${ROZ_ITK_LIB})
    INSTALL_TARGETS(/bin cardiovasc_changetype)

    ADD_EXECUTABLE(cardiovasc_subsample cardiovasc_subsample.cpp)
        TARGET_LINK_LIBRARIES(cardiovasc_subsample ${ROZ_ITK_LIB})
    INSTALL_TARGETS(/bin cardiovasc_subsample)
