/* $Id: AbaqusOutputT.h,v 1.2.2.2 2001-11-06 20:22:51 sawimme Exp $ */
/* created: sawimme (05/31/2000)                                          */

#ifndef _ABAQUSOUTPUT_T_H_
#define _ABAQUSOUTPUT_T_H_

/* base class */
#include "OutputBaseT.h"
#include "AbaqusResultsT.h"

/* forward declarations */


class AbaqusOutputT: public OutputBaseT
{
public:
  /** constructor
   * \param out error stream
   * \param out_strings see OutputBaseT::OutputBaseT
   * \param binary Binary file format flag */
	AbaqusOutputT(ostream& out, const ArrayT<StringT>& out_strings, bool binary);

	/** write geometry for all output sets */
	virtual void WriteGeometry(void);

	/** write geometry and node variables for output set ID */
	virtual void WriteOutput(double time, int ID, const dArray2DT& n_values, const dArray2DT& e_values);

private:

	/** create filename */
	void FileName(int ID, StringT& filename) const;

	/** open new file and write geometry */
	void CreateResultsFile (int ID, AbaqusResultsT& aba);

	/** translate variable names to keys */
	void SetRecordKey (AbaqusResultsT& aba, const ArrayT<StringT>& labels, iArrayT& keys) const;
	
private:
	bool fBinary; /**< binary format flag */
	int fBufferWritten; /**< amount of buffer written, used when reopening file */
	double fOldTime; /**< previous time step, used to determine time increment */
};

#endif