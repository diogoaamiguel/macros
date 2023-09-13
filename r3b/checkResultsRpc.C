/////////////////////////////////////////////////////////////////////////////
//			Checks the primary, rpcHits and caloHits characteristics.
//			User configurable for additional studies.
//
//   Usage:
//      > root -l checkResultsRpc.C
//
//     BUT FIRST, select in the //SETTINGS section the simulation features
//	(the macro will plot and text information as a function of these settings)
/////////////////////////////////////////////////////////////////////////////

void checkResultsRpc()
{

    char title1[250];

    // SETTINGS
    char calVersion[50] = "2020"; // Calorimeter version 2020
    Double_t Eproj = 6.00;        // Gamma Energy in projectile frame in MeV
    Int_t totalEvents = 1000;   // Events
    Int_t multiplicity = 1;       // Multiplicity (particles per event)

    Double_t threshold = 0.010; // Threshold in MeV
    Int_t ExpRes = 6;           // Exp. Resol in MeV

    // FOR THE HISTOGRAMS AND PLOTS:
    Double_t maxE = 1.15 * Eproj; // Maximum energy in MeV in the histos

    //FOR THE MOMENTUM CALCULATIONS

    Double_t proton = 1.67262192e-27;
    Double_t deuteron = 2 * proton;
    Double_t alpha = 4 * proton;

    Double_t mass = alpha;

    Double_t c = 299792458;
    Double_t momentumTransformation = 5.3442e-22; //to change between kg.m/s and MeV/C

    Double_t xO = 0;
    Double_t yO = 0;
    Double_t zO = -3;

    sprintf(title1, "%s", "sim_rpc_small_10phi_wt_califa_proton_25dg_400MeV.root");
    TFile* file1 = TFile::Open(title1);

    Double_t beta = 0.8197505718204776;

    // END OF THE SETTING AREA

    //CREATING OUTPUT ROOT FILE NAME

    std::string outputFileName = "analysis_" + std::string(title1);
    const char* outputFileNameCStr = outputFileName.c_str();
    
    TFile* outputFile = new TFile(outputFileNameCStr, "RECREATE");
    if (!outputFile->IsOpen()) {
        std::cout<< "Error: Unable to open the output ROOT file '" << outputFileName << "'." << std::endl;
        return;
    }


    // ----    Debug option   -------------------------------------------------
    gDebug = 0;
    // ------------------------------------------------------------------------

    gROOT->SetStyle("Default");
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(0);

    // HISTOGRAMS DEFINITION  

    // RPC
    TH1F* h1_Rpc = new TH1F("h1_Rpc", "Position X IN", 250, 40, 100);
    TH1F* h6_Rpc = new TH1F("h6_Rpc", "Position Y IN", 200, 0., 50.);
    TH1F* h2_Rpc = new TH1F("h2_Rpc", "Position Z IN", 250, 80, 150);
    TH1F* h3_Rpc = new TH1F("h3_Rpc", "Time", 150, 4., 10.);
    TH1F* h4_Rpc = new TH1F("h4_Rpc", "Energy loss", 100, 0., 0.005);  
    TH1F* h5_Rpc = new TH1F("h5_Rpc", "Momentum in MeV/c", 500, 1500, 10000);    

    // CALIFA Crystal

    TH1F* h1_Cry = new TH1F("h1_Cry", "Energy loss", 200, 0., .25);
    TH1F* h2_Cry = new TH1F("h2_Cry", "Time", 200,0., 5.);
    TH1F* h3_Cry = new TH1F("h3_Cry", "Position X", 200, 0., 50.);
    TH1F* h4_Cry = new TH1F("h4_Cry", "Position Y", 200, 0., 50.);
    TH1F* h5_Cry = new TH1F("h5_Cry", "Position Z", 200, 0., 80.);
    
    // MCTracks
    TH1F* h1_T = new TH1F("h1_T", "Primary PDG Code", 500, 1000020000, 1000020100);
    TH1F* h2_T = new TH1F("h2_T", "Primary Energy (MeV)", 200, 0, 2 * maxE); // Change this maximum energy

    //To make the gaussian fit
    TF1* gaussianFit = new TF1("gaussianFit", "gaus");
    gaussianFit->SetLineColor(kRed);

    TTree* TRpc = (TTree*)file1->Get("evt");

    //RPC Hits (input)
    TClonesArray* rpcHitCA;
    R3BRpcPoint** rpcHit;
    rpcHitCA = new TClonesArray("R3BRpcPoint", 5);
    TBranch* branchRpcHit = TRpc->GetBranch("Rpc1Point");
    branchRpcHit->SetAddress(&rpcHitCA);


    // Crystal Hits (input)
    TClonesArray* crystalHitCA;
    R3BCalifaPoint** crystalHit;
    crystalHitCA = new TClonesArray("R3BCalifaPoint", 5);
    TBranch* branchCrystalHit = TRpc->GetBranch("CrystalPoint");
    branchCrystalHit->SetAddress(&crystalHitCA);

    // Calo Hits (output)
    /*TClonesArray* caloHitCA;
    R3BCalifaClusterData** caloHit;
    caloHitCA = new TClonesArray("R3BCalifaClusterData", 5);
    TBranch* branchCaloHit = TRpc->GetBranch("CalifaClusterData");
    branchCaloHit->SetAddress(&caloHitCA); */

    // MCTrack(input)
    TClonesArray* MCTrackCA;
    R3BMCTrack** track;
    MCTrackCA = new TClonesArray("R3BMCTrack", 5);
    TBranch* branchMCTrack = TRpc->GetBranch("MCTrack");
    branchMCTrack->SetAddress(&MCTrackCA);

    Long64_t nevents = TRpc->GetEntries();
    if (nevents != totalEvents * multiplicity)
        cout << "warn: The number of events (" << nevents << ") in the tree is not totalEvents*multiplicity ("
             << totalEvents << "*" << multiplicity << ") !!" << endl;

    Int_t RpcHitsPerEvent = 0;
    Int_t crystalHitsPerEvent = 0;
    //Int_t caloHitsPerEvent = 0;
    Int_t MCtracksPerEvent = 0;

    Int_t primary = 0;

    TVector3 momentum;
    
    

    for (Int_t i = 0; i < nevents; i++)
    {
        
        if (i % 10000 == 0)
            printf("Event:%i\n", i);
            
        rpcHitCA->Clear();
        crystalHitCA->Clear();
        //caloHitCA->Clear();
        MCTrackCA->Clear();
        TRpc->GetEvent(i);
        
        RpcHitsPerEvent = rpcHitCA->GetEntries();
        crystalHitsPerEvent = crystalHitCA->GetEntries();
        //caloHitsPerEvent = caloHitCA->GetEntries();
        MCtracksPerEvent = MCTrackCA->GetEntries();
        
        if (RpcHitsPerEvent > 0)
        {
            rpcHit = new R3BRpcPoint*[RpcHitsPerEvent];
            for (Int_t j = 0; j < RpcHitsPerEvent; j++)
            {
                rpcHit[j] = new R3BRpcPoint;
                rpcHit[j] = (R3BRpcPoint*)rpcHitCA->At(j);
            }
        }

        if (crystalHitsPerEvent > 0)
        {
            crystalHit = new R3BCalifaPoint*[crystalHitsPerEvent];
            for (Int_t j = 0; j < crystalHitsPerEvent; j++)
            {
                crystalHit[j] = new R3BCalifaPoint;
                crystalHit[j] = (R3BCalifaPoint*)crystalHitCA->At(j);
            }
        }
        /*if (caloHitsPerEvent > 0)
        {
            caloHit = new R3BCalifaClusterData*[caloHitsPerEvent];
            for (Int_t j = 0; j < caloHitsPerEvent; j++)
            {
                caloHit[j] = new R3BCalifaClusterData;
                caloHit[j] = (R3BCalifaClusterData*)caloHitCA->At(j);
            }
        }*/
        if (MCtracksPerEvent > 0)
        {
            track = new R3BMCTrack*[MCtracksPerEvent];
            for (Int_t j = 0; j < MCtracksPerEvent; j++)
            {
                track[j] = new R3BMCTrack;
                track[j] = (R3BMCTrack*)MCTrackCA->At(j);
            }
        }

        // loop in Rpc Hits
        for (Int_t h = 0; h < RpcHitsPerEvent; h++)
        {
            h1_Rpc->Fill(rpcHit[h]->GetXIn());
            std::cout << "X in: " << rpcHit[h]->GetXIn() << std::endl;
            h2_Rpc->Fill(rpcHit[h]->GetZIn());
            std::cout << "Z in: " << rpcHit[h]->GetZIn() << std::endl;
            h3_Rpc->Fill(rpcHit[h]->GetTime());
            std::cout << "Time: " << rpcHit[h]->GetTime() << " ns"<< std::endl;
            h4_Rpc->Fill(rpcHit[h]->GetELoss());
            std::cout << "Eloss: " << rpcHit[h]->GetELoss() << std::endl;
            h6_Rpc->Fill(rpcHit[h]->GetYIn());
            std::cout << "Y in: " << rpcHit[h]->GetYIn() << std::endl;
            
            //momentum
            Double_t dist = sqrt(pow((rpcHit[h]->GetXIn()-xO), 2) + pow((rpcHit[h]->GetYIn()-yO), 2) + pow((rpcHit[h]->GetZIn()-zO), 2)) * pow(10, -2);
            std::cout << "dist: " << dist << std::endl;

            Double_t velocity = dist/((rpcHit[h]->GetTime()) * pow(10, -9));
            std::cout << "velocity: " << velocity << " m/s"<< std::endl;

            Double_t gamma = 1/(sqrt(1-(velocity/c) * (velocity/c)));
            std::cout << "gamma: " << gamma << std::endl;

            Double_t momentum = (mass * velocity * gamma)/momentumTransformation;
            std::cout << "momentum: " << momentum << "MeV/c" <<std::endl;

            h5_Rpc->Fill(momentum);
            //if (rpcHit[h]->GetEnergy() * 1000 > threshold)
            //    h1_Cry_count->Fill(rpcHit[h]->GetCrystalId());
            //h2_Cry->Fill(rpcHit[h]->GetEnergy() * 1000);
        }

        //LOOP IN CRYSTAL HITS
        
        for (Int_t h = 0; h < crystalHitsPerEvent; h++)
        {
            h1_Cry->Fill(crystalHit[h]->GetELoss());
            std::cout << "Califa ELoss: " << crystalHit[h]->GetELoss() << std::endl;
            h2_Cry->Fill(crystalHit[h]->GetTime());
            std::cout << "Califa Time: " << crystalHit[h]->GetTime() << std::endl;
            h3_Cry->Fill(crystalHit[h]->GetXIn());
            std::cout << "Califa X In: " << crystalHit[h]->GetXIn() << std::endl;
            h4_Cry->Fill(crystalHit[h]->GetYIn());
            std::cout << "Califa Y In: " << crystalHit[h]->GetYIn() << std::endl;
            h5_Cry->Fill(crystalHit[h]->GetZIn());
            std::cout << "Califa Z In: " << crystalHit[h]->GetZIn() << std::endl;
        }

        //LOOP IN CALOREMETER HITS
        /*for (Int_t h = 0; h < caloHitsPerEvent; h++)
        {
            h1_Cal->Fill(caloHit[h]->GetNbOfCrystalHits());
            h2_Cal->Fill(caloHit[h]->GetEnergy() * 1000);
            h3_Cal->Fill(caloHit[h]->GetTheta());
            h4_Cal->Fill(caloHit[h]->GetPhi());
            h2_CC->Fill(GetCMEnergy(caloHit[h]->GetTheta(), caloHit[h]->GetEnergy() * 1000, beta));
            h2_CC2->Fill(caloHit[h]->GetTheta(),
                         GetCMEnergy(caloHit[h]->GetTheta(), caloHit[h]->GetEnergy() * 1000, beta));
            if (BARREL && caloHit[h]->GetTheta() > minThetaBarrel * TMath::Pi() / 180.)
            {
                caloHitsPerEvent_barrel++;
                h1_Cal_barrel->Fill(caloHit[h]->GetNbOfCrystalHits());
                h2_Cal_barrel->Fill(caloHit[h]->GetEnergy() * 1000);
                h3_Cal_barrel->Fill(caloHit[h]->GetTheta());
                h4_Cal_barrel->Fill(caloHit[h]->GetPhi());
                h2_CC_barrel->Fill(GetCMEnergy(caloHit[h]->GetTheta(), caloHit[h]->GetEnergy() * 1000, beta));
            }
            if (IPHOS && caloHit[h]->GetTheta() < maxThetaiPhos * TMath::Pi() / 180.)
            {
                caloHitsPerEvent_iPhos++;
                h1_Cal_iPhos->Fill(caloHit[h]->GetNbOfCrystalHits());
                h2_Cal_iPhos->Fill(caloHit[h]->GetEnergy() * 1000);
                h3_Cal_iPhos->Fill(caloHit[h]->GetTheta());
                h4_Cal_iPhos->Fill(caloHit[h]->GetPhi());
                h2_CC_iPhos->Fill(GetCMEnergy(caloHit[h]->GetTheta(), caloHit[h]->GetEnergy() * 1000, beta));
            }
        }*/

        // loop in MC mother tracks
        for (Int_t h = 0; h < MCtracksPerEvent; h++)
        {
            if (track[h]->GetMotherId() < 0)
            {
                track[h]->GetMomentumMass(); //momentum
                h1_T->Fill(track[h]->GetPdgCode());
                h2_T->Fill(track[h]->GetEnergy());// * 1000 - track[h]->GetMass() * 1000
                
                primary++;
            }
        }

        //h1_CryMul->Fill(RpcHitsPerEvent);
        //h1_CalMul->Fill(caloHitsPerEvent);




        primary = 0;

        // User defined additions


        if (RpcHitsPerEvent)
            delete[] rpcHit;
        if (crystalHitsPerEvent)
            delete[] crystalHit;
        //if (caloHitsPerEvent)
        //    delete[] caloHit;
        if (MCtracksPerEvent)
            delete[] track;
    }

    /*TCanvas* c1 = new TCanvas("MCTrack", "MCTrack", 0, 0, 720, 900);
    c1->SetFillColor(0);
    c1->SetFrameFillColor(0);
 
    // MC TRACK CANVAS
    c1->cd();
    c1->Divide(2, 1);
    c1->cd(1);
    h1_T->Draw();
    c1->cd(2);
    h2_T->Draw();
   
    //TLine* line1 = new TLine(0, 0, 0, 0);
    //line1->SetLineStyle(2);
    //line1->Draw();
    
    //c1->cd(4);
    //h4_T->Draw();
    c1->cd();
    //TLatex* title = new TLatex(0.1, 0.2, "Information from the primaries generated by the MC");
    //title->SetTextSize(0.1);
    //title->Draw();
    //c1->cd();*/
  
    //SAVE TO ROOT OUTPUTFILE
    outputFile->cd();
    
    //rpcHits 
    TCanvas* c2 = new TCanvas("rpcHit", "rpcHit", 0, 0, 720, 900);
    c2->SetFillColor(0);
    c2->SetFrameFillColor(0);

    gStyle->SetOptStat(1); // Set the stat box option

    c2->cd();
    c2->Divide(3, 2);

    c2->cd(1);
    h1_Rpc->SetStats(1);
    h1_Rpc->Write();
    h1_Rpc->Fit(gaussianFit, "Q");
    h1_Rpc->Draw();
    gaussianFit->Draw("same");
    
    c2->cd(2);
    h6_Rpc->SetStats(1);
    h6_Rpc->Write();
    h6_Rpc->Fit(gaussianFit, "Q");
    h6_Rpc->Draw();
    gaussianFit->Draw("same");
    
    c2->cd(3);
    h2_Rpc->SetStats(1);
    h2_Rpc->Write();
    h2_Rpc->Fit(gaussianFit, "Q");
    h2_Rpc->Draw();
    gaussianFit->Draw("same");
    
    c2->cd(4);
    h3_Rpc->SetStats(1);
    h3_Rpc->Write();
    h3_Rpc->Fit(gaussianFit);
    h3_Rpc->Draw();
    gaussianFit->Draw("same");
    
    c2->cd(5);
    h4_Rpc->SetStats(1);
    h4_Rpc->Write();
    h4_Rpc->Fit(gaussianFit, "Q");
    h4_Rpc->Draw();
    gaussianFit->Draw("same");
    
    c2->cd(6);
    h5_Rpc->SetStats(1);
    h5_Rpc->Write();
    h5_Rpc->Fit(gaussianFit, "Q");
    h5_Rpc->Draw();
    gaussianFit->Draw("same");
    
    Double_t integral = gaussianFit->Integral(953.5, 956.5);
    std::cout << "Integral = " << integral << std::endl; 
    c2->cd();

    // CRYSTAL HIT CANVAS

    TCanvas* c3 = new TCanvas("crystalHit", "crystalHit", 0, 0, 720, 900);
    c3->SetFillColor(0);
    c3->SetFrameFillColor(0);

    c3->cd();
    c3->Divide(3, 2);
    
    c3->cd(1);
    h1_Cry->SetStats(1);
    h1_Cry->Write();
    h1_Cry->Fit(gaussianFit, "Q");
    h1_Cry->Draw();
    
    c3->cd(2);
    h2_Cry->SetStats(1);
    h2_Cry->Write();
    h2_Cry->Fit(gaussianFit, "Q");
    h2_Cry->Draw();
    
    c3->cd(4);
    h3_Cry->SetStats(1);
    h3_Cry->Write();
    h3_Cry->Fit(gaussianFit, "Q");
    h3_Cry->Draw();
    
    c3->cd(5);
    h4_Cry->SetStats(1);
    h4_Cry->Write();
    h4_Cry->Fit(gaussianFit, "Q");
    h4_Cry->Draw();
    c3->cd();

    c3->cd(6);
    h5_Cry->SetStats(1);
    h5_Cry->Write();
    h5_Cry->Fit(gaussianFit, "Q");
    h5_Cry->Draw();
    c3->cd();

    


    
    // OUTPUT FILE
    //outputFile->Close();
    c2->Print("output.ps");

    // Create histograms inside the ROOT file
    //outputFile->cd();
    //h1_Rpc->Write();
    //h2_Rpc->Write();
    // ... (Write any other histograms you want to save)

    
}
