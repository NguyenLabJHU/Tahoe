#ifndef __vtkInteractorStyle_h
#define __vtkInteractorStyle_h

#include "vtkRenderWindowInteractor.h"

// motion flags
#define VTKIS_START   0
#define VTKIS_ROTATE 1
#define VTKIS_ZOOM   2
#define VTKIS_PAN    3
#define VTKIS_SPIN   4
#define VTKIS_DOLLY  5
#define VTKIS_USCALE 6
#define VTKIS_TIMER  7 
#define VTKIS_ANIM_OFF 0
#define VTKIS_ANIM_ON  1

class vtkPolyDataMapper;
class vtkOutlineSource;

class VTK_RENDERING_EXPORT vtkInteractorStyle : public vtkObject 
{
public:
  // Description:
  // This class must be supplied with a vtkRenderWindowInteractor wrapper or
  // parent. This class should not normally be instantiated by application
  // programmers.
  static vtkInteractorStyle *New();

  vtkTypeMacro(vtkInteractorStyle,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the Interactor wrapper being controlled by this object.
  virtual void SetInteractor(vtkRenderWindowInteractor *interactor);
  vtkGetObjectMacro(Interactor, vtkRenderWindowInteractor);

  // Description:
  // If AutoAdjustCameraClippingRange is on, then before each render the
  // camera clipping range will be adjusted to "fit" the whole scene. Clipping
  // will still occur if objects in the scene are behind the camera or
  // come very close. If AutoAdjustCameraClippingRange is off, no adjustment
  // will be made per render, but the camera clipping range will still
  // be reset when the camera is reset.
  vtkSetClampMacro( AutoAdjustCameraClippingRange, int, 0, 1 );
  vtkGetMacro( AutoAdjustCameraClippingRange, int );
  vtkBooleanMacro( AutoAdjustCameraClippingRange, int );
  
  // Description:
  // When an event occurs, we must determine which Renderer the event
  // occurred within, since one RenderWindow may contain multiple
  // renderers. We also need to know what camera to operate on.
  // This is just the ActiveCamera of the poked renderer.
  void FindPokedCamera(int,int);
  void FindPokedRenderer(int,int);

  // Description:
  // When picking successfully selects an actor, this method highlights the
  // picked prop appropriately. Currently this is done by placing a bounding 
  // box around a picked vtkProp3D, and using the PickColor to highlight a
  // vtkProp2D. 
  virtual void HighlightProp(vtkProp *prop);
  virtual void HighlightActor2D(vtkActor2D *actor2D);
  virtual void HighlightProp3D(vtkProp3D *prop3D);

  // Description:
  // Set/Get the pick color (used by default to color vtkActor2D's).
  // The color is expressed as red/green/blue values between (0.0,1.0).
  vtkSetVector3Macro(PickColor,float);
  vtkGetVectorMacro(PickColor, float, 3);

  // Description:
  // Generic event bindings must be overridden in subclasses
  virtual void OnMouseMove  (int ctrl, int shift, int X, int Y);
  virtual void OnLeftButtonDown(int ctrl, int shift, int X, int Y);
  virtual void OnLeftButtonUp  (int ctrl, int shift, int X, int Y);
  virtual void OnMiddleButtonDown(int ctrl, int shift, int X, int Y);
  virtual void OnMiddleButtonUp  (int ctrl, int shift, int X, int Y);
  virtual void OnRightButtonDown(int ctrl, int shift, int X, int Y);
  virtual void OnRightButtonUp  (int ctrl, int shift, int X, int Y);

  // Description:
  // OnChar implements keyboard functions, but subclasses can override this 
  // behavior
  virtual void OnChar   (int ctrl, int shift, char keycode, int repeatcount);
  virtual void OnKeyDown(int ctrl, int shift, char keycode, int repeatcount);
  virtual void OnKeyUp  (int ctrl, int shift, char keycode, int repeatcount);
  virtual void OnKeyPress(int ctrl, int shift, char keycode, char *keysym, 
                          int repeatcount);
  virtual void OnKeyRelease(int ctrl, int shift, char keycode, char *keysym,
                            int repeatcount);

  // Description:
  // These are more esoteric events, but are useful in some cases.
  virtual void OnConfigure(int width, int height);
  virtual void OnEnter(int ctrl, int shift, int x, int y);
  virtual void OnLeave(int ctrl, int shift, int x, int y);

  // Description:
  // OnTimer calls RotateCamera, RotateActor etc which should be overridden by
  // style subclasses.
  virtual void OnTimer();

  // Description:
  // Callbacks so that the application can override the default behaviour.
  void SetLeftButtonPressMethod(void (*f)(void *), void *arg);
  void SetLeftButtonPressMethodArgDelete(void (*f)(void *));
  void SetLeftButtonReleaseMethod(void (*f)(void *), void *arg);
  void SetLeftButtonReleaseMethodArgDelete(void (*f)(void *));
  void SetMiddleButtonPressMethod(void (*f)(void *), void *arg);
  void SetMiddleButtonPressMethodArgDelete(void (*f)(void *));
  void SetMiddleButtonReleaseMethod(void (*f)(void *), void *arg);
  void SetMiddleButtonReleaseMethodArgDelete(void (*f)(void *));
  void SetRightButtonPressMethod(void (*f)(void *), void *arg);
  void SetRightButtonPressMethodArgDelete(void (*f)(void *));
  void SetRightButtonReleaseMethod(void (*f)(void *), void *arg);
  void SetRightButtonReleaseMethodArgDelete(void (*f)(void *));

protected:
  vtkInteractorStyle();
  ~vtkInteractorStyle();

  // Will the clipping range be automatically adjust before each render?
  int AutoAdjustCameraClippingRange;
  void ResetCameraClippingRange();
  
  // convenience methods for converting between coordinate systems
  virtual void ComputeDisplayToWorld(double x, double y, double z,
                                     double *worldPt);
  virtual void ComputeWorldToDisplay(double x, double y, double z,
                                     double *displayPt);
  virtual void ComputeDisplayToWorld(double x, double y, double z,
                                     float *worldPt);
  virtual void ComputeWorldToDisplay(double x, double y, double z,
                                     float *displayPt);

  virtual void UpdateInternalState(int ctrl, int shift, int X, int Y);

  // These methods for the different interactions in different modes
  // are overridden in subclasses to perform the correct motion
  // This class provides a default implementation.

  virtual void RotateCamera(int x, int y);
  virtual void SpinCamera(int x, int y);
  virtual void PanCamera(int x, int y);
  virtual void DollyCamera(int x, int y);

  // utility routines used by state changes below
  virtual void StartState(int newstate);
  virtual void StopState();

  // Interaction mode entry points used internally.  
  virtual void StartAnimate();  
  virtual void StopAnimate();  
  virtual void StartRotate();
  virtual void EndRotate();
  virtual void StartZoom();
  virtual void EndZoom();
  virtual void StartPan();
  virtual void EndPan();
  virtual void StartSpin();
  virtual void EndSpin();
  virtual void StartDolly();
  virtual void EndDolly();
  virtual void StartUniformScale();
  virtual void EndUniformScale();
  virtual void StartTimer();
  virtual void EndTimer();

  // Data we need to maintain internally
  vtkRenderWindowInteractor *Interactor;
  //
  vtkCamera          *CurrentCamera;
  vtkLight           *CurrentLight;
  vtkRenderer        *CurrentRenderer;
  //
  float Center[2];
  float DeltaAzimuth;
  float DeltaElevation;
  int   CtrlKey;
  int   ShiftKey;
  int   LastPos[2];
  int   State;  
  int   AnimState;  
  float FocalDepth;  

  // for picking and highlighting props
  vtkOutlineSource   *Outline;
  vtkPolyDataMapper  *OutlineMapper;
  vtkActor           *OutlineActor;
  vtkRenderer        *PickedRenderer;
  vtkProp            *CurrentProp;
  int                PropPicked;          // boolean: prop picked?
  float              PickColor[3];        // support 2D picking
  vtkActor2D         *PickedActor2D;

  unsigned long LeftButtonPressTag;
  unsigned long LeftButtonReleaseTag;
  unsigned long MiddleButtonPressTag;
  unsigned long MiddleButtonReleaseTag;
  unsigned long RightButtonPressTag;
  unsigned long RightButtonReleaseTag;
private:
  vtkInteractorStyle(const vtkInteractorStyle&);  // Not implemented.
  void operator=(const vtkInteractorStyle&);  // Not implemented.
};

#endif