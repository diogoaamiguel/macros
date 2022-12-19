
#include <iostream>
#include <string>
#include "math.h"
using namespace std;
unsigned long hex2dec(string hex)
{
    unsigned long result = 0;
    for (int i=0; i<hex.length(); i++) {
        if (hex[i]>=48 && hex[i]<=57)
        {
            result += (hex[i]-48)*pow(16,hex.length()-i-1);
        } else if (hex[i]>=65 && hex[i]<=70) {
            result += (hex[i]-55)*pow(16,hex.length( )-i-1);
        } else if (hex[i]>=97 && hex[i]<=102) {
            result += (hex[i]-87)*pow(16,hex.length()-i-1);
        }
    }
    return result;
}


void make_pedestal_file_r3broot(){

   TString c1 = "/home/joseluis/pushcodes/ams2021/sidped_r3broot.txt_";
   TString c2 = "/home/joseluis/pushcodes/ams2021/sidsig_r3broot.txt_";

   // File name
   ifstream *file=new ifstream(c1);
   // Input file 
   if ( ! file->is_open() ) 
    Fatal("R3B","Cannot open input file.");
   else{cout << "Open " << c1 << endl;}
   
   // File name
   ifstream *file2=new ifstream(c2);
   // Input file 
   if ( ! file2->is_open() ) 
    Fatal("R3B","Cannot open input file.");
   else{cout << "Open " << c2 << endl;}
   
   
   FairRuntimeDb* rtdb = FairRuntimeDb::instance();
   
       // Parameters for AMS mapping
    TString dir = gSystem->Getenv("VMCWORKDIR");
    TString amsmapdir = dir + "/macros/macros/r3b/unpack/s455/ams/parameters/";
    TString amsmapfilename = amsmapdir + "AMS_mapping.par";
    amsmapfilename.ReplaceAll("//", "/");
       // AMS mapping
    FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo(); // Ascii
    parIo1->open(amsmapfilename, "in");
    rtdb->setFirstInput(parIo1);
    rtdb->print();
    
    R3BAmsMappingPar *fMap_Par = (R3BAmsMappingPar*)rtdb->getContainer("amsMappingPar");
   
    R3BAmsStripCalPar *fStrip_Par = (R3BAmsStripCalPar*)rtdb->getContainer("amsStripCalPar");

    UInt_t runId = 1;
    rtdb->initContainers(runId);

    //fStrip_Par->printParams();
    
    int numPars=3;
    int fNumDets=6;
    int fNumStrips=1024;
    int fNumStripsS=640;
    int fNumStripsK=384;
    fStrip_Par->SetNumDets(fNumDets);
    fStrip_Par->SetNumStrips(fNumStrips);
    fStrip_Par->SetNumStripsS(fNumStripsS);
    fStrip_Par->SetNumStripsK(fNumStripsK);
    fStrip_Par->GetStripCalParams()->Set(numPars * fNumStrips * fNumDets);
       
   int b,b1,nbstrip;
   double c,s;
   
   for(int d=0;d<6;d++){
   double a;
   double a2;
   *file >> b;
   *file2>> b1;
   if(b!=b1){cout<<"error"<<endl; break;}
   //cout << b << endl;
   for(int ch=0;ch<1024;ch++){/*scanf("%x", &hex );  cout << hex << endl;printf ("You have entered %#x %d.\n",hex, hex);*/
   *file >> a;
   c = a;
   
   *file2 >> a2;
   s = a2;
   
   nbstrip = numPars * ch + d * numPars * fNumStrips;
   if(c>0){
     fStrip_Par->SetStripCalParams(0., nbstrip);      
     fStrip_Par->SetStripCalParams(c, nbstrip + 1);
     fStrip_Par->SetStripCalParams(s, nbstrip + 2);
   }
   else
   {
     fStrip_Par->SetStripCalParams(-1, nbstrip + 1); // dead strip
     fStrip_Par->SetStripCalParams(0, nbstrip + 2);
   }
   
   //printf ("You have entered %#x %d.\n",a,a);
   }

   }
   
       fStrip_Par->setChanged();
    
    //fStrip_Par->printParams();
    
    
      //2-Ascii file with the Calibartion Parameters
  FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
  parIo2->open("Ams_CalPar_20210312.par","out");
  rtdb->setOutput(parIo2);
  
    /* Save parameters (if needed) -------------------------- */
  rtdb->saveOutput();
   
}
