

#include "DD4hep/DetFactoryHelper.h"

using dd4hep::Volume;
using dd4hep::DetElement;
using dd4hep::xml::Dimension;
using dd4hep::PlacedVolume;

namespace det {


static dd4hep::Ref_t createSimpleTrackerBarrel(dd4hep::Detector& lcdd,
                                                 dd4hep::xml::Handle_t xmlElement,
                                                 dd4hep::SensitiveDetector sensDet) {
  // shorthands
  dd4hep::xml::DetElement xmlDet = static_cast<dd4hep::xml::DetElement>(xmlElement);
  Dimension dimensions(xmlDet.dimensions());
  // get sensitive detector type from xml
  dd4hep::xml::Dimension sdTyp = xmlElement.child(_Unicode(sensitive));
  // sensitive detector used for all sensitive parts of this detector
  sensDet.setType(sdTyp.typeStr());

  // definition of top volume
  // has min/max dimensions of tracker for visualization etc.
  std::string detectorName = xmlDet.nameStr();
  DetElement topDetElement(detectorName, xmlDet.id());

  dd4hep::Tube topVolumeShape(dimensions.rmin(), dimensions.rmax(), (dimensions.zmax() - dimensions.zmin()) * 0.5);
  Volume topVolume(detectorName, topVolumeShape, lcdd.air());
  topVolume.setVisAttributes(lcdd.invisible());

  // counts all layers - incremented in the inner loop over repeat - tags
  unsigned int layerCounter = 0;
  // loop over 'layer' nodes in xml
  dd4hep::xml::Component xLayers = xmlElement.child(_Unicode(layers));
  for (dd4hep::xml::Collection_t xLayerColl(xLayers, _U(layer)); nullptr != xLayerColl; ++xLayerColl) {
    dd4hep::xml::Component xLayer = static_cast<dd4hep::xml::Component>(xLayerColl);
    std::cout << "building layer at " << xLayer.rmin() << std::endl;
    //dd4hep::Tube layerShape(xLayer.rmin(), xLayer.rmax(), dimensions.zmax());
    dd4hep::Tube layerShape(xLayer.rmin(), xLayer.rmin()+0.1, dimensions.zmax());
    Volume layerVolume("layer", layerShape, lcdd.material("Air"));
    layerVolume.setSensitiveDetector(sensDet);
    layerVolume.setVisAttributes(lcdd, dimensions.visStr());
    PlacedVolume placedLayerVolume = topVolume.placeVolume(layerVolume);
    placedLayerVolume.addPhysVolID("layer", layerCounter);
    //DetElement lay_det(topDetElement, "layer" + std::to_string(layerCounter), layerCounter);
    layerCounter++;

  }
  Volume motherVol = lcdd.pickMotherVolume(topDetElement);
  PlacedVolume placedGenericTrackerBarrel = motherVol.placeVolume(topVolume);
  placedGenericTrackerBarrel.addPhysVolID("system", topDetElement.id());
  topDetElement.setPlacement(placedGenericTrackerBarrel);
  return topDetElement;
}
}  // namespace det

DECLARE_DETELEMENT(SimpleBrlTracker, det::createSimpleTrackerBarrel)
