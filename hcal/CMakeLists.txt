# Create a library called "libhcal" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

set(INCLUDE_DIRECTORIES
${CMAKE_SOURCE_DIR}/shipdata
${CMAKE_SOURCE_DIR}/hcal
${genfit2_INCDIR}
)

include_directories(${INCLUDE_DIRECTORIES} ${VMC_INCLUDE_DIRS} ${FAIRROOT_INCLUDE_DIR})
include_directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
${genfit2_LIBDIR}
)

link_directories( ${LINK_DIRECTORIES})

set(SRCS
#Put here your sourcefiles
hcalContFact.cxx
hcalPoint.cxx
hcal.cxx
hcalLightMap.cxx
hcalInf.cxx
hcalModule.cxx
hcalModuleMC.cxx
hcalStructure.cxx
hcalStructureFiller.cxx
hcalAnalysisSimple.cxx
)

Set(LINKDEF hcalLinkDef.h)
Set(LIBRARY_NAME hcal)
Set(DEPENDENCIES
    Base ShipData FairLogger::FairLogger
)

GENERATE_LIBRARY()
