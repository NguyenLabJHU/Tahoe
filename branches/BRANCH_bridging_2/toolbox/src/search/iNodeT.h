/* $Id: iNodeT.h,v 1.2.8.1 2003-05-12 16:24:29 paklein Exp $ */
/* created: paklein (12/07/1997) */
#ifndef _I_NODE_T_H_
#define _I_NODE_T_H_

#include <time.h> //defines NULL

namespace Tahoe {

/** data cell for the search grid. Stores a single integer plus coordinates
 * for each node */
class iNodeT
{
public:

	/* constructor */
	iNodeT(void);
	iNodeT(double* coords, int n);
	
	/* make blank */
	void Clear(void);
	
	/* initializer */
	void Set(double* coords, int n);

	/* accessors */
	double* Coords(void) const;
	int Tag(void) const;
		
	/* assigment operator */
	iNodeT& operator=(const iNodeT& RHS);
	
	/* logical operator - by tag */
	int operator==(const iNodeT& RHS) const;

private:

	double* fCoords;
	int		fTag;
	
};

/* inlines */

/* constructors */
#ifdef __MWERKS__ /* VC++ doesn't like inline constructors */
inline iNodeT::iNodeT(void): fCoords(NULL), fTag(-1) { }
inline iNodeT::iNodeT(double* coords, int n) { Set(coords,n); }
#endif /* __MWERKS__ */
	
/* initializer */
inline void iNodeT::Set(double* coords, int n)
{
	fCoords = coords;
	fTag    = n;
}

/* accessors */
inline double* iNodeT::Coords(void) const { return fCoords; }
inline int iNodeT::Tag(void) const        { return fTag;    }

/* assigment operator */
inline iNodeT& iNodeT::operator=(const iNodeT& RHS)
{
	Set(RHS.fCoords,RHS.fTag);
	return *this;
}

/* logical operator - by tag */
inline int iNodeT::operator==(const iNodeT& RHS) const
{
	return (fTag == RHS.fTag);
}

} // namespace Tahoe 
#endif /* _I_NODE_T_H_ */