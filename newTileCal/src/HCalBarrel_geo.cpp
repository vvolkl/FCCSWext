// DD4hep
#include "DD4hep/DetFactoryHelper.h"

// Gaudi
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ServiceHandle.h"

using dd4hep::Volume;
using dd4hep::DetElement;
using dd4hep::xml::Dimension;
using dd4hep::PlacedVolume;


//rename xml_h to xml_det_T?

namespace det {

static dd4hep::Ref_t createHCal(dd4hep::Detector& lcdd, xml_det_t xmlDet, dd4hep::SensitiveDetector sensDet) {
  // Get the Gaudi message service and message stream:
  ServiceHandle<IMessageSvc> msgSvc("MessageSvc", "HCalConstruction");
  MsgStream lLog(&(*msgSvc), "HCalConstruction");

  // Make volume that envelopes the whole barrel; set material to air
  Dimension xDimensions(xmlDet.dimensions());

  // top level det element representing whole hcal barrel
  DetElement hCal(xmlDet.nameStr(), xmlDet.id());

  // sensitive detector type read from xml (for example "SimpleCalorimeterSD")
  Dimension xSensitive = xmlDet.child(_U(sensitive));
  sensDet.setType(xSensitive.typeStr());



  dd4hep::Tube envelopeShape(xDimensions.rmin(), xDimensions.rmax(), xDimensions.dz());
  Volume envelopeVolume("envelopeVolume", envelopeShape, lcdd.air());
  envelopeVolume.setVisAttributes(lcdd, xDimensions.visStr());

  xml_comp_t xEndPlate = xmlDet.child(_Unicode(end_plate));
  double dZEndPlate = xEndPlate.thickness();
  xml_comp_t xFacePlate = xmlDet.child(_Unicode(face_plate));
  xml_comp_t xSpace = xmlDet.child(_Unicode(plate_space));  // to avoid overlaps
  double space = xSpace.thickness();
  xml_comp_t xSteelSupport = xmlDet.child(_Unicode(steel_support));
  double dSteelSupport = xSteelSupport.thickness();

  lLog << MSG::DEBUG << "steel support thickness: " << dSteelSupport << " [cm]" << endmsg;
  lLog << MSG::DEBUG << "steel support material:  " << xSteelSupport.materialStr() << endmsg;

  double sensitiveBarrelRmin = xDimensions.rmin() + xFacePlate.thickness() + space;

  // Hard-coded assumption that we have two different sequences for the modules
  std::vector<xml_comp_t> sequences = {xmlDet.child(_Unicode(sequence_a)), xmlDet.child(_Unicode(sequence_b))};
  // NOTE: This assumes that both have the same dimensions!
  Dimension sequenceDimensions(sequences[1].dimensions());
  double dzSequence = sequenceDimensions.dz();
  lLog << MSG::DEBUG << "sequence thickness " << dzSequence << endmsg;

  // calculate the number of modules fitting in Z
  unsigned int numSequencesZ = static_cast<unsigned>((2 * xDimensions.dz() - 2 * dZEndPlate - 2 * space) / dzSequence);

  // Hard-coded assumption that we have three different layer types for the modules
  std::vector<xml_comp_t> Layers = {xmlDet.child(_Unicode(layer_1)), xmlDet.child(_Unicode(layer_2)),
                                    xmlDet.child(_Unicode(layer_3))};
  unsigned int numSequencesR = 0;
  double moduleDepth = 0.;
  std::vector<double> layerDepths = std::vector<double>();
  for (std::vector<xml_comp_t>::iterator it = Layers.begin(); it != Layers.end(); ++it) {
    xml_comp_t layer = *it;
    Dimension layerDimension(layer.dimensions());
    numSequencesR += layerDimension.nModules();
    for (int nLayer = 0; nLayer < layerDimension.nModules(); nLayer++) {
      moduleDepth += layerDimension.dr();
      layerDepths.push_back(layerDimension.dr());
    }
  }
  lLog << MSG::DEBUG << "retrieved number of layers:  " << numSequencesR
       << " , which end up to a full module depth in rho of " << moduleDepth << endmsg;
  lLog << MSG::DEBUG << "retrieved number of layers:  " << layerDepths.size() << endmsg;

  lLog << MSG::INFO << "constructing: " << numSequencesZ << " rings in Z, " << numSequencesR
       << " layers in Rho, " << numSequencesR * numSequencesZ << " tiles" << endmsg;

  // Calculate correction along z based on the module size (can only have natural number of modules)
  double dzDetector = (numSequencesZ * dzSequence) / 2 + dZEndPlate + space;
  lLog << MSG::DEBUG << "dzDetector:  " <<  dzDetector << endmsg;
  lLog << MSG::INFO << "correction of dz (negative = size reduced):" << dzDetector - xDimensions.dz() << endmsg;


  // Add structural support made of steel inside of HCal
  DetElement facePlate(hCal, "FacePlate", 0);
  dd4hep::Tube facePlateShape(xDimensions.rmin(), sensitiveBarrelRmin, (dzDetector - dZEndPlate - space));
  Volume facePlateVol("facePlate", facePlateShape, lcdd.material(xFacePlate.materialStr()));
  facePlateVol.setVisAttributes(lcdd, xFacePlate.visStr());
  PlacedVolume placedFacePlate = envelopeVolume.placeVolume(facePlateVol);
  facePlate.setPlacement(placedFacePlate);

  // Add structural support made of steel at both ends of HCal
  dd4hep::Tube endPlateShape(xDimensions.rmin(), (xDimensions.rmax() - dSteelSupport), dZEndPlate / 2);
  Volume endPlateVol("endPlate", endPlateShape, lcdd.material(xEndPlate.materialStr()));
  endPlateVol.setVisAttributes(lcdd, xEndPlate.visStr());

  DetElement endPlatePos(hCal, "endPlatePos", 0);
  dd4hep::Position posOffset(0, 0, dzDetector - (dZEndPlate / 2));
  PlacedVolume placedEndPlatePos = envelopeVolume.placeVolume(endPlateVol, posOffset);
  endPlatePos.setPlacement(placedEndPlatePos);

  DetElement endPlateNeg(hCal, "endPlateNeg", 1);
  dd4hep::Position negOffset(0, 0, -dzDetector + (dZEndPlate / 2));
  PlacedVolume placedEndPlateNeg = envelopeVolume.placeVolume(endPlateVol, negOffset);
  endPlateNeg.setPlacement(placedEndPlateNeg);

  // Calculation the dimensions of one whole module:
  double spacing = sequenceDimensions.x();
  
  double rminSupport = sensitiveBarrelRmin + moduleDepth - spacing;
  double rmaxSupport = sensitiveBarrelRmin + moduleDepth + dSteelSupport - spacing;

  // DetElement vectors for placement in loop at the end
  std::vector<dd4hep::PlacedVolume> layers;
  layers.reserve(layerDepths.size());
  std::vector<std::vector<dd4hep::PlacedVolume>> tilesInLayers;
  tilesInLayers.reserve(numSequencesZ*sequences.size());
  
  double layerR = 0.;

  // Placement of subWedges in Wedge
  for (unsigned int idxLayer = 0; idxLayer < layerDepths.size(); ++idxLayer) {
    auto layerName = dd4hep::xml::_toString(idxLayer, "layer%d");
    unsigned int sequenceIdx = idxLayer % 2;

    // in Module rmin = 0  for first wedge, changed radius to the full radius starting at (0,0,0)
    double rminLayer = sensitiveBarrelRmin + layerR;
    double rmaxLayer = sensitiveBarrelRmin + layerR + layerDepths.at(idxLayer);
    lLog << MSG::DEBUG << "layer minumum radius:  " << rminLayer << endmsg;
    lLog << MSG::DEBUG << "layer maximum radius:  " << rmaxLayer << endmsg;
    
    layerR += layerDepths.at(idxLayer);

    dd4hep::Tube layerShape(rminLayer, rmaxLayer, (dzDetector - dZEndPlate - space));
    Volume layerVolume("layerVolume", layerShape, lcdd.material("Air"));
    
    layerVolume.setVisAttributes(lcdd.invisible());
    unsigned int idxSubMod = 0;
    unsigned int idxActMod = 0;
    double modCompZOffset = - (dzDetector - dZEndPlate - space);
    
    // this matches the order of sequences of standalone HCAL geo description
    if (sequenceIdx == 0) {
      sequenceIdx = 1;
    } else {
      sequenceIdx = 0;
    }

    layers.push_back(envelopeVolume.placeVolume(layerVolume));
    layers.back().addPhysVolID("layer", idxLayer);
   
    std::vector<dd4hep::PlacedVolume> tiles;
    // Filling of the layer tube with tile tubes in full z
    for (uint numSeq=0; numSeq<numSequencesZ; numSeq++){
      for (xml_coll_t xCompColl(sequences[sequenceIdx], _Unicode(module_component)); xCompColl;
	   ++xCompColl, ++idxSubMod) {
	xml_comp_t xComp = xCompColl;
	double dyComp = xComp.thickness() * 0.5;
	dd4hep::Tube tileShape(rminLayer, rmaxLayer, dyComp);
	
	Volume modCompVol("modCompVolume", tileShape,
			  lcdd.material(xComp.materialStr()));
	modCompVol.setVisAttributes(lcdd, xComp.visStr());
	
	dd4hep::Position offset(0, 0, modCompZOffset + dyComp);
	
	if (xComp.isSensitive()) {
	  modCompVol.setSensitiveDetector(sensDet);
	  tiles.push_back(layerVolume.placeVolume(modCompVol, offset));
	  tiles.back().addPhysVolID("row", numSeq);
	  idxActMod++;
	} else {
	  tiles.push_back(layerVolume.placeVolume(modCompVol, offset));
	}
	modCompZOffset += xComp.thickness();
      }
    }
    // Fill vector for DetElements
    tilesInLayers.push_back(tiles);
  }
  dd4hep::Tube supportShape(rminSupport, rmaxSupport, (dzDetector - dZEndPlate - space));
  Volume steelSupportVolume("steelSupportVolume", supportShape,
			    lcdd.material(xSteelSupport.materialStr()));
  steelSupportVolume.setVisAttributes(lcdd.invisible());
  PlacedVolume placedSupport = envelopeVolume.placeVolume(steelSupportVolume);
  DetElement support(hCal, "support", 0);
  support.setPlacement(placedSupport);

  
  // Placement of DetElements
  lLog << MSG::DEBUG << "Layers in r :    " << layers.size() << endmsg;
  lLog << MSG::DEBUG << "Tiles in layers :" << tilesInLayers[1].size() << endmsg;
  
  
  /*
  for (uint iLayer = 0; iLayer < numSequencesR; iLayer++) {
    DetElement layerDet(hCal, dd4hep::xml::_toString(iLayer, "layer%d"), iLayer);
    layerDet.setPlacement(layers[iLayer]);
    
    for (uint iTile = 0; iTile < tilesInLayers[iLayer].size(); iTile++) {
      DetElement tileDet(layerDet, dd4hep::xml::_toString(iTile, "tile%d"), iTile);
      tileDet.setPlacement(tilesInLayers[iLayer][iTile]);
    }
  }
  */
   
  // Place envelope (or barrel) volume
  Volume motherVol = lcdd.pickMotherVolume(hCal);
  motherVol.setVisAttributes(lcdd.invisible());
  PlacedVolume placedHCal = motherVol.placeVolume(envelopeVolume);
  placedHCal.addPhysVolID("system", hCal.id());
  hCal.setPlacement(placedHCal);
  return hCal;
}
}  // namespace hcal

DECLARE_DETELEMENT(newCaloBarrel, det::createHCal)