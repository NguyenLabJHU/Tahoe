/* $Id: SSQ2P1MF.h,v 1.2 2003-08-18 03:28:34 thao Exp $ */

#ifndef _SS_Q2P1_MF_H_
#define _SS_Q2P1_MF_H_

/* base class */
#include "SmallStrainQ2P1.h"
#include "MFSupportT.h"
#include "ofstreamT.h"
namespace Tahoe {

/* forward declarations */
class SSMatSupportT;
class SSSolidMatT;
class ifstreamT;
class StringT;

/** Interface for linear strain deformation and field gradients */
class SSQ2P1MF: public SmallStrainQ2P1, public MFSupportT
{
  public:
    /** constructor */
    SSQ2P1MF(const ElementSupportT& support, const FieldT& field);

    virtual void Initialize(void);
    virtual void SetGlobalShape(void);
    
    /*accessor for displacement gradient*/
    const dMatrixT& DisplacementGradient(void) const;

    /*output*/
    /* register self for output */
    virtual void RegisterOutput(void);

    /* send output */
    virtual void WriteOutput(void);

 private:
    /*material force evaluation*/
    void ComputeMatForce(dArray2DT& output);
    void MatForceVolMech(dArrayT& elem_val);
    void MatForceDissip(dArrayT& elem_val, const dArray2DT& internalstretch);
    void MatForceSurfMech(dArrayT& global_val);

    /*extrapolate element ip state variables values to nodes*/
    void Extrapolate(void);
  
 protected:	

    /*current material*/
    SSSolidMatT* fCurrSSMat;


 private:

    /*stress and strains*/
    ArrayT<dMatrixT> fGradU_List;
    dMatrixT fEshelby;
    dSymMatrixT fCauchy;

    /*nodal and interpolated body force*/
    dArrayT fBodyForce;
    dArrayT fip_body;
    
    /*material force */
    dArrayT fMatForce;
    dArrayT fDissipForce;
    dArrayT felem_rhs;

    /*internal variables*/
    iArrayT fInternalDOF;
    int fNumInternalVal;

    dArrayT fGlobalMass;
    dArray2DT fGlobalVal;

    dArrayT felem_mass;
    dArray2DT felem_val;
    
    dArray2DT fGradInternalStrain;

};

/* inlines */
inline const dMatrixT& SSQ2P1MF::DisplacementGradient(void) const
{
	return fGradU_List[CurrIP()];
}

} // namespace Tahoe 
#endif /* _SS_Q2P1_MF_ */
