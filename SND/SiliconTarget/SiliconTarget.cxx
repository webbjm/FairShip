// SPDX-License-Identifier: LGPL-3.0-or-later
// SPDX-FileCopyrightText: Copyright CERN for the benefit of the SHiP
// Collaboration

#include "SiliconTarget.h"

#include "ShipDetectorList.h"
#include "ShipGeoUtil.h"
#include "ShipStack.h"
#include "ShipUnit.h"

// ROOT / TGeo headers
#include "TGeoBBox.h"
#include "TGeoCompositeShape.h"
#include "TGeoManager.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoVolume.h"
#include "TParticle.h"
#include "TVector3.h"

// FairROOT headers
#include "FairVolume.h"
#include "TVirtualMC.h"

using ShipUnit::mm;
using ShipUnit::um;

SiliconTarget::SiliconTarget()
    : Detector("SiliconTarget", kTRUE, kSiliconTarget) {}

SiliconTarget::SiliconTarget(const char* name, Bool_t Active,
                             const char* /*Title*/)
    : Detector(name, Active, kSiliconTarget) {}

void SiliconTarget::SetSiliconTargetParameters(const TargetConfig& config) {
    fCfg = config;
}

TGeoVolume* SiliconTarget::CreateSiliconPlanes(const char* name,
                                             Double_t spacing,
                                             TGeoMedium* silicon,
                                             Int_t layerId) {
const Int_t nRows = 2;
const Int_t nCols = 4;
const Int_t nPlanes = 2;
const Double_t gap = 1.0 * mm; // 1mm gap between sensors

auto CreateSensorVolume = [&](const char* volName, Double_t width, Double_t length, Double_t thickness, Double_t pitch) {
  TGeoBBox* shape = new TGeoBBox("SensorShape", width / 2.0, length / 2.0, thickness / 2.0);
  TGeoVolume* volume = new TGeoVolume(volName, shape, silicon);
  volume->SetLineColor(kRed);
  volume->SetTransparency(40);

  Int_t nStrips = std::floor(width / pitch);
  auto* strips = volume->Divide("SLICEX", 1, nStrips, -width / 2.0, pitch);
  AddSensitiveVolume(strips);
  return volume;
};


  TGeoVolume* sensorVolume1 = CreateSensorVolume("SensorVolume_1", fCfg.sensors[0].width, fCfg.sensors[0].length, fCfg.sensors[0].thickness, fCfg.sensors[0].stripPitch);
  TGeoVolume* sensorVolume2 = CreateSensorVolume("SensorVolume_2", fCfg.sensors[1].width, fCfg.sensors[1].length, fCfg.sensors[1].thickness, fCfg.sensors[1].stripPitch);

  TGeoVolumeAssembly* trackingStation = new TGeoVolumeAssembly(name);

  for (Int_t plane = 0; plane < nPlanes; ++plane) {
    TGeoVolumeAssembly* trackerPlane = new TGeoVolumeAssembly(Form("TrackerPlane_%d", plane));

    TGeoVolume* activeVolume = (plane == 0) ? sensorVolume1 : sensorVolume2;
    Double_t w = fCfg.sensors[plane].width;
    Double_t l = fCfg.sensors[plane].length;

    // Populate the 2x4 grid for the plane
    for (Int_t column = 0; column < nCols; ++column) {
      for (Int_t row = 0; row < nRows; ++row) {

        Int_t sensorId = (layerId << 5) + (plane << 4) + (column << 2) + (row << 1);


        Double_t posX = (-1.5 * w - 2.0 * mm) + (column * w + column * gap);
        Double_t posY = (l / 2.0 + 0.5 * mm) - row * (l + gap);

        trackerPlane->AddNode(activeVolume, sensorId, new TGeoTranslation(posX, posY, 0.0));
      }
    }

    if (plane == 0) {
      trackingStation->AddNode(trackerPlane, plane);
    } else {
      // Rotate the second plane by 90 degrees around Z and offset by spacing
      TGeoRotation* rot = new TGeoRotation();
      rot->RotateZ(90.0);
      trackingStation->AddNode(trackerPlane, plane, new TGeoCombiTrans(TGeoTranslation(0.0, 0.0, spacing), *rot));
    }
  }

  return trackingStation;
}

void SiliconTarget::ConstructGeometry() {
  ShipGeo::InitMedium("tungstensifon");
  TGeoMedium* tungsten = gGeoManager->GetMedium("tungstensifon");
  ShipGeo::InitMedium("air");
  TGeoMedium* air = gGeoManager->GetMedium("air");
  ShipGeo::InitMedium("silicon");
  TGeoMedium* Silicon = gGeoManager->GetMedium("silicon");

  Double_t totalLength = fCfg.nLayers * fCfg.targetSpacing;

  // Create an envelope volume for the detector (green, semi-transparent)
  auto envBox = new TGeoBBox("SiliconTarget_env", fCfg.targetWidth / 2.,
                             fCfg.targetHeight / 2., totalLength / 2.);
  auto envVol = new TGeoVolume("SiliconTarget", envBox, air);
  envVol->SetLineColor(kGreen);
  envVol->SetTransparency(50);

  auto target = new TGeoBBox("Target", fCfg.targetWidth / 2., fCfg.targetHeight / 2.,
                             fCfg.targetThickness / 2.);
  auto targetVol = new TGeoVolume("TargetVol", target, tungsten);
  targetVol->SetLineColor(kGray);
  targetVol->SetTransparency(40);

  for (Int_t i = 0; i < fCfg.nLayers; i++) {
    // Compute the center position (z) for the current W layer
    Double_t zPos = -totalLength / 2 + i * fCfg.targetSpacing;

    // Place the tungsten layer
    envVol->AddNode(targetVol, i,
                    new TGeoTranslation(0, 0, zPos + fCfg.targetThickness / 2.));

    // Calculate spacing gaps
    TGeoVolume* siliconPlanes = CreateSiliconPlanes(
        "TrackerPlane",
        fCfg.targetSpacing - fCfg.targetThickness - 2. * fCfg.moduleOffset, Silicon, i);
    envVol->AddNode(
        siliconPlanes, i,
        new TGeoTranslation(0, 0, zPos + fCfg.targetThickness + fCfg.moduleOffset));
  }

  gGeoManager->GetTopVolume()->AddNode(envVol, 1,
                                       new TGeoTranslation(0, 0, fCfg.zPosition));
}

Bool_t SiliconTarget::ProcessHits(FairVolume* vol) {
  /** This method is called from the MC stepping */
  // Set parameters at entrance of volume. Reset ELoss.
  if (gMC->IsTrackEntering()) {
    fELoss = 0.;
    fTime = gMC->TrackTime() * 1.0e09;
    fLength = gMC->TrackLength();
    gMC->TrackPosition(fPos);
    gMC->TrackMomentum(fMom);
  }

  // Sum energy loss for all steps in the active volume
  fELoss += gMC->Edep();

  // Create SiliconTargetPoint at exit of active volume
  if (gMC->IsTrackExiting() || gMC->IsTrackStop() ||
      gMC->IsTrackDisappeared()) {
    if (fELoss == 0.) {
      return kFALSE;
    }

    TParticle* p = gMC->GetStack()->GetCurrentTrack();
    fTrackID = gMC->GetStack()->GetCurrentTrackNumber();
    Int_t pdgCode = p->GetPdgCode();
    TLorentzVector Pos;
    gMC->TrackPosition(Pos);
    TLorentzVector Mom;
    gMC->TrackMomentum(Mom);

    Double_t xmean = (fPos.X() + Pos.X()) / 2.;
    Double_t ymean = (fPos.Y() + Pos.Y()) / 2.;
    Double_t zmean = (fPos.Z() + Pos.Z()) / 2.;

    Int_t strip_id = 0;
    Int_t sensor_id = 0;
    gMC->CurrentVolID(strip_id);
    gMC->CurrentVolOffID(1, sensor_id);
    fVolumeID = (sensor_id << 12) + strip_id;

    AddHit(fTrackID, fVolumeID, TVector3(xmean, ymean, zmean),
           TVector3(fMom.Px(), fMom.Py(), fMom.Pz()), fTime, fLength, fELoss,
           pdgCode);

    ShipStack* stack = dynamic_cast<ShipStack*>(gMC->GetStack());
    stack->AddPoint(kSiliconTarget);
  }
  return kTRUE;
}
