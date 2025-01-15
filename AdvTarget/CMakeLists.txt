
# Create a library called "libstrawtubes" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

set(INCLUDE_DIRECTORIES
${CMAKE_SOURCE_DIR}/shipdata
${CMAKE_SOURCE_DIR}/AdvTarget
)

include_directories( ${INCLUDE_DIRECTORIES} ${VMC_INCLUDE_DIRS} ${FAIRROOT_INCLUDE_DIR} ${FairLogger_INCDIR} ${FMT_INCLUDE_DIR}
)
include_directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
${genfit2_LIBDIR}
)

link_directories( ${LINK_DIRECTORIES})
#link_directories(${genfit2_LIBDIR})
link_directories(${Rave_LIBDIR})

set(SRCS
#Put here your sourcefiles
AdvTarget.cxx
AdvTargetPoint.cxx
AdvTargetHit.cxx
)

Set(LINKDEF AdvTargetLinkDef.h)
Set(LIBRARY_NAME AdvTarget)
Set(DEPENDENCIES
    Base ShipData
)

GENERATE_LIBRARY()
