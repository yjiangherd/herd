#ifndef Event_hh
#define Event_hh

#include "TObject.h"
#include "TClonesArray.h"
#include "Cluster.hh"

#include "LadderConf.hh"

#define NTDRS 24
#define NJINF 1
#define NVAS  16

typedef short int shortint;

/*!  Tracker Event class contains all the information about a Event
 */

class DecodeData;
class RHClass;

class Event: public TObject{
  friend class DecodeData;

public:
  //! Default contructor
  Event();
  //! Default destructor
  ~Event();

  //! Clear the event
  void Clear();

  //! Add an Cluster to the array
  Cluster* AddCluster(int lad,int side);
  //! Get the Cluster in the postion ii of the array
  Cluster* GetCluster(int ii);

  inline int GetNClusTot(){ return NClusTot;};

  static int GetVAS() { return NVAS;};
  static int GetNTDRS() { return NTDRS;};
  static int GetNJINFS() { return NJINF;};
  static double ComputeCN(int size, shortint * RawSignal, float* pede, float* RawSoN, double threshold=3.0);
  
  //  int NGoldenClus(int lad, int side);
  //! Load LadderConf parameter from an ASCII file
  static void ReadLadderConf(TString filename, bool DEBUG=false);
  //! Get Gain Correction "component" for the vanum VA of the tdrnum TDR for the jinfnum JINF
  static LadderConf* GetLadderConf(){ return ladderconf; };
  //! Load Alignment parameter from an ASCII file
  static void ReadAlignment(TString filename, bool DEBUG=false);
  //! Get Alignment parameter "component" for the tdrnum TDR of the jinfnum JINF
  static float GetAlignPar(int jinfnum, int tdrnum, int component);
  static float GetMultiplicityFlip(int jinfnum, int tdrnum);
  //! Load VA Gain Correction from an ASCII file
  static void ReadGainCorrection(TString filename, bool DEBUG=false);
  //! Get Gain Correction "component" for the vanum VA of the tdrnum TDR for the jinfnum JINF
  static float GetGainCorrectionPar(int jinfnum, int tdrnum, int vanum, int component);

  void ExcludeTDRFromTrack(int jinfnum, int tdrnum, int side);//to be called just one, before event loop
  bool FindTrackAndFit(int nptsS, int nptsK, bool verbose=false);
  bool FindHigherChargeTrackAndFit(int nptsS, double threshS, int nptsK, double threshK, bool verbose=false);
  double RefineTrack(double nsigmaS=5.0, int nptsS=3, double nsigmaK=5.0, int nptsK=3, bool verbose=false);
  double GetThetaTrack() { return _theta; };
  double GetPhiTrack() { return _phi; };
  double GetS0Track() { return _S0; };
  double GetK0Track() { return _K0; };
  double GetChiTrack() { return _chisq; };
  double GetChiTrackS() { return _chisqS; };
  double GetChiTrackK() { return _chisqK; };
  double ExtrapolateTrack(double z, int component);
  bool IsClusterUsedInTrack(int index_cluster);
  inline unsigned int GetTrackHitPattern(int side, int jinfnum=0){ return _track_cluster_pattern[jinfnum][side];};//is in binary format! Use std::bitset to show!
  bool IsTDRInTrack(int side, int tdrnum, int jinfnum=0);
  inline std::vector<std::pair<int, std::pair<int, int> > > GetHitVector(){ return _v_trackhit; }
  inline unsigned int GetNHitsTrack(){ return (unsigned int)(_v_trackhit.size()); }
  inline unsigned int GetNHitsSTrack(){ return (unsigned int)(_v_trackS.size()); }
  inline unsigned int GetNHitsKTrack(){ return (unsigned int)(_v_trackK.size()); }
  double GetChargeTrack(int side);

  double GetCalPed_PosNum(int tdrposnum, int channel, int Jinfnum=0);
  double GetCalSigma_PosNum(int tdrposnum, int channel, int Jinfnum=0);
  double GetRawSignal_PosNum(int tdrposnum, int channel, int Jinfnum=0);
  double GetCN_PosNum(int tdrposnum, int va, int Jinfnum=0);
  float GetRawSoN_PosNum(int tdrposnum, int channel, int Jinfnum=0);
  
  double GetCalPed(RHClass* rh, int tdrnum, int channel, int Jinfnum=0);
  double GetCalSigma(RHClass* rh, int tdrnum, int channel, int Jinfnum=0);
  double GetRawSignal(RHClass* rh, int tdrnum, int channel, int Jinfnum=0);
  double GetCN(RHClass* rh, int tdrnum, int va, int Jinfnum=0);
  float GetRawSoN(RHClass* rh, int tdrnum, int channel, int Jinfnum=0);
  
private:
  static bool ladderconfnotread;
  static LadderConf* ladderconf;
  static bool alignmentnotread;
  static float alignpar[NJINF][NTDRS][3];
  static bool multflip[NJINF][NTDRS];

  static bool gaincorrectionnotread;
  static float gaincorrectionpar[NJINF][NTDRS][NVAS][2];

  double CombinatorialFit(
			std::vector<std::pair<int, std::pair<double, double> > > v_cog_laddS[NJINF][NTDRS],
			std::vector<std::pair<int, std::pair<double, double> > > v_cog_laddK[NJINF][NTDRS],
			int ijinf, int itdr,
			std::vector<std::pair<int, std::pair<double, double> > > v_cog_trackS,
			std::vector<std::pair<int, std::pair<double, double> > > v_cog_trackK,
			int nptsS, int nptsK,
			bool verbose=false
			);
  double SingleFit(std::vector<std::pair<int, std::pair<double, double> > > vS,
		   std::vector<std::pair<int, std::pair<double, double> > > vK,
		   std::vector<double>& v_chilayS,
		   std::vector<double>& v_chilayK,
		   double& theta, double& thetaerr,
		   double& phi, double& phierr,
		   double& iDirS, double& iDirSerr,
		   double& iDirK, double& iDirKerr,
		   double& iDirZ, double& iDirZerr,
		   double& mS, double& mSerr,
		   double& mK, double& mKerr,
		   double& s0, double& s0err,
		   double& k0, double& k0err,
		   double& chisqS, double& chisqK,
		   bool verbose=false);
  double SingleFit(std::vector<std::pair<int, std::pair<double, double> > > vS,
		   std::vector<std::pair<int, std::pair<double, double> > > vK,
		   bool verbose=false);
  void AssignAsBestTrackFit();
  void ClearTrack();
  void ClearTrack_sf();

  void StoreTrackClusterPatterns();
  void FillHitVector();

  //! Progressive Event number
  int Evtnum;
  //! Jinj Status
  int JINJStatus;
  //! Jinf Status
  int JINFStatus[NJINF];
  //! Status word for the TDRs (  TDRStatus & 0x1f == TDR ID)
  int TDRStatus[NTDRS];
  //! Common Noise from Calibration
  double CNoise[NTDRS][NVAS];
  //! Cluster number for (side 0(S) 1(K))
  int NClus[NTDRS][2];
  //! Total number of clusters
  int NClusTot;
  //! 0 if there are hits on all the ladders
  int notgood;
  //! (TClones) Array of the recontructed clusters
  TClonesArray *Cls;
  //! (TClones) Array of the recontructed hits

  //! pointer to the data (filled just when reading RAW data) and just for the first 8 TRDs (even when there're more Jinf's)
  //8 since more than 8 raw TDRs cannot be read by a single Jinf
  double     CalSigma[8][1024];   
  double       CalPed[8][1024];
  short int RawSignal[8][1024];
  float        RawSoN[8][1024];//! (do not stream on file! Can be recomputed easily!)

  short int ReadTDR[NTDRS];

  //track parameters and points
  double _chisq;//!
  double _chisqS;//!
  double _chisqK;//!
  double _mS;//! angular coefficient in the S-Z view (mX = iDirS/iDirZ)
  double _mK;//! angular coefficient in the K-Z view (mX = iDirK/iDirZ)
  double _mSerr;//!
  double _mKerr;//!
  double _iDirS;//! cosine director S
  double _iDirK;//! cosine director K
  double _iDirZ;//! cosine director Z (DirS*DirS+DirK*DirK+DirZ*DirZ=1)
  double _iDirSerr;//!
  double _iDirKerr;//!
  double _iDirZerr;//!
  double _theta;//!
  double _phi;//!
  double _thetaerr;//!
  double _phierr;//!
  double _S0;//!
  double _K0;//!
  double _S0err;//!
  double _K0err;//!
  std::vector<std::pair<int, std::pair<double, double> > > _v_trackS;//!
  std::vector<std::pair<int, std::pair<double, double> > > _v_trackK;//!
  std::vector<double> _v_chilayS;//!
  std::vector<double> _v_chilayK;//!
  //! filled by FillHitVector(). Here the int is the ladder number and the second pair is <cluster index S, cluster index K>
  std::vector<std::pair<int, std::pair<int, int> > > _v_trackhit;//!
  //! filled by StoreTrackClusterPatterns()
  unsigned int _track_cluster_pattern[NJINF][2];//!

  // int is jinfnum*100+tdrnum
  std::vector<int> _v_ladderS_to_ignore;//!
  std::vector<int> _v_ladderK_to_ignore;//!

  ClassDef(Event,4)
};

//! Run Header Class
class RHClass: public TObject{

private:
  //! Run number
  int Run;
  char date[30];
  int nJinf;
  int JinfMap[NJINF];
  int ntdrRaw;
  int tdrRawMap[NTDRS];//is wrong! ot has another index, for the Jinf, or must be NJINF*NTDRS
  int ntdrCmp;
  int tdrCmpMap[NTDRS];//is wrong! ot has another index, for the Jinf, or must be NJINF*NTDRS
  double CNMean[NTDRS][NVAS];//is wrong! ot has another index, for the Jinf, or must be NJINF*NTDRS
  double CNSigma[NTDRS][NVAS];//is wrong! ot has another index, for the Jinf, or must be NJINF*NTDRS

public:
  //! default constructor
  RHClass();
  //! default destructor
  virtual ~RHClass(){};
  //! Prints the Header infos
  void Print();

  inline void SetRun(int _run) { Run=_run; return;}
  inline int GetRun() { return Run;}

  inline void SetDate(char* _date) { snprintf(date, 30, "%s", _date); return;}
  inline const char* GetDate() { return date;}
  
  inline int GetNJinfs() { return nJinf; }
  inline void SetNJinfs(int _nJinf) { nJinf=_nJinf; return; }
    
  inline int GetNTdrs() { return ntdrRaw+ntdrCmp; }
  inline int GetNTdrsCmp() { return ntdrCmp; }
  inline int GetNTdrsRaw() { return ntdrRaw; }
  inline void SetNTdrsCmp(int _nTdrCmp) { ntdrCmp=_nTdrCmp; return; }
  inline void SetNTdrsRaw(int _nTdrRaw) { ntdrRaw=_nTdrRaw; return; }

  void SetJinfMap(int* _JinfMap);
  void SetTdrRawMap(int* _TdrRawMap);
  void SetTdrCmpMap(int* _TdrCmpMap);
  
  int FindLadderNumCmp(int tdrpos);
  int FindLadderNumRaw(int tdrpos);
  
  int FindPosCmp(int tdrnum);
  int FindPosRaw(int tdrnum);

  ClassDef(RHClass,2)
};


#endif
