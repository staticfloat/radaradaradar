# - Find bladeRF
# Find the native bladeRF includes and library
#
#  bladeRF_INCLUDES    - where to find bladeRF3.h
#  bladeRF_LIBRARIES   - List of libraries when using bladeRF.
#  bladeRF_FOUND       - True if bladeRF found.

if (bladeRF_INCLUDES)
  # Already in cache, be silent
  set (bladeRF_FIND_QUIETLY TRUE)
endif (bladeRF_INCLUDES)

find_path (bladeRF_INCLUDES libbladeRF.h)

find_library (bladeRF_LIBRARIES NAMES bladeRF)

# handle the QUIETLY and REQUIRED arguments and set bladeRF_FOUND to TRUE if
# all listed variables are TRUE
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (bladeRF DEFAULT_MSG bladeRF_LIBRARIES bladeRF_INCLUDES)

mark_as_advanced (bladeRF_LIBRARIES bladeRF_INCLUDES)
