/* $Id: GlobalT.h,v 1.2 2001-07-17 00:25:09 paklein Exp $ */
/* created: paklein (02/03/1999) */

#ifndef _GLOBAL_T_H_
#define _GLOBAL_T_H_

#include "Environment.h"
#include "ios_fwd_decl.h"

/** class to handle "global" enumerated types */
class GlobalT
{
public:

	/** types of analysis */
	enum AnalysisCodeT {
		      kLinStatic = 1,
		     kLinDynamic = 2,
		       kNLStatic = 3,
		      kNLDynamic = 4,
		             kDR = 5, // this is really a solver method
		  kLinExpDynamic = 6,
		   kNLExpDynamic = 7,
		kVarNodeNLStatic = 15, // will be gone soon
		kVarNodeNLExpDyn = 16, // will be gone soon
		   kAugLagStatic = 17,
		  kLinStaticHeat = 19, // linear static heat conduction
		   kLinTransHeat = 20}; // linear transient heat conduction
		
	/** stream extraction operator */
	friend istream& operator>>(istream& in, GlobalT::AnalysisCodeT& code);
	
	/** deprecated analysis codes */
	enum OldAnalysisCodeT {
		       kCBStatic = 8,   // converted to KBC controller: PAK (12/10/2000)
	     kNLStaticKfield = 11,  // converted to KBC controller: PAK (09/10/2000)
		 kNLExpDynKfield = 18}; // converted to KBC controller: PAK (09/10/2000)

// Currently nonlinear <=> large deformation, will probably
// need to break this into:
//    (a) linear, small deformation
//    (b) nonlinear, small deformation
//    (c) (nonlinear) large deformation

	/** analysis stage */
	enum StateT {
	            kNone = 0,
        kConstruction = 1,
      kInitialization = 2,
    kInitialCondition = 3,
         kReadRestart = 4,
	        kInitStep = 5,
	         kFormRHS = 6,
	         kFormLHS = 7,
	       kResetStep = 8,
	       kCloseStep = 9,
	     kWriteOutput =10,
		kWriteRestart =11,
		   kException =12,
		 kDestruction =13};

	/** global system types, ordered so n_1 > n_2 implies that n_1 is a
	 * superset of n_2. */
	enum SystemTypeT {
		    kDiagonal = 0,
		   kSymmetric = 1,
		kNonSymmetric = 2};

	/** relaxation level */
	enum RelaxCodeT {
		kNoRelax = 0, /**< do nothing */
		   kReEQ = 1, /**< reset global equation numbers, but still at force equilirbium */
		  kRelax = 2, /**< relax, ie. re-find equilibrium */
	  kReEQRelax = 3  /**< reset global equation numbers and relax */ };

	/** returns flag with precedence */
	static RelaxCodeT MaxPrecedence(RelaxCodeT code1, RelaxCodeT code2);

	/** equation numbering scope mainly for parallel solvers */
	enum EquationNumberScopeT {
		kLocal  = 0, /**< equations numbered per processor */
		kGlobal = 1  /**< equations numbered over entire system */};
};

#endif // _GLOBAL_T_H_