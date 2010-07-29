//  $Id: timeid.h,v 1.9 2008/11/14 10:03:54 haino Exp $
#ifndef __AMSTimeID__
#define __AMSTimeID__

#include <time.h>
#include "node.h"
#include "astring.h"
#include <list>


using namespace std;
#ifdef __DARWIN__
struct dirent;
#else
struct dirent64;
#endif
/*! \class AMSTimeID
 \brief Invented to manipulate time or "run" dependent variables : calibration const,
 pedestals etc etc etc.


 Initialization

 For each set of such variables (e.g. array or class instance) 
 the following instance of timeid class should be set up in 
 
 AMSJob::_timeinitjob()
 
 TID.add( new AMSTimeID(AMSID id, tm begin, tm end, int nbytes, void* pdata));
 
 where id is unique identifier (char * part is arbitary string with length < 256,
 integer part is equal to AMSJob::getjobtype()  )
 
 tm begin - start validity time
 
 tm end   - end   validity time
 
 nbytes   - sizeof data in bytes
 
 pdata     - pointer to first element of the data 
             data should consist of 32 bit words ( so no double please) 
             to be portable between bend/lend machines

 tm structure is defined in sys <time.h>

relevant elements:

structure tm{

...

int tm_sec;    // seconds 0-59

int tm_min;    // minutes 0-59

int tm_hour   //  hours   0-23

int tm_mday   //  day in month 1-31

int tm_mon    //  month   0-11

int tm_year   //  year from 1900 (96 for 1996)

...

}



The following methods exist to exchange dbase<->timeid
------------

access :

AMSTimeID *ptid=  AMSJob::gethead()->gettimestructure();       

AMSTimeID * offspring=(AMSTimeID*)ptid->down();  // get first timeid instance

while(offspring){

.........

    offspring=(AMSTimeID*)offspring->next();   // get one by one

}


public methods:

integer UpdateMe()      // update(!=0) or not update dbase

integer GetNbytes()     // get number of bytes

integer CopyOut(void *pdataNew) // copy timeid-> pdatanew

integer CopyIn(void *pdataNew)  // copy pdatanew->timeid

void gettime(time_t & insert, time_t & begin, time_t & end) // get times

void SetTime (time_t insert, time_t begin, time_t end) // set times

methods from AMSNode & AMSID class (getname, getid etc)




DataBase Engine
-----------------

Provides search the relevant record by event time key, i.e.

Max(Insert){Begin < Time < End}



Implementation I 
-----------------

Every record correspond the file with name  getname().{0,1}.uid

     0 - MC

     1 - Data

     uid- unique integer identifier
   
File content:

uinteger array[GetNbytes] | CRC | InsertTime | BeginTime | EndTime

CRC is calculated by _CalcCRC() function

*/


typedef void  (*trigfun_type)(void);

class AMSTimeID: public AMSNode {

private:
  AMSTimeID(const AMSTimeID & o){};

  trigfun_type _trigfun;

public:
  struct iibe{
    uinteger id;
    time_t    insert;
    time_t begin;
    time_t end;
  };
  typedef list<iibe> IBE;
  typedef list<iibe>::iterator IBEI;
  enum CType{Standalone,Client,Server};

protected:
  static uinteger * _Table;
  static AString *  _selectEntry;


  IBE _ibe;
  time_t _Insert;    //! insert time
  time_t _Begin;     //! validity starts
  time_t _End;       //!  validity ends
  integer _Nbytes;   //! Number of bytes in _pData
  uinteger _CRC;     //! Control Sum
  mutable integer _UpdateMe;
  bool _verify;
  uinteger * _pData; //! pointer to data
  integer _DataBaseSize;
  CType _Type;
  uinteger * _pDataBaseEntries[5];  //! Insert Insert Begin End SortedBeg


  void      _init(){};
  uinteger  _CalcCRC();
  void      _convert(uinteger *pdata, integer nw);
  time_t    _stat_adv(const char * dir);
  void      _fillDB(const char * dir,int reenter, bool force=false);
  void      _fillDBaux();
  integer   _getDBRecord(time_t & time, int & index);
  void      _getDefaultEnd(uinteger time, time_t & endt);
  void      _checkcompatibility (const char* dir);
  void      _rewrite(const char * dir, AString & ffile);
  char*     _getsubdirname(time_t time);




  static void   _InitTable();

#ifdef __DARWIN__
  static integer _select(   dirent * entry=0);
  static integer _selectsdir(  dirent * entry=0);
#endif
#ifdef __LINUXGNU__
  static integer _select(  const dirent64 * entry=0);
  static integer _selectsdir(  const dirent64 * entry=0);
#endif



public:
  static const uinteger CRC32;


  AMSTimeID():AMSNode(),_trigfun(0),_Insert(0),_Begin(0),_End(0),_Nbytes(0),
      _CRC(0),_UpdateMe(0),_verify(true),_pData(0),_DataBaseSize(0),_Type(Standalone)
  {for(int i=0;i<5;i++)_pDataBaseEntries[i]=0;}
  
  AMSTimeID(AMSID  id,integer nbytes=0, void* pdata=0,bool verify=true,CType server=Standalone,trigfun_type fun=0):
    AMSNode(id),_Insert(0),_Begin(0),_End(0),_Nbytes(nbytes),_UpdateMe(0),_verify(verify),_pData((uinteger*)pdata),
    _DataBaseSize(0),_Type(server)
  {for(int i=0;i<5;i++)_pDataBaseEntries[i]=0;_CalcCRC();_trigfun=fun;}
  
  AMSTimeID( AMSID  id, tm  begin, tm end, integer nbytes,  void *pdata, CType server, bool verify=true,trigfun_type fun=0);
  
  ~AMSTimeID(){for(int i=0;i<5;i++)delete[] _pDataBaseEntries[i];_trigfun=0;}
  
  integer  GetNbytes() const { return _Nbytes;}
  integer  CopyOut (void *pdataNew) const;
  integer  CopyIn( const void *pdataNew);
  uinteger getCRC()const {return _CRC;}
  void     UpdCRC();
  bool &   Verify() {return _verify;}
  IBE &    findsubtable(time_t begin, time_t end);
  void     checkupdate(const char * name);
  integer& UpdateMe() {return _UpdateMe;}
  void     gettime(time_t & insert, time_t & begin, time_t & end) const;
  void     SetTime (time_t insert, time_t begin, time_t end) ;
  integer  validate(time_t & Time,integer reenter=0);
  bool     updatedb();
  void     rereaddb(bool force=false);
  bool     updatemap(const char *dir,bool slp=false);
  bool     write(const char * dir, int sleep=1);
  bool     read(const char * dir,int run, time_t begin, int index=0);
  integer  readDB(const char * dir, time_t time,integer reenter=0);
  void     fillDB(int length, uinteger * ibe[5]);

#ifdef __CORBA__
  friend class AMSProducer;
#endif

#ifdef __DB__
  void     _fillfromDB();
  integer   readDB(integer reenter=0);
#endif

};

#endif