/*
  File: PowerLawIKinetics.h
*/

#ifndef _POWER_LAW_I_KINETICS_H_
#define _POWER_LAW_I_KINETICS_H_

#include "SlipKinetics.h"

class PolyCrystalMatT;

class PowerLawIKinetics: public SlipKinetics
{
 public:
  // constructor
  PowerLawIKinetics(PolyCrystalMatT& poly);

  // destructor
  ~PowerLawIKinetics();

  // power law equation and its derivatives
  virtual double Phi(double tau, int is);
  virtual double DPhiDTau(double tau, int is);
  virtual double DPhiDIso(double tau, int is);
  virtual double DPhiDKin(double tau, int is);

  // inverse of power law equation and its derivatives
  virtual double Psi(double gamdot, int is);
  virtual double DPsiDGamdot(double gamdot, int is);
  virtual double DPsiDIso(double gamdot, int is);
  virtual double DPsiDKin(double gamdot, int is);

  // print kinetic equation data and model name
  virtual void Print(ostream& out) const;
  virtual void PrintName(ostream& out) const;

  // continuation method using rate sensitivity exponent
  virtual void SetUpRateSensitivity();
  virtual void ComputeRateSensitivity();
  virtual bool IsMaxRateSensitivity();

 private:

  // variables used in setting up continuation method
  double fxm;
  double fk;
  double fkmax;
};

#endif  /* _POWER_LAW_I_KINETICS_H_ */