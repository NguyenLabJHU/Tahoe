/*
  File: LatentHardening.h
*/

#ifndef _LATENT_HARDENING_H_
#define _LATENT_HARDENING_H_

#include <iostream>

#include "SlipHardening.h"
#include "dArrayT.h"
#include "dMatrixT.h"

/*TDN: 6/20/2016*/
/*Modification using Aurelie's formulation*/
/*Flow rule: \dot{gamma}^\alpha = \dot{\gamma}_0 sinh[\tau^\alpha/(H(p)*tau^\alpha_0)], where \alpha is the counter for slip system.  Each slip system has a different initial strength \tau^\alpha
    but the same initial flow rate \dot{\gamma}_0.
 Hardening
    \dot{H} = b*(Q-H)*\dot{p}, where \dot{p} = \sqrt(2/3L_p:L_p)*/
namespace Tahoe {

class PolyCrystalMatT;
class LocalCrystalPlast;

class LatentHardening : public SlipHardening
{
 public:
  // constructor
  LatentHardening(PolyCrystalMatT& poly);

  // destructor
  ~LatentHardening();

  // initialize hardening variables at t_0
  virtual void InitializeHardVariables();

  // magnitude of hardening array
  virtual double Magnitude() const;

  // accesor to total number of hardening variables
  virtual const int NumberOfVariables() const;

  // update/reset history of hardening variables
  virtual void UpdateHistory();
  virtual void ResetHistory();

  // fetch hardening variables
  virtual void LoadHardData(int dim, int dex, dArrayT& d_array);

  // solution-related methods
  virtual void ExplicitUpdateHard();            // forward Euler estimate
  virtual void ImplicitUpdateHard();            // implicit estimate
  virtual void ImplicitSolveHard();             // backward Euler estimate
  virtual void FormRHS(const dArrayT& tauIso, dArrayT& rhs);  // residual
  virtual void FormLHS(const dArrayT& tauIso, dMatrixT& lhs); // Jacobian
  virtual bool Converged(double toler);         // check convergence

  virtual void SaveCurrentSolution();
  virtual void RestoreSavedSolution();

  // hardening modulus 
  virtual double HardeningModulus() const;

  // overides accesor to isotropic hardening variable in base class
  virtual const double IsoHardeningStress (int is) const;

  // compute hardening quantities
//  virtual const dArrayT& ComputeHardQnts();

 private:
  // compute some hardening quantities
  void InternalHardQnts();

  // hardening law
  const double HardeningLaw(double crss, int kcode);

 private:
  // hardness saturation level
  double fTauIsoSat;

  // norms of tauIso to check convergence of solution
  double fNormHard0;
  double fNormHard;

  // Isotropic hardening stress at t_n
  dArrayT fTauIso_n; /*\tau^\alpha_n*/
  dArrayT ftauiso_save;

  // some internal variables (accumulated shear rate)
  dArrayT fInternal;

  // workspace
  dArrayT farray;
};

} // namespace Tahoe 
#endif /* _LATENT_HARDENING_H_ */
