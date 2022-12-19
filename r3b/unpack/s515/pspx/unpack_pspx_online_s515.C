/* In order to generate input for this, please go to $UCESB_DIR and run:
 *
 * Additional info:
 * To generate the header file used for the R3BUcesbSource (ext_h101.h), use:
 *
 * $UCESB_DIR/upexps/s438b/s438b --ntuple=UNPACK:TRIGGER,UNPACK:EVENTNO,RAW\
 *     STRUCT_HH,ext_h101.h
 *
 * Put this header file into the 'r3bsource' directory and recompile.
 * */

extern "C"
{
#include "/u/sstorck/R3BRoot/r3bsource/ext_h101_psp.h"
}
typedef struct EXT_STR_h101_t
{
    //  EXT_STR_h101_LOS_onion_t los;
    EXT_STR_h101_unpack_t unpack;
    EXT_STR_h101_PSP_onion_t psp;
} EXT_STR_h101;

/*
 * To run this macro: root -l -b 'unpack_pspx_online.C()'
 * To change the range for the energy plots of the pspx, go to psp/R3BPspxOnlineSpectra.cxx, change the value Emax in
 * Init() to a propriate number, recompile.
 */

void unpack_pspx_online_s515(Int_t RunId = 133)
{
    //    Int_t RunId=100;
    TString runNumber = Form("%d", RunId);
    TStopwatch timer;
    timer.Start();

    const Int_t nev = -1; /* number of events to read, -1 - until CTRL+C */
    // const Int_t nev = 200;
    /* Create source using ucesb for input ------------------ */

    // TString filename = "trans://x86l-99 --allow-errors"; 
    TString filename = "/d/land5/pspx/alpha_2021/pspx2_alpha_run" + runNumber + "_00*.lmd";
    //TString filename = "/lustre/hebe/r3b/201902_s473/main" + runNumber + "_0001.lmd";

    // Eventbuilder
    //   TString outputFileName = "/tmp/pspx_stream_test.root";
    TString outputFileName = "/lustre/land/sstorck/rootfiles/alpha_2021/pspx2_alpha_run" + runNumber + "_online.root";

    TString ntuple_options = "RAW,PSPX";
    // TString ucesb_dir = getenv("UCESB_DIR");

    TString ucesb_path =
	// "/u/land/fake_cvmfs/9.13/upexps/202006_test/202006_test --input-buffer=100Mi";
	// "/u/sstorck/upexps/202006_test/202006_test --input-buffer=100Mi";
	"/u/sstorck/upexps/pspx/pspx --input-buffer=100Mi --allow-errors";

    TString pspxpar_dir = "/u/sstorck/R3BRoot/psp/par/";
    TString parPspxMappedFileName = "default_mapped_1psp.par";
    TString parPspxPrecalFileName = "default_precal_1psp.par";
    TString parPspxCalFileName = "default_cal_1psp.par";
    TString parPspxHitFileName = "default_hit_1psp.par";

    // for data taken with an alpha source during the preparation of the phase0 experiments. only one detector read out
    // with a single filter
    // TString parPspxMappedFileName = "alpha18_pspx_mapped.par";
    // TString parPspxPrecalFileName = Form("alpha18_det%d_pspx_precal.par", detector);
    // TString parPspxCalFileName = Form("alpha18_det%d_pspx_cal.par", detector);
    // TString parPspxHitFileName = "alpha18_pspx_hit_default.par";

    EXT_STR_h101 ucesb_struct;
    R3BUcesbSource* source =
        new R3BUcesbSource(filename, ntuple_options, ucesb_path, &ucesb_struct, sizeof(ucesb_struct));
    source->SetMaxEvents(nev);

    source->AddReader(new R3BUnpackReader((EXT_STR_h101_unpack*)&ucesb_struct.unpack, offsetof(EXT_STR_h101, unpack)));
    source->AddReader(new R3BPspxReader((EXT_STR_h101_PSP*)&ucesb_struct.psp, offsetof(EXT_STR_h101, psp)));

    /* Create online run ------------------------------------ */
    const Int_t refresh = 100; // refresh rate for saving
    FairRunOnline* run = new FairRunOnline(source);
    run->SetRunId(RunId);
    run->ActivateHttpServer(refresh, 1234);

    // To see the canvases open an internet browser and open localhost:8080 on the same machine or machinename:8080 from
    // within the same network

    // Create analysis run ----------------------------------
    // FairRunAna* run = new FairRunAna();
    run->SetOutputFile(outputFileName.Data());

    /* Runtime data base ------------------------------------ */
    FairRuntimeDb* rtdb1 = run->GetRuntimeDb();
    /*
        Bool_t kParameterMerged = kTRUE;
        FairParRootFileIo* parOut1 = new FairParRootFileIo(kParameterMerged);
        TList *parList = new TList();
        parList->Add(new TObjString("tofd_time_params_run057.root"));
        parList->Add(new TObjString("los_params_25-05-16.root"));
        parList->Add(new TObjString("tofd_hit_params_run054.root"));
        parOut1->open(parList);
        rtdb1->setFirstInput(parOut1);
        */

    FairParAsciiFileIo* parInput = new FairParAsciiFileIo();
    TList* parList = new TList();
    parList->Add(new TObjString(pspxpar_dir + parPspxMappedFileName));
    parList->Add(new TObjString(pspxpar_dir + parPspxPrecalFileName));
    parList->Add(new TObjString(pspxpar_dir + parPspxCalFileName));
    parList->Add(new TObjString(pspxpar_dir + parPspxHitFileName));
    parInput->open(parList);
    rtdb1->setFirstInput(parInput);
    rtdb1->print();

    /* Create ALADIN field map ------------------------------ */
    /*    R3BAladinFieldMap* magField = new R3BAladinFieldMap("AladinMaps");
        Double_t fMeasCurrent = 2500.; // I_current [A]
        magField->SetCurrent(fMeasCurrent);
        magField->SetScale(1.);
        run->SetField(magField);
        */

    /* Add analysis task ------------------------------------ */

    R3BPspxMapped2Precal* pspxMapped2Precal = new R3BPspxMapped2Precal("PspxMapped2Precal", 1);
    run->AddTask(pspxMapped2Precal);

    R3BPspxPrecal2Cal* pspxPrecal2Cal = new R3BPspxPrecal2Cal("PspxPrecal2Cal", 1);
    run->AddTask(pspxPrecal2Cal);

    R3BPspxCal2Hit* pspxCal2Hit = new R3BPspxCal2Hit("PspxCal2Hit", 1);
    run->AddTask(pspxCal2Hit);

    /* ------------------------------------------------------ */

    R3BPspxOnlineSpectra* r3bOnlineSpectra = new R3BPspxOnlineSpectra("PspxOnlineSpectra", 1);
    run->AddTask(r3bOnlineSpectra);

    /* Initialize ------------------------------------------- */
    run->Init();
    rtdb1->print();
    //  FairLogger::GetLogger()->SetLogScreenLevel("warn");
    //    FairLogger::GetLogger()->SetLogScreenLevel("debug");
    FairLogger::GetLogger()->SetLogScreenLevel("error");
    // FairLogger::GetLogger()->SetLogScreenLevel("info");

    /* Run -------------------------------------------------- */
    run->Run((nev < 0) ? nev : 0, (nev < 0) ? 0 : nev);
    //    rtdb1->saveOutput();

    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    cout << endl << endl;
    cout << "Macro finished succesfully." << endl;
    cout << "Output file is " << outputFileName << endl;
    cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl << endl;

    gApplication->Terminate();
}
