/* $Id: VTKBodyT.cpp,v 1.28 2002-06-19 16:43:41 recampb Exp $ */

#include "VTKBodyT.h"
#include "VTKBodyDataT.h"
#include "VTKFrameT.h"
#include "VTKUGridT.h"
#include "VTKMappedIdFilterT.h"
#include "CommandSpecT.h"
#include "ArgSpecT.h"

#include "vtkCubeAxesActor2D.h"
#include "vtkRenderer.h"
#include "vtkUnstructuredGrid.h"
#include "vtkWarpVector.h"
#include "vtkCellCenters.h"
#include "vtkSelectVisiblePoints.h"
#include "vtkLabeledDataMapper.h"
#include "vtkActor2D.h"
#include "vtkContourGrid.h"
#include "vtkLODActor.h"

/* array behavior */
const bool ArrayT<VTKBodyT*>::fByteCopy = true;
const bool ArrayT<vtkCubeAxesActor2D*>::fByteCopy = true;
const bool ArrayT<vtkCellCenters*>::fByteCopy = true;
const bool ArrayT<vtkMappedIdFilterT*>::fByteCopy = true;
const bool ArrayT<vtkSelectVisiblePoints*>::fByteCopy = true;
const bool ArrayT<vtkLabeledDataMapper*>::fByteCopy = true;
const bool ArrayT<vtkActor2D*>::fByteCopy = true;

/* constructor */
VTKBodyT::VTKBodyT(VTKFrameT* frame, VTKBodyDataT* body_data):
	fFrame(frame),
	fBodyData(body_data)
{
	/* set name */
	iSetName(fBodyData->iName());

	/* add all variables */
	AddVariables(*fBodyData);

	/* add some frame commands */
	CommandSpecT* command;
	command = fFrame->iCommand("Update");
	if (!command) throw eGeneralFail;
	iAddCommand(*command);

	command = fFrame->iCommand("Interactive");
	if (!command) throw eGeneralFail;
	iAddCommand(*command);

	command = fFrame->iCommand("Rotate");
	if (!command) throw eGeneralFail;
	iAddCommand(*command);

	command = fFrame->iCommand("ShowColorBar");
	if (!command) throw eGeneralFail;
	iAddCommand(*command);

	command = fFrame->iCommand("HideColorBar");
	if (!command) throw eGeneralFail;
	iAddCommand(*command);
	
	/* other commands */
	iAddCommand(CommandSpecT("ShowNodeNumbers"));
	iAddCommand(CommandSpecT("HideNodeNumbers"));
	iAddCommand(CommandSpecT("ShowElementNumbers"));
	iAddCommand(CommandSpecT("HideElementNumbers"));
	iAddCommand(CommandSpecT("ShowAxes"));
	iAddCommand(CommandSpecT("HideAxes"));
	iAddCommand(CommandSpecT("ShowGlyphs"));
	iAddCommand(CommandSpecT("HideCuttingPlane"));

	/* commands from body data */
	command = fBodyData->iCommand("Wire");
	if (!command) throw eGeneralFail;
	iAddCommand(*command);
	command = fBodyData->iCommand("Surface");
	if (!command) throw eGeneralFail;
	iAddCommand(*command);
	command = fBodyData->iCommand("Point");
	if (!command) throw eGeneralFail;
	iAddCommand(*command);
	command = fBodyData->iCommand("ShowContours");
	if (!command) throw eGeneralFail;
	iAddCommand(*command);
	command = fBodyData->iCommand("HideContours");
	if (!command) throw eGeneralFail;
	iAddCommand(*command);

// 	command = fBodyData->iCommand("ShowGlyphs");
// 	if (!command) throw eGeneralFail;
// 	iAddCommand(*command);
	command = fBodyData->iCommand("HideGlyphs");
	if (!command) throw eGeneralFail;
	iAddCommand(*command);

	
	CommandSpecT cut("ShowCuttingPlane", false);
	ArgSpecT oX(ArgSpecT::double_, "oX");
	oX.SetDefault(0.0);
	oX.SetPrompt("x-coordinate of origin");
	ArgSpecT oY(ArgSpecT::double_, "oY");
	oY.SetDefault(0.0);
	oY.SetPrompt("y-coordinate of origin");
	ArgSpecT oZ(ArgSpecT::double_, "oZ");
	oZ.SetDefault(0.0);
	oZ.SetPrompt("z-coordinate of origin");
	cut.AddArgument(oX);
	cut.AddArgument(oY);
	cut.AddArgument(oZ);

	ArgSpecT nX(ArgSpecT::double_, "nX");
	nX.SetDefault(0.0);
	nX.SetPrompt("x-coordinate of normal");
	ArgSpecT nY(ArgSpecT::double_, "nY");
	nY.SetDefault(1.0);
	nY.SetPrompt("y-coordinate of normal");
	ArgSpecT nZ(ArgSpecT::double_, "nZ");
	nZ.SetDefault(0.0);
	nZ.SetPrompt("z-coordinate of normal");
	cut.AddArgument(nX);
	cut.AddArgument(nY);
	cut.AddArgument(nZ);
	iAddCommand(cut);

}

/* destructor */
VTKBodyT::~VTKBodyT(void)
{
  if (fFrame)
	{
	vtkRenderer* renderer = fFrame->Renderer();

	/* free axis actors */
	for (int i = 0; i < fAxes.Length(); i++)
		if (fAxes[i])
		{
			renderer->RemoveActor(fAxes[i]);
			fAxes[i]->Delete();
		}
		
	/* node number actors */
	for (int i = 0; i < fNodeLabelActor.Length(); i++)
	{
		if (fNodeLabelActor[i]) {
			renderer->RemoveActor(fNodeLabelActor[i]);
			fNodeLabelActor[i]->Delete();
		}
		if (fNodeLabelMapper[i]) fNodeLabelMapper[i]->Delete();
		if (fVisPoints[i]) fVisPoints[i]->Delete();
	}

	/* element number actors */
	for (int i = 0; i < fCellLabelActor.Length(); i++)
	{
		/* element labels */
		if (fCellLabelActor[i]) {
			renderer->RemoveActor(fCellLabelActor[i]);
			fCellLabelActor[i]->Delete();
		}
		if (fCellCenters[i]) fCellCenters[i]->Delete();
		if (fVisCells[i]) fVisCells[i]->Delete();
		if (fCellLabelMapper[i]) fCellLabelMapper[i]->Delete();
	}

	/* id filters */
	for (int i = 0; i < fIDFilter.Length(); i++)
		if (fIDFilter[i]) fIDFilter[i]->Delete();
	}
  else /* without a frame these should be empty */
	{
	  if (fAxes.Length() > 0) cout << "~VTKBodyT: axes list not empty"<< endl;
	  if (fIDFilter.Length() > 0) cout << "~VTKBodyT: id filter list not empty"<< endl;
	}
}

/* execute console command. \return true is executed normally */
bool VTKBodyT::iDoCommand(const CommandSpecT& command, StringT& line)
{
	/* check */
	if (!fFrame) {
		cout << "\n VTKBodyT::iDoCommand: frame pointer not set" << endl;
		throw eGeneralFail;
	}

	/* resolve command */
	if (command.Name() == "Update")
		return fFrame->iDoCommand(command, line);
	else if (command.Name() == "Interactive")
		return fFrame->iDoCommand(command, line);
	else if (command.Name() == "Rotate")
		return fFrame->iDoCommand(command, line);
	else if (command.Name() == "ShowColorBar")
		return fFrame->iDoCommand(command, line);
	else if (command.Name() == "HideColorBar")
		return fFrame->iDoCommand(command, line);

	else if (command.Name() == "Wire")
		return fBodyData->iDoCommand(command, line);
	else if (command.Name() == "Surface")
		return fBodyData->iDoCommand(command, line);
	else if (command.Name() == "Point")
		return fBodyData->iDoCommand(command, line);
	else if (command.Name() == "ShowContours")
		return fBodyData->iDoCommand(command, line);
	else if (command.Name() == "HideContours")
		return fBodyData->iDoCommand(command, line);

// 	else if (command.Name() == "ShowGlyphs")
// 		return fBodyData->iDoCommand(command, line);
	else if (command.Name() == "HideGlyphs")
		return fBodyData->iDoCommand(command, line);


	else if (command.Name() == "ShowNodeNumbers")
	{
		if (fNodeLabelActor.Length() > 0)
		{
			cout << "hide node numbers first" << endl;
			return false;
		}
		else
		{
			/* unstructured grids */
			const ArrayT<VTKUGridT*>& ugrids = fBodyData->UGrids();
			
			/* need filters */
			if (fIDFilter.Length() != ugrids.Length())
			{
				/* cell labels showing */
				bool show_cells = false;
				if (fCellLabelActor.Length() > 0) {
					StringT dummy;
					const CommandSpecT* comm = iResolveCommand("HideElementNumbers", dummy);
					if (!comm) return false;
					iDoCommand(*comm, dummy);
					show_cells = true;
				}
				
				/* initialize ID filters */
				SetIDFilters();
				
				/* restore cell labels */
				if (show_cells) {
					StringT dummy;
					const CommandSpecT* comm = iResolveCommand("ShowElementNumbers", dummy);
					if (!comm) return false;
					iDoCommand(*comm, dummy);
				}
			}
			
			/* coordinate axis list */
			fVisPoints.Allocate(ugrids.Length());
			fNodeLabelMapper.Allocate(ugrids.Length());
			fNodeLabelActor.Allocate(ugrids.Length());

			fVisPoints = NULL;
			fNodeLabelMapper = NULL;
			fNodeLabelActor = NULL;

			for (int i = 0; i < ugrids.Length(); i++)
				if (ugrids[i]->Type() == VTKUGridT::kElementSet)
				{
					VTKUGridT* ugrid = ugrids[i];

					/* generate node id's */
					fIDFilter[i]->SetPointMap(fBodyData->PointNumberMap().Pointer());
					fIDFilter[i]->PointIdsOn();

					/* label mapper */
					vtkLabeledDataMapper* nodeLabelMapper = vtkLabeledDataMapper::New();
					//nodeLabelMapper->SetLabelModeToLabelIds();
					//nodeLabelMapper->SetLabelModeToLabelFieldData();
					nodeLabelMapper->SetLabelModeToLabelScalars(); /* idFilter output's id's as scalars */
					nodeLabelMapper->ShadowOff();

					/* visibility */
					if (ugrid->NumSD() == 3)
					{
						/* visibility filter */
						vtkSelectVisiblePoints* visPoints = vtkSelectVisiblePoints::New();
						visPoints->SetInput(fIDFilter[i]->GetOutput());
						visPoints->SetRenderer(fFrame->Renderer());
						//visPoints->SelectionWindowOn(); // this slows things down considerably
						fVisPoints[i] = visPoints;
			
						/* label mapper */
						nodeLabelMapper->SetInput(visPoints->GetOutput());
					}
					/* assume ALL visible in 2D */
					else
						/* label mapper */
						nodeLabelMapper->SetInput(fIDFilter[i]->GetOutput());

					/* labels */
					vtkActor2D* nodeLabelActor = vtkActor2D::New();
			 
					nodeLabelActor->SetMapper(nodeLabelMapper);
					nodeLabelActor->VisibilityOn();
					nodeLabelActor->GetProperty()->SetColor(0,1,1);
							
					fFrame->Renderer()->AddActor(nodeLabelActor);
					
					/* add to lists */
					fNodeLabelMapper[i] = nodeLabelMapper;
					fNodeLabelActor[i] = nodeLabelActor;
				}
		
			return true;
		}
	}
	else if (command.Name() == "HideNodeNumbers")
	{
		if (fNodeLabelActor.Length() == 0)
		{
			cout << "node numbers not showing" << endl;
			return false;
		}
		else
		{
			vtkRenderer* renderer = fFrame->Renderer();
			for (int i = 0; i < fIDFilter.Length(); i++)
			{
				/* reset filter */
				if (fIDFilter[i])
				{
					fIDFilter[i]->PointIdsOff();
					fIDFilter[i]->SetPointMap(NULL);
				}

				/* clean-up */
				if (fNodeLabelActor[i])
				{
					renderer->RemoveActor(fNodeLabelActor[i]);
					fNodeLabelActor[i]->Delete();
				}
				if (fVisPoints[i]) fVisPoints[i] ->Delete();
				if (fNodeLabelMapper[i]) fNodeLabelMapper[i]->Delete();
			}
			
			fVisPoints.Allocate(0);
			fNodeLabelMapper.Allocate(0);
			fNodeLabelActor.Allocate(0);

			/* no cell labels showing */
			if (fCellLabelActor.Length() == 0) {
				for (int i = 0; i < fIDFilter.Length(); i++)
					if (fIDFilter[i]) fIDFilter[i] ->Delete();
				fIDFilter.Allocate(0);
			}

			return true;
		}	
	}
	else if (command.Name() == "ShowElementNumbers")
	{
		if (fCellLabelActor.Length() > 0)
		{
			cout << "hide element numbers first" << endl;
			return false;
		}
		else
		{
			/* unstructured grids */
			const ArrayT<VTKUGridT*>& ugrids = fBodyData->UGrids();
			
			/* need filters */
			if (fIDFilter.Length() != ugrids.Length())
			{
				/* node labels showing */
				bool show_nodes = false;
				if (fNodeLabelActor.Length() > 0) {
					StringT dummy;
					const CommandSpecT* comm = iResolveCommand("HideNodeNumbers", dummy);
					if (!comm) return false;
					iDoCommand(*comm, dummy);
					show_nodes = true;
				}
				
				/* initialize ID filters */
				SetIDFilters();
				
				/* restore cell labels */
				if (show_nodes) {
					StringT dummy;
					const CommandSpecT* comm = iResolveCommand("ShowNodeNumbers", dummy);
					if (!comm) return false;
					iDoCommand(*comm, dummy);
				}
			}
			
			/* coordinate axis list */
			fCellCenters.Allocate(ugrids.Length());
			fVisCells.Allocate(ugrids.Length());
			fCellLabelMapper.Allocate(ugrids.Length());
			fCellLabelActor.Allocate(ugrids.Length());
			
			fCellCenters = NULL;
			fVisCells = NULL;
			fCellLabelMapper = NULL;
			fCellLabelActor = NULL;

			for (int i = 0; i < ugrids.Length(); i++)
				if (ugrids[i]->Type() == VTKUGridT::kElementSet)
				{
					VTKUGridT* ugrid = ugrids[i];

					/* generate node id's */
					fIDFilter[i]->SetCellMap(ugrid->CellNumberMap().Pointer());
					fIDFilter[i]->CellIdsOn();

					/* label mapper */
					vtkLabeledDataMapper* cellLabelMapper = vtkLabeledDataMapper::New();
					//cellLabelMapper->SetLabelModeToLabelIds();
					//cellLabelMapper->SetLabelModeToLabelFieldData();
					cellLabelMapper->SetLabelModeToLabelScalars(); /* idFilter output's id's as scalars */
					cellLabelMapper->ShadowOff();

					/* cell centers */
					vtkCellCenters* cellCenter = vtkCellCenters::New();

					/* visibility */
					if (0 && ugrid->NumSD() == 3)
					{
						/* cell centers */
						cellCenter->SetInput(fIDFilter[i]->GetOutput());

						/* visibility filter */
						vtkSelectVisiblePoints* visCells = vtkSelectVisiblePoints::New();
						visCells->SetInput(cellCenter->GetOutput());
						visCells->SetRenderer(fFrame->Renderer());
						//visCells->SelectionWindowOn(); // this slows things down considerably
						fVisCells[i] = visCells;
			
						/* label mapper */
						cellLabelMapper->SetInput(visCells->GetOutput());
					}
					/* assume ALL visible in 2D */
					else
					{
						/* straight from the filter */
						cellCenter->SetInput(fIDFilter[i]->GetOutput());
					
						/* label mapper */
						cellLabelMapper->SetInput(cellCenter->GetOutput());	
					}

					/* labels */
					vtkActor2D* cellLabelActor = vtkActor2D::New();
					cellLabelActor->SetMapper(cellLabelMapper);
					cellLabelActor->VisibilityOn();
					cellLabelActor->GetProperty()->SetColor(1,0,0);
							
					fFrame->Renderer()->AddActor(cellLabelActor);
					
					/* add to lists */
					fCellCenters[i] = cellCenter;
					fCellLabelMapper[i] = cellLabelMapper;
					fCellLabelActor[i] = cellLabelActor;
				}
		
			return true;
		}
	}
	else if (command.Name() == "HideElementNumbers")
	{
		if (fCellLabelActor.Length() == 0)
		{
			cout << "element numbers not showing" << endl;
			return false;
		}
		else
		{
			vtkRenderer* renderer = fFrame->Renderer();
			for (int i = 0; i < fIDFilter.Length(); i++)
			{
				/* reset filter */
				if (fIDFilter[i])
				{
					fIDFilter[i]->CellIdsOff();
					fIDFilter[i]->SetCellMap(NULL);
				}
			
				/* clean-up */
				if (fCellLabelActor[i])
				{
					renderer->RemoveActor(fCellLabelActor[i]);
					fCellLabelActor[i]->Delete();
				}
				if (fVisCells[i]) fVisCells[i] ->Delete();
				if (fCellLabelMapper[i]) fCellLabelMapper[i]->Delete();
				if (fCellCenters[i]) fCellCenters[i]->Delete();
			}
			
			fVisCells.Allocate(0);
			fCellLabelMapper.Allocate(0);
			fCellLabelActor.Allocate(0);
			fCellCenters.Allocate(0);

			/* no node labels showing */
			if (fNodeLabelActor.Length() == 0) {
				for (int i = 0; i < fIDFilter.Length(); i++)
					if (fIDFilter[i]) fIDFilter[i] ->Delete();
				fIDFilter.Allocate(0);
			}

			return true;
		}	
	}
	else if (command.Name() == "ShowAxes")
	{
		if (fAxes.Length() != 0) 
		{
			cout << "hide axes first" << endl;
			return false;
		} 
		else 
		{
			/* unstructured grids */
			const ArrayT<VTKUGridT*>& ugrids = fBodyData->UGrids();
		
			/* coordinate axis list */
			fAxes.Allocate(ugrids.Length());
			fAxes = NULL;
		
			vtkRenderer* renderer = fFrame->Renderer();
			for (int i = 0; i < ugrids.Length(); i++)
				if (ugrids[i]->Type() == VTKUGridT::kElementSet)
				{
					vtkCubeAxesActor2D* axes = vtkCubeAxesActor2D::New();

					/* track deformation */
					if (ugrids[i]->Warp())
						axes->SetInput(ugrids[i]->Warp()->GetOutput());
					else
						axes->SetInput(ugrids[i]->UGrid());

					axes->SetCamera(renderer->GetActiveCamera());
					axes->SetLabelFormat("%6.4g");
					if (ugrids[i]->NumSD() == 2) axes->ZAxisVisibilityOff();
					//axes->SetCornerOffset(.2);
					//axes->ShadowOn();
					//axes->SetFlyModeToOuterEdges();
					axes->SetFlyModeToClosestTriad();
					//axes->SetFontFactor(1.8);
					axes->GetProperty()->SetColor(1,1,1);
					axes->ShadowOff();
					// axes->SetBounds(0,1,0,1,0,1);
					axes->VisibilityOn();
					renderer->AddActor(axes);
	
					fAxes[i] = axes;
				}
				
			return true;
		}	
	}
	else if (command.Name() == "HideAxes")
	{
		if (fAxes.Length() == 0)
		{
			cout << "no axes not showing" << endl;
			return false;
		}
		else
		{
			/* free all axes actors */
			vtkRenderer* renderer = fFrame->Renderer();
			for (int i = 0; i < fAxes.Length(); i++)
				if (fAxes[i])
				{
					fAxes[i]->VisibilityOff();
					renderer->RemoveActor(fAxes[i]);
					fAxes[i]->Delete();
				}
			fAxes.Allocate(0);
			return true;
		}
	}


	else if (command.Name() == "ShowGlyphs")
	  {
      
	    ArrayT<VTKUGridT*> fUGrids = fBodyData->UGrids();
	       for (int i = 0; i < fBodyData->UGrids().Length(); i++)
		 {
		   fUGrids[i]->Glyphing(fBodyData->GetVectors(), fFrame->Renderer());

		 }
	     return true;
	    
	  }

	else if (command.Name() == "ShowCuttingPlane")
	  {
	    ArrayT<VTKUGridT*> fUGrids = fBodyData->UGrids();
	    double oX, oY, oZ, nX, nY, nZ;
	    bool warp = false;
	    if (fBodyData->NumVectors() > 0)
	      warp = true;
	    
	    for (int i = 0; i < fBodyData->UGrids().Length(); i++)
	      {
		command.Argument("oX").GetValue(oX);
		command.Argument("oY").GetValue(oY);
		command.Argument("oZ").GetValue(oZ);
		command.Argument("nX").GetValue(nX);
		command.Argument("nY").GetValue(nY);
		command.Argument("nZ").GetValue(nZ);
		    fUGrids[i]->CuttingPlane(fFrame->Renderer(), oX, oY, oZ, nX, nY, nZ, warp );
		    
	      }
	    return true;
	    
	    
	  }

	else if (command.Name() == "HideCuttingPlane")
	  {
	    ArrayT<VTKUGridT*> fUGrids = fBodyData->UGrids();
	    for (int i = 0; i < fBodyData->UGrids().Length(); i++)
	      {
		
		fUGrids[i]->HideCuttingPlane(fFrame->Renderer());
		
	      }
	    return true;
	  }
	

	else
		/* inherited */
		return iConsoleObjectT::iDoCommand(command, line);
}

/* change the plot variable */
bool VTKBodyT::ChangeVars(const StringT& var)
{
	if (fBodyData->ChangeVars(var))
	{
		/* remove all variables */
		DeleteVariables();
		
		/* re-add all the VTKBodyDataT variables */
		AddVariables(*fBodyData);
		
		return true;
	}
	else return false;
}

/* add actors in self to the given renderer */
void VTKBodyT::AddToFrame(void)
{
	/* the body */
	fBodyData->AddToRenderer(fFrame->Renderer());

	/* axes */
	if (fAxes.Length())
	{
		StringT tmp;	
		iDoCommand(*iCommand("ShowAxes"), tmp);
	}
}

/* add actors in self to the given renderer */
void VTKBodyT::RemoveFromFrame(void)
{
	/* the body */
	fBodyData->RemoveFromRenderer(fFrame->Renderer());

	/* axes */
	if (fAxes.Length())
	{
		StringT tmp;	
		iDoCommand(*iCommand("HideAxes"), tmp);
	}
}

/*************************************************************************
* private
*************************************************************************/

 /* (re-) set the list of ID filters */
void VTKBodyT::SetIDFilters(void)
{
	/* unstructured grids */
	const ArrayT<VTKUGridT*>& ugrids = fBodyData->UGrids();

	/* size has changed */
	if (fIDFilter.Length() != ugrids.Length())
	{
		/* free existing */
		for (int i = 0; i < fIDFilter.Length(); i++)
			if (fIDFilter[i]) fIDFilter[i]->Delete();
			
		fIDFilter.Allocate(ugrids.Length());
		fIDFilter = NULL;
		
		/* configure */
		for (int i = 0; i < fIDFilter.Length(); i++)
			if (ugrids[i]->Type() == VTKUGridT::kElementSet)
			{
				VTKUGridT* ugrid = ugrids[i];
				
				/* initialize all OFF */
				vtkMappedIdFilterT* idFilter = vtkMappedIdFilterT::New();
				idFilter->PointIdsOff();
				idFilter->CellIdsOff();
				idFilter->FieldDataOff();

				/* warping */
				if (ugrid->Warp())
					idFilter->SetInput(ugrid->Warp()->GetOutput());
				else
					idFilter->SetInput(ugrid->UGrid());

				/* add to list */
				fIDFilter[i] = idFilter;
			}
	}
}

