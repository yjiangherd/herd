// file treedef.h
//
// AMS-02 DAQ Tree Table
//
// A.Lebedev Jul-2008...

//~---------------------------------------------------------------------------- 
 
#ifndef _TREEDEF_H 
#define _TREEDEF_H 

#include "mylib.h"
 
typedef struct {
  char *nam;
  struct _link {
    char *crt;
    char *nam;
    int   adr;
  } lnk[24];
  int   adr;
} _tree;

//~ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

static _tree tree[] = {
  {"JINJ-2",
   {{"T2A", "JINF-T-2-A"}, {"T3A", "JINF-T-3-A"}, 
    {"U1A", "JINF-U-1-A"}, {"T0A", "JINF-T-0-A"}, 
    {"S1A", "SDR2-1-A"  }, {"S1B", "SDR2-1-B"  }, 
    {"S0A", "SDR2-0-A"  }, {"S0B", "SDR2-0-B"  }, 
    {"U0A", "JINF-U-0-A"}, {"T1A", "JINF-T-1-A"}, 
    {"R0A", "JINF-R-0-A"}, {"R1A", "JINF-R-1-A"}, 
    {"E0A", "JINF-E-0-A"}, {"E1A", "JINF-E-1-A"}, 
    {"JTA", "JLV1-A"    }, {"JTB", "JLV1-B"    }, 
    {"T4A", "JINF-T-4-A"}, {"T5A", "JINF-T-5-A"}, 
    {"S2A", "SDR2-2-A"  }, {"S2B", "SDR2-2-B"  }, 
    {"S3A", "SDR2-3-A"  }, {"S3B", "SDR2-3-B"  }, 
    {"T6A", "JINF-T-6-A"}, {"T7A", "JINF-T-7-A"}}},

  {"JINJ-P-A",                                        // this is needed for early files
   {{"T2A", "JINF-T-2-A"}, {"T3A", "JINF-T-3-A"}, 
    {"U1A", "JINF-U-1-A"}, {"T0A", "JINF-T-0-A"}, 
    {"S1A", "SDR2-1-A"  }, {"S1B", "SDR2-1-B"  }, 
    {"S0A", "SDR2-0-A"  }, {"S0B", "SDR2-0-B"  }, 
    {"U0A", "JINF-U-0-A"}, {"T1A", "JINF-T-1-A"}, 
    {"R0A", "JINF-R-0-A"}, {"R1A", "JINF-R-1-A"}, 
    {"E0A", "JINF-E-0-A"}, {"E1A", "JINF-E-1-A"}, 
    {"JTA", "JLV1-A"    }, {"JTB", "JLV1-B"    }, 
    {"T4A", "JINF-T-4-A"}, {"T5A", "JINF-T-5-A"}, 
    {"S2A", "SDR2-2-A"  }, {"S2B", "SDR2-2-B"  }, 
    {"S3A", "SDR2-3-A"  }, {"S3B", "SDR2-3-B"  }, 
    {"T6A", "JINF-T-6-A"}, {"T7A", "JINF-T-7-A"}}},


  {"JINF-E-0",
   {{"00", "EDR-0-0-A"}, {"01", "EDR-0-0-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"04", "EDR-0-1-A"}, {"05", "EDR-0-1-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"08", "EDR-0-2-A"}, {"09", "EDR-0-2-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"0C", "EDR-0-3-A"}, {"0D", "EDR-0-3-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"10", "EDR-0-4-A"}, {"11", "EDR-0-4-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"14", "EDR-0-5-A"}, {"15", "EDR-0-5-B"}, 
    {"16", "ETRG-0-A" }, {"17", "ETRG-0-B"}}},
  {"JINF-E-1",
   {{"00", "EDR-1-0-A"}, {"01", "EDR-1-0-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"04", "EDR-1-1-A"}, {"05", "EDR-1-1-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"08", "EDR-1-2-A"}, {"09", "EDR-1-2-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"0C", "EDR-1-3-A"}, {"0D", "EDR-1-3-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"10", "EDR-1-4-A"}, {"11", "EDR-1-4-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"14", "EDR-1-5-A"}, {"15", "EDR-1-5-B"}, 
    {"16", "ETRG-1-A" }, {"17", "ETRG-1-B"}}},
  {"JINF-R-0-A",
   {{"07", "RDR-0-07"}, {"05", "RDR-0-05"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"06", "RDR-0-06"}, {"09", "RDR-0-09"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"10", "RDR-0-10"}, {"11", "RDR-0-11"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"08", "RDR-0-08"}, {"02", "RDR-0-02"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"01", "RDR-0-01"}, {"00", "RDR-0-00"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"03", "RDR-0-03"}, {"04", "RDR-0-04"}, 
    {"??", NULL      }, {"??", NULL      }}},

  {"JINF-R-0-P",                                       // this is needed for early files
   {{"07", "RDR-0-07"}, {"05", "RDR-0-05"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"06", "RDR-0-06"}, {"09", "RDR-0-09"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"10", "RDR-0-10"}, {"11", "RDR-0-11"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"08", "RDR-0-08"}, {"02", "RDR-0-02"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"01", "RDR-0-01"}, {"00", "RDR-0-00"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"03", "RDR-0-03"}, {"04", "RDR-0-04"}, 
    {"??", NULL      }, {"??", NULL      }}},

  {"JINF-R-0-B",
   {{"07", "RDR-0-07"}, {"11", "RDR-0-11"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"06", "RDR-0-06"}, {"09", "RDR-0-09"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"10", "RDR-0-10"}, {"05", "RDR-0-05"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"08", "RDR-0-08"}, {"02", "RDR-0-02"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"01", "RDR-0-01"}, {"00", "RDR-0-00"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"03", "RDR-0-03"}, {"04", "RDR-0-04"}, 
    {"??", NULL      }, {"??", NULL      }}},

  {"JINF-R-0-S",                                       // this is needed for early files
   {{"07", "RDR-0-07"}, {"11", "RDR-0-11"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"06", "RDR-0-06"}, {"09", "RDR-0-09"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"10", "RDR-0-10"}, {"05", "RDR-0-05"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"08", "RDR-0-08"}, {"02", "RDR-0-02"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"01", "RDR-0-01"}, {"00", "RDR-0-00"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"03", "RDR-0-03"}, {"04", "RDR-0-04"}, 
    {"??", NULL      }, {"??", NULL      }}},

  {"JINF-R-1-A",
   {{"19", "RDR-1-07"}, {"23", "RDR-1-11"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"18", "RDR-1-06"}, {"21", "RDR-1-09"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"22", "RDR-1-10"}, {"17", "RDR-1-05"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"20", "RDR-1-08"}, {"14", "RDR-1-02"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"13", "RDR-1-01"}, {"12", "RDR-1-00"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"15", "RDR-1-03"}, {"16", "RDR-1-04"}, 
    {"??", NULL      }, {"??", NULL      }}},

  {"JINF-R-1-P",                                       // this is needed for early files
   {{"19", "RDR-1-07"}, {"23", "RDR-1-11"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"18", "RDR-1-06"}, {"21", "RDR-1-09"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"22", "RDR-1-10"}, {"17", "RDR-1-05"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"20", "RDR-1-08"}, {"14", "RDR-1-02"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"13", "RDR-1-01"}, {"12", "RDR-1-00"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"15", "RDR-1-03"}, {"16", "RDR-1-04"}, 
    {"??", NULL      }, {"??", NULL      }}},

  {"JINF-R-1-B",
   {{"19", "RDR-1-07"}, {"17", "RDR-1-05"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"18", "RDR-1-06"}, {"21", "RDR-1-09"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"22", "RDR-1-10"}, {"23", "RDR-1-11"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"20", "RDR-1-08"}, {"14", "RDR-1-02"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"13", "RDR-1-01"}, {"12", "RDR-1-00"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"15", "RDR-1-03"}, {"16", "RDR-1-04"}, 
    {"??", NULL      }, {"??", NULL      }}},

  {"JINF-R-1-S",                                       // this is needed for early files
   {{"19", "RDR-1-07"}, {"17", "RDR-1-05"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"18", "RDR-1-06"}, {"21", "RDR-1-09"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"22", "RDR-1-10"}, {"23", "RDR-1-11"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"20", "RDR-1-08"}, {"14", "RDR-1-02"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"13", "RDR-1-01"}, {"12", "RDR-1-00"}, 
    {"??", NULL      }, {"??", NULL      }, 
    {"15", "RDR-1-03"}, {"16", "RDR-1-04"}, 
    {"??", NULL      }, {"??", NULL      }}},

  {"JINF-T-0",
   {{"00", "TDR-0-00-A"}, {"01", "TDR-0-00-B"}, 
    {"02", "TDR-0-01-A"}, {"03", "TDR-0-01-B"}, 
    {"04", "TDR-0-02-A"}, {"05", "TDR-0-02-B"}, 
    {"06", "TDR-0-03-A"}, {"07", "TDR-0-03-B"}, 
    {"08", "TDR-0-04-A"}, {"09", "TDR-0-04-B"}, 
    {"10", "TDR-0-05-A"}, {"11", "TDR-0-05-B"}, 
    {"12", "TDR-0-06-A"}, {"13", "TDR-0-06-B"}, 
    {"14", "TDR-0-07-A"}, {"15", "TDR-0-07-B"}, 
    {"16", "TDR-0-08-A"}, {"17", "TDR-0-08-B"}, 
    {"18", "TDR-0-09-A"}, {"19", "TDR-0-09-B"}, 
    {"20", "TDR-0-10-A"}, {"21", "TDR-0-10-B"}, 
    {"22", "TDR-0-11-A"}, {"23", "TDR-0-11-B"}}}, 
  {"JINF-T-1",
   {{"00", "TDR-1-00-A"}, {"01", "TDR-1-00-B"}, 
    {"02", "TDR-1-01-A"}, {"03", "TDR-1-01-B"}, 
    {"04", "TDR-1-02-A"}, {"05", "TDR-1-02-B"}, 
    {"06", "TDR-1-03-A"}, {"07", "TDR-1-03-B"}, 
    {"08", "TDR-1-04-A"}, {"09", "TDR-1-04-B"}, 
    {"10", "TDR-1-05-A"}, {"11", "TDR-1-05-B"}, 
    {"12", "TDR-1-06-A"}, {"13", "TDR-1-06-B"}, 
    {"14", "TDR-1-07-A"}, {"15", "TDR-1-07-B"}, 
    {"16", "TDR-1-08-A"}, {"17", "TDR-1-08-B"}, 
    {"18", "TDR-1-09-A"}, {"19", "TDR-1-09-B"}, 
    {"20", "TDR-1-10-A"}, {"21", "TDR-1-10-B"}, 
    {"22", "TDR-1-11-A"}, {"23", "TDR-1-11-B"}}}, 
  {"JINF-T-2",
   {{"00", "TDR-2-00-A"}, {"01", "TDR-2-00-B"}, 
    {"02", "TDR-2-01-A"}, {"03", "TDR-2-01-B"}, 
    {"04", "TDR-2-02-A"}, {"05", "TDR-2-02-B"}, 
    {"06", "TDR-2-03-A"}, {"07", "TDR-2-03-B"}, 
    {"08", "TDR-2-04-A"}, {"09", "TDR-2-04-B"}, 
    {"10", "TDR-2-05-A"}, {"11", "TDR-2-05-B"}, 
    {"12", "TDR-2-06-A"}, {"13", "TDR-2-06-B"}, 
    {"14", "TDR-2-07-A"}, {"15", "TDR-2-07-B"}, 
    {"16", "TDR-2-08-A"}, {"17", "TDR-2-08-B"}, 
    {"18", "TDR-2-09-A"}, {"19", "TDR-2-09-B"}, 
    {"20", "TDR-2-10-A"}, {"21", "TDR-2-10-B"}, 
    {"22", "TDR-2-11-A"}, {"23", "TDR-2-11-B"}}}, 
  {"JINF-T-3",
   {{"00", "TDR-3-00-A"}, {"01", "TDR-3-00-B"}, 
    {"02", "TDR-3-01-A"}, {"03", "TDR-3-01-B"}, 
    {"04", "TDR-3-02-A"}, {"05", "TDR-3-02-B"}, 
    {"06", "TDR-3-03-A"}, {"07", "TDR-3-03-B"}, 
    {"08", "TDR-3-04-A"}, {"09", "TDR-3-04-B"}, 
    {"10", "TDR-3-05-A"}, {"11", "TDR-3-05-B"}, 
    {"12", "TDR-3-06-A"}, {"13", "TDR-3-06-B"}, 
    {"14", "TDR-3-07-A"}, {"15", "TDR-3-07-B"}, 
    {"16", "TDR-3-08-A"}, {"17", "TDR-3-08-B"}, 
    {"18", "TDR-3-09-A"}, {"19", "TDR-3-09-B"}, 
    {"20", "TDR-3-10-A"}, {"21", "TDR-3-10-B"}, 
    {"22", "TDR-3-11-A"}, {"23", "TDR-3-11-B"}}}, 
  {"JINF-T-4",
   {{"00", "TDR-4-00-A"}, {"01", "TDR-4-00-B"}, 
    {"02", "TDR-4-01-A"}, {"03", "TDR-4-01-B"}, 
    {"04", "TDR-4-02-A"}, {"05", "TDR-4-02-B"}, 
    {"06", "TDR-4-03-A"}, {"07", "TDR-4-03-B"}, 
    {"08", "TDR-4-04-A"}, {"09", "TDR-4-04-B"}, 
    {"10", "TDR-4-05-A"}, {"11", "TDR-4-05-B"}, 
    {"12", "TDR-4-06-A"}, {"13", "TDR-4-06-B"}, 
    {"14", "TDR-4-07-A"}, {"15", "TDR-4-07-B"}, 
    {"16", "TDR-4-08-A"}, {"17", "TDR-4-08-B"}, 
    {"18", "TDR-4-09-A"}, {"19", "TDR-4-09-B"}, 
    {"20", "TDR-4-10-A"}, {"21", "TDR-4-10-B"}, 
    {"22", "TDR-4-11-A"}, {"23", "TDR-4-11-B"}}}, 
  {"JINF-T-5",
   {{"00", "TDR-5-00-A"}, {"01", "TDR-5-00-B"}, 
    {"02", "TDR-5-01-A"}, {"03", "TDR-5-01-B"}, 
    {"04", "TDR-5-02-A"}, {"05", "TDR-5-02-B"}, 
    {"06", "TDR-5-03-A"}, {"07", "TDR-5-03-B"}, 
    {"08", "TDR-5-04-A"}, {"09", "TDR-5-04-B"}, 
    {"10", "TDR-5-05-A"}, {"11", "TDR-5-05-B"}, 
    {"12", "TDR-5-06-A"}, {"13", "TDR-5-06-B"}, 
    {"14", "TDR-5-07-A"}, {"15", "TDR-5-07-B"}, 
    {"16", "TDR-5-08-A"}, {"17", "TDR-5-08-B"}, 
    {"18", "TDR-5-09-A"}, {"19", "TDR-5-09-B"}, 
    {"20", "TDR-5-10-A"}, {"21", "TDR-5-10-B"}, 
    {"22", "TDR-5-11-A"}, {"23", "TDR-5-11-B"}}}, 
  {"JINF-T-6",
   {{"00", "TDR-6-00-A"}, {"01", "TDR-6-00-B"}, 
    {"02", "TDR-6-01-A"}, {"03", "TDR-6-01-B"}, 
    {"04", "TDR-6-02-A"}, {"05", "TDR-6-02-B"}, 
    {"06", "TDR-6-03-A"}, {"07", "TDR-6-03-B"}, 
    {"08", "TDR-6-04-A"}, {"09", "TDR-6-04-B"}, 
    {"10", "TDR-6-05-A"}, {"11", "TDR-6-05-B"}, 
    {"12", "TDR-6-06-A"}, {"13", "TDR-6-06-B"}, 
    {"14", "TDR-6-07-A"}, {"15", "TDR-6-07-B"}, 
    {"16", "TDR-6-08-A"}, {"17", "TDR-6-08-B"}, 
    {"18", "TDR-6-09-A"}, {"19", "TDR-6-09-B"}, 
    {"20", "TDR-6-10-A"}, {"21", "TDR-6-10-B"}, 
    {"22", "TDR-6-11-A"}, {"23", "TDR-6-11-B"}}}, 
  {"JINF-T-7",
   {{"00", "TDR-7-00-A"}, {"01", "TDR-7-00-B"}, 
    {"02", "TDR-7-01-A"}, {"03", "TDR-7-01-B"}, 
    {"04", "TDR-7-02-A"}, {"05", "TDR-7-02-B"}, 
    {"06", "TDR-7-03-A"}, {"07", "TDR-7-03-B"}, 
    {"08", "TDR-7-04-A"}, {"09", "TDR-7-04-B"}, 
    {"10", "TDR-7-05-A"}, {"11", "TDR-7-05-B"}, 
    {"12", "TDR-7-06-A"}, {"13", "TDR-7-06-B"}, 
    {"14", "TDR-7-07-A"}, {"15", "TDR-7-07-B"}, 
    {"16", "TDR-7-08-A"}, {"17", "TDR-7-08-B"}, 
    {"18", "TDR-7-09-A"}, {"19", "TDR-7-09-B"}, 
    {"20", "TDR-7-10-A"}, {"21", "TDR-7-10-B"}, 
    {"22", "TDR-7-11-A"}, {"23", "TDR-7-11-B"}}}, 
  {"JINF-U-0",
   {{"00", "UDR-0-0-A"}, {"01", "UDR-0-0-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"04", "UDR-0-1-A"}, {"05", "UDR-0-1-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"08", "UDR-0-2-A"}, {"09", "UDR-0-2-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"12", "UDR-0-3-A"}, {"13", "UDR-0-3-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"16", "UDR-0-4-A"}, {"17", "UDR-0-4-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"20", "UDR-0-5-A"}, {"21", "UDR-0-5-B"}, 
    {"??", NULL       }, {"??", NULL       }}},
  {"JINF-U-1",
   {{"00", "UDR-1-0-A"}, {"01", "UDR-1-0-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"04", "UDR-1-1-A"}, {"05", "UDR-1-1-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"08", "UDR-1-2-A"}, {"09", "UDR-1-2-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"12", "UDR-1-3-A"}, {"13", "UDR-1-3-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"16", "UDR-1-4-A"}, {"17", "UDR-1-4-B"}, 
    {"??", NULL       }, {"??", NULL       }, 
    {"20", "UDR-1-5-A"}, {"21", "UDR-1-5-B"}, 
    {"??", NULL       }, {"??", NULL       }}}};

//~ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
#endif // _TREEDEF_H 

//~============================================================================
