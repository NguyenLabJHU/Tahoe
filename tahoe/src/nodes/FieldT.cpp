/* $Id: FieldT.cpp,v 1.17 2003-08-08 16:29:16 paklein Exp $ */
#include "FieldT.h"
#include "fstreamT.h"
#include "nIntegratorT.h"
#include "KBC_ControllerT.h"
#include "FBC_ControllerT.h"
#include "RaggedArray2DT.h"
#include "LocalArrayT.h"
#include "FieldSupportT.h"

using namespace Tahoe;

/* constructor */
FieldT::FieldT(const StringT& name, int ndof, nIntegratorT& controller):
	BasicFieldT(name, ndof, controller.Order()),
	fnIntegrator(controller),
	fField_last(fnIntegrator.Order()+1),
	fEquationStart(0),
	fNumEquations(0)
{
	/* register arrays */
	for (int i = 0; i < fField_last.Length(); i++)
		RegisterArray2D(fField_last[i]);
	RegisterArray2D(fUpdate);
}

/* destructor */
FieldT::~FieldT(void)
{ 
	for (int i = 0; i < fSourceOutput.Length(); i++)
		delete fSourceOutput[i];
		
	for (int i = 0; i < fKBC_Controllers.Length(); i++)
		delete fKBC_Controllers[i];

	for (int i = 0; i < fFBC_Controllers.Length(); i++)
		delete fFBC_Controllers[i];
}

void FieldT::RegisterLocal(LocalArrayT& array) const	
{
	const char caller[] = "FieldT::RegisterLocal";
	switch (array.Type())
	{
		case LocalArrayT::kDisp:
		{
			array.SetGlobal(fField[0]);
			break;
		}
		case LocalArrayT::kVel:
		{
			if (Order() < 1) ExceptionT::GeneralFail(caller, "only up to order %d: 1", Order());
			array.SetGlobal(fField[1]);
			break;			
		}
		case LocalArrayT::kAcc:
		{
			if (Order() < 2) ExceptionT::GeneralFail(caller, "only up to order %d: 2", Order());
			array.SetGlobal(fField[2]);
			break;			
		}
		case LocalArrayT::kLastDisp:
		{
			array.SetGlobal(fField_last[0]);
			break;
		}
		case LocalArrayT::kLastVel:
		{
			if (Order() < 1) ExceptionT::GeneralFail(caller, "only up to order %d: 1", Order());
			array.SetGlobal(fField_last[1]);
			break;			
		}
		case LocalArrayT::kLastAcc:
		{
			if (Order() < 2) ExceptionT::GeneralFail(caller, "only up to order %d: 2", Order());
			array.SetGlobal(fField_last[2]);
			break;			
		}
		default:
			ExceptionT::GeneralFail(caller, "unrecognized type: %d", array.Type());
	}
}

/* set all field values to 0.0 */
void FieldT::Clear(void)
{
	/* inherited */
	BasicFieldT::Clear();
	
	/* clear */
	for (int i = 0; i < fField_last.Length(); i++)
		fField_last[i] = 0.0;
}

/* set number of nodes */
void FieldT::Dimension(int nnd, bool copy_in)
{
	/* inherited */
	BasicFieldT::Dimension(nnd, copy_in);

	/* set dimensions with integrator */
	fnIntegrator.Dimension(*this);
}

/* append the equation sets generated by the KBC_ControllerT's and
 * FBC_ControllerT's. */
void FieldT::EquationSets(AutoArrayT<const iArray2DT*>& eq_1, 
	AutoArrayT<const RaggedArray2DT<int>*>& eq_2)
{
	/* KBC controllers */
	for (int i = 0; i < fKBC_Controllers.Length(); i++)
		fKBC_Controllers[i]->Equations(eq_1);

	/* FBC controllers */
	for (int i = 0; i < fFBC_Controllers.Length(); i++)
		fFBC_Controllers[i]->Equations(eq_1, eq_2);
}

/* append connectivities */
void FieldT::Connectivities(AutoArrayT<const iArray2DT*>& connects_1,
	AutoArrayT<const RaggedArray2DT<int>*>& connects_2,
	AutoArrayT<const iArray2DT*>& equivalents) const
{
	/* KBC controllers */
	for (int i = 0; i < fKBC_Controllers.Length(); i++)
		fKBC_Controllers[i]->Connectivities(connects_1, equivalents);

	/* FBC controllers */
	for (int i = 0; i < fFBC_Controllers.Length(); i++)
		fFBC_Controllers[i]->Connectivities(connects_1, connects_2, equivalents);
}

/* return the GlobalT::SystemTypeT for the  group */
GlobalT::SystemTypeT FieldT::SystemType(void) const
{
	/* normally no contribution to the stiffness */
	GlobalT::SystemTypeT type = GlobalT::kUndefined;

	/* FBC controllers */
	for (int i = 0; i < fFBC_Controllers.Length(); i++)
		type = GlobalT::MaxPrecedence(type, fFBC_Controllers[i]->TangentType());

	return type;
}

/* beginning of time series */
void FieldT::InitialCondition(void)
{
	/* initial fields */
	for (int i = 0; i < fField.Length(); i++)
		fField[i] = 0.0;

	/* apply initial cards */
	bool all_active = true;
	for (int i = 0; i < fIC.Length(); i++)
		if (!Apply_IC(fIC[i]))
			all_active = false;
	if (!all_active)
		cout << "\n FieldT::InitialCondition: initial conditions applied to prescribed\n" 
		     <<   "     equations in field \"" << Name() << "\" are being ignored" << endl;

	/* KBC controllers */
	for (int k = 0; k < fKBC_Controllers.Length(); k++)
		fKBC_Controllers[k]->InitialCondition();

	/* FBC controllers */
	for (int i = 0; i < fFBC_Controllers.Length(); i++)
		fFBC_Controllers[i]->InitialCondition();

	/* apply KBC cards */
	for (int i = 0; i < fKBC.Length(); i++)
		fnIntegrator.ConsistentKBC(*this, fKBC[i]);
		
	/* apply KBC cards generated by KBC controllers */
	for (int j = 0; j < fKBC_Controllers.Length(); j++)
	{
		/* cards */
		const ArrayT<KBC_CardT>& cards = fKBC_Controllers[j]->KBC_Cards();
		/* apply KBC cards */
		for (int i = 0; i < cards.Length(); i++)
			fnIntegrator.ConsistentKBC(*this, cards[i]);
	}

	/* initial history */
	fField_last = fField;
}

/* apply predictor to all degrees of freedom */
void FieldT::InitStep(void)
{
	/* predictor to all DOF's */
	fnIntegrator.Predictor(*this);

	/* KBC controllers */
	for (int i = 0; i < fKBC_Controllers.Length(); i++)
		fKBC_Controllers[i]->InitStep();

	/* FBC controllers */
	for (int j = 0; j < fFBC_Controllers.Length(); j++)
		fFBC_Controllers[j]->InitStep();

	/* apply KBC cards */
	for (int i = 0; i < fKBC.Length(); i++)
		fnIntegrator.ConsistentKBC(*this, fKBC[i]);
		
	/* apply KBC cards generated by KBC controllers */
	for (int j = 0; j < fKBC_Controllers.Length(); j++)
	{
		/* cards */
		const ArrayT<KBC_CardT>& cards = fKBC_Controllers[j]->KBC_Cards();
		/* apply KBC cards */
		for (int i = 0; i < cards.Length(); i++)
			fnIntegrator.ConsistentKBC(*this, cards[i]);
	}
}

/* assemble contributions to the residual */
void FieldT::FormRHS(const FieldSupportT& support)
{
	/* has force bpundary conditions */
	if (fFBC.Length() > 0)
	{
		/* collect nodal forces */
		for (int i = 0; i < fFBC.Length(); i++)
			fFBCValues[i] = fFBC[i].CurrentValue();	
	
		/* assemble */
		support.AssembleRHS(Group(), fFBCValues, fFBCEqnos);
	}

	/* KBC controllers */
	for (int i = 0; i < fKBC_Controllers.Length(); i++)
		fKBC_Controllers[i]->FormRHS();
	
	/* FBC controllers */
	for (int i = 0; i < fFBC_Controllers.Length(); i++)
		fFBC_Controllers[i]->ApplyRHS();
}

/* assemble contributions to the tangent */
void FieldT::FormLHS(const FieldSupportT& support, GlobalT::SystemTypeT sys_type)
{
#pragma unused(support)

	/* KBC controllers */
	for (int i = 0; i < fKBC_Controllers.Length(); i++)
		fKBC_Controllers[i]->FormLHS(sys_type);

	/* FBC controllers */
	for (int i = 0; i < fFBC_Controllers.Length(); i++)
		fFBC_Controllers[i]->ApplyLHS(sys_type);
}

/* update history */
void FieldT::CloseStep(void)
{
	/* update history */
	fField_last = fField;

	/* KBC controllers */
	for (int i = 0; i < fKBC_Controllers.Length(); i++)
		fKBC_Controllers[i]->CloseStep();

	/* FBC controllers */
	for (int i = 0; i < fFBC_Controllers.Length(); i++)
		fFBC_Controllers[i]->CloseStep();
}

/* overwrite the update values in the FieldT::Update array */
void FieldT::AssembleUpdate(const dArrayT& update)
{
	int *peq = fEqnos.Pointer();
	int len = fEqnos.Length();
	double *p = fUpdate.Pointer();
	for (int i = 0; i < len; i++)
	{
		/* local equation */
		int eq = *peq++ - fEquationStart;
		
		/* active dof */
		if (eq > -1 && eq < fNumEquations)
			*p = update[eq];
		else
			*p = 0.0;

		/* next */
		p++;
	}
#pragma message("FieldT -- Needs FBC controllers too?")	
	/* KBC controllers */
	for (int i = 0; i < fKBC_Controllers.Length(); i++)
		fKBC_Controllers[i]->Update(update);

}

/* update the active degrees of freedom */
void FieldT::ApplyUpdate(void)
{
	/* corrector */
	fnIntegrator.Corrector(*this, fUpdate);
}

/* copy nodal information */
void FieldT::CopyNodeToNode(const ArrayT<int>& source, const ArrayT<int>& target)
{
	/* copy data from source nodes to target nodes */
	for (int i = 0 ; i < fField.Length() ; i++ )
	{
		dArray2DT& field = fField[i];
		dArray2DT& field_last = fField_last[i];
		for (int j = 0; j < source.Length(); j++)
		{
			int from = source[j];
			int to = target[j];
			field.CopyRowFromRow(to,from);
			field_last.CopyRowFromRow(to,from);
		}
	}
}

/* check for relaxation */
GlobalT::RelaxCodeT FieldT::RelaxSystem(void)
{
	GlobalT::RelaxCodeT relax = GlobalT::kNoRelax;
	for (int i = 0; i < fKBC_Controllers.Length(); i++)
	{
		/* check controller */
		GlobalT::RelaxCodeT code = fKBC_Controllers[i]->RelaxSystem();

//NOTE - do the boundary condition really need to be reapplied here?

		/* reset BC */
		if (code != GlobalT::kNoRelax)
		{
			/* boundary condition cards generated by the controller */
			const ArrayT<KBC_CardT>& cards = fKBC_Controllers[i]->KBC_Cards();

			/* apply BC's */
			for (int j = 0; j < cards.Length(); j++)
				fnIntegrator.ConsistentKBC(*this, cards[j]);
		}
		relax = GlobalT::MaxPrecedence(relax, code);
	}
	return relax;
}

/* reset displacements (and configuration to the last known solution) */
void FieldT::ResetStep(void)
{
	/* reset field */
	fField = fField_last;

	/* KBC controllers */
	for (int j = 0; j < fKBC_Controllers.Length(); j++)
		fKBC_Controllers[j]->Reset();

	/* FBC controllers */
	for (int i = 0; i < fFBC_Controllers.Length(); i++)
		fFBC_Controllers[i]->Reset();
}

/* mark prescribed equations */
void FieldT::InitEquations(void)
{
	/* use the allocated space */
	fEqnos.Dimension(NumNodes(), NumDOF());
	fEqnos = FieldT::kInit;
	
	/* mark KBC nodes */
	for (int j = 0; j < fKBC.Length(); j++)
		SetBCCode(fKBC[j]);
	
	/* mark nodes used by KBC controllers */
	for (int j = 0; j < fKBC_Controllers.Length(); j++)
	{
		/* boundary condition cards generated by the controller */
		const ArrayT<KBC_CardT>& cards = fKBC_Controllers[j]->KBC_Cards();

		if (!fKBC_Controllers[j]->IsICController())
		{
			/* mark global equation numbers */
			for (int i = 0; i < cards.Length(); i++)
				SetBCCode(cards[i]);
		}
	}
}

/* set the equations array and the number of active equations */
void FieldT::FinalizeEquations(int eq_start, int num_eq)
{
	/* store parameters */
	fEquationStart = eq_start;
	fNumEquations = num_eq;

	/* set force boundary condition destinations */
	SetFBCEquations();

	/* run through KBC controllers */
	for (int j = 0; j < fKBC_Controllers.Length(); j++)
	  fKBC_Controllers[j]->SetEquations();
}

/* Collect the local element lists */
void FieldT::SetLocalEqnos(const iArray2DT& nodes, iArray2DT& eqnos) const
{
/* consistency checks */
#if __option (extended_errorcheck)
	if (nodes.MajorDim() != eqnos.MajorDim() ||
	    eqnos.MinorDim() < nodes.MinorDim()*NumDOF()) ExceptionT::GeneralFail("FieldT::SetLocalEqnos");
		//must have enough space (and maybe more)
#endif

	int numel = nodes.MajorDim();
	int nen   = nodes.MinorDim();
	int ndof  = NumDOF();
	for (int i = 0; i < numel; i++)
	{
		int* pnodes = nodes(i);
		int* pien   = eqnos(i);
		for (int j = 0; j < nen; j++)
		{
			int nodenum = *pnodes++;
			for (int k = 0; k < ndof; k++)
				*pien++ = fEqnos(nodenum, k);
		}
	}
}

/* collect equation numbers */
void FieldT::SetLocalEqnos(ArrayT<const iArray2DT*> nodes, iArray2DT& eqnos) const
{
	const char caller[] = "FieldT::SetLocalEqnos";
	
	int row = 0;
	for (int i = 0; i < nodes.Length(); i++)
	{
		const iArray2DT& nd = *(nodes[i]);
	
		/* check */
		if (row + nd.MajorDim() > eqnos.MajorDim()) ExceptionT::OutOfRange(caller);

		/* single block */	
		iArray2DT eq(nd.MajorDim(), eqnos.MinorDim(), eqnos(row));
		SetLocalEqnos(nd, eq);
	
		/* next */
		row += nd.MajorDim();
	}
	
	/* check - must fill all of eqnos */
	if (row != eqnos.MajorDim()) ExceptionT::SizeMismatch(caller);
}

void FieldT::SetLocalEqnos(const RaggedArray2DT<int>& nodes,
	RaggedArray2DT<int>& eqnos) const
{
/* consistency checks */
#if __option(extended_errorcheck)
	const char caller[] = "FieldT::SetLocalEqnos";
	if (nodes.MajorDim() != eqnos.MajorDim()) ExceptionT::SizeMismatch(caller);
#endif
	
	int numel = nodes.MajorDim();
	for (int i = 0; i < numel; i++)
	{
#if __option(extended_errorcheck)
		if (eqnos.MinorDim(i) < nodes.MinorDim(i)*NumDOF()) ExceptionT::SizeMismatch(caller);
		//must have enough space (and maybe more)
#endif
		int  nen    = nodes.MinorDim(i);
		int* pnodes = nodes(i);
		int* pien   = eqnos(i);
		int ndof    = NumDOF();
		for (int j = 0; j < nen; j++)
		{
			int nodenum = *pnodes++;
			for (int k = 0; k < ndof; k++)
				*pien++ = fEqnos(nodenum, k);
		}
	}
}

void FieldT::ReadRestart(ifstreamT& in, const ArrayT<int>* nodes)
{
	/* external file */
	StringT file;
	ifstreamT u_in;

	/* read fields */
	StringT deriv;
	for (int i = 0; i < fField.Length(); i++)
	{
		/* file name */
		StringT file = in.filename();
		file.Append(".", fName);
		file.Append(".", deriv, "u");

		/* read */
		u_in.open(file);
		if (u_in.is_open()) 
		{
			if (nodes) /* select nodes */
			{
				dArray2DT tmp(nodes->Length(), NumDOF());
				u_in >> tmp;
				fField[i].Assemble(*nodes, tmp);
			}
			else /* all nodes */
				u_in >> fField[i];

			u_in.close();
		} 
		else 
		{
			cout << "\n FieldT::ReadRestart: file not found: " 
                 << file << '\n' <<   "     assuming order " << i << " = 0.0" << endl;
			fField[i] = 0.0;
		}

		/* next */
		deriv.Append("D");
	}

	/* KBC controllers */
	for (int j = 0; j < fKBC_Controllers.Length(); j++)
		fKBC_Controllers[j]->ReadRestart(in);

	/* FBC controllers */
	for (int i = 0; i < fFBC_Controllers.Length(); i++)
		fFBC_Controllers[i]->ReadRestart(in);
}

void FieldT::WriteRestart(ofstreamT& out, const ArrayT<int>* nodes) const
{
	/* external file */
	ofstreamT u_out;
	u_out.precision(out.precision());

	/* write field */
	StringT deriv;
	for (int i = 0; i < fField.Length(); i++)
	{
		/* file name */
		StringT file = out.filename();
		file.Append(".", fName);
		file.Append(".", deriv, "u");

		/* write */
		u_out.open(file);
		if (nodes) /* select nodes */
		{
			dArray2DT tmp(nodes->Length(), NumDOF());
			tmp.RowCollect(*nodes, fField[i]);
			u_out << tmp << '\n';
		}
		else /* all nodes */
			u_out << fField[i] << '\n';
		u_out.close();
	
		/* next */
		deriv.Append("D");
	}

	/* KBC controllers */
	for (int j = 0; j < fKBC_Controllers.Length(); j++)
		fKBC_Controllers[j]->WriteRestart(out);

	/* FBC controllers */
	for (int i = 0; i < fFBC_Controllers.Length(); i++)
		fFBC_Controllers[i]->WriteRestart(out);
}

/* write output data */
void FieldT::WriteOutput(ostream& out) const
{
	/* KBC controllers */
	for (int j = 0; j < fKBC_Controllers.Length(); j++)
		fKBC_Controllers[j]->WriteOutput(out);

	/* FBC controllers */
	for (int i = 0; i < fFBC_Controllers.Length(); i++)
		fFBC_Controllers[i]->WriteOutput(out);
}

/* write field parameters to output stream */
void FieldT::WriteParameters(ostream& out) const
{
	out << "\n F i e l d : \"" << Name() << "\"\n\n";
	out << " Number of degrees of freedom. . . . . . . . . . = " << NumDOF() << '\n';
	for (int i = 0; i < fLabels.Length(); i++)
		out << '\t' << fLabels[i] << '\n';
	out << " Number of time derivatives. . . . . . . . . . . = " << Order() << '\n';
	out << " Group number. . . . . . . . . . . . . . . . . . = " << Group() << '\n';
	out.flush();
}

/* accumulate source terms */
void FieldT::RegisterSource(const StringT& ID, const dArray2DT& source) const
{
	/* search */
	int dex = SourceIndex(ID);
	
	/* NOTE: need this function to be const else ElementBaseT cannot call it */
	FieldT* non_const_this = const_cast<FieldT*>(this);

	/* add new */
	if (dex == -1) 
	{
		/* add ID to list */
		non_const_this->fID.Append(ID);
	
		/* source */
		dArray2DT* new_source = new dArray2DT;
		non_const_this->fSourceOutput.Append(new_source);
	}

	/* register sources */
	non_const_this->fSourceID.Append(ID);
	non_const_this->fSourceBlocks.Append(&source);
}

/* element source terms */
const dArray2DT* FieldT::Source(const StringT& ID) const
{
	/* search */
	int dex = SourceIndex(ID);

	/* accmulate and return */
	if (dex > -1) {
	
		/* return value */
		dArray2DT& source = *(fSourceOutput[dex]);
	
		/* accumulate */
		int count = 0;
		for (int i = 0; i < fSourceID.Length(); i++)
			if (fSourceID[i] == ID)
			{
				/* implied that all sources have the same dimension */
				if (count == 0)
					source = *(fSourceBlocks[i]);
				else
					source += *(fSourceBlocks[i]);
				count++;
			}
			
		return &source;
	}
	else
		return NULL;
}

/**********************************************************************
 * Private
 **********************************************************************/

/* return the index for the source of the given ID */
int FieldT::SourceIndex(const StringT& ID) const
{
	/* search */
	for (int i = 0; i < fID.Length(); i++)
		if (fID[i] == ID)
			return i;

	/* fail */
	return -1;
}

bool FieldT::Apply_IC(const IC_CardT& card)
{
	const char caller[] = "FieldT::Apply_IC";

	/* check order - do not allow initial conditions on highest order derivative
	 * of the field */
	if (card.Order() > Order()-1)
		ExceptionT::OutOfRange(caller, "order is out of range {0,%d}: %d", 
			Order()-1, card.Order());

	/* decode */
	int node     = card.Node();
	int dof      = card.DOF();
	double value = card.Value();

	/* set */
	bool active = true;
	dArray2DT& field = fField[card.Order()];
	if (node == -1)
	{
		/* must all be free */
		iArrayT tmp(fEqnos.MajorDim());
		fEqnos.ColumnCopy(dof,tmp);
		if (tmp.Min() < 1) active = false;

		/* set value */
		field.SetColumn(dof, value);
	}
	else
	{
		/* must be free DOF */
		if (node != -1 && EquationNumber(node,dof) < 1) active = false;

		/* set value */
		field(node, dof) = value;		
	}
	
	return active;
}

/* mark global equations with the specified BC */
void FieldT::SetBCCode(const KBC_CardT& card)
{
	/* mark equation */
	fEqnos(card.Node(), card.DOF()) = kPrescribed;
}

/* resolve destinations for force BC's */
void FieldT::SetFBCEquations(void)
{
	/* allocate */
	int nbc = fFBC.Length();
	fFBCValues.Dimension(nbc);
	fFBCEqnos.Dimension(nbc);

	/* force cards */
	int bad_count = 0;
	for (int i = 0; i < nbc; i++)
	{
		int nodenum, dofnum;
		fFBC[i].Destination(nodenum, dofnum);
		fFBCEqnos[i] = fEqnos(nodenum, dofnum);

		/* component is prescribed */
		if (fFBCEqnos[i] == kPrescribed) bad_count++;
	}
	
	/* warn */
	if (bad_count > 0)
		cout << "\n FieldT::SetFBCEquations: WARNING: found "
		     << bad_count << " prescribed forces\n"
		     <<   "     on equations with prescribed values" << endl;
}
