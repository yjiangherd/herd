#ifndef MainDaq_HH
#define MainDaq_HH

#include "GonClass.hh"
// #include "DaqClass.hh"
// #include "RMSClass.hh"
// #include "SciClass.hh"
// #include "TrigClass.hh"
// #include "SelfClass.hh"


class MainDaq{

private:
  int run;
  int trigtype;


public:

 //  DaqClass* PgDaq0;
//   DaqClass* PgDaq1;
//   RMSClass* RmDaq;
//   SciClass* SciDaq;
//   SelfClass* Self;
  GonClass* Gon;

  //  TrigClass* Trig;

  int CalDone[6];
  
  MainDaq();
  ~MainDaq();
  
  int InitRunNumber();
  int SaveRunNumber(int runnum);
  int StartRun();
  int StopRun();
  int CalibratePG(int crun);
  // int CalibPg0();
  void SetTrigType(int tt){trigtype=tt;}
  int GetTrigType(){return trigtype;}

  int GetRun(){return run;}
  int GetEvents();
// int StopRun();
  
  

};

#endif

