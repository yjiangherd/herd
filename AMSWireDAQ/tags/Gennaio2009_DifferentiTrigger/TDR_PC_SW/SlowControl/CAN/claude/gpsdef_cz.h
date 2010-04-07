// file gpsdef_cz.h
// Tables for TM 
// Claude Zurbach
// September 2008
//
/**/


typedef struct __tm_param {
   char *nam_value;
   int  param_bgn;
   int  param_end;
   int  param_value;
} _tm_param;

typedef struct __TM_SRECEIV {
   char *nam_param;
   int  treatement;
   _tm_param param[50];
} _TM_SRECEIV;

/**/

_TM_SRECEIV PARAM_SRECEIV[25] = {
/* Description of TM (used during normal operation) */

  {"Validity",                                   0, {
  {NULL}}},

  {"Local Time",                                 0, {
  {NULL}}},

  {"Health status of PEGASE 1 ASIC",             1, {
  {"Channel",                     0, 14, 0,  },
  {"Reserved",                    15, 15, 0, },
  {NULL}}},

  {"Health status of PEGASE 2 ASIC",             0, {
  {NULL}}},

  {"Health status of signal processor",          1, {
  {"CASTOR",                      0, 0, 0,   },
  {"PEGASE 1",                    1, 1, 0,   },
  {"PEGASE 2",                    2, 2, 0,   },
  {"Time Keeper",                 3, 3, 0,   },
  {"Datation/SP",                 4, 4, 0,   },
  {"RFDC 1 to 4",                 5, 8, 0,   },
  {"Time Keeper",                 3, 3, 0,   },
  {NULL}}},

  {"Configuration status of hardware",           2, {
  {"512 Ko RAM",                  0, 1, 0,   },
  {"1 Mo RAM",                    0, 1, 1,   },
  {"1.5 Mo RAM",                  0, 1, 2,   },
  {"2 Mo RAM",                    0, 1, 3,   },
  {"RS422 Interface",             2, 3, 0,   },
  {"MIL 1553 Interface",          2, 3, 1,   },
  {"14 Mhz proc. clock",          4, 4, 0,   },
  {"10 Mhz proc. clock",          4, 4, 1,   },
  {"TCXO clock",                  5, 5, 0,   },
  {"OCXO clock",                  5, 5, 1,   },
  {"1 RFDC & 1 PEGASE",           6, 8, 0,   },
  {"2 RFDC & 1 PEGASE",           6, 8, 2,   },
  {"4 RFDC & 2 PEGASE",           6, 8, 7,   },
  {"No external amplifier",       9, 9, 0,   },
  {"External amplifier",          9, 9, 1,   },
  {"RS422 9600 baud",             10, 11, 0, },
  {"RS422 19200 baud",            10, 11, 1, },
  {"RS422 38400 baud",            10, 11, 2, },
  {"RS422 115200 baud",           10, 11, 3, },
  {"OCXO warm",                   12, 12, 0, },
  {"OCXO not warm",               12, 12, 1, },
  {"Reserved",                    13, 14, 0, },
  {"Start-up without context",    15, 15, 0, },
  {"Start-up with context",       15, 15, 1, },
  {NULL}}},
  
  {"Health status of processor and memory",      1, {
  {"IU",                          0, 1, 0,   },
  {"FPU",                         2, 3, 0,   },
  {"Checksum of code RAM",        4, 5, 0,   },
  {"Reserved",                    6, 7, 0,   },
  {"Checksum of code in FPROM",   8, 9, 0,   },
  {"Reserved",                    10, 15, 0, },
  {NULL}}},

  {"Configuration for localisation",             2, {
  {"Snapshot",                    0, 2, 1,   },
  {"Navigator",                   0, 2, 2,   },
  {"Snapshot+navigator",          0, 2, 3,   },
  {NULL}}},

  {"Configuration for attitude",                 2, {
  {"Without measurements",        0, 1, 0,   },
  {"With measurements",           0, 1, 1,   },
  {NULL}}},

  {"Reserved ALCATEL",                           0, {
  {NULL}}},

  {"Configuration for cold start",               2, {
  {"Normal",                      0, 1, 1,   },
  {"Reduced",                     0, 1, 2,   },
  {NULL}}},
  
  {"Configuration for PPS",                      2, {
  {"PPS free-running",            0, 1, 0,   },
  {"PPS locked",                  0, 1, 1,   },
  {NULL}}},

  {"Y2015 extension",                            2, {
  {"Before January 2015",         0, 1, 0,   },
  {"After  1st January 2017",     0, 1, 1,   },
  {NULL}}},

  {"Configuration for Antenna 1",                2, {
  {"Antenna inactive",            0, 1, 0,   },
  {"Antenna active",              0, 1, 1,   },
  {"Orientation ext. aiding",     2, 5, 1,   },
  {"Orientation Earth",           2, 5, 2,   },
  {"Orientation Anti-Earth",      2, 5, 10,  },
  {"Orientation Sun",             2, 5, 4,   },
  {"Orientation Anti-Sun",        2, 5, 12,  },
  {"GPS Frequency",               6, 7, 0,   },
  {"Antenna priority",            8, 11, 0,  },
  {"Associated master antenna",   12, 15, 0, },
  {NULL}}},

  {"Masking angle Antenna 1",                    0, {
  {NULL}}},
  
  {"Snapshot resolution",                        2, {
  {"Not valid",                   0, 7, 0,   },
  {"Approximate",                 0, 7, 3,   },
  {"Valid",                       0, 7, 255, },
  {NULL}}}, 
	  
  {"N2G resolution",                             2, {
  {"Not valid",                   0, 7, 0,   },
  {"Approximate",                 0, 7, 3,   },
  {"Valid",                       0, 7, 255, },
  {NULL}}},  

  {"Almanach status",                            2, {
  {"Not valid",                   0, 7, 0,   },
  {"Received from RALEPH",        0, 7, 1,   },
  {"Collected from sky",          0, 7, 255, },
  {NULL}}},

  {"Status of satellite selection",              2, {
  {"Selection by navigation",     0, 1, 0,   },
  {"Received from RALEPH",        0, 1, 1,   },
  {"Number satellites tracked",   2, 7, 0,   },
  {NULL}}},

  {"Status of visibility",                       0, {
  {NULL}}},

  {"Snapshot status",                            2, {
  {"Not valid",                   0, 7, 0,   },
  {"Not converged",               0, 7, 1,   },
  {"Converged",                   0, 7, 2,   },
  {"Approximate",                 0, 7, 3,   },
  {"Valid",                       2, 7, 4,   },
  {NULL}}},

  {"N2G status",                            2, {
  {"No error",                    0, 7, 0,   },
  {"Error NMANO",                 0, 7, 1,   },
  {"Error NEMCO",                 0, 7, 2,   },
  {"Error date in NMANO",         0, 7, 3,   },
  {"Error orbital parameters",    0, 7, 4,   },
  {"Error filter divergence",     0, 7, 5,   },
  {"Error NORBIT",                0, 7, 6,   },
  {"Error filter overflow",       0, 7, 7,   },
  {"Error TC stack full",         0, 7, 8,   },
  {"Error internal",              0, 7, 9,   },
  {NULL}}},

  {"Automatic gain control (AGC-1)",             0, {
  {NULL}}},

  {"Number valid TC since RESET or WMODE",       0, {
  {NULL}}},

  {"Number of TC N2G in standby mode",           0, {
  {NULL}}}};
