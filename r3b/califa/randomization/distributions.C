////////////////////////////////////////////////////////////////////////////////
//      This Macro Creates a File with Angular Coverage for each Crystal
//                   Needs a ROOT file with simulated Gammas
//             GGJ (gabrielgarcia.jimenez@usc.es), February 2022
////////////////////////////////////////////////////////////////////////////////


struct angular_coverages {

  std::vector<Float_t> polarDistribution;
  std::vector<Float_t> azimuthDistribution;

};

void distributions(){


  TStopwatch timer;
  timer.Start();

  // ---------- Put here the root file -----------
  TString fileList = "gammas_s455.root";

  TFile *eventFile;
  TTree* eventTree;

  eventFile = TFile::Open(fileList);
  eventTree = (TTree*)eventFile->Get("evt");
  eventTree->SetBranchStatus("*",0);

  eventTree->SetBranchStatus("CalifaCrystalCalData*",1);
  eventTree->SetBranchStatus("MCTrack*",1);



  TClonesArray *calCA = new TClonesArray("R3BCalifaCrystalCalData",5);
  TBranch  *calBranch = eventTree->GetBranch("CalifaCrystalCalData");
  calBranch->SetAddress(&calCA);

  TClonesArray *primaryCA = new TClonesArray("R3BMCTrack",5);
  TBranch  *primaryBranch = eventTree->GetBranch("MCTrack");
  primaryBranch->SetAddress(&primaryCA);


  Int_t nEvents = eventTree->GetEntries();

  Int_t nTracks,fCrystalId,nCals;

  TVector3 *myVec;

  Float_t fPx,fPy,fPz,fTheta,fPhi;

  std::vector<angular_coverages> allDistributions(2432);



  // --------------- Event Loop ----------------
  for (Int_t j = 0; j<nEvents; j++) {

    primaryCA->Clear();
    calCA->Clear();

    eventTree->GetEvent(j);
    nTracks = primaryCA->GetEntries();
    nCals = calCA->GetEntries();

    if(!(j%100000))
     cout<<"Reading event "<<j<<" out of "<<nEvents<<" ("<<100.0*Float_t(j)/Float_t(nEvents)<<" % ) "<<endl;


     for(Int_t k =0;k<nCals;k++)
       fCrystalId = ((R3BCalifaCrystalCalData*)calCA->At(k))->GetCrystalId();

      for(Int_t k =0;k<nTracks;k++){
      if(((R3BMCTrack*)primaryCA->At(k))->GetMotherId() < 0 && nCals==1){

        fPx = ((R3BMCTrack*)primaryCA->At(k))->GetPx();
        fPy = ((R3BMCTrack*)primaryCA->At(k))->GetPy();
        fPz = ((R3BMCTrack*)primaryCA->At(k))->GetPz();

        myVec  = new TVector3(fPx,fPy,fPz);


        fTheta = TMath::RadToDeg()*myVec->Theta();
        fPhi = TMath::RadToDeg()*myVec->Phi();

        allDistributions.at(fCrystalId-1).polarDistribution.push_back(fTheta);
        allDistributions.at(fCrystalId-1).azimuthDistribution.push_back(fPhi);

   }
  }
 }

/* --------------- Aux Set of Vectors ----------------*/
std::vector<angular_coverages> auxDistributions(2432);

for(Int_t i =0; i<allDistributions.size();i++){

  auxDistributions.at(i).polarDistribution   = allDistributions.at(i).polarDistribution;
  auxDistributions.at(i).azimuthDistribution = allDistributions.at(i).azimuthDistribution;

  std::sort(auxDistributions.at(i).polarDistribution.begin(),auxDistributions.at(i).polarDistribution.end());
  std::sort(auxDistributions.at(i).azimuthDistribution.begin(),auxDistributions.at(i).azimuthDistribution.end());

}

/* -------------- Creating Histograms ---------------*/


TH2F** all_Histograms;

char name[100];

all_Histograms = new TH2F*[2432];

cout<<"Creating Histograms...."<<endl;

Double_t leftPolarLimit,rightPolarLimit,leftAziLimit,rightAziLimit;

for (Int_t i = 0; i < 2432; i++){
 if(auxDistributions.at(i).polarDistribution.size()>10){

   leftPolarLimit = auxDistributions.at(i).polarDistribution.at(10);
   rightPolarLimit = auxDistributions.at(i).polarDistribution.at(auxDistributions.at(i).polarDistribution.size()-10);

   leftAziLimit = auxDistributions.at(i).azimuthDistribution.at(10);
   rightAziLimit = auxDistributions.at(i).azimuthDistribution.at(auxDistributions.at(i).azimuthDistribution.size()-10);


   sprintf(name, "distributionCrystalID_%i", i + 1);
   all_Histograms[i] = new TH2F(name,name,30,leftAziLimit-0.5,rightAziLimit+0.5,30,leftPolarLimit-0.5,rightPolarLimit+0.5);
   all_Histograms[i]->SetOption("colz");

  }

  else{
    sprintf(name, "distributionCrystalID_%i", i + 1);
    all_Histograms[i] = new TH2F(name,name,30,1,2,30,1,2);
    all_Histograms[i]->SetOption("colz");

  }
}


/* ------------ Writing ------------ */

for (Int_t i = 0; i < 2432; i++){
  if(allDistributions.at(i).polarDistribution.size()>0){

    for(Int_t j=0;j<allDistributions.at(i).polarDistribution.size();j++)
     all_Histograms[i]->Fill(allDistributions.at(i).azimuthDistribution.at(j),allDistributions.at(i).polarDistribution.at(j));

  }
 }


TFile *histogramFile = new TFile("angular_histograms.root","RECREATE");
cout<<"Writing Histograms...."<<endl;



for (Int_t i = 0; i < 2432; i++)
  all_Histograms[i]->Write();

histogramFile->Close();

}
