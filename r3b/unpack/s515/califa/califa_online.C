/*paCh2 Additional info:
 *
 * One needs to set up the ID experiment: s515, the unpacker is:
 *
 * at $UCESB_DIR/../upexps/202104_s515 and $UCESB_DIR/../upexps/202104_s515
 *
 * Before executing the macro, set up the paths to lmd files or to stream server,
 * upexps and califa mapping file "califamapfilename". Also set up the name of file
 * "califacalfilename" that contains the energy calibration parameters of CALIFA.
 *
 * @since April 19th, 2021
 * */

typedef struct EXT_STR_h101_t
{
    EXT_STR_h101_unpack_t unpack;
    EXT_STR_h101_CALIFA_t califa;
    EXT_STR_h101_WRMASTER_t wrm;
    EXT_STR_h101_WRCALIFA_t wrcalifa;
} EXT_STR_h101;

void califa_online()
{
    TStopwatch timer;
    timer.Start();

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");

    const Int_t nev = -1; // number of events to read, -1 - until CTRL+C

    const Int_t expId = 515; // select experiment: s515

    // Create input -----------------------------------------
    // TString filename = "--stream=lxir133.gsi.de:9000";
    TString filename = "--stream=lxlanddaq01.gsi.de:9002";
    // TString filename = "--stream=lxir123.gsi.de:9760";

    // Output file ------------------------------------------
    // TString outputpath = "/tmp/";
    TString outputpath = "/d/land5/202104_s515/rootfiles/califa/";
    // TString outputpath = "/lustre/r3b/202104_s515/rootfiles/";
    TString outputFileName = outputpath + "s515_data_califa_online_" + oss.str() + ".root";
    // TString outputFileName = "test.root";
    Bool_t Cal_level = true;          // set true if there exists a file with the calibration parameters
    Bool_t NOTstoremappeddata = true; // if true, don't store mapped data in the root file
    Bool_t NOTstorecaldata = true;    // if true, don't store cal data in the root file
    Bool_t NOTstorehitdata = true;    // if true, don't store hit data in the root file

    // Online server configuration --------------------------
    Int_t refresh = 1; // Refresh rate for online histograms
    Int_t port = 8887; // Port number for the online visualization, example lxgXXXX:8887

    // Calibration files ------------------------------------
    TString dir = gSystem->Getenv("VMCWORKDIR");
    // CALIFA detector
    // Parameters for CALIFA mapping
    TString califadir = dir + "/macros/r3b/unpack/s515/califa/parameters/";
    TString califamapfilename = califadir + "Califa_Mapping_3March2021.par";
    califamapfilename.ReplaceAll("//", "/");
    // Parameters for CALIFA calibration in keV
    TString califacalfilename = califadir + "Califa_CalPar_4March2021.par";
    califacalfilename.ReplaceAll("//", "/");

    // UCESB configuration ----------------------------------
    TString ntuple_options = "RAW";
    TString ucesb_dir = getenv("UCESB_DIR");
    TString upexps_dir = "/u/land/fake_cvmfs/9.13/upexps";
    // TString upexps_dir = ucesb_dir + "../upexps";
    TString ucesb_path;
    if (expId == 444)
    {
        ucesb_path = upexps_dir + "/202002_s444/202002_s444 --allow-errors --input-buffer=100Mi";
        // ucesb_path = upexps_dir + "/202006_s444/202006_s444 --allow-errors --input-buffer=100Mi";
    }
    else if (expId == 455)
    {
        ucesb_path = upexps_dir + "/202103_s455/202103_s455_part2 --allow-errors --input-buffer=100Mi";
    }
    else if (expId == 467)
    {
        ucesb_path = upexps_dir + "/202002_s467/202002_s467 --allow-errors --input-buffer=100Mi";
    }
    else if (expId == 515)
    {
        ucesb_path = upexps_dir + "/202104_s515/202104_s515 --allow-errors --input-buffer=100Mi";
    }
    else
    {
        std::cout << "Experiment was not selected!" << std::endl;
        gApplication->Terminate();
    }
    ucesb_path.ReplaceAll("//", "/");

    // Load ucesb structure ---------------------------------
    EXT_STR_h101 ucesb_struct;
    
    // Create online run ------------------------------------
    FairRunOnline* run = new FairRunOnline();
    R3BEventHeader* EvntHeader = new R3BEventHeader();
    run->SetEventHeader(EvntHeader);
    run->SetRunId(1);
    run->SetSink(new FairRootFileSink(outputFileName));
    run->ActivateHttpServer(refresh, port);
    
    // Create source using ucesb for input ------------------
    R3BUcesbSource* source =
        new R3BUcesbSource(filename, ntuple_options, ucesb_path, &ucesb_struct, sizeof(ucesb_struct));
    source->SetMaxEvents(nev);

    // Definition of reader ---------------------------------
    R3BUnpackReader* unpackreader =
        new R3BUnpackReader((EXT_STR_h101_unpack*)&ucesb_struct.unpack, offsetof(EXT_STR_h101, unpack));
    R3BCalifaFebexReader* unpackcalifa =
        new R3BCalifaFebexReader((EXT_STR_h101_CALIFA*)&ucesb_struct.califa, offsetof(EXT_STR_h101, califa));

    R3BWhiterabbitMasterReader* unpackWRM =
        new R3BWhiterabbitMasterReader((EXT_STR_h101_WRMASTER*)&ucesb_struct.wrm, offsetof(EXT_STR_h101, wrm), 0x1000);

    R3BWhiterabbitCalifaReader* unpackWRC = new R3BWhiterabbitCalifaReader(
        (EXT_STR_h101_WRCALIFA*)&ucesb_struct.wrcalifa, offsetof(EXT_STR_h101, wrcalifa), 0xa00, 0xb00);

    // Add readers ------------------------------------------
    source->AddReader(unpackreader);
    unpackWRM->SetOnline(NOTstoremappeddata);
    source->AddReader(unpackWRM);
    unpackWRC->SetOnline(NOTstoremappeddata);
    source->AddReader(unpackWRC);
    unpackcalifa->SetOnline(NOTstoremappeddata);
    source->AddReader(unpackcalifa);
    
    run->SetSource(source);

    // Runtime data base ------------------------------------
    FairRuntimeDb* rtdb = run->GetRuntimeDb();

    // Load parameters --------------------------------------
    if (!Cal_level)
    {
        // CALIFA mapping
        FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo(); // Ascii file
        parIo1->open(califamapfilename, "in");
        rtdb->setFirstInput(parIo1);
        rtdb->print();
    }
    else
    {
        // CALIFA mapping and calibration in keV (both parameters should be here)
        // Bool_t kParameterMerged = kFALSE;
        // FairParRootFileIo* parIo2 = new FairParRootFileIo(kParameterMerged); // Root file
        FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo(); // Ascii file
        parIo2->open(califacalfilename, "in");
        rtdb->setFirstInput(parIo2);
        rtdb->print();
    }

    // Create analysis task ---------------------------------
    if (Cal_level)
    {
        // R3BCalifaMapped2CrystalCal ---
        R3BCalifaMapped2CrystalCal* Map2Cal = new R3BCalifaMapped2CrystalCal();
        Map2Cal->SetOnline(NOTstorecaldata);
        run->AddTask(Map2Cal);
        // R3BCalifaCrystalCal2Hit ---
        R3BCalifaCrystalCal2Hit* Cal2Hit = new R3BCalifaCrystalCal2Hit();
        Cal2Hit->SetCrystalThreshold(100.); // 100keV
        Cal2Hit->SetDRThreshold(12000.);    // 12MeV
        Cal2Hit->SelectGeometryVersion(2021);
        Cal2Hit->SetOnline(NOTstorehitdata);
        run->AddTask(Cal2Hit);
    }

    // R3BOnlineSpectra -------------------------------------
    R3BCalifaOnlineSpectra* CalifaOnline = new R3BCalifaOnlineSpectra();
    // CalifaOnline->SetRange_max(500000);//10MeV original *****
    CalifaOnline->SetRange_max(50000); // 50MeV
    CalifaOnline->SetRange_bins(500);
    CalifaOnline->SetBinChannelFebex(500);
    CalifaOnline->SetMaxBinFebex(30000);
    run->AddTask(CalifaOnline);

    // Initialize -------------------------------------------
    run->Init();
    FairLogger::GetLogger()->SetLogScreenLevel("INFO");
    //    FairLogger::GetLogger()->SetLogScreenLevel("WARNING");
    //    FairLogger::GetLogger()->SetLogScreenLevel("DEBUG");

    // Informations about portnumber and main data stream.
    cout << "\n\n" << endl;
    cout << "Data stream is: " << filename << endl;
    cout << "Portnumber for califa online is: " << port << endl;
    cout << "\n\n" << endl;
    // Run --------------------------------------------------
    run->Run((nev < 0) ? nev : 0, (nev < 0) ? 0 : nev);

    // -----   Finish   -------------------------------------
    cout << endl << endl;
    // Extract the maximal used memory an add is as Dart measurement
    // This line is filtered by CTest and the value send to CDash
    FairSystemInfo sysInfo;
    Float_t maxMemory = sysInfo.GetMaxMemory();
    cout << "MaxMemory: ";
    cout << maxMemory << endl;

    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    Float_t cpuUsage = ctime / rtime;
    cout << "CPU used: " << cpuUsage << endl;

    cout << endl;
    std::cout << "Output file is " << outputFileName << std::endl;
    cout << "Real time " << rtime << " s, CPU time " << ctime << "s" << endl << endl;
    cout << "Macro finished successfully." << endl;
    gApplication->Terminate();
}
