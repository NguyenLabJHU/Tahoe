/* $Id: CSEBaseT.h,v 1.10 2002-12-11 23:13:17 cjkimme Exp $ */
/* created: paklein (11/19/1997) */
#ifndef _CSE_BASE_T_H_
#define _CSE_BASE_T_H_

/* base class */
#include "ElementBaseT.h"

/* direct members */
#include "iArrayT.h"
#include "LocalArrayT.h"
#include "GeometryT.h"
#include "fstreamT.h"

namespace Tahoe {

/* forward declarations */
class SurfaceShapeT;
class StringT;

/** base class for cohesive surface elements */
class CSEBaseT: public ElementBaseT
{
public:

	/* flags for derived class support */
	enum FormulationT {Isotropic = 0,
	                 Anisotropic = 1, 
	         NoRotateAnisotropic = 2};

	/** indicies for nodal output */
	enum NodalOutputCodeT {
	                  NodalCoord = 0, /**< reference coordinates */
                       NodalDisp = 1, /**< displacements */
                   NodalDispJump = 2, /**< opening displacements */
                   NodalTraction = 3, /**< traction */
                    MaterialData = 4  /**< output from constitutive relations */ };

	/** indicies for element output */
	enum ElementOutputCodeT {
	             Centroid = 0, /**< reference coordinates of centroid */
           CohesiveEnergy = 1, /**< dissipated energy */
                 Traction = 2  /**< element-averaged traction */ };

	/* constructors */
#ifndef _SIERRA_TEST_
	CSEBaseT(const ElementSupportT& support, const FieldT& field);
#else
	CSEBaseT(ElementSupportT& support);
#endif

	/* destructor */
	~CSEBaseT(void);

	/* allocates space and reads connectivity data */
	virtual void Initialize(void);

	/* start of new time sequence */
	virtual void InitialCondition(void);

	/* finalize time increment */
	virtual void CloseStep(void);

	/* resets to the last converged solution */
	virtual void ResetStep(void);

#ifndef _SIERRA_TEST_
	/* solution calls */
	virtual void AddNodalForce(const FieldT& field, int node, dArrayT& force);
#endif

	/* returns the energy as defined by the derived class types */
	virtual double InternalEnergy(void); //not implemented

	/* writing output */
	virtual void RegisterOutput(void);
	virtual void WriteOutput(void);

	/* compute specified output parameter and send for smoothing */
	virtual void SendOutput(int kincode);

#ifdef _SIERRA_TEST_	
	/* Initialize fields passed in from the outside */
	virtual void InitStep(void);
#endif

protected:

	/** element status flags */
	enum StatusT {kOFF = 0,
                   kON = 1,
               kMarked = 2};

	/** print element group data */
	virtual void PrintControlData(ostream& out) const;

	/** read element connectivity data. Cohesive elements with higher order
	 * elements may need to revise the connectivity read from the geometry
	 * file. The problem is that the element topologies resulting from
	 * cohesive elements with higher order interpolations are not supported
	 * by some database types and post-processors. Therefore, a second set
	 * of connectivities is generated for the element calculations, while
	 * output is written to the original connectivities. */
	virtual void ReadConnectivity(ifstreamT& in, ostream& out);

	/** \name output data */
	/*@{*/
	virtual void SetNodalOutputCodes(IOBaseT::OutputModeT mode, const iArrayT& flags,
		iArrayT& counts) const;
	virtual void SetElementOutputCodes(IOBaseT::OutputModeT mode, const iArrayT& flags,
		iArrayT& counts) const;
	virtual void ComputeOutput(const iArrayT& n_codes, dArray2DT& n_values,
		const iArrayT& e_codes, dArray2DT& e_values) = 0;

	/** set output labels array */
	virtual void GenerateOutputLabels(const iArrayT& n_codes, ArrayT<StringT>& n_labels,
		const iArrayT& e_codes, ArrayT<StringT>& e_labels) const;
	/*@}*/

	/* write current element information to the output */
	void CurrElementInfo(ostream& out) const;
	
private:

	/* close surfaces to zero gap */
	void CloseSurfaces(void) const;

	/* return the default number of element nodes */
	virtual int DefaultNumElemNodes(void) const;
	//NOTE: needed because ExodusII does not store ANY information about
	//      empty element groups, which causes trouble for parallel execution
	//      when a partition contains no element from a group.
	
protected:

	/* parameters */
	GeometryT::CodeT fGeometryCode;
	int fNumIntPts;
	int fCloseSurfaces;
	int fOutputArea;

	/* output control */
	int fOutputID;
	iArrayT	fNodalOutputCodes;
	iArrayT	fElementOutputCodes;
	iArrayT fNodesUsed;

	/* output stream for fracture area */
	ofstreamT farea_out;
	double fFractureArea;

	/* shape functions */
	SurfaceShapeT* fShapes;

	/* local arrays */
	LocalArrayT	fLocInitCoords1; // ref geometry of 1st facet
	LocalArrayT	fLocCurrCoords;  // current geometry
	iArrayT		fNodes1;         // nodes on 1st facet

	/* work space */
	dArrayT  fNEEvec;	
	dMatrixT fNEEmat;

	/* parameters */
	static const int NumNodalOutputCodes;
	static const int NumElementOutputCodes;
	
	/** output connectivities. For low-order element types, there will
	 * be the same as ElementBaseT::fConnectivities. These will be
	 * different if the connectivities needed for the element calculations
	 * is not compatible with the element topologies supported by
	 * most database types or post-processors */
	ArrayT<const iArray2DT*> fOutput_Connectivities;
};

} // namespace Tahoe 
#endif /* _CSE_BASE_T_H_ */
