// file: IOBaseT.h
//
// Base class for InputBaseT and OutputBaseT

// created      : SAW (09/28/1999)
// last modified: PAK (11/07/1999)

#ifndef _IOBASE_T_H_
#define _IOBASE_T_H_

/* forward declarations */
#include "ios_fwd_decl.h"

class IOBaseT
{
  public:

	enum OutputMode {kAtFail =-2,
	                kAtFinal =-1,
	                kAtNever = 0, 
	                  kAtInc = 1};

	enum IOFileType {kTahoe = 0, 
	               kTahoeII = 1, 
	               kTecPlot = 2, 
	               kEnSight = 3, 
	         kEnSightBinary = 4,
	              kExodusII = 5,
			kAbaqus = 6,
		  kAbaqusBinary = 7 };
	
	/* constructor */
	IOBaseT(ostream& out);
	
	/* destructor */
	virtual ~IOBaseT(void);

	/* convert integer to IOFileType */
	friend IOFileType int_to_IOFileType(int i);
	friend istream& operator>>(istream& in, IOBaseT::IOFileType& file_type);

  protected:

	/* format the output stream */
	void SetStreamPrefs(ostream& stream) const;

	/* returns 1 if the stream is open */
	int IsOpen(ofstream& stream) const;
	
  protected:
	
	ostream& fout;
};

#endif // _IOBASE_T_H_
