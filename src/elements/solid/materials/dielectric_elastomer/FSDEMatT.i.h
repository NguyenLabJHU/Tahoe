#include "FSDE_inc.h"

namespace Tahoe {

  inline FSDEMatT::FSDEMatT() :
    ParameterInterfaceT("Finite Strain Dielectric Elastomer"),
        fFSDEMatSupport(0)
  {
    SetName(FSDEMatT::Name);
    Initialize();
  }

  //
  // Set electrical permittivity
  //
  inline void FSDEMatT::SetElectricPermittivity(double epsilon)
  {	
    fElectricPermittivity = epsilon;
  }

  //
  // Get electrical permittivity
  //
  inline double FSDEMatT::GetElectricPermittivity() const
  {
    return fElectricPermittivity;
  }

  //
  //
  //
  inline void FSDEMatT::SetFSDEMatSupport(
      const FSDEMatSupportT* support)
  {
    fFSDEMatSupport = support;
  }


  //
  //
  //
  inline const dArrayT FSDEMatT::ElectricField()
  {
  	cout << "FSDEMatT::ElectricField()" << endl;
    fElectricField = fFSDEMatSupport->ElectricField();
    return fElectricField;
  }

  //
  //
  //
  inline const dArrayT FSDEMatT::ElectricField(int ip)
  {
  	cout << "FSDEMatT::ElectricField(ip)" << endl;
    fElectricField = fFSDEMatSupport->ElectricField(ip);
    return fElectricField;
  }

  //
  //
  //
  inline const dMatrixT FSDEMatT::RightCauchyGreenDeformation()
  {
    const dMatrixT F = F_mechanical();
    dMatrixT FTF(3);
    FTF.MultATB(F, F);

    return FTF;

  }

  //
  // material energy density
  //
  inline double FSDEMatT::StrainEnergyDensity()
  {

//    fEnergyDensity = EnergyDensity(C, D);
	  return 0.0;
//    return fEnergyDensity;

  }


  //
  // material mechanical tangent modulus
  //
  inline const dMatrixT&
  FSDEMatT::C_IJKL()
  {
	cout << "FSDEMatT::C_IJKL" << endl;
//     const dMatrixT& C = RightCauchyGreenDeformation();
//     const dArrayT& E = ElectricField();
//     
// 	/* call C function for mechanical tangent modulus */
// 	get_ddC(fParams.Pointer(), E.Pointer(),  
// 		C.Pointer(), fTangentMechanical.Pointer()); 
// 
// 	fTangentMechanical*=4.0;
	fTangentMechanical = 0.0;
    return fTangentMechanical;
  }

  //
  // material electromechanical tangent modulus
  //
  inline const dMatrixT&
  FSDEMatT::E_IJK()
  {
	cout << "FSDEMatT::E_IJK" << endl;
    const dMatrixT& C = RightCauchyGreenDeformation();
	const dArrayT& E = ElectricField();

	/* call C function for electromechanical tangent modulus */
 	get_ddCE(fParams.Pointer(), E.Pointer(),  
 		C.Pointer(), fTangentElectromechanical.Pointer()); 
 
 	fTangentElectromechanical*=2.0;
 	cout << "E_IJK = " << fTangentElectromechanical << endl;
    return fTangentElectromechanical;

  }

  //
  // material electric tangent modulus
  //
  inline const dMatrixT&
  FSDEMatT::B_IJ()
  {
	cout << "FSDEMatT::B_IJ" << endl;
    const dMatrixT& C = RightCauchyGreenDeformation();
	const dArrayT& E = ElectricField();

	/* call C function for electrical tangent modulus */
 	get_ddE(fParams.Pointer(), E.Pointer(),  
 		C.Pointer(), fTangentElectrical.Pointer()); 
 
 	fTangentElectrical *= -1.0;
 	cout << "B_IJ = " << fTangentElectrical << endl;
    return fTangentElectrical;

  }

  //
  // Second Piola-Kirchhoff stress (mechanical)
  //
  inline const dSymMatrixT&
  FSDEMatT::S_IJ()
  {
	cout << "FSDEMatT::S_IJ" << endl;
    const dMatrixT& C = RightCauchyGreenDeformation();
   const dArrayT& E = ElectricField();
    
	/* call C function for mechanical stress */
	get_dUdC(fParams.Pointer(), E.Pointer(),  
		C.Pointer(), stress_temp.Pointer()); 

    fStress.FromMatrix(stress_temp);
	fStress*=2.0;
	cout << "S_IJ = " << fStress << endl;
    return fStress;

  }

  //
  // Electric displacement - is it necessary to pass Efield?
  //
  inline const dArrayT&
  FSDEMatT::D_I()
  {
  	cout << "FSDEMatT::D_I" << endl;
    const dMatrixT& C = RightCauchyGreenDeformation();
	const dArrayT& E = ElectricField();

	/* call C function for electric displacement */
 	get_dUdE(fParams.Pointer(), E.Pointer(),  
 		C.Pointer(), fElectricDisplacement.Pointer());     
     
    fElectricDisplacement *= -1.0;
    cout << "fElectricDisplacement = " << fElectricDisplacement << endl;
    return fElectricDisplacement;
  }

  //
  // Electric field
  //
  inline const dArrayT&
  FSDEMatT::E_I()
  {
	cout << "FSDEMatT::E_I" << endl;

    return fElectricField;

  }

  //
  // spatial tangent modulus
  //
  inline const dMatrixT&
  FSDEMatT::c_ijkl()
  {
	cout << "FSDEMatT::c_ijkl" << endl;
    const dMatrixT F = F_mechanical();
    const double J = F.Det();

    // prevent aliasing
    const dMatrixT CIJKL = C_IJKL();
    fTangentMechanical.SetToScaled(1.0 / J, PushForward(F, CIJKL));

    return fTangentMechanical;

  }

  //
  // Cauchy stress
  //
  inline const dSymMatrixT&
  FSDEMatT::s_ij()
  {
	cout << "FSDEMatT::s_ij" << endl;
    const dMatrixT F = F_mechanical();
    const double J = F.Det();
	
    // prevent aliasing
    const dSymMatrixT S = S_IJ();
    fStress.SetToScaled(1.0 / J, PushForward(F, S));
    return fStress;

  }

  //
  // pressure associated with the last computed stress
  //
  inline double FSDEMatT::Pressure() const
  {

    return 0.0;

  }

  //
  // compute symmetric Cij reduced index matrix */
  //
  inline void FSDEMatT::ComputeModuli(const dSymMatrixT& E, dMatrixT& moduli) 
  {


  }

  //
  // compute symmetric 2nd PK2 reduced index vector */
  //
  inline void FSDEMatT::ComputePK2(const dSymMatrixT& E, dSymMatrixT& PK2) 
  {


  }

  //
  // compute strain energy density for the specified strain */
  //
  inline double FSDEMatT::ComputeEnergyDensity(const dSymMatrixT& E) 
  {
	 return 0.0;

  }

} //namespace Tahoe
