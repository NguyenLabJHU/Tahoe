
#if !defined(_FSDielectricElastomer2DT_)
#define _FSDielectricElastomer2DT_

#include <cassert>

#include "FiniteStrainT.h"
#include "FSDEMat2DT.h"

namespace Tahoe {

  /* Forward declarations */
  class FSDEMatSupport2DT;

  //
  // 2D interface for finite deformation dielectric elastomers 
  // based on 2008 JMPS paper of Suo et al.
  
  class FSDielectricElastomer2DT: public FiniteStrainT {

  public:

    //
    // constructor
    //
    FSDielectricElastomer2DT(const ElementSupportT& support);

    //
    // destructor
    //
    virtual ~FSDielectricElastomer2DT();

    //
    // specify parameters needed by the interface
    //
    virtual void DefineParameters(ParameterListT& list) const;

    //
    // accept parameter list
    //
    virtual void TakeParameterList(const ParameterListT& list);

	/* define total # of DOFs/node, i.e. 3 (2 mech, 1 electric) */
	virtual int TotalNumDOF() const;

    //
    //
    //
    virtual void Equations(AutoArrayT<const iArray2DT*>& eq_1,
        AutoArrayT<const RaggedArray2DT<int>*>& eq_2);

    //
    // \name Electric fields
    // @{
    //

    //
    // Electric field at current integration point
    //
    const dArrayT& ElectricField() const;

    //
    // Electric field at given integration point
    //
    const dArrayT& ElectricField(int ip) const;

    //
    // increment current element
    //
    virtual bool NextElement();

    //
    // element stiffness matrix
    //
    virtual void FormStiffness(double constK);

    //
    // internal force
    //
    virtual void FormKd(double constK);

    //
    // @}
    //

  protected:

    //
    // Construct a new material support and return a
    // pointer. Recipient is responsible for freeing the pointer.
    //


    //
    // \param p an existing MaterialSupportT to be initialized. If
    // 0, allocate a new MaterialSupportT and initialize it.
    //
    virtual MaterialSupportT*
    NewMaterialSupport(MaterialSupportT* p = 0) const;

    //
    // Return a pointer to a new material list. Recipient is
    // responsible for freeing the pointer.
    //
    // \param name list identifier
    // \param size length of the list
    //
    virtual MaterialListT* NewMaterialList(const StringT& name, int size);

    //
    // form shape functions and derivatives
    //
    virtual void SetGlobalShape(void);

    //
    // write all current element information to the stream. used to
    // generate debugging information after runtime errors
    //
    virtual void CurrElementInfo(ostream& out) const;

    //
    // Initialize local arrays
    //
    virtual void SetLocalArrays();

    //
    // driver for calculating output values
    //
    virtual void ComputeOutput(const iArrayT& n_codes, dArray2DT& n_values,
        const iArrayT& e_codes, dArray2DT& e_values);

  private:

    //
    //
    //
    void Workspace();
    void Set_B_C(const dArray2DT& DNaX, dMatrixT& B_C);
    void AccumulateGeometricStiffness(dMatrixT& Kg, const dArray2DT& DNaX,
        dSymMatrixT& S);

  protected:

    //
    // \name work space
    //

    //
    // @{
    //

    // electric field
    ArrayT<dArrayT> fE_List;
    dArrayT fE_all;

    //
    // @}
    //

    //
    // The material support used to construct materials lists. This
    // pointer is only set the first time
    // FSDielectricElastomerT::NewMaterialList is called.
    //
    FSDEMatSupport2DT* fFSDEMatSupport2D;

  private:

	LocalArrayT fLocScalarPotential;	// electric potential
    FSDEMat2DT* fCurrMaterial;
    
    //
    // Stiffness storage
    //
    dMatrixT fAmm_mat;	// mechanical material part of Hessian matrix
    dMatrixT fAmm_geo;	// mechanical geometric part of Hessian matrix
    dMatrixT fAme;	// mechanical-electrical coupling part of Hessian matrix
    dMatrixT fAem;	// electrical-mechanical coupling part of Hessian matrix
    dMatrixT fAee;	// electrical-electrical coupling part of Hessian matrix
    dMatrixT fGradNa;	// shape function gradients matrix
    
    /* Electric potential */
    const FieldT* fElectricScalarPotentialField;
    
  };

} // namespace Tahoe

#include "FSDielectricElastomer2DT.i.h"
#endif // _FSDielectricElastomer2DT_
