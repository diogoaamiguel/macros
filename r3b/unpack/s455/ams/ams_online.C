/* Additional info:
 *
 * One needs to set up the s455 experiment, the unpackers is:
 *
 * at $UCESB_DIR/../upexps/202104_s455
 *
 * or in the land account at /u/land/lynx.landexp/202104_s455/upexps/202104_s455
 *
 * Before executing the macro, set up the paths to lmd files or to stream server,
 * upexps and ams mapping file "amsmapfilename". Also set up the name of file
 * "amscalfilename" that contains the pedestal parameters of AMS detectos.
 *
 * @since March 1st, 2021
 * */

typedef struct EXT_STR_h101_t
{
    EXT_STR_h101_unpack_t unpack;
    EXT_STR_h101_AMS_onion_t ams;
} EXT_STR_h101;

void ams_online()
{
    TStopwatch timer;
    timer.Start();

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");

    // const Int_t nev = -1; /* number of events to read, -1 - until CTRL+C */
    const Int_t nev = -1; /* number of events to read, -1 - until CTRL+C */

    const Int_t expId = 455; // select experiment

    // Create input -----------------------------------------
    TString filename = "--stream=lxir123:7743";
    // TString filename = "~/lmd/ams_2019-12-11_Ubeam.lmd";

    // Output file ------------------------------------------
    TString outputpath = "/lustre/land/202104_s455/rootfiles/ams/";
    TString outputFileName = outputpath + "s455_data_ams_online_" + oss.str() + ".root";
    Bool_t Cal_level = false;         // set true if there exists a file with the calibration parameters
    Bool_t NOTstoremappeddata = true; // if true, don't store mapped data in the root file
    Bool_t NOTstorecaldata = true;    // if true, don't store cal data in the root file
    Bool_t NOTstorehitdata = true;    // if true, don't store hit data in the root file

    // Online server configuration --------------------------
    Int_t refresh = 1; // Refresh rate for online histograms
    Int_t port = 8886; // Port number for the online visualization, example lxgXXXX:8886

    // Calibration files ------------------------------------
    TString dir = gSystem->Getenv("VMCWORKDIR");
    // AMS detectors
    // Parameters for AMS mapping
    TString amsdir = dir + "/macros/r3b/unpack/s455/ams/parameters/";
    TString amsmapfilename = amsdir + "Ams_mapping.par";
    amsmapfilename.ReplaceAll("//", "/");
    // Parameters for AMS calibration (pedestals)
    TString amscalfilename = amsdir + "Ams_pedestals_1March_2021.par";
    amscalfilename.ReplaceAll("//", "/");

    // UCESB configuration ----------------------------------
    TString ntuple_options = "RAW";
    TString ucesb_dir = getenv("UCESB_DIR");
    TString upexps_dir = ucesb_dir + "/../upexps/";
    TString ucesb_path;
    if (expId == 444)
    {
        // ucesb_path = upexps_dir + "/202002_s444/202002_s444 --allow-errors --input-buffer=100Mi";
        ucesb_path = "/u/land/fake_cvmfs/upexps/202006_s444/202006_s444 --allow-errors --input-buffer=100Mi";
    }
    else if (expId == 455)
    {
        // ucesb_path = upexps_dir + "/202104_s455/202104_s455 --allow-errors --input-buffer=100Mi";
        ucesb_path = "/u/land/fake_cvmfs/9.13/upexps/202104_s455/202104_s455 --allow-errors --input-buffer=100Mi";
    }
    else
    {
        std::cout << "Experiment was not selected!" << std::endl;
        gApplication->Terminate();
    }
    ucesb_path.ReplaceAll("//", "/");

    // Create source using ucesb for input ------------------
    EXT_STR_h101 ucesb_struct;
    R3BUcesbSource* source =
        new R3BUcesbSource(filename, ntuple_options, ucesb_path, &ucesb_struct, sizeof(ucesb_struct));
    source->SetMaxEvents(nev);

    // Definition of reader ---------------------------------
    R3BUnpackReader* unpackreader =
        new R3BUnpackReader((EXT_STR_h101_unpack*)&ucesb_struct, offsetof(EXT_STR_h101, unpack));

    R3BAmsReader* unpackams = new R3BAmsReader((EXT_STR_h101_AMS_onion*)&ucesb_struct.ams, offsetof(EXT_STR_h101, ams));

    // Add readers ------------------------------------------
    source->AddReader(unpackreader);
    unpackams->SetOnline(NOTstoremappeddata);
    source->AddReader(unpackams);

    // Create online run ------------------------------------
    FairRunOnline* run = new FairRunOnline(source);
    run->SetRunId(1);
    run->SetSink(new FairRootFileSink(outputFileName));
    run->ActivateHttpServer(refresh, port);

    // Runtime data base ------------------------------------
    FairRuntimeDb* rtdb = run->GetRuntimeDb();

    // Load parameters --------------------------------------
    if (!Cal_level)
    {
        // AMS mapping
        FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo(); // Ascii file
        parIo1->open(amsmapfilename, "in");
        rtdb->setFirstInput(parIo1);
        rtdb->print();
    }
    else
    {
        // AMS mapping and pedestals (both parameters should be here)
        // Bool_t kParameterMerged = kFALSE;
        FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo(); // Ascii file
        // FairParRootFileIo* parIo2 = new FairParRootFileIo(kParameterMerged); // Root file
        parIo2->open(amscalfilename, "in");
        rtdb->setFirstInput(parIo2);
        rtdb->print();
    }

    // Add analysis task ------------------------------------
    if (Cal_level)
    {
        R3BAmsMapped2StripCal* Map2Cal = new R3BAmsMapped2StripCal();
        Map2Cal->SetThresholdSigma(3);
        Map2Cal->SetOnline(NOTstorecaldata);
        run->AddTask(Map2Cal);

        R3BAmsStripCal2Hit* Cal2Hit = new R3BAmsStripCal2Hit();
        Cal2Hit->SetOnline(NOTstorehitdata);
        Cal2Hit->SetClusterEnergy(15.);
        run->AddTask(Cal2Hit);
    }

    // Add online task --------------------------------------
    R3BAmsOnlineSpectra* online = new R3BAmsOnlineSpectra();
    run->AddTask(online);

    // Initialize -------------------------------------------
    run->Init();
    FairLogger::GetLogger()->SetLogScreenLevel("info");
    // FairLogger::GetLogger()->SetLogScreenLevel("warn");
    // FairLogger::GetLogger()->SetLogScreenLevel("debug");

    // Informations about portnumber and main data stream.
    cout << "\n\n" << endl;
    cout << "Data stream is: " << filename << endl;
    cout << "AMS online port server: " << port << endl;
    cout << "\n\n" << endl;
    //
    // Run --------------------------------------------------
    run->Run((nev < 0) ? nev : 0, (nev < 0) ? 0 : nev);

    // Finish -----------------------------------------------
    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    Float_t cpuUsage = ctime / rtime;
    cout << "CPU used: " << cpuUsage << endl;
    std::cout << std::endl << std::endl;
    std::cout << "Macro finished succesfully." << std::endl;
    std::cout << "Output file is " << outputFileName << std::endl;
    std::cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << std::endl << std::endl;
    gApplication->Terminate();
}
