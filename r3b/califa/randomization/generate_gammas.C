////////////////////////////////////////////////////////////////////////////////
//          This Macro Generates gammas for randomization purposes
//               Recommendation : Generate at least +10M events
//            GGJ (gabrielgarcia.jimenez@usc.es), February 2022
////////////////////////////////////////////////////////////////////////////////


void generate_gammas(Int_t nEvents)
{

    // =========== Configuration area =============================

    TString OutFile = "gammas_s455.root"; // Output file for data

    Bool_t fUserPList = false;      // Use of R3B special physics list
    Bool_t fCalifaDigitizer = true; // Apply hit digitizer task
    Bool_t fCalifaHitFinder = true; // Apply hit finder task

    TString fMC = "TGeant4";       // MonteCarlo engine: TGeant3, TGeant4, TFluka
    TString fGenerator = "box"; // Event generator type: box, gammas, r3b, ion, ascii
    TString fEventFile = "";       // Input event file in the case of ascii generator

    TString fCalifaGeo;
    Int_t fCalifaGeoVer;
    Double_t fCalifaNonU;


    // ------------- Put Here The Geometry For Obtaining Angular Coverages --------------
    fCalifaGeo = "califa_2021_s455.geo.root";
    fCalifaGeoVer = 2021;
    fCalifaNonU = 1.0; // Non-uniformity: 1 means +-1% max deviation

    // --------------- IMPORTANT -----------------
    // If you know the position you can set califa:
    // R3BCalifa* califa = new R3BCalifa(fCalifaGeo, { X, Y, Z});
    // and you have to create first the correct geometry.


    TString dir = gSystem->Getenv("VMCWORKDIR");
    TString r3bdir = dir + "/macros/";
    r3bdir.ReplaceAll("//", "/");

    TString r3b_geomdir = dir + "/geometry/";
    gSystem->Setenv("GEOMPATH", r3b_geomdir.Data());
    r3b_geomdir.ReplaceAll("//", "/");

    TString r3b_confdir = dir + "/gconfig/";
    gSystem->Setenv("CONFIG_DIR", r3b_confdir.Data());
    r3b_confdir.ReplaceAll("//", "/");


    // ----    Debug option   -------------------------------------------------
    gDebug = 0;
    // ------------------------------------------------------------------------

    // -----   Timer   --------------------------------------------------------
    TStopwatch timer;
    timer.Start();
    // ------------------------------------------------------------------------

    // -----   Create simulation run   ----------------------------------------
    FairRunSim* run = new FairRunSim();
    run->SetName(fMC);           // Transport engine
    run->SetOutputFile(OutFile); // Output file

    FairRuntimeDb *rtdb = run->GetRuntimeDb();

    FairParAsciiFileIo *parIo1 = new FairParAsciiFileIo();
    parIo1->open(fSetupFile.Data(), "in");
    rtdb->setFirstInput(parIo1);
    rtdb->print();

    R3BTGeoPar *califaPar = (R3BTGeoPar *)rtdb->getContainer("CalifaGeoPar");

    //  R3B Special Physics List in G4 case
    if ((fUserPList) && (fMC.CompareTo("TGeant4") == 0))
    {
        run->SetUserConfig("g4R3bConfig.C");
        run->SetUserCuts("SetCuts.C");
    }

    // -----   Create media   -------------------------------------------------
    run->SetMaterials("media_r3b.geo"); // Materials

    // -----   Create R3B geometry --------------------------------------------

    // Cave definition
    FairModule* cave = new R3BCave("CAVE");
    cave->SetGeometryFileName("r3b_cave.geo");
    run->AddModule(cave);

      R3BCalifa *califa = new R3BCalifa(fCalifaGeo);
      califa->SelectGeometryVersion(fCalifaGeoVer);
      run->AddModule(califa);




    FairPrimaryGenerator* primGen = new FairPrimaryGenerator();

    // ---------- Define the BOX generator
    Double_t pdgId = 22;
    Double_t theta1 = 0.;
    Double_t theta2 = 180.;
    Double_t momentum = 0.0001; // 100 KeV = No Compton

    FairBoxGenerator* boxGen = new FairBoxGenerator(pdgId, 1);
    boxGen->SetThetaRange(theta1, theta2);
    boxGen->SetPRange(momentum, momentum);
    boxGen->SetPhiRange(0., 360.);
    boxGen->SetXYZ(0.0, 0.0, 0.0);

    primGen->AddGenerator(boxGen);

    run->SetGenerator(primGen);




    FairLogger::GetLogger()->SetLogVerbosityLevel("LOW");

    R3BCalifaDigitizer* califaDig = new R3BCalifaDigitizer();
    califaDig->SetNonUniformity(fCalifaNonU);
    califaDig->SetExpEnergyRes(6.);            // 5. means 5% at 1 MeV
    califaDig->SetComponentRes(6.);
    califaDig->SetDetectionThreshold(0.000010); // in GeV!! 0.000010 means 10 keV

    run->AddTask(califaDig);




    // -----   Initialize simulation run   ------------------------------------
    run->Init();


    // -----   Start run   ----------------------------------------------------
    if (nEvents > 0)
        run->Run(nEvents);

    // -----   Finish   -------------------------------------------------------
    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    cout << endl << endl;
    cout << "Macro finished succesfully." << endl;
    cout << "Output file is " << OutFile << endl;
    cout << "Real time " << rtime << " s, CPU time " << ctime << "s" << endl << endl;
    // ------------------------------------------------------------------------

    cout << " Test passed" << endl;
    cout << " All ok " << endl;
}
