void califaEventDisplay()
{
    FairRunAna* fRun = new FairRunAna();

    FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
    FairParRootFileIo* parIo1 = new FairParRootFileIo();
    parIo1->open("sim_par.root");
    rtdb->setFirstInput(parIo1);
    rtdb->print();

    fRun->SetSource(new FairFileSource("sim_out.root"));
    fRun->SetOutputFile("test.root");

    R3BEventManager* fMan = new R3BEventManager();
    R3BMCTracks* Track = new R3BMCTracks("Monte-Carlo Tracks");

    R3BCalifaEventDisplay* CalifaEvtVis = new R3BCalifaEventDisplay("R3BCalifaEventDisplay");
    R3BCalifaClusterEventDisplay* CalifaClusterEvtVis = new R3BCalifaClusterEventDisplay("R3BCalifaClusterEventDisplay");
    CalifaEvtVis->SelectGeometryVersion(10);
    fMan->AddTask(CalifaEvtVis);
    fMan->AddTask(CalifaClusterEvtVis);

    fMan->AddTask(Track);

    fMan->Init();
}
