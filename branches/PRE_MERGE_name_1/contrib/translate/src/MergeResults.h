/* $Id: MergeResults.h,v 1.1 2002-05-19 17:45:42 paklein Exp $ */
#ifndef _TRANSLATE_MERGE_H_
#define _TRANSLATE_MERGE_H_

/* base class */
#include "TranslateIOManager.h"

/** merge results data from multiple results files */
class MergeResults: public TranslateIOManager
{
public:

	/** constructor */
	MergeResults (ostream& message, istream& in, bool write);

	/** destructor */
	virtual ~MergeResults(void);

	/** run */
	virtual void Translate (const StringT& program, const StringT& version, 
		const StringT& title);

protected:

	/** set input sources */
	void SetInput(void);

private:

	/** generate combined coordinates list
	 * \param coords combined, expanded coordinate list. The rows in this
	 *        row correspond to the (global) id of the node.
	 * \param nodes_used list nodes in the expanded list that were used */
	void CombinedCoordinates(dArray2DT& coords, iArrayT& nodes_used);
		
private:

	/** array of input sources */
	ArrayT<ModelManagerT*> fInputs;

	/** node maps from the input sources */
	ArrayT<iArrayT> fNodeMaps;
};

#endif /* _TRANSLATE_MERGE_H_ */