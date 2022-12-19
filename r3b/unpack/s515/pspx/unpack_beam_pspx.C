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
//#include "/home/bloeher/git/R3BRoot/r3bsource/ext_h101_full.h"
//#include "ext_data_client.h"
#include "/u/sstorck/R3BRoot/r3bsource/ext_h101_psp.h"
    //#include "/home/land/R3BRoot_PSPX/r3bsource/ext_h101_los.h"
}

typedef struct EXT_STR_h101_t
{
    // EXT_STR_h101_LOS_onion_t los;
    EXT_STR_h101_unpack_t unpack;
    EXT_STR_h101_PSP_onion_t psp;
} EXT_STR_h101;

void unpack_beam_pspx(Int_t RunId = 1)
{
    TString runNumber = Form("%04d", RunId);
    TStopwatch timer;
    timer.Start();

    const Int_t nev = -1; /* number of events to read, -1 - until CTRL+C */
    // const Int_t nev = 20000;

    /* Create source using ucesb for input ------------------ */

//	 TString filename="trans://x86l-30 --allow-errors";
    TString filename = "/d/land1/202103_s455/lmd/pspx_run6_notrace_longfilter_0001.lmd";

    TString outputFileName =
        Form("/u/sstorck/pspx_2433-21_Pb_run%i.root", RunId); 

    TString ntuple_options = "RAW,PSPX";

    TString ucesb_path =
        //"/u/land/fake_cvmfs/9.13/upexps/201902_s473/201902_s473 --input-buffer=100Mi"; // official unpacker for Sn
        // "/u/land/fake_cvmfs/9.13/upexps/202006_test/202006_test --input-buffer=100Mi"; // official unpacker for 202006_test
        "/u/sstorck/upexps/202006_test/202006_test --input-buffer=100Mi"; // 

    TString pspxpar_dir = "/u/sstorck/R3BRoot/psp/par/";
    TString parPspxMappedFileName = "default_mapped_1psp.par";
    // TString parPspxPrecalFileName = "default_precal_3psps.par";
    TString parPspxPrecalFileName = "default_precal_1psp.par";
    TString parPspxCalFileName = "default_cal_1psp.par";
    // TString parPspxCalFileName = "default_cal_3psps.par";
    TString parPspxHitFileName = "default_hit_1psp.par";

    EXT_STR_h101 ucesb_struct;
    R3BUcesbSource* source =
        new R3BUcesbSource(filename, ntuple_options, ucesb_path, &ucesb_struct, sizeof(ucesb_struct));
    source->SetMaxEvents(nev);

    source->AddReader(new R3BUnpackReader((EXT_STR_h101_unpack*)&ucesb_struct.unpack, offsetof(EXT_STR_h101, unpack)));
    //  source->AddReader( new R3BLosReader ((EXT_STR_h101_LOS*)&ucesb_struct.los, offsetof(EXT_STR_h101, los)) );
    source->AddReader(new R3BPspxReader((EXT_STR_h101_PSP*)&ucesb_struct.psp, offsetof(EXT_STR_h101, psp)));

    /* Create online run ------------------------------------ */
    // const Int_t refresh = 100;
    FairRunOnline* run = new FairRunOnline(source);
    run->SetRunId(01);

    // Create analysis run ----------------------------------
    // FairRunAna* run = new FairRunAna();
    // run->SetSource(new FairFileSource(filename)); //necessary for rootfile
    // run->SetInputFile(filename);

    run->SetOutputFile(outputFileName.Data());

    /* Runtime data base ------------------------------------ */
    FairRuntimeDb* rtdb1 = run->GetRuntimeDb();

    FairParAsciiFileIo* parInput = new FairParAsciiFileIo();
    TList* parList = new TList();
    parList->Add(new TObjString(pspxpar_dir + parPspxMappedFileName));
    parList->Add(new TObjString(pspxpar_dir + parPspxPrecalFileName));
    parList->Add(new TObjString(pspxpar_dir + parPspxCalFileName));
    parList->Add(new TObjString(pspxpar_dir + parPspxHitFileName));
    parInput->open(parList);
    rtdb1->setFirstInput(parInput);
    rtdb1->print();

    /* Add analysis task ------------------------------------ */

   R3BPspxMapped2Precal* pspxMapped2Precal = new R3BPspxMapped2Precal("PspxMapped2Precal", 1);
   run->AddTask(pspxMapped2Precal);
   R3BPspxPrecal2Cal* pspxPrecal2Cal = new R3BPspxPrecal2Cal("PspxPrecal2Cal", 1);
   run->AddTask(pspxPrecal2Cal);
   R3BPspxCal2Hit* pspxCal2Hit = new R3BPspxCal2Hit("PspxCal2Hit", 1);
   run->AddTask(pspxCal2Hit);

    /* Initialize ------------------------------------------- */
    // std::cout << "TEST  ****************************************** " << endl;
    ((R3BPspxPrecalPar*)FairRuntimeDb::instance()->getContainer("R3BPspxPrecalPar")) /*->printparams()*/;
    run->Init();
    rtdb1->print();
    //    FairLogger::GetLogger()->SetLogScreenLevel("warn");
    //    FairLogger::GetLogger()->SetLogScreenLevel("debug");
    FairLogger::GetLogger()->SetLogScreenLevel("info");

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
