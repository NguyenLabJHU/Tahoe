#ifdef STRESS_STRAIN

#ifndef STRESS_H
#define STRESS_H

#include "realtypes.h"
#include <iostream>
#include <iomanip>
#include <boost/mpi.hpp>

namespace dem {
  
  // This class is only for parallel gathering and printing, because Eigen cannot be used by Boost C++.
  // Matrix operations should use Eigen in Assembly.cpp
  class Stress {
    
  public:
    Stress();
    void setZero();
    void print(std::ostream &ofs) const;
    
  public:
    REAL coord[3];
    REAL stress[6];           // symmetric
    REAL stressRate[6];       // symmetric

    REAL OldroStressRate[9];  // unsymmetric
    REAL TruesStressRate[9];  // unsymmetric

    REAL deformGradient[9];   // unsymmetric
    REAL rotation[9];         // unsymmetric
    REAL stretch[6];          // symmetric

    REAL velocityGradient[9]; // unsymmetric
    REAL rateOfDeform[6];     // symmetric
    REAL spin[3];             // skew-symmetric

    REAL norm[9];             // norm of above tensors, except for rotation

    REAL stressEigenValue[3];
    REAL stressEigenVector[9];
    REAL stressRateEigenValue[3];
    REAL stressRateEigenVector[9];
    REAL rateOfDeformEigenValue[3];
    REAL rateOfDeformEigenVector[9];
    
    friend class boost::serialization::access;
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
      ar & coord;
      ar & stress;
      ar & stressRate;
      ar & OldroStressRate;
      ar & TruesStressRate;
      ar & deformGradient;
      ar & rotation;
      ar & stretch;
      ar & velocityGradient;
      ar & rateOfDeform;
      ar & spin;
      ar & norm;
      ar & stressEigenValue;
      ar & stressEigenVector;
      ar & stressRateEigenValue;
      ar & stressRateEigenVector;
      ar & rateOfDeformEigenValue;
      ar & rateOfDeformEigenVector;
    }
    
  };
  
} // namespace dem

#endif

#endif
