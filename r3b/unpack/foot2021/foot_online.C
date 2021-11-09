/* Additional info:
 * Author: Jose Luis <joseluis.rodriguez.sanchez@usc.es>
 * @since october 28, 2021
 * */

typedef struct EXT_STR_h101_t
{
    EXT_STR_h101_unpack_t unpack;
    EXT_STR_h101_FOOT_onion_t foot;
} EXT_STR_h101;

void foot_online(const Int_t nev = -1, const Int_t fRunId = 1, const Int_t fExpId = 1)
{
    TString cRunId = Form("%04d", fRunId);
    TString cExpId = Form("%03d", fExpId);

    TStopwatch timer;

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    timer.Start();

    Int_t refresh = 10;
    Int_t port = 8886;

    /* Create source using ucesb for input ------------------ */

    // TString filename = "--stream=lxir123:7803";
    TString filename = "/lustre/r3b/202111_julich/lmd/foot004_*.lmd";
    // TString filename = "~/lmd/julich/foot004_*.lmd";

    TString outputFilename;

    TString outputpath = "./";
    outputFilename = outputpath + "s" + cExpId + "_data_online_" + oss.str() + ".root";
    // outputFilename = "s" + cExpId + "_data_online_" + oss.str() + ".root";

    TString ntuple_options = "RAW";
    TString ucesb_dir = getenv("UCESB_DIR");
    TString ucesb_path = ucesb_dir + "/../upexps/foot/foot --input-buffer=70Mi";
    ucesb_path.ReplaceAll("//", "/");

    // Create source using ucesb for input ------------------
    EXT_STR_h101 ucesb_struct;

    // Create online run ------------------------------------
    R3BEventHeader* EvntHeader = new R3BEventHeader();
    FairRunOnline* run = new FairRunOnline();
    run->SetEventHeader(EvntHeader);
    run->SetRunId(fRunId);
    run->SetSink(new FairRootFileSink(outputFilename));
    run->ActivateHttpServer(refresh, port);

    R3BUcesbSource* source =
        new R3BUcesbSource(filename, ntuple_options, ucesb_path, &ucesb_struct, sizeof(ucesb_struct));
    source->SetMaxEvents(nev);

    /* Definition of reader --------------------------------- */
    R3BUnpackReader* unpackreader =
        new R3BUnpackReader((EXT_STR_h101_unpack*)&ucesb_struct, offsetof(EXT_STR_h101, unpack));

    R3BFootSiReader* unpackfoot =
        new R3BFootSiReader((EXT_STR_h101_FOOT_onion*)&ucesb_struct.foot, offsetof(EXT_STR_h101, foot));

    /* Add readers ------------------------------------------ */
    source->AddReader(unpackreader);
    // unpackfoot->SetOnline(true);
    source->AddReader(unpackfoot);

    run->SetSource(source);

    /* Runtime data base ------------------------------------ */
    FairRuntimeDb* rtdb = run->GetRuntimeDb();

    /* Add analysis task ------------------------------------ */
    R3BFootMapped2StripCal* Map2Cal = new R3BFootMapped2StripCal();
    // Map2Cal->SetOnline(true);
    run->AddTask(Map2Cal);

    /* Add online task ------------------------------------ */
    R3BFootOnlineSpectra* online = new R3BFootOnlineSpectra();
    online->SetNbDet(3);
    online->SetTrigger(1);
    run->AddTask(online);

    /* Initialize ------------------------------------------- */
    run->Init();
    FairLogger::GetLogger()->SetLogScreenLevel("INFO");
    // FairLogger::GetLogger()->SetLogScreenLevel("WARNING");
    // FairLogger::GetLogger()->SetLogScreenLevel("DEBUG");
    /* ------------------------------------------------------ */

    /* Run -------------------------------------------------- */
    run->Run((nev < 0) ? nev : 0, (nev < 0) ? 0 : nev);

    /* Finish ----------------------------------------------- */
    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    std::cout << std::endl << std::endl;
    std::cout << "Macro finished succesfully." << std::endl;
    std::cout << "Output file is " << outputFilename << std::endl;
    std::cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << std::endl << std::endl;
    // gApplication->Terminate();
}
