#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class std::pair < int, std::pair < double, double>> + ;

#pragma link C++ class Cluster + ;
#pragma link C++ class RHClass < 1, 24> + ;
//AMS
#pragma link C++ class GenericEvent < 1, 24, 64, 3, 16, 10> + ;
//OCA
#pragma link C++ class GenericEvent < 1, 24, 64, 5, 10, 0> + ;
//FOOT
#pragma link C++ class GenericEvent < 1, 24, 64, 5, 10, 0> + ;

#endif