/* $Id: Aztec_fe.h,v 1.4 2002-07-05 22:28:35 paklein Exp $ */
/* created: paklein (08/01/1998) */

#ifndef _AZTEC_FE_H_
#define _AZTEC_FE_H_

/* base class */
#include "AztecBaseT.h"

/* library support options */
#ifdef __AZTEC__

/* direct members */
#include "iArrayT.h"
#include "dArrayT.h"

namespace Tahoe {

/* forward declarations */
class ifstreamT;
class iArray2DT;
class MSRBuilderT;
template <class TYPE> class RaggedArray2DT;

/** interface to the Aztec iterative solver library */
class Aztec_fe: public AztecBaseT
{
public:

	/** constuctor 
	 * \param in stream to read input parameters 
	 * \param msg output stream for logging messages */
	Aztec_fe(ifstreamT& in, ostream& msg);

	/* destructor */
	virtual ~Aztec_fe(void);

	/* set solver options */
	virtual void SetAztecOptions(void);

	/* clear values in the matrix */
	void Clear(void);

	/* add to structure - active equations only (# > 0)
	 * NOTE: structure not set until #CALL#. equation data
	 * must persist outside of Aztec until (at least) then */
	void AddEquationSet(const iArray2DT& eqnos);
	void AddEquationSet(const RaggedArray2DT<int>& eqnos);
	 	
	/* solve system based on data passed in for the rhs
	 * and return solution in result */
	void Solve(dArrayT& rhs2result);
	void Solve(const dArrayT& initguess, dArrayT& rhs2result);

	/* statistics */
	int NumNonZeroValues(void) const;

private:

	/* copy MSR data to RCV */
	void GenerateRCV(iArrayT& r, iArrayT& c, dArrayT& v);

	/* configure the update, bindx, and values array and return
	 * their length. if the columns indices for each row in bindx
	 * are sorted, is_sorted returns 1 and 0 otherwise */
	virtual int SetMSRData(int** update, int** bindx, double** val,
		int& is_sorted);

	/* read (non-default) Aztec solver options and parameters */
	void ReadOptionsParameters(ifstreamT& in);

private:

	/* Aztec option and parameter overrides */
	iArrayT AZ_options_dex;
	iArrayT AZ_options;
	iArrayT AZ_params_dex;
	dArrayT AZ_params;

	/* Aztec workspace */
	iArrayT fupdate;
	iArrayT fbindx;
	dArrayT fval;
	dArrayT finitguess;
	
	/* MSR database builder */
	MSRBuilderT* fMSRBuilder;
	
	/* runtime flags */
	int fMSRSet;
};

/* library support options */
} // namespace Tahoe 
#endif /* __AZTEC__ */
#endif /* _AZTEC_FE_H_ */
