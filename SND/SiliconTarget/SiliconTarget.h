// SPDX-License-Identifier: LGPL-3.0-or-later
// SPDX-FileCopyrightText: Copyright CERN for the benefit of the SHiP
// Collaboration

#ifndef SND_SILICONTARGET_SILICONTARGET_H_
#define SND_SILICONTARGET_SILICONTARGET_H_

#include "Detector.h"
#include "SiliconTargetPoint.h"
#include "TGeoMedium.h"
#include <vector>

class FairVolume;

class SiliconTarget : public SHiP::Detector<SiliconTargetPoint> {
 public:
     
  struct SensorConfig {
    Double_t width;
    Double_t length;
    Double_t thickness;
    Double_t stripPitch;
  };

  struct TargetConfig {
    Double_t targetWidth;
    Double_t targetHeight;
    Int_t nLayers;
    Double_t zPosition;
    Double_t targetThickness;
    Double_t targetSpacing;
    Double_t moduleOffset;
    std::vector<SensorConfig> sensors;
  };

  SiliconTarget(const char* name, Bool_t Active, const char* Title = "");
  SiliconTarget();

  void SetSiliconTargetParameters(const TargetConfig& config);

  TGeoVolume* CreateSiliconPlanes(const char* name,
                                  Double_t planeSpacing,
                                  TGeoMedium* material, Int_t layerId);

  void ConstructGeometry() override;

  Bool_t ProcessHits(FairVolume* vol = nullptr) override;

 private:
  TargetConfig fCfg;

  SiliconTarget(const SiliconTarget&) = delete;
  SiliconTarget& operator=(const SiliconTarget&) = delete;
};

#endif  // SND_SILICONTARGET_SILICONTARGET_H_
