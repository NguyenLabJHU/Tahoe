/*
  File: HaasenKinetics.h
*/

#ifndef _HAASEN_KINETICS_H_
#define _HAASEN_KINETICS_H_

#include "SlipKinetics.h"

class PolyCrystalMatT;

class HaasenKinetics: public SlipKinetics
{
 public:

  // constructor
  HaasenKinetics(PolyCrystalMatT& poly);

  // destructor
  ~HaasenKinetics();

  // Haasen power law equation and its derivatives
  virtual double Phi(double tau, int is);
  virtual double DPhiDTau(double tau, int is);
  virtual double DPhiDIso(double tau, int is);
  virtual double DPhiDKin(double tau, int is);

  // inverse of Haasen power law equation and its derivatives
  virtual double Psi(double gamdot, int is);
  virtual double DPsiDGamdot(double gamdot, int is);
  virtual double DPsiDIso(double gamdot, int is);
  virtual double DPsiDKin(double gamdot, int is);

  // print kinetic equation data and model name
  virtual void Print(ostream& out) const;
  virtual void PrintName(ostream& out) const;

 private:
  // compute preliminary quantities
  double ComputeInternalQnts(double& tau, const int is);
};

#endif  /* _HAASEN_KINETICS_H_ */
