/* Additional info:
 *
 * at $UCESB_DIR/../upexps/202104_s515
 *
 */

typedef struct EXT_STR_h101_t {
  EXT_STR_h101_unpack_t unpack;
  EXT_STR_h101_CALIFA_t califa;
} EXT_STR_h101;

void califa_calParFinder() {
  TStopwatch timer;
  timer.Start();
  
  const Int_t nev = -1; /* number of events to read, -1 - until CTRL+C */
  const Int_t fRunId = 1;
  Bool_t NOTstoremappeddata = true; // if true, don't store mapped data in the root file
  
  /* Create source using ucesb for input ------------------ */
  //TString filename = "--stream=lxg0898:6002";
  TString filename = "~/s515/Co_lmd/data_0040.lmd";
  TString outputFileName = "./datacalpar_0001.root";

   // Calibration files ------------------------------------
    TString dir = gSystem->Getenv("VMCWORKDIR");
   // Parameters for CALIFA mapping
    TString califadir = dir + "/macros/r3b/unpack/s515/califa/parameters/";
    TString califamapfilename = califadir + "CALIFA_mapping_s515.par";
    califamapfilename.ReplaceAll("//", "/");

  /* Create source using ucesb for input ------------------ */
  //UCESB paths
  TString ntuple_options = "RAW";
  TString ucesb_dir = getenv("UCESB_DIR");
  //TString ucesb_path = ucesb_dir + "/../upexps/202104_s515/202104_s515 --allow-errors --input-buffer=70Mi";
  TString ucesb_path = "/u/land/fake_cvmfs/9.13/upexps/202104_s515/202104_s515 --allow-errors --input-buffer=70Mi --time-stitch=wr,4000";
  ucesb_path.ReplaceAll("//","/");

  EXT_STR_h101 ucesb_struct;
  
  R3BUcesbSource* source = new R3BUcesbSource(filename, ntuple_options,
					      ucesb_path, &ucesb_struct, sizeof(ucesb_struct));
  source->SetMaxEvents(nev);
  
  source->AddReader(new R3BUnpackReader((EXT_STR_h101_unpack*)&ucesb_struct,
					offsetof(EXT_STR_h101, unpack)));
					
    R3BCalifaFebexReader* unpackcalifa = new R3BCalifaFebexReader((EXT_STR_h101_CALIFA*)&ucesb_struct.califa,
					     offsetof(EXT_STR_h101, califa));					
  unpackcalifa->SetOnline(NOTstoremappeddata);					     
  source->AddReader(unpackcalifa);

  
  /* Create online run ------------------------------------ */
  FairRunOnline* run = new FairRunOnline(source);
  run->SetRunId(fRunId);
  run->SetSink(new FairRootFileSink(outputFileName));
  
  /* Runtime data base ------------------------------------ */
  FairRuntimeDb* rtdb = run->GetRuntimeDb();

  FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo(); // Ascii 
  parIo1->open(califamapfilename, "in");
  rtdb->setFirstInput(parIo1);
  rtdb->print();
  
  
  /* Add analysis task ------------------------------------ */
  //R3BCalifaMapped2CrystalCalPar ----
  TArrayF* EnergythePeaks = new TArrayF();
  Float_t e1=1332.5;
  Float_t e2=1173.2;
  EnergythePeaks->Set(2);
  EnergythePeaks->AddAt(e1,0);
  EnergythePeaks->AddAt(e2,1);
  
  R3BCalifaMapped2CrystalCalPar* CalPar = new R3BCalifaMapped2CrystalCalPar();
  CalPar->SetMinStadistics(1000);
  CalPar->SetNumParameterFit(2);//OPTIONAL by default 2
  //Gamma range
  CalPar->SetCalRange_left(700);  
  CalPar->SetCalRange_right(1300);
  CalPar->SetCalRange_bins(100);
  //particle range  
  CalPar->SetCalRangeP_left(60);  
  CalPar->SetCalRangeP_right(130);
  CalPar->SetCalRangeP_bins(50);   
  CalPar->SetSigma(3.0);
  CalPar->SetThreshold(0.0001);      
  CalPar->SetEnergyPeaks(EnergythePeaks);
  run->AddTask(CalPar);


  /* Initialize ------------------------------------------- */
  run->Init();
  //    FairLogger::GetLogger()->SetLogScreenLevel("warn");
  //    FairLogger::GetLogger()->SetLogScreenLevel("debug");
  FairLogger::GetLogger()->SetLogScreenLevel("info");

  
  //Choose Root or Ascii file	
  //1-Root file with the Calibartion Parameters
  /*Bool_t kParameterMerged = kTRUE;
  FairParRootFileIo* parOut = new FairParRootFileIo(kParameterMerged);
  parOut->open("califa_CalibParamFeb2019.root");
  rtdb->setOutput(parOut);
  */
  //2-Ascii file with the Calibartion Parameters
  FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
  parIo2->open("Califa_CalPar_s515_23April2021.par","out");
  rtdb->setOutput(parIo2);


  /* Run -------------------------------------------------- */
  run->Run((nev < 0) ? nev : 0, (nev < 0) ? 0 : nev);


  /* Save parameters (if needed) -------------------------- */
  rtdb->saveOutput();


  /* Finish ----------------------------------------------- */
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  cout << endl << endl;
  cout << "Macro finished succesfully." << endl;
  cout << "Output file is " << outputFileName << endl;
  cout << "Real time " << rtime << " s, CPU time " << ctime << " s"
       << endl << endl;
  gApplication->Terminate();
}
