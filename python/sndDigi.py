import ROOT
import os
import shipunit as u
#import SciFiMapping
from array import array

stop  = ROOT.TVector3()
start = ROOT.TVector3()

class sndDigi:
    " convert  MC hits to digitized hits"
    def __init__(self,fout):

        self.iEvent = 0

        outdir=os.getcwd()
        outfile=outdir+"/"+fout
        self.fn = ROOT.TFile(fout,'update')
        self.sTree = self.fn.cbmsim

        # event header
        self.header  = ROOT.FairEventHeader()
        self.eventHeader  = self.sTree.Branch("EventHeader.",self.header,32000,1)
        #AdvTarget
        self.digiTarget = ROOT.TClonesArray("AdvTargetHit")
        self.digiTargetBranch = self.sTree.Branch("Digi_advTargetHits",self.digiTarget, 32000, 1)
        self.digiTarget2MCPoints = ROOT.TClonesArray("Hit2MCPoints")
        self.digiTarget2MCPoints.BypassStreamer(ROOT.kTRUE)
        self.digiTarget2MCPointsBranch = self.sTree.Branch("Digi_TargetHits2MCPoints",self.digiTarget2MCPoints, 32000, 1)
        self.digiTargetCluster = ROOT.TClonesArray("AdvTargetHit")
        self.digiTargetClusterBranch = self.sTree.Branch("Digi_advTargetClusters", self.digiTargetCluster, 32000, 1)
        self.digiTargetCluster2MCPoints = ROOT.TClonesArray("Hit2MCPoints")
        self.digiTargetCluster2MCPoints.BypassStreamer(ROOT.kTRUE)
        self.digiTargetCluster2MCPointsBranch = self.sTree.Branch("Digi_TargetClusterHits2MCPoints",self.digiTargetCluster2MCPoints, 32000, 1)

        lsOfGlobals  = ROOT.gROOT.GetListOfGlobals()
#        self.scifiDet = lsOfGlobals.FindObject('Scifi')
#        self.mufiDet = lsOfGlobals.FindObject('MuFilter')
        # self.targetDet = lsOfGlobals.FindObject('AdvTarget')
        # if not self.scifiDet or not self.mufiDet:
        #       exit('detector(s) not found, stop')

    def digitize(self):

        self.header.SetRunId( self.sTree.MCEventHeader.GetRunID() )
#        self.header.SetEventNumber( self.sTree.MCEventHeader.GetEventID() )  # counts from 1
        self.header.SetMCEntryNumber( self.sTree.MCEventHeader.GetEventID() )  # counts from 1
#        self.header.SetBunchType(101);
        self.eventHeader.Fill()

        self.digiTarget.Delete()
        self.digiTarget2MCPoints.Delete()
        self.digitizeTarget()
        self.digiTargetBranch.Fill()
        self.digiTarget2MCPointsBranch.Fill()
        self.digiTargetCluster.Delete()
        self.digiTargetCluster2MCPoints.Delete()
        self.clusterTarget()
        self.digiTargetClusterBranch.Fill()
        self.digiTargetCluster2MCPointsBranch.Fill()


    def digitizeTarget(self):
        hitContainer = {}
        mcLinks = ROOT.Hit2MCPoints()
        mcPoints = {}
        norm = {}
        for k, p in enumerate(self.sTree.AdvTargetPoint):
            #  collect all hits in same detector element
            detID = p.GetDetectorID()
            if not detID in hitContainer:
                  hitContainer[detID] = []
                  mcPoints[detID] = {}
                  norm[detID] = 0
            hitContainer[detID].append(p)
            mcPoints[detID][k] = p.GetEnergyLoss()
            norm[detID] += p.GetEnergyLoss()
        for index, detID in enumerate(hitContainer):
            allPoints = ROOT.std.vector('AdvTargetPoint*')()
            for p in hitContainer[detID]:
                 allPoints.push_back(p)

            if self.digiTarget.GetSize() == index:
                self.digiTarget.Expand(index + 100)
            self.digiTarget[index] = ROOT.AdvTargetHit(detID, allPoints)
            for k in mcPoints[detID]:
                mcLinks.Add(detID, k, mcPoints[detID][k] / norm[detID])
        self.digiTarget2MCPoints[0] = mcLinks

    def clusterTarget(self):
        cluster_index = 0
        hitDict = {}
        mcLinks = ROOT.Hit2MCPoints()
        for k, p in enumerate(self.sTree.Digi_advTargetHits):
            if not p.isValid():
                continue
            hitDict[p.GetDetectorID()] = k
        hitList = list(hitDict.keys())
        if hitList:
              hitList.sort()
              tmp = [hitList[0]]
              cprev = hitList[0]
              ncl = 0
              last = len(hitList) - 1
              hits = ROOT.RVec("AdvTargetHit*")()
              for i in range(len(hitList)):
                   if i == 0 and len(hitList) > 1:
                       continue
                   c = hitList[i]
                   if (c-cprev) == 1:
                        tmp.append(c)
                   if (c-cprev) != 1 or c == hitList[last]:
                       mcPoints = {}
                       norm = 0
                       first = tmp[0]
                       N = len(tmp)
                       hits.clear()
                       for hit in tmp:
                           hits.push_back(self.sTree.Digi_advTargetHits[hitDict[hit]],)
                       # Take leading hit of each cluster, discard all other
                       signals = ROOT.RVec("double")([h.GetSignal() for h in hits])
                       leading_hit = hits[ROOT.VecOps.ArgMax(signals)]
                       detID = leading_hit.GetDetectorID()
                       allPoints = ROOT.std.vector('AdvTargetPoint*')()
                       # Use truth to find all points (shortcut, can be done using hits directly)
                       link = self.sTree.Digi_TargetHits2MCPoints[0]
                       for hit in hits:
                           wlist = link.wList(hit.GetDetectorID())
                           point_indices = [index for index,_ in wlist]
                           for index, _ in wlist:
                               point = self.sTree.AdvTargetPoint[index]
                               allPoints.push_back(point)
                               mcPoints[index] = point.GetEnergyLoss()
                               norm += point.GetEnergyLoss()
                       for index in mcPoints:
                           mcLinks.Add(detID, index, mcPoints[index] / norm)
                       if self.digiTargetCluster.GetSize() == cluster_index:
                           self.digiTargetCluster.Expand(cluster_index + 10)
                       self.digiTargetCluster[cluster_index] = ROOT.AdvTargetHit(detID, allPoints)
                       cluster_index += 1
                       if c != hitList[last]:
                            ncl += 1
                            tmp = [c]
                   cprev = c
        self.digiTargetCluster2MCPoints[0] = mcLinks
            
    def finish(self):
        print('finished writing tree')
        self.sTree.Write()
