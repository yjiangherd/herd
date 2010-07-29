//////////////////////////////////////////////////////////
//   This class has been generated by TFile::MakeProject
//     (Tue Oct 15 13:48:19 2002 by ROOT version 3.03/06)
//      from the StreamerInfo in file prv3.root
//////////////////////////////////////////////////////////


#ifndef AMSEventHeaderRoot_h
#define AMSEventHeaderRoot_h

#include "TObject.h"

class AMSEventHeaderRoot : public TObject {

public:
   int         _Run;              //
   int         _RunType;          //
   int         _Eventno;          //
   int         _Time[2];          //
   int         _EventStatus[2];   //
   int         _RawWords;         //
   float       _StationRad;       //cm
   float       _StationTheta;     //
   float       _StationPhi;       //
   float       _NorthPolePhi;     //
   float       _Yaw;              //
   float       _Pitch;            //
   float       _Roll;             //
   float       _StationSpeed;     //
   float       _SunRad;           //
   float       _VelTheta;         //
   float       _VelPhi;           //
   int         Tracks;            //
   int         TrRecHits;         //
   int         TrClusters;        //
   int         TrRawClusters;     //
   int         TrMCClusters;      //
   int         TOFClusters;       //
   int         TOFMCClusters;     //
   int         AntiMCClusters;    //
   int         TRDMCClusters;     //
   int         AntiClusters;      //
   int         EcalClusters;      //
   int         EcalHits;          //
   int         RICMCClusters;     //CJM
   int         RICHits;           //CJM
   int         TRDRawHits;        //
   int         TRDClusters;       //
   int         TRDSegments;       //
   int         TRDTracks;         //

   AMSEventHeaderRoot();
   virtual ~AMSEventHeaderRoot();

   ClassDef(AMSEventHeaderRoot,1) //
};

#endif