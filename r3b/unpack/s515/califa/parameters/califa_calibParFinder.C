/* Additional info:
 *
 * One needs to set up the 2021 experiment s515, unpackers is:
 *
 * at $UCESB_DIR/../upexps/202104_s515 and $UCESB_DIR/../upexps/202104_s515
 *
 * Before executing the macro, set up the paths to lmd files, upexps and
 * califa mapping file "califamapfilename". Also set up the name of output
 * file "outputCalFile"
 *
 * @since April 19th, 2021
 */

typedef struct EXT_STR_h101_t
{
    EXT_STR_h101_unpack_t unpack;
    EXT_STR_h101_CALIFA_t califa;
} EXT_STR_h101;

void califa_calibParFinder()
{
    TStopwatch timer;
    timer.Start();

    const Int_t nev = -1; /* number of events to read, -1 - until CTRL+C */

    /* Create source using ucesb for input ------------------ */
    TString filename = "/u/land/califa/data/calib_March_Mixed_21/_000*.lmd";
    TString outputFileName = "calib_Results_March21.root";

    /* Create source using ucesb for input ------------------ */
    // UCESB paths
    TString ntuple_options = "RAW";
    TString ucesb_dir = getenv("UCESB_DIR");
    // TString upexps_dir = ucesb_dir + "../upexps";
    TString upexps_dir = "/u/land/fake_cvmfs/9.13/upexps/202104_s515/";

    TString ucesb_path;
    ucesb_path = upexps_dir + "/202104_s515 --allow-errors --input-buffer=100Mi";
    ucesb_path.ReplaceAll("//", "/");

    // Parameters for CALIFA
    TString dir = gSystem->Getenv("VMCWORKDIR");
    TString califamapdir = dir + "/macros/r3b/unpack/s515/califa/parameters/";
    TString califamapfilename = califamapdir + "CALIFA_mapping_s515.par";
    califamapfilename.ReplaceAll("//", "/");

    // CALIFA output file with parameters for calibrating in keV
    TString outputCalFile = "Califa_CalPar_s515_23April2021.par";

    /* Definition of reader --------------------------------- */
    EXT_STR_h101 ucesb_struct;
    
    /* Create online run ------------------------------------ */
    FairRunOnline* run = new FairRunOnline();
    run->SetRunId(1);
    run->SetSink(new FairRootFileSink(outputFileName));

    R3BUcesbSource* source =
        new R3BUcesbSource(filename, ntuple_options, ucesb_path, &ucesb_struct, sizeof(ucesb_struct));
    source->SetMaxEvents(nev);

    source->AddReader(new R3BUnpackReader((EXT_STR_h101_unpack*)&ucesb_struct, offsetof(EXT_STR_h101, unpack)));
    source->AddReader(
        new R3BCalifaFebexReader((EXT_STR_h101_CALIFA*)&ucesb_struct.califa, offsetof(EXT_STR_h101, califa)));

    run->SetSource(source);

    /* Runtime data base ------------------------------------ */
     FairRuntimeDb* rtdb = run->GetRuntimeDb();

    // CALIFA mapping
    FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo(); // Ascii
    parIo1->open(califamapfilename, "in");
    rtdb->setFirstInput(parIo1);
    rtdb->print();
	   
    /* Add analysis task ------------------------------------ */

    // R3BCalifaMapped2CrystalCalPar ----
    TArrayF* EnergythePeaks = new TArrayF();
    Float_t e1 = 1332.5;
    Float_t e2 = 1173.2;
    EnergythePeaks->Set(2);
    EnergythePeaks->AddAt(e1, 0);
    EnergythePeaks->AddAt(e2, 1);

    R3BCalifaMapped2CrystalCalPar* CalPar = new R3BCalifaMapped2CrystalCalPar();
    CalPar->SetMinStadistics(1000);
    CalPar->SetNumParameterFit(2); // OPTIONAL by default 2
    // Gamma range
    CalPar->SetCalRange_left(750);
    CalPar->SetCalRange_right(1300);
    CalPar->SetCalRange_bins(130);
    // particle range
    CalPar->SetCalRangeP_left(80);
    CalPar->SetCalRangeP_right(130);
    CalPar->SetCalRangeP_bins(100);
    CalPar->SetSigma(3.0);
    CalPar->SetThreshold(0.0001);
    CalPar->SetEnergyPeaks(EnergythePeaks);
    CalPar->SetDebugMode(1);
    run->AddTask(CalPar);

    /* Initialize ------------------------------------------- */
    run->Init();
    //    FairLogger::GetLogger()->SetLogScreenLevel("warn");
    //    FairLogger::GetLogger()->SetLogScreenLevel("debug");
    FairLogger::GetLogger()->SetLogScreenLevel("info");

    // Choose Root or Ascii file
    // 1-Root file with the Calibration Parameters
    /*Bool_t kParameterMerged = kTRUE;
    FairParRootFileIo* parOut = new FairParRootFileIo(kParameterMerged);
    parOut->open(outputCalFile);
    rtdb->setOutput(parOut);*/
    // 2-Ascii file with the Calibration Parameters
    FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo();
    parIo1->open(outputCalFile,"out");
    rtdb->setOutput(parIo1);
     
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
    cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl << endl;
    gApplication->Terminate();
}
