/* $Id: BondLatticeT.h,v 1.3.42.2 2004-06-16 00:31:50 paklein Exp $ */
/* created: paklein (01/07/1997) */
#ifndef _BONDLATTICET_H_
#define _BONDLATTICET_H_

/* direct members */
#include "iArrayT.h"
#include "dArrayT.h"
#include "dSymMatrixT.h"
#include "dArray2DT.h"
#include "dMatrixT.h"

namespace Tahoe {

/** container for bond information */
class BondLatticeT
{
public:

	/** constructor must be followed by call to BondLatticeT::Initialize to
	 * initialize the bond table information */
	BondLatticeT(void);
	
	/* destructor */
	virtual ~BondLatticeT(void);

	/** The Q matrix is used to rotate the
	 * bond vectors into the orientation prescribed by Q.  No check is
	 * performed on the orthogonality of Q, only its dimensions.  Q is
	 * deep copied.  Q is defined as:
	 *
	 *			Q = d x_natural / d x_global
	 *
	 * So that the vectors are transformed by:
	 *
	 *			r_global = Transpose[Q].r_natural
	 */
	void Initialize(const dMatrixT* Q = NULL);
		
	/* references to the lattice lists */
	const iArrayT& BondCounts(void) const;
	const dArrayT& DeformedLengths(void) const;

	/* accessors */
//	int NumberOfLatticeDim(void) const;
//  int NumberOfSpatialDim(void) const;
	int NumberOfBonds(void) const { return fBonds.MajorDim(); };

	/* compute deformed bond lengths from the given Green strain */
	void ComputeDeformedLengths(const dSymMatrixT& strain);

protected:

	/* initialize bond table values */
	virtual void LoadBondTable(void) = 0;
	
protected:

#if 0
	int fNumLatticeDim;	/* dim of the bond vectors */
	int	fNumSpatialDim;	/* dim of the model geometry */
	int fNumBonds;
#endif

	iArrayT		fBondCounts;
	dArray2DT	fBonds;			/* undeformed bond vector components */
	dArrayT 	fDefLength;		/* list of deformed bond lengths */
	dMatrixT	fQ;				/* bond vector transformation matrix */

	/** \name work space */
	/*@{*/
	dArrayT		fBondSh;		/**< shallow bond vector */
	dArrayT 	fBondDp;		/**< deep bond vector */
	dMatrixT	fLatDimMatrix;	/**< matrix with same dimensions as lattice */
	dSymMatrixT	fStrain;		/**< needed if LatticeDim != SpatialDim */  		
	/*@}*/
};

/* inlines */
inline const iArrayT& BondLatticeT::BondCounts(void) const { return fBondCounts; }
inline const dArrayT& BondLatticeT::DeformedLengths(void) const { return fDefLength; }

} /* namespace Tahoe */

#endif /* _BONDLATTICET_H_ */
