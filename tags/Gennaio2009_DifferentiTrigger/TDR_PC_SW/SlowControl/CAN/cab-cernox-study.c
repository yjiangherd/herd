// file cab-cernox-study.c
//
// Calculate some things for CERNOX sensors
//
// A.Lebedev Jan-2009...

#include "cablib.h"

real R, RR, T;

struct _cernox Table[] = {
  {"X33101", 124,
 {{  4.000, 3.268648E+03},
  {  4.200, 3.069372E+03},
  {  4.400, 2.894588E+03},
  {  4.600, 2.740023E+03},
  {  4.800, 2.602279E+03},
  {  5.000, 2.478786E+03},
  {  5.200, 2.367456E+03},
  {  5.400, 2.266618E+03},
  {  5.600, 2.174864E+03},
  {  5.800, 2.091023E+03},
  {  6.000, 2.014143E+03},
  {  6.500, 1.847212E+03},
  {  7.000, 1.708955E+03},
  {  7.500, 1.592463E+03},
  {  8.000, 1.492945E+03},
  {  8.500, 1.406808E+03},
  {  9.000, 1.331474E+03},
  {  9.500, 1.264939E+03},
  { 10.000, 1.205704E+03},
  { 10.500, 1.152563E+03},
  { 11.000, 1.104588E+03},
  { 11.500, 1.061016E+03},
  { 12.000, 1.021242E+03},
  { 12.500, 9.847571E+02},
  { 13.000, 9.511513E+02},
  { 13.500, 9.200739E+02},
  { 14.000, 8.912353E+02},
  { 14.500, 8.643856E+02},
  { 15.000, 8.393148E+02},
  { 15.500, 8.158397E+02},
  { 16.000, 7.938041E+02},
  { 16.500, 7.730704E+02},
  { 17.000, 7.535199E+02},
  { 17.500, 7.350470E+02},
  { 18.000, 7.175600E+02},
  { 18.500, 7.009765E+02},
  { 19.000, 6.852239E+02},
  { 19.500, 6.702370E+02},
  { 20.000, 6.559586E+02},
  { 21.000, 6.293215E+02},
  { 22.000, 6.049455E+02},
  { 23.000, 5.825656E+02},
  { 24.000, 5.619373E+02},
  { 25.000, 5.428014E+02},
  { 26.000, 5.249879E+02},
  { 27.000, 5.083764E+02},
  { 28.000, 4.928425E+02},
  { 29.000, 4.782787E+02},
  { 30.000, 4.645953E+02},
  { 31.000, 4.517132E+02},
  { 32.000, 4.395618E+02},
  { 33.000, 4.280794E+02},
  { 34.000, 4.172108E+02},
  { 35.000, 4.069078E+02},
  { 36.000, 3.971265E+02},
  { 37.000, 3.878270E+02},
  { 38.000, 3.789743E+02},
  { 39.000, 3.705365E+02},
  { 40.000, 3.624842E+02},
  { 42.000, 3.474348E+02},
  { 44.000, 3.336419E+02},
  { 46.000, 3.209522E+02},
  { 48.000, 3.092366E+02},
  { 50.000, 2.983848E+02},
  { 52.000, 2.883028E+02},
  { 54.000, 2.789111E+02},
  { 56.000, 2.701390E+02},
  { 58.000, 2.619258E+02},
  { 60.000, 2.542192E+02},
  { 65.000, 2.368762E+02},
  { 70.000, 2.218334E+02},
  { 75.000, 2.086522E+02},
  { 77.350, 2.030024E+02},
  { 80.000, 1.969999E+02},
  { 85.000, 1.866189E+02},
  { 90.000, 1.773071E+02},
  { 95.000, 1.689034E+02},
  {100.000, 1.612777E+02},
  {105.000, 1.543212E+02},
  {110.000, 1.479507E+02},
  {115.000, 1.420961E+02},
  {120.000, 1.366945E+02},
  {125.000, 1.316914E+02},
  {130.000, 1.270447E+02},
  {135.000, 1.227185E+02},
  {140.000, 1.186813E+02},
  {145.000, 1.149053E+02},
  {150.000, 1.113668E+02},
  {155.000, 1.080443E+02},
  {160.000, 1.049194E+02},
  {165.000, 1.019752E+02},
  {170.000, 9.919701E+01},
  {175.000, 9.657154E+01},
  {180.000, 9.408696E+01},
  {185.000, 9.173255E+01},
  {190.000, 8.949876E+01},
  {195.000, 8.737685E+01},
  {200.000, 8.535901E+01},
  {205.000, 8.343805E+01},
  {210.000, 8.160753E+01},
  {215.000, 7.986148E+01},
  {220.000, 7.819452E+01},
  {225.000, 7.660168E+01},
  {230.000, 7.507841E+01},
  {235.000, 7.362051E+01},
  {240.000, 7.222415E+01},
  {245.000, 7.088573E+01},
  {250.000, 6.960198E+01},
  {255.000, 6.836984E+01},
  {260.000, 6.718647E+01},
  {265.000, 6.604922E+01},
  {270.000, 6.495565E+01},
  {273.150, 6.428809E+01},
  {275.000, 6.390346E+01},
  {280.000, 6.289052E+01},
  {285.000, 6.191482E+01},
  {290.000, 6.097449E+01},
  {295.000, 6.006778E+01},
  {300.000, 5.919303E+01},
  {305.000, 5.834870E+01},
  {310.000, 5.753334E+01},
  {315.000, 5.674558E+01},
  {320.000, 5.598412E+01},
  {325.000, 5.524775E+01}}}};

//~----------------------------------------------------------------------------

real lint(real x, int i) {

  real x1 = 1.0 / Table[0]._[i-1].R;
  real x2 = 1.0 / Table[0]._[i+1].R;
  real y1 =       Table[0]._[i-1].T;
  real y2 =       Table[0]._[i+1].T;
  
  return (y1 + (y2 - y1) / (x2 - x1) * (x - x1));
}

//~----------------------------------------------------------------------------

real qint1(real x, int i) {

  real x1, x2, x3, y1, y2, y3, A2, A1, A0, D;

  x1 = 1.0 / Table[0]._[i-1].R;
  x2 = 1.0 / Table[0]._[i+1].R;
  x3 = 1.0 / Table[0]._[i+2].R;
  y1 =       Table[0]._[i-1].T;
  y2 =       Table[0]._[i+1].T;
  y3 =       Table[0]._[i+2].T;
  
  D  =   (x2 - x1) * (x3 - x1) * (x2 - x3);  
  A2 =  ((y2 - y1) * (x3 - x1)             - (y3 - y1) * (x2 - x1))              / D;
  A1 = -((y2 - y1) * (x3 - x1) * (x3 + x1) - (y3 - y1) * (x2 - x1) * ( x2 + x1)) / D;
  A0 = y1 - (A2 * x1 + A1) * x1;
  
  return ((A2 * x + A1) * x + A0);
}

//~----------------------------------------------------------------------------

real qint2(real x, int i) {

  real x1, x2, x3, y1, y2, y3, A2, A1, A0, D;

  x1 = 1.0 / Table[0]._[i-2].R;
  x2 = 1.0 / Table[0]._[i-1].R;
  x3 = 1.0 / Table[0]._[i+1].R;
  y1 =       Table[0]._[i-2].T;
  y2 =       Table[0]._[i-1].T;
  y3 =       Table[0]._[i+1].T;
  
  D  =   (x2 - x1) * (x3 - x1) * (x2 - x3);  
  A2 =  ((y2 - y1) * (x3 - x1)             - (y3 - y1) * (x2 - x1))              / D;
  A1 = -((y2 - y1) * (x3 - x1) * (x3 + x1) - (y3 - y1) * (x2 - x1) * ( x2 + x1)) / D;
  A0 = y1 - (A2 * x1 + A1) * x1;
  
  return ((A2 * x + A1) * x + A0);
}

//~----------------------------------------------------------------------------

int main(int argc, char *argv[]) {

  int i, n;
  real lin, quad1, quad2;
  real err2, err3, err4;
  real rms2, rms3, rms4;
  int  n2,   n3,   n4;

  n = Table[0].n;

  rms2 = rms3 = rms4 = 0.0;
  n2   = n3   = n4   = 0;

  printf("--- R -- --1/R -- -- T -- ");
  printf("------ Linear -------- ---- Quadratic 1 ----- ---- Quadratic 2 ----- \n");

  for (i=0; i<n; i++) {
    err2 = err3 = err4 = 0.0;
    R  = Table[0]._[i].R;
    RR = 1.0 / R;
    printf("%8.3Lf %8Lf %7.3Lf ", R, RR, Table[0]._[i].T);

    if ((i > 0) && (i < (n-1))) {
      lin = lint(1.0 / Table[0]._[i].R, i);
      err2 = lin - Table[0]._[i].T;
      rms2 = rms2 + err2 * err2;
      n2++;
      printf("%10Lf (%+8Lf) ", lin, err2);
    }
    else {
      printf("                       ");
    }

    if ((i > 0) && (i < (n-2))) {
      quad1 = qint1(1.0 / Table[0]._[i].R, i);
      err3  = quad1 - Table[0]._[i].T;
      rms3 = rms3 + err3 * err3;
      n3++;
      printf("%10Lf (%+8Lf) ", quad1, err3);
    }
    else {
      printf("                       ");
    }

    if ((i > 1) && (i < (n-1))) {
      quad2 = qint2(1.0 / Table[0]._[i].R, i);
      err4  = quad2 - Table[0]._[i].T;
      rms4 = rms4 + err4 * err4;
      n4++;
      printf("%10Lf (%+8Lf) ", quad2, err4);
    }
    else {
      printf("                       ");
    }
    printf("\n");
  }

  rms2 = sqrtl(rms2/(real)n2);
  rms3 = sqrtl(rms3/(real)n3);
  rms4 = sqrtl(rms4/(real)n4);
  
  printf("                        ");
  printf("         rms = %7Lf", rms2);
  printf("         rms = %7Lf", rms3);
  printf("         rms = %7Lf", rms4);
  printf("\n");

  return 0;
}

//~============================================================================