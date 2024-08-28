import sndVertex
import global_variables
import ROOT

##open inputfile
inputFile = "~/simulationFiles/SiSND/muons/s13/ship.conical.PG_13-TGeant4_dig_PR_tracked.root"
geoFile = "~/simulationFiles/SiSND/muons/s13/geofile_full.conical.PG_13-TGeant4.root"
##create outputfile
outputFile = "~/simulationFiles/SiSND/muons/s13/ship.conical.PG_13-TGeant4_dig_PR_tracked_vertexed_new.root"

#Vertex = sndVertex.Task(global_variables.h, self.sTree)
geofile = ROOT.TFile.Open(geoFile)
#geoMat =  ROOT.genfit.TGeoMaterialInterface()  # if only called in ShipDigiReco -> crash, reason unknown
geo = geofile.FAIRGeom  # noqa: F841
ROOT.gInterpreter.Declare('#include "TGeoMaterialInterface.h"')
ROOT.gInterpreter.Declare('#include "MaterialEffects.h"')
ROOT.gInterpreter.Declare('#include "FieldManager.h"')
ROOT.gInterpreter.Declare('#include "ConstField.h"')
geo_mat = ROOT.genfit.TGeoMaterialInterface()
ROOT.genfit.MaterialEffects.getInstance().init(geo_mat)
bfield = ROOT.genfit.ConstField(0, 0, 0)
field_manager = ROOT.genfit.FieldManager.getInstance()
field_manager.init(bfield)

SHiP = sndVertex.SNDVertex(inputFile)
nEvents   = SHiP.sTree.GetEntries()
# main loop
for global_variables.iEvent in range(0, nEvents):
    if global_variables.iEvent % 100 == 0:# or global_variables.debug:
        print('event ', global_variables.iEvent)
    rc = SHiP.sTree.GetEvent(global_variables.iEvent)
    SHiP.execute()
 # memory monitoring
 # mem_monitor()
# end loop over events
SHiP.finish()
