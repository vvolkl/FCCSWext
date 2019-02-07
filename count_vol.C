
void count_vol() {
  TGeoManager* t = gGeoManager->Import("test.root");
  t->GetCurrentVolume()->Print();
  t->GetCurrentNode()->GetMatrix()->Print();
  t->CountLevels();

  int numberOfPlacedVolumes = 0;
  TGeoVolume* v = t->GetCurrentVolume();
  TGeoNode* node;
  TGeoIterator next(v);
    TString s;
  while ((node = next())) {
    std::string currentNodeName = node->GetName();
    
    next.GetPath(s);
    cout << s << endl;
    //if (currentNodeName.find("component") != std::string::npos) {
      ++numberOfPlacedVolumes;
    //}
  }
	std::cout << numberOfPlacedVolumes << std::endl;
  gApplication->Terminate();


}
