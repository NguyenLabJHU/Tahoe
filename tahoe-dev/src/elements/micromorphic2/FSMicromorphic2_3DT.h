/* $Id: FSMicromorphic2_3DT.h,v 1.11 2016-03-10 01:45:24 tahoe.fash5153 Exp $ */
//DEVELOPMENT
#ifndef _FS_MICROMORPHIC2_3D_T_H_
#define _FS_MICROMORPHIC2_3D_T_H_

/* base classes */
#include "ElementBaseT.h"
#include "StringT.h"
#include "Traction_CardT.h"
#include "ShapeFunctionT.h"
#include "eIntegratorT.h"
#include "ModelManagerT.h"

#include "ifstreamT.h"
#include "ofstreamT.h"

#include "iAutoArrayT.h"
#include "ScheduleT.h"

/* direct members */
#include "LocalArrayT.h"
#include "GeometryT.h"
#include "dTensor3DT.h"// I added this header file to use Tensor built-in functions!

#include "VariArrayT.h"
#include "nVariArray2DT.h"
#include "VariLocalArrayT.h"
#include "IntegratorT.h"

namespace Tahoe {

/* forward declarations */
class ShapeFunctionT;
class Traction_CardT;
class StringT;

/** FSMicromorphic2_3DT: This class contains a coupled finite deformation
 * micromorphic (displacement and micro-displacement gradient dofs) finite
 * element implementation in 3D.  It is based on the FSMicromorphic3DT implementation
 * with some modifications.
 **/

class FSMicromorphic2_3DT: public ElementBaseT
{

public:
/* material parameters */
    enum fMaterial_T         {
        kMu,
        kLambda,
        kKappa,
        kNu,
        kSigma_const,
        kTau,
        kEta,
        kRho_0,
        kTau1,
        kTau2,
        kTau3,
        kTau4,
        kTau5,
        kTau6,
        kTau7,
        kTau8,
        kTau9,
        kTau10,
        kTau11,
        // plasticity parameters
        kHc,
        kc0,
        kHc_chi,
        kc0_chi,
        kHc_nablachi,
        kc0_nablachi,
        kc1_nablachi,
        kc2_nablachi,
        kHGc_chi,
        kGc0_chi1,
        kGc0_chi2,
        kGc0_chi3,
        //kZ0c,
        kFphi,
        kDpsi,
        kFphi_chi,
        kDpsi_chi,
        kFphi_nablachi,
        kDpsi_nablachi,
        kFGphi_chi,
        kDGpsi_chi,
        //
        kKappa0,
        kKappa0_chi,
        kKappa0_nablachi,
        kKappa1_nablachi,
        kKappa2_nablachi,
        kHkappa,
        kHKappa_chi,
        kHKappa_nablachi,
        kR,
        kR_chi,
        kR_nablachi,
        kAlpha,
        kAlpha_chi,
        kAlpha_nablachi,
        kCapped_Model_Flag,
        kDynamic_Analysis_Flag,
        kLength_scale1,
        kLength_scale2,
        kLength_scale3,
        kQ8P8_0_Q27P8_1_Flag,
        kg,
        kg1,
        kg2,
        kg3,
        //add to this list
        kNUM_FMATERIAL_TERMS        };

    enum fMaterialState_T         {

        kc,
        kc_current_configuration,
        kc_chi,
        kc_nablachi0,
        kc_nablachi1,
        kc_nablachi2,

        Kappa,
        Kappa_chi,
        Kappa_nablachi0,
        Kappa_nablachi1,
        Kappa_nablachi2,

        khc,
        khc_chi,
        khc_nablachi,

        khKappa,
        khKappa_chi,
        khKappa_nablachi,

        kEpsVolp,
        kEpsVolp_chi,
        kEpsVolp_nablachi,

        kZkappa,
        kZkappa_chi,
        kZkappa_nablachi,


        kDelgamma,
        kDelgammachi,
        kDelgammanablachi,
        kMeanSPK,
        ktrm,
        km_inv,
        kinvdevS,
        kMeanSIGMA,
        kinvdevSIGMA,
        kinvtrM,
        kinvdevM,
        kinvPhi,
        kinvGPhi,
        ktreps,
        kdeveps,
        kinvtrgammastn,
        kinvdevgammastn,
/*        kGc_chi1,
        kGc_chi2,
        kGc_chi3,
        kDelgammaGchi,
        khGc_chi,*/
//        kF11,
//        kF12,
//        kF13,
//        kF21,
//        kF22,
//        kF23,
//        kF31,
//        kF32,
//        kF33,
//        kFe11,
//        kFe12,
//        kFe13,
//        kFe21,
//        kFe22,
//        kFe23,
//        kFe31,
//        kFe32,
//        kFe33,
//        kX11,
//        kX12,
//        kX13,
//        kX21,
//        kX22,
//        kX23,
//        kX31,
//        kX32,
//        kX33,
//        kXe11,
//        kXe12,
//        kXe13,
//        kXe21,
//        kXe22,
//        kXe23,
//        kXe31,
//        kXe32,
//        kXe33,
        kNUM_FMATERIAL_STATE_TERMS
    };

///////////// Cap Model Parameters /////////////
    dMatrixT Kappa_nablachi_n;
    double fMacfunc_nablachi_tr;
    double fMacfunc_nablachi;
    double Xphi_n;
    double Xphi_m_n;
    double fMacfunc_tr;
    double fPhiCap_tr;
    double Xphi_chi_n;
    double Xphi_chi_m_n;
    double fMacfunc_chi_tr;
    double fMacfunc_chi;
    double fPhiCap_chi_tr;
    double fPhiCap_chi;
    double Xphi_nablachi_n;
    double Xphi_nablachi_m_n;
    double fPhiCap_nablachi_tr;
    double Cpsi_tr;
    double Cpsi_chi_tr;
    double Cpsi_nablachi_tr;
    double Cpsi;
    double Cpsi_chi;
    double Cpsi_nablachi;
    double Cphi;
    double Cphi_chi;
    double Cphi_nablachi;
    double Cphi_tr;
    double Cphi_chi_tr;
    double Cphi_nablachi_tr;
    double Xphi;
    double Xphi_m;
    double Xphi_chi;
    double Xphi_chi_m;
    double fMacfunc;
    double fPhiCap;
    double dKappadDelgamma;
    double dXphi_m_dDelgamma;
    double dFphicapdDelgamma;
    double dKappachidDelgammachi;
    double dXphi_chi_m_dDelgammachi;
    double dFphichicapdDelgammachi;
    double dFphicapdDelgammachi;
    double dFphichicapdDelgamma;
    double Norm_Kappa_nablachi_n;
    dMatrixT dfKappa_nablachidDelgammanablachi;
    double dNorm_Kappa_nablachidDelgammanablachi;
    dMatrixT Kappa_nablachi;
    double Norm_Kappa_nablachi;
    double devMKLMddevMKLMdDelgammanablachi;
    dMatrixT fDelKappa_nablachi;
    double dXphi_nablachi_mdDelgammanablachi;
    double dFphi_nabalchidDelgammanablachi;
    double Xphi_nablachi;
    double Xphi_nablachi_m;
    double fPhiCap_nablachi;
    double signMacfunc;
    double signfMacfunc_chi;
    double signfMacfunc_nablachi;
    double dFphidKappa;
    double dFYdKappa;
    double dFphichidKappachi;
    double dFYchidKappachi;
    dMatrixT dFphinablachidKappanablachi;
    dMatrixT dFYnablachidKappanablachi;
    double Jp;
    double Xpsi_n;
    double Xpsi_m_n;
    double Xpsi_chi_n;
    double Xpsi_chi_m_n;
    double Xpsi_nablachi_n;
    double Xpsi_nablachi_m_n;
    double fPsiCap_tr;
    double fPsiCap_chi_tr;
    double fPsiCap_nablachi_tr;
    double Xpsi_m;
    double fPsiCap;
    double Xpsi_chi_m;
    double fPsiCap_chi;
    double Xpsi_nablachi_m;
    double fPsiCap_nablachi;

//////////////////////////////////////////////////////////////////////
    double fdGdc;
    double fdGYchidcchi;
    double dFpsidKappa;
    double dGdKappa;
    double dFpsichidKappachi;
    double dGchidKappachi;
    dMatrixT dFpsinablachidKappanablachi;
    dMatrixT dGnablachidKappanablachi;
    dMatrixT dGnablachidcohesion_nablachi;
    double signMacfunc_tr;
    double signMacfunc_chi_tr;
    double signMacfunc_nablachi_tr;

//////////////////////////////////////////////////////////////////////




//  enum fIntegrate_T         {
//      kBeta,
//      kGamma,
//      kNUM_FINTEGRATE_TERMS        };

    /** constructor */
    FSMicromorphic2_3DT( const ElementSupportT& support );

    /** destructor */
    ~FSMicromorphic2_3DT(void);

    /** reference to element shape functions */
    const ShapeFunctionT& ShapeFunctionDispl(void) const;
    const ShapeFunctionT& ShapeFunctionMicro(void) const;

    /** echo input */
    void Echo_Input_Data (void);

    /** return true if the element contributes to the solution of the
     * given group. ElementBaseT::InGroup returns true if group is the
     * same as the group of the FieldT passed in to ElementBaseT::ElementBaseT. */
    virtual bool InGroup(int group) const;

    /* initialize/finalize time increment */
    /*@{*/
    virtual void InitStep(void);
    virtual void CloseStep(void);
    //virtual GlobalT::RelaxCodeT ResetStep(void); // restore last converged state

    /** element level reconfiguration for the current time increment */
    //virtual GlobalT::RelaxCodeT RelaxSystem(void);
    /*@}*/

    /** collecting element group equation numbers. See ElementBaseT::Equations
     * for more information */
    virtual void Equations( AutoArrayT<const iArray2DT*>& eq_d,
                AutoArrayT<const RaggedArray2DT<int>*>& eq_phi);

    /** return a const reference to the run state flag */
    virtual GlobalT::SystemTypeT TangentType(void) const;

    /** accumulate the residual force on the specified node
     * \param node test node
     * \param force array into which to assemble to the residual force */
    virtual void AddNodalForce(const FieldT& field, int node, dArrayT& force);

    /** returns the energy as defined by the derived class types */
    virtual double InternalEnergy(void);

    /** \name writing output */
    /*@{*/
    /** register element for output */
    virtual void RegisterOutput(void);

    /** write element output */
    virtual void WriteOutput(void);

    /** compute specified output parameter and send for smoothing */
    virtual void SendOutput(int kincode);
    /*@}*/

    /** return geometry and number of nodes on each facet */
    void FacetGeometry(ArrayT<GeometryT::CodeT>& facet_geometry, iArrayT& num_facet_nodes) const;

    /** return the geometry code */
    virtual GeometryT::CodeT GeometryCode(void) const;

    /*set active elements*/
    //virtual void SetStatus(const ArrayT<ElementCardT::StatusT>& status);

    /** initial condition/restart functions (per time sequence) */
    virtual void InitialCondition(void);

    /** mass types */
    enum MassTypeT {kNoMass = 0, /**< do not compute mass matrix */
            kConsistentMass = 1, /**< variationally consistent mass matrix */
                kLumpedMass = 2, /**< diagonally lumped mass */
             kAutomaticMass = 3  /**< select the mass type base on the time integration scheme */};
    MassTypeT static int2MassTypeT(int i);

    /** \name restart functions */
    /*@{*/
    /** write restart data to the output stream. Should be paired with
     * the corresponding ElementBaseT::ReadRestart implementation. */
    virtual void WriteRestart(ostream& out) const;

    /** read restart data to the output stream. Should be paired with
     * the corresponding ElementBaseT::WriteRestart implementation. */
    virtual void ReadRestart(istream& in);
    /*@}*/

    /** \name implementation of the ParameterInterfaceT interface */
    /*@{*/
    /** information about subordinate parameter lists */
    virtual void DefineSubs(SubListT& sub_list) const;

    /** return the description of the given inline subordinate parameter list */
    virtual void DefineInlineSub(const StringT& name, ParameterListT::ListOrderT& order,
                     SubListT& sub_lists) const;

    /** a pointer to the ParameterInterfaceT of the given subordinate */
    virtual ParameterInterfaceT* NewSub(const StringT& name) const;

    /** \name implementation of the ParameterInterfaceT interface */
    /*@{*/
    /** describe the parameters needed by the interface */
    virtual void DefineParameters(ParameterListT& list) const;

    /** accept parameter list */
    virtual void TakeParameterList(const ParameterListT& list);
    /*@}*/

protected:

    /** \name drivers called by ElementBaseT::FormRHS and ElementBaseT::FormLHS */
    /*@{*/
    /** form group contribution to the stiffness matrix */
    virtual void LHSDriver(GlobalT::SystemTypeT);

    /** form group contribution to the residual */
    virtual void RHSDriver(void);
    /*@}*/

    /** compute shape functions and derivatives */
    virtual void SetGlobalShape(void);

    void Select_Equations ( const int &iBalLinMom, const int &iBalFirstMomMom );

private:

    /** \name solution methods.
     * Both of these drivers assemble the LHS as well as the residual.
     */
    /*@{*/
    /** driver for staggered solution */
    void RHSDriver_staggered(void);

    /** driver for monolithic solution */
    void RHSDriver_monolithic(void);
    /*@}*/

protected:

    /* output control */
    iArrayT        fNodalOutputCodes;
    iArrayT        fElementOutputCodes;

private:

    /** Gradients and other matrices */
    dMatrixT fgrad_u, fgrad_u_n;
    dMatrixT fgrad_Phi, fgrad_Phi_n;

    dMatrixT fShapeDispl, fShapeDisplGrad, fShapeDisplGrad_t;
    dMatrixT fShapeDisplGrad_t_Transpose, fShapeDisplGradGrad, fShapeDisplGrad_temp;
    dMatrixT fShapeMicro,fShapeMicroGrad,fShapeMicroGrad_temp;

    dMatrixT fDefGrad, fDefGradInv, fDefGradInvMatrix;

    /** \name  values read from input in the constructor */
    /*@{*/
    /** element geometry */
    GeometryT::CodeT fGeometryCode_displ, fGeometryCodeSurf_displ;
    GeometryT::CodeT fGeometryCode_micro, fGeometryCodeSurf_micro;
    int fGeometryCode_displ_int,fGeometryCode_micro_int, fGeometryCodeSurf_displ_int;

    /** number of integration points */
    int        fNumIP_displ, fNumIPSurf_displ, fNumIP_micro, fNumIPSurf_micro;
    int knum_d_state, knum_i_state, knumstress, knumstrain;//,knumdispl;
    int num_sidesets;

    /*@}*/

    /** \name element displacements in local ordering */
    /*@{*/
    LocalArrayT u;                //solid displacement
    LocalArrayT u_n;         //solid displacement from time t_n
    LocalArrayT u_dot;         //solid velocity
    LocalArrayT u_dot_n;         //solid velocity from time t_n
    LocalArrayT u_dotdot;         //solid acceleration
    LocalArrayT u_dotdot_n;         //solid acceleration from time t_n
    LocalArrayT del_u;        //displacement increment including Newton-R update, i.e. del_u = u_{n+1}^{k+1} - u_n
    LocalArrayT Phi;        //micro-displacement-gradient dof
    LocalArrayT Phi_dot;        //micro-displacement-gradient first time derivative
    LocalArrayT Phi_dot_n;        //micro-displacement-gradient first time derivative from time t_n
    LocalArrayT Phi_dotdot;        //micro-displacement-gradient second derivative
    LocalArrayT Phi_dotdot_n;        //micro-displacement-gradient second derivative from time t_n
    LocalArrayT Phi_n;        //micro-displacement-gradient from time t_n
    LocalArrayT del_Phi;        //micro-displacement-gradient increment
    dArrayT                del_u_vec;          // vector form
    dArrayT                del_Phi_vec;        // vector form
    dArrayT                u_vec;          // solid displacement in vector form
    dArrayT                u_dot_vec;          // solid velocity in vector form
    dArrayT                u_dotdot_vec;          // solid acceleration in vector form
    dArrayT                u_vec_n;          // solid displacement for previous time step in vector form
    dArrayT                u_dot_vec_n;          // solid velocity for previous time step in vector form
    dArrayT                u_dotdot_vec_n;          // solid acceleration for previous time step in vector form
    dArrayT                Phi_vec;        // micro-displacement-gradient in vector form
    dArrayT                Phi_dot_vec;        // first derivative of micro-displacement-gradient in vector form
    dArrayT                Phi_dotdot_vec;        // second derivative of micro-displacement-gradient in vector form
    dArrayT                Phi_vec_n;        // micro-displacement-gradient for previous time step in vector form
    dArrayT                Phi_dot_vec_n;        // first time derivative of micro-displacement tensor for previous time step in vector form
    dArrayT                Phi_dotdot_vec_n;        // second time derivative of micro-displacement tensor for previous time step in vector form

    /*@}*/

    // problem size definitions
    int n_en_displ, n_en_displ_x_n_sd, n_sd_x_n_sd,n_sd_x_n_sd_x_n_sd,n_en_micro_x_n_sd_x_n_sd, n_en_micro_x_n_sd;
    int n_el, n_sd, n_sd_surf, n_en_surf;
    int n_en_micro, ndof_per_nd_micro, n_en_micro_x_ndof_per_nd_micro, ndof_per_nd_micro_x_n_sd;
    int step_number;
    int iConstitutiveModelType;
    double Alpha_int_param;

    //name of output vector
    StringT output;

    dArrayT fForces_at_Node;
    bool bStep_Complete;

    double time, fRho, fRho_0;

    void Get_Fd_ext ( dArrayT &fFd_ext );

    //-- Material Parameters
    dArrayT fMaterial_Params;

    //-- Newmark Time Integration Parameters
    dArrayT fIntegration_Params;

    /** \name shape functions wrt to current coordinates */
    /*@{*/
    /** shape functions and derivatives. The derivatives are wrt to the
      * reference coordinates */
    ShapeFunctionT* fShapes_displ;
    ShapeFunctionT* fShapes_micro;

    /** reference coordinates */
    LocalArrayT fInitCoords_displ, fInitCoords_micro;
    /** current coordinates */
    LocalArrayT fCurrCoords_displ, fCurrCoords_micro;
    /*@}*/

    /* Data Storage */
    ElementMatrixT fKdd, fKdphi;
    ElementMatrixT fKphid, fKphiphi;
    dArrayT         fFd_int;
    dArrayT         fFd_ext;
    dArrayT         fFphi_int;
    dArrayT         fFphi_ext;

    dMatrixT fCdd;
    dMatrixT fCdphi;
    dMatrixT fCphid;
    dMatrixT fCphiphi;

    dMatrixT fCdd_temp;
    dMatrixT fCdphi_temp;
    dMatrixT fCphid_temp;
    dMatrixT fCphiphi_temp;

    dArrayT fKdd_n;
    dArrayT fKdphi_n;
    dArrayT fKphid_n;
    dArrayT fKphiphi_n;

    dArrayT fCdd_dn;
    dArrayT fCdphi_dn;
    dArrayT fCphiphi_dn;
    dArrayT fCphid_dn;
    dArrayT fCdd_d;
    dArrayT fCdphi_d;
    dArrayT fCphiphi_d;
    dArrayT fCphid_d;



    dArrayT         fGrad_disp_vector;
    dMatrixT        fGrad_disp_matrix;

    dArrayT         fTemp_nine_values;
    dArrayT         fTemp_six_values;

 //   dMatrixT        fDeformation_Gradient;
    dMatrixT        fRight_Cauchy_Green_tensor;
    dMatrixT        fRight_Cauchy_Green_tensor_Inverse;
    dMatrixT        fRight_Elastic_Cauchy_Green_tensor;
    dMatrixT        fRight_Elastic_Cauchy_Green_tensor_tr;

    dMatrixT        fLeft_Cauchy_Green_tensor;
    dMatrixT        fLeft_Cauchy_Green_tensor_Inverse;
    dMatrixT        fDeformation_Gradient_Inverse;
    dMatrixT        fDeformation_Gradient_Transpose;
    dMatrixT        fDefGradInv_Grad_grad;

    dMatrixT        fIdentity_matrix;
    dMatrixT        fTemp_matrix_nsd_x_one;
    dMatrixT        fTemp_matrix_nsd_x_one1;
    dMatrixT        fTemp_matrix_nsd_x_nsd;
    dMatrixT        fTemp_matrix_nsd_x_nsd2;
    dMatrixT        fTemp_matrix_nsd_x_nsd3;
    dMatrixT        fTemp_matrix_nsd_x_nsd4;
    dMatrixT        fTemp_matrix_nsd_x_nsd5;
    dMatrixT        fTemp_matrix_nsd_x_nsd6;
    dMatrixT        fTemp_matrix_nsd_x_nsd7;
    dMatrixT        fTemp_matrix_nsd_x_nsd8;
    dMatrixT        fTemp_matrix_nsd_x_nsd9;

    dMatrixT        fShapeMicro_row_matrix;


    dMatrixT        u_dotdot_column_matrix;
    dMatrixT        u_dot_column_matrix;
    dMatrixT        u_dot_column_matrix_Transpose;
    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    ////////////////Definitions for the Current configuration Plasticity/////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    dArray2DT       fdGdCauchy_Stress_Elements_IPs;
    dArray2DT       fdGdCauchy_Stress_n_Elements_IPs;
    dArray2DT		fdGdCauchy_Stress_IPs;
    dArray2DT		fdGdCauchy_Stress_n_IPs;
    dMatrixT		fdGdCauchy_Stress;
    dMatrixT		fdGdCauchy_Stress_n;


    dArray2DT   	fCauchy_stress_Elements_IPs;
    dArray2DT   	fCauchy_stress_IPs;
    dMatrixT    	fCauchy_stress_tensor_current_IP;
    dArray2DT   	fCauchy_stress_Elements_n_IPs;
    dArray2DT   	fCauchy_stress_n_IPs;
    dMatrixT    	fCauchy_stress_tensor_current_n_IP;
    dMatrixT    	fdev_Cauchy_stress_tensor_current_IP;

    double			fNorm_dev_Cauchy_stress_tensor_current_IP;


    dMatrixT    	fCauchy_stress_tensor_current_IP_tr;
    double       	mean_Cauchy_stress_tr;
    dMatrixT		dev_Cauchy_stress_tr;
    double			Cauchy_Stress_Norm_tr;
    dMatrixT		fdGdCauchy_Stress_tr;

    double			mean_Cauchy_stress_n;
    double			mean_Cauchy_stress;

    dMatrixT		dev_Cauchy_stress_n;
    dMatrixT		Predictor_stress_terms;
    double			fVelocity_Gradient_current_IP_trace;
    double			Temp_trace_value;
    dMatrixT		fVelocity_Gradient_current_IP_transpose;
    dMatrixT		Corrector_stress_terms;
    dMatrixT		fdGdCauchy_Stress_tr_transpose;
    double			Predictor_mean_stress_terms;
    double			Corrector_mean_stress_terms;
    double			fTemp_matrix_one_x_one;
    double			fTemp_matrix_one_x_one2;
    double			fDelgamma_current_configuration_root_one;
    double			fDelgamma_current_configuration_root_two;
    double			fDelgamma_current_configuration;
    double			fTemp_delgamma_coeff;
    double			fTemp_delgamma_coeff1;
    double			Je;
    double			fYield_function_current_configuration;
    double			cohesion_current_configuration;

    double			Micro_Plasticity_Occurrence;
    double			Macro_Plasticity_Occurrence;
    double			fMicroYield_function_check;
    double			fYield_function_check;



    dMatrixT    	fVelocity_Gradient_current_IP;

    dArray2DT       fDeformation_Gradient_Elements_IPs;
    dArray2DT       fDeformation_Gradient_n_Elements_IPs;
    dArray2DT		fDeformation_Gradient_IPs;
    dArray2DT		fDeformation_Gradient_n_IPs;
    dMatrixT        fDeformation_Gradient;
    dMatrixT		fDeformation_Gradient_n;


    dArray2DT       fMicro_Deformation_Gradient_Elements_IPs;
    dArray2DT		fMicro_Deformation_Gradient_IPs;
    dMatrixT        fMicro_Deformation_Gradient;
    dMatrixT        Length_scale_vector;

    ///Runge Kutta matrices
    dMatrixT		fTemp_Runge_Kutta_K1_nsd_x_nsd;
    dMatrixT		fTemp_Runge_Kutta_K2_nsd_x_nsd;
    dMatrixT		fTemp_Runge_Kutta_K3_nsd_x_nsd;
    dMatrixT		fTemp_Runge_Kutta_K4_nsd_x_nsd;

    dMatrixT		fCauchy_stress_tensor_current_IP_from_piola_stress;
    dMatrixT    	fLeft_Cauchy_Green_tensor_current_IP;
    dMatrixT    	fLeft_Cauchy_Green_tensor_current_IP_transpose;
    dMatrixT		fLeft_Cauchy_Green_tensor_current_IP_Inverse;
    double			fLeft_Cauchy_Green_tensor_current_IP_Trace;

    dMatrixT		fRight_Elastic_Cauchy_Green_tensor_Inverse;




    //////////////////////////// A(delgamma)^2+B(delgamma)+D = 0//////////////////////////
    double			fA_coeff;
    double			fB_coeff;
    double			fD_coeff;
    double			ftemp_D_coeff;
    double			ftemp_D_coeff1;
    double			ftemp_B_coeff;
    double			fDelta_function;


    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    //////////////End of definitions for the current configuration Plasticity////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////
    ///////////// New tensors for the implementation of new plasticity approach ////////////
    /////////////////////////////// based upon micro stress tensor /////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    double fMeanSIGMA_etr;
    dMatrixT SIGMA_tr;
    dMatrixT fdevSIGMA_tr;
    double fNormdevSIGMA_tr;
    dMatrixT fdGchidSIGMA_tr;
    dMatrixT fdGchidSIGMA_tr_transpose;
    double fdGchidSIGMA_tr_trace;
    dMatrixT fdGchidSIGMA_n_transpose;
    dMatrixT fdGchidSIGMA_n;
    dMatrixT SIGMA;
    dMatrixT SIGMA_n;
    dMatrixT fdevSIGMA;
    dMatrixT Symm_Elastic_MicroStnTensor_tr;
    dMatrixT Symm_Elastic_MicroStnTensor;
    double fMeanSIGMA_e;
    double fNormdevSIGMA;
    dMatrixT DeltaLbarchiP;
    dMatrixT dDeltaLbarchiPdDelgammachi;
    dMatrixT dFpdDelgammachi;
    dMatrixT dFedDelgammachi;
    dMatrixT dEedDelgammachi;
    dMatrixT symdEpsilonedDelgammachi;
    dMatrixT dSIGMAdDelgammachi;
    double fMeandSIGMAdDelgammachi;
    dMatrixT ddevSIGMAdDelgammachi;
    double dNormdevSIGMAdDelgammachi;
    double dNormdevSIGMAdDelgamma;
    dMatrixT Elastic_MicroStnTensor_tr_transpose;
    dMatrixT Elastic_MicroStnTensor_transpose;
    double dEedDelgamma_trace;
    double dEpsilonedDelgamma_trace;
    double dEpsilonedDelgammachi_trace;
    double dEedDelgammachi_trace;
    dMatrixT dEpsilonedDelgamma_transpose;
    dMatrixT dEpsilonedDelgammachi_transpose;
    dMatrixT symdEpsilonedDelgamma;
    dMatrixT dSIGMAdDelgamma;
    dMatrixT ddevSIGMAdDelgamma;
    double Elastic_LagrangianStn_trace;
    double Elastic_MicroStnTensor_trace;
    dMatrixT fdFYchidSIGMA;
    double fdFYchidSIGMA_trace;
    dMatrixT fdGchidSIGMA;
    dMatrixT VardeltaLbarchiP;
    dMatrixT Term1;
    double Term1_trace;
    dMatrixT Term1_transpose;
    double fdFYdS_Term1;
    double fdFYdS_Term1T;
    double fdFYchidSIGMA_Term1;
    double fdFYchidSIGMA_Term1T;

    dMatrixT Term2;
    dMatrixT Term2_transpose;
    double Term2_trace;
    double fdFYdS_Term2;
    double fdFYdS_Term2T;
    double fdFYchidSIGMA_Term2;
    double fdFYchidSIGMA_Term2T;
    dMatrixT VarfFp_Delgammachi;
    dMatrixT VarfFe_Delgammachi;
    dMatrixT fMatrix_1;
    dMatrixT fMatrix_2;
    double fMatrix_0_trace;
    double fMatrix_1_trace;
    double fMatrix_2_trace;
    double scale_1;
    dTensor3DT Temp_II14p_1;

    dMatrixT fTemp_matrix_n_sd_x_nsd_kdd;
    dMatrixT fTemp_matrix_n_sd_x_nsd_kdphi;
    dMatrixT fTemp_matrix_n_sd_x_nsd_kphid;
    dMatrixT fTemp_matrix_n_sd_x_nsd_kphiphi;
    dMatrixT fTemp_matrix_n_sd_x_nsd_kphid_GRAD;
    dMatrixT fTemp_matrix_n_sd_x_nsd_kphiphi_GRAD;
    dMatrixT fTemp_matrix_n_sd_x_nsd_kphiphi_GRAD_GRAD;

    dMatrixT fKuu_Plastic;
    dMatrixT fKuphi_Plastic;
    dMatrixT fKphiu_Plastic;
    dMatrixT fKphiphi_Plastic;

    dMatrixT fKphiu_GRADPlastic;
    dMatrixT fKphiphi_GRADPlastic;

    dMatrixT VarfFe_Delgammachi_transpose;
    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////// Variables defined for dynamic analysis //////////////////
    //////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////
    //dMatrixT fMicroInertia;
    dMatrixT Chi_dotdotM;
    dArrayT Chi_dotdot_vec;
    dMatrixT fShapedispl_row_matrix;
    dMatrixT NU;
    dMatrixT disp_dotdot;
    dArrayT disp_dotdot_vec;
    dArrayT fV2_mass;
    dArrayT Vintp_1_temp_mass;
    dArrayT Vintp_2_temp_mass;
    dArrayT Vint_1_temp_mass;
    dArrayT Vint_2_temp_mass;

    double delta_t;
    double integrate_param;
    double gamma_delta_t;
    double J;

    dMatrixT IJ_1;
    dMatrixT fKu_IJ;

    dMatrixT IIJ_1;
    dMatrixT fKphiu_IIJ;

    dMatrixT III_J;
    dMatrixT fKphiu_IIIJ;

    dMatrixT fMMuu_I;
    dMatrixT fMMphiphi_II_1;
    dMatrixT fMMphiphi_II_2;

    dMatrixT fMMuu_Ip;
    dMatrixT fMMphiphi_IIp_1;
    dMatrixT fMMphiphi_IIp_2;

    dMatrixT II_1_mass;
    dMatrixT II_2_mass;


    dMatrixT fMdd;
    dMatrixT fMphiphi;

    void Form_NU_matrix(const dMatrixT &fShapedispl_row_matrix); //shape function matrice for displacement deformation
    void Form_micro_deformation_tensor_Accelerate_Chi(void);
    void Form_u_dotdot(void);
    void Form_fV2_mass(void);
    void Form_II_1_mass(void);
    void Form_II_2_mass(void);
    void Form_IJ_1(void);
    void Form_IIJ_1(void);
    void Form_III_J(void);


    //////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////


    //////////////////////////////////////////////////////////
    /////DEFINITIONS FOR MICROMORPHIC MATRICES////////////////
    //////////////////////////////////////////////////////////
    double KrDelta[3][3];
    double trdeltad;
    double trdeltaEp;

    double Counter;
    dArray2DT Counter_IPs_el_n;
    dArray2DT Counter_IPs_el;
    dArrayT Counter_IPs;
    dMatrixT fIota_temp_matrix;

    //Varitional Matrices coming from the Balance of linear Momentum
    dMatrixT Tsigma_1;
    dMatrixT fG1_1;
    double SigN[3][3];//unsymmetric Cauchy stress tensor found at previous step
    dMatrixT SigN_m;
    dMatrixT Fn_m;
    dMatrixT Finv_m;
    dMatrixT deltaL;
    dMatrixT deltaL_Tr;
    dMatrixT tempSig;
    dMatrixT deltad;
    dMatrixT SigN_ar;


    dMatrixT Sigma8;
    dArray2DT SigN_IPs;
    dArray2DT Sig_IPs;
    dArray2DT SigN_IPs_n;
    dArray2DT SigN_IPs_el;
    dArray2DT Sig_IPs_el;
    dArray2DT SigN_IPs_el_n;
    dArrayT Temp_Identity_array;


    dMatrixT SPiolaN;
    dMatrixT SPiola;

    dArray2DT SPiolaN_IPs;
    dArray2DT SPiola_IPs;
    dArray2DT SPiolaN_IPs_el;
    dArray2DT SPiola_IPs_el;
    dArray2DT SPiolaN_IPs_el_n;



    dMatrixT Sigma; // unsymetric Cauchy stress tensor at current step
    double Fn[3][3];
    dMatrixT Fn_ar;
    dArray2DT Fn_ar_IPs;
    dArray2DT F_ar_IPs;
    dArray2DT Fn_ar_IPs_el;
    dArray2DT F_ar_IPs_el;
    dArray2DT Fn_ar_IPs_el_n;

    double FnInv[3][3];
    dMatrixT FnInv_ar;
    dArray2DT FnInv_ar_IPs;
    dArray2DT FInv_ar_IPs;
//  dArray2DT FInv_ar_IPs_n;
    dArray2DT FnInv_ar_IPs_el;
    dArray2DT FInv_ar_IPs_el;
    dArray2DT FnInv_ar_IPs_el_n;

    double Finv[3][3];

    double Chi[3][3];
    dMatrixT Chi_m;
    double ChiInv[3][3];
    dMatrixT ChiInv_m;
    dMatrixT ChiN_m;
    dMatrixT deltaEp;
    dMatrixT deltaNu;
    double ChiN[3][3];
    dMatrixT ChinN_m;
    dMatrixT ChiN_ar;
    dMatrixT Chi_ar;
    dArray2DT ChiN_ar_IPs;
    dArray2DT Chi_ar_IPs;
    dArray2DT ChiN_ar_IPs_n;
    dArray2DT Chi_ar_IPs_el;
    dArray2DT ChiN_ar_IPs_el;
    dArray2DT ChiN_ar_IPs_el_n;

//  double  Trial[6];
    dMatrixT fIota_w_temp_matrix;
    dMatrixT fTemp_matrix_nudof_x_nchidof;
    dMatrixT fTemp_matrix_nchidof_x_nchidof;
    dMatrixT fTemp_matrix_nchidof_x_nudof;
    dMatrixT fTemp_matrix_nudof_x_nudof;
    dMatrixT fTemp_matrix_nudof_x_nudof_Mdd;
    dMatrixT fTemp_matrix_nchidof_x_nchidof_Mphiphi;


    dMatrixT fH1_Etagrad;
    dMatrixT TransShapeDisplGrad;
    dMatrixT Var_F;
    dMatrixT GRAD_Nuw;
    dMatrixT GRAD_Nuw_Tr;
    dMatrixT Finv_w; // to create Iota_w which is different than Iota because sequence of the components in wk,l
    dMatrixT Tsigma_2;
    dMatrixT fG1_2;//not being calculated yet
    dMatrixT Tsigma_3;
    dMatrixT fG1_3;// not being calculated yet
    dMatrixT TFn_1;// to be multiplied by (lamda+Tau)
    dMatrixT fG1_4;
    dMatrixT TFn_2;// to be multiplied by (Mu+sigma)
    dMatrixT fG1_5;
    dMatrixT TFn_3;// to be multiplied by (Mu+sigma)
    dMatrixT fG1_6;
    dMatrixT TChi_1;// to be multiplied by eta
    dMatrixT fG1_7;
    dMatrixT TFn_4;// to be multiplied by eta
    dMatrixT fG1_8;
    dMatrixT TChi_2;// to be multiplied by kappa
    dMatrixT fG1_9;
    dMatrixT TFn_5;// to be multiplied by kappa
    dMatrixT fG1_10;
    dMatrixT TChi_3;// to be multiplied by nu
    dMatrixT fG1_11;
    dMatrixT TFn_6;// to be multiplied by nu
    dMatrixT fG1_12;
    dMatrixT SigCurr;
    dMatrixT fG1_13;
    dMatrixT fG1_14;
 // Variational Matrices coming from the Balance of First Moment of Momentum
    double mn[3][3][3];
    dArrayT mn_ar;
    dArray2DT mn_IPs;
    dArray2DT mn_IPs_n;
    dArray2DT mn_IPs_el;
    dArray2DT mn_IPs_el_n;

    double Mnplus1[3][3][3];
    double Gamma[3][3][3];
    double GammaN[3][3][3];
    dArrayT GammaN_ar;

    dArray2DT GammaN_IPs;
    dArray2DT GammaN_IPs_n;
    dArray2DT GammaN_IPs_el;
    dArray2DT GammaN_IPs_el_n;

    double CCof[3][3][3][3][3][3];
    double GRAD_ChiN[3][3][3];// GRADIENT  in reference configuration!
    dArrayT GRAD_Chi_ar;
    dArrayT GRAD_ChiN_ar;
    dArray2DT GRAD_ChiN_ar_IPs;
    dArray2DT GRAD_Chi_ar_IPs;
    dArray2DT GRAD_ChiN_ar_IPs_n;
    dArray2DT GRAD_ChiN_ar_IPs_el;
    dArray2DT GRAD_Chi_ar_IPs_el;
    dArray2DT GRAD_ChiN_ar_IPs_el_n;

    double GRAD_Chi[3][3][3];// GRADIENT in reference configuration!



    dMatrixT fIota_eta_temp_matrix;
    dMatrixT Finv_eta; // to create Iota_eta


    dMatrixT Etagrad;
    dMatrixT Mm_1;
    dMatrixT Mm_2;
    dMatrixT Mm_3;
    dMatrixT Mm_4;
    dMatrixT Mm_5;
    dMatrixT Mm_6;
    dMatrixT Mm_7;
    dMatrixT Mm_71;
    dMatrixT Mm_72;
    dMatrixT Mm_73;
    dMatrixT Mm_74;
    dMatrixT Mm_75;
    dMatrixT Mm_76;
    dMatrixT Mm_77;
    dMatrixT Mm_78;
    dMatrixT Mm_8;
    dMatrixT Mm_9;
    dMatrixT Mm_10;
    dMatrixT Mm_11;
    dMatrixT Mm_12;
    dMatrixT Mm_13;
    dMatrixT Mm_14;
    dMatrixT Ru_1;//u
    dMatrixT Ru_2;
    dMatrixT Ru_3;
    dMatrixT RChi_1;
    dMatrixT Ru_4;
    dMatrixT RChi_2;
    dMatrixT Ru_5;
    dMatrixT Ru_6;
    dMatrixT Ru_7;
    dMatrixT Ru_8;
    dMatrixT Ru_9;
    dMatrixT RChi_3;
    dMatrixT Rs_sigma;
    dMatrixT R_gradu;
    dMatrixT R_Capital_Gamma_Chi;
    dMatrixT CapitalLambda;

    dMatrixT sn_sigman;
    dArray2DT sn_sigman_IPs;
    dArray2DT sn_sigman_IPs_n;
    dArray2DT sn_sigman_IPs_el;
    dArray2DT sn_sigman_IPs_el_n;

    dMatrixT s_sigma;

    dMatrixT fShapeDispl_Tr;


    dMatrixT NCHI;
    dMatrixT NCHI_Tr;
//    dMatrixT NCHI_eta;//  same with the one above no need!
//    dMatrixT GRAD_NCHI_Phi;//no need for this same with GRAD_NCHI
    dMatrixT GRAD_NCHI;


    dMatrixT fH1_1;
    dMatrixT fH1_2;
    dMatrixT fH1_3;
    dMatrixT fH1_4;
    dMatrixT fH1_5;
    dMatrixT fH1_6;
    dMatrixT fH1_7;
    dMatrixT fH1_71;
    dMatrixT fH1_72;
    dMatrixT fH1_73;
    dMatrixT fH1_74;
    dMatrixT fH1_75;
    dMatrixT fH1_76;
    dMatrixT fH1_77;
    dMatrixT fH1_78;
    dMatrixT fH1_8;
    dMatrixT fH1_9;
    dMatrixT fH1_10;
    dMatrixT fH1_11;
    dMatrixT fH1_12;
    dMatrixT fH1_13;
    dMatrixT fH1_14;

    dMatrixT fH2_1;
    dMatrixT fH2_2;
    dMatrixT fH2_3;
    dMatrixT fH2_4;
    dMatrixT fH2_5;
    dMatrixT fH2_6;
    dMatrixT fH2_7;
    dMatrixT fH2_8;
    dMatrixT fH2_9;
    dMatrixT fH2_10;
    dMatrixT fH2_11;
    dMatrixT fH2_12;
    dMatrixT fH2_13;
   // dMatrixT fH2_14;

    dMatrixT fH3_1;

    dArrayT Chi_vec;
    dArrayT GRAD_Chi_vec;
    //internal Force Matrices
    dArrayT G1;
    dArrayT Uint_1;
    dArrayT Uint_1_temp;
    dArrayT Pint_1_temp;
    dArrayT Pint_2_temp;
    dArrayT Pint_3_temp;
    dArrayT Uext_1;
    dArrayT Text;
    dArrayT Gext;

    dArrayT H1;
    dArrayT Pint_1;
    dArrayT H2;
    dArrayT Pint_2;
    dArrayT H3;
    dArrayT Pint_3;
    dArrayT Hext;
    dArrayT Pext;

    dMatrixT Lambda;
    dMatrixT Omega;
    //////////////////////////////////////////////////////////
    //////FINITE STRAIN ELASTICITY MATRICES START HERE/////////
    //////////////////////////////////////////////////////////
    //dMatrixT V_1;
    dMatrixT Jmat;
    dMatrixT KJmat;
    dMatrixT SPK;
    dMatrixT devSPK;
    dMatrixT KirchhoffST;// The second Piola-Kirchhoff Matrix
    dMatrixT Temp_SPK;//temporary Matrix used in calculation of SPK
  //  dMatrixT FSF;
  //  dMatrixT LST;//Lagrangian strain tensor used in some functions to get rid of long name
    dMatrixT LagrangianStn;
    dMatrixT Elastic_LagrangianStn;
    dMatrixT Elastic_LagrangianStn_tr;

    dMatrixT MicroStnTensor;//Micro-strain tensor
    dMatrixT eps;// Micro strain tensor in current config.
    dMatrixT psi;// micro-deformation tensor in current config.
    dMatrixT PSI;//deformation measure PSI=Transpose(F).chi
    dMatrixT ChiM; //Micro-deformation tensor Chi ( used a different tensor this time )
    dMatrixT ChiM_Inverse; //Micro-deformation tensor Chi ( used a different tensor this time )
    dMatrixT I1_1;
    dMatrixT I1_2;
    dMatrixT I1_3;
    dMatrixT I1_4;
    dMatrixT I1_5;
    dMatrixT I1_6;
    dMatrixT I1_7;
    dMatrixT I2_1;
    dMatrixT I1_8;
    dMatrixT I2_2;
    dMatrixT I1_9;
    dMatrixT I2_3;
/*********************/
    dMatrixT fFJ;
    dMatrixT fJF;
    dMatrixT fJ1_1;
    dMatrixT fJ1_2;
    dMatrixT fJ1_3;
    dMatrixT fJ1_4;
    dMatrixT fJ2_1;
    dMatrixT fJ1_5;
    dMatrixT fJ2_2;
    dMatrixT fJ1_6;
    dMatrixT fJ2_3;

    dArrayT Vint_1;
    dArrayT Vint_1_temp;
    dArrayT Vint_2;
    dArrayT Vint_2_temp;
    dArrayT Vint_3_temp;
    dArrayT Vint_3;


    dArray2DT fFd_int_temp_Elements;
    dArrayT fFd_int_temp_n;
    dArrayT fFd_int_temp;
    dArray2DT fFd_int_temp_n_Elements;
    dArrayT Vint_1_stiffness_save;


    dArray2DT fFphi_int_temp_Elements;
    dArrayT fFphi_int_temp_n;
    dArrayT fFphi_int_temp;
    dArray2DT fFphi_int_temp_n_Elements;
    dArrayT Vint_2_stiffness_save;


    dArrayT fV1;
    dArrayT fV2;
    dArrayT fV3;
    dMatrixT fKu_1;
    dMatrixT fKu_2;
    dMatrixT fKu_3;
    dMatrixT fKu_4;
    dMatrixT fKu_5;
    dMatrixT fKu_6;
    dMatrixT fKu_7;
    dMatrixT fKuphi_1;
    dMatrixT fKu_8;
    dMatrixT fKuphi_2;
    dMatrixT fKu_9;
    dMatrixT fKuphi_3;
    dMatrixT SIGMA_S;
    dMatrixT devSIGMA_S;
    dMatrixT fKFJu;
    dMatrixT fKJFu;
    dMatrixT fKphiu_1;
    dMatrixT fKphiu_2;
    dMatrixT fKphiu_3;
    dMatrixT fKphiu_4;
    dMatrixT fKphiphi_1;
    dMatrixT fKphiu_5;
    dMatrixT fKphiphi_2;
    dMatrixT fKphiu_6;
    dMatrixT fKphiphi_3;

    dMatrixT fFM;
    dMatrixT fMF;
    dMatrixT fMchi;
    dMatrixT fEtaM;
    dMatrixT fMpu_1_1;
    dMatrixT fMpp_1_1;
    dMatrixT fMpu_1_2;
    dMatrixT fMpp_1_2;
    dMatrixT fMpu_2_1;
    dMatrixT fMpp_2_1;
    dMatrixT fMpu_2_2;
    dMatrixT fMpp_2_2;
    dMatrixT fMpu_3;
    dMatrixT fMpp_3;
    dMatrixT fMpu_4;
    dMatrixT fMpp_4;
    dMatrixT fMpu_5_1;
    dMatrixT fMpp_5_1;
    dMatrixT fMpu_5_2;
    dMatrixT fMpp_5_2;
    dMatrixT fMpu_6;
    dMatrixT fMpp_6;
    dMatrixT fMpu_7;
    dMatrixT fMpp_7;
    dMatrixT fMpu_8_1;
    dMatrixT fMpp_8_1;
    dMatrixT fMpu_8_2;
    dMatrixT fMpp_8_2;
    dMatrixT fMpu_9;
    dMatrixT fMpp_9;
    dMatrixT fMpu_10;
    dMatrixT fMpp_10;
    dMatrixT fMpu_11;
    dMatrixT fMpp_11;

    dMatrixT fKMphiu_1_1;
    dMatrixT fKMphiphi_1_1;
    dMatrixT fKMphiu_1_2;
    dMatrixT fKMphiphi_1_2;
    dMatrixT fKMphiu_2_1;
    dMatrixT fKMphiphi_2_1;
    dMatrixT fKMphiu_2_2;
    dMatrixT fKMphiphi_2_2;
    dMatrixT fKMphiu_3;
    dMatrixT fKMphiphi_3;
    dMatrixT fKMphiu_4;
    dMatrixT fKMphiphi_4;
    dMatrixT fKMphiu_5_1;
    dMatrixT fKMphiphi_5_1;
    dMatrixT fKMphiu_5_2;
    dMatrixT fKMphiphi_5_2;
    dMatrixT fKMphiu_6;
    dMatrixT fKMphiphi_6;
    dMatrixT fKMphiu_7;
    dMatrixT fKMphiphi_7;
    dMatrixT fKMphiu_8_1;
    dMatrixT fKMphiphi_8_1;
    dMatrixT fKMphiu_8_2;
    dMatrixT fKMphiphi_8_2;
    dMatrixT fKMphiu_9;
    dMatrixT fKMphiphi_9;
    dMatrixT fKMphiu_10;
    dMatrixT fKMphiphi_10;
    dMatrixT fKMphiu_11;
    dMatrixT fKMphiphi_11;

    dTensor3DT fMKLM;
    dTensor3DT fdevMKLM;
    dTensor3DT GAMMA;
    dTensor3DT GRAD_CHIM;
    dTensor3DT fTemp_tensor_n_sd_x_n_sd_x_n_sd;
    dTensor3DT fTemp_tensor_n_sd_x_n_sd_x_n_sd1;
    dTensor3DT fTemp_tensor_n_sd_x_n_sd_x_n_sd2;


    dTensor3DT devMeKLM;
    dTensor3DT fMeKLM_tr;
    dTensor3DT GAMMAe_tr;
    dTensor3DT GXe;
    dTensor3DT GXe_tr;
    dTensor3DT GXp;
    dTensor3DT GXp_n;


    dTensor3DT fMKLM_tr;
    dTensor3DT fdevMKLM_tr;
    dMatrixT Mean_fMKLM_tr;
    dTensor3DT fMeKLM;
    dTensor3DT fdevMeKLM;
    dMatrixT Mean_fMeKLM;
    dTensor3DT fdGnablachidMKLM_tr;
    dTensor3DT fdGnablachidMKLM_n;
    dTensor3DT fdGnablachidMKLM;
    dTensor3DT fdFnablachidMKLM;
    dArray2DT dGnablachidMKLM_IPs;
    dArray2DT dGnablachidMKLM_Element_IPs;
    dArray2DT dGnablachidMKLM_n_IPs;
    dArray2DT dGnablachidMKLM_Element_n_IPs;


    dTensor3DT GAMMAe;
    dTensor3DT GAMMAe_n;
    dArray2DT GAMMAe_n_Elements_IPs;
    dArray2DT GAMMAe_Elements_IPs;
    dArray2DT GAMMAe_n_IPs;
    dArray2DT GAMMAe_IPs;

    dTensor3DT fMeKLM_n;
    dArray2DT fMeKLM_n_Elements_IPs;
    dArray2DT fMeKLM_Elements_IPs;
    dArray2DT fMeKLM_n_IPs;
    dArray2DT fMeKLM_IPs;

    dMatrixT SPK_n;
    dArray2DT SPK_n_Elements_IPs;
    dArray2DT SPK_Elements_IPs;
    dArray2DT SPK_n_IPs;
    dArray2DT SPK_IPs;

    dMatrixT SIGMA_S_n;
    dArray2DT SIGMA_S_n_Elements_IPs;
    dArray2DT SIGMA_S_Elements_IPs;
    dArray2DT SIGMA_S_n_IPs;
    dArray2DT SIGMA_S_IPs;

    dArray2DT SIGMA_n_Elements_IPs;
    dArray2DT SIGMA_Elements_IPs;
    dArray2DT SIGMA_n_IPs;
    dArray2DT SIGMA_IPs;

    dMatrixT Elastic_LagrangianStn_n;
    dArray2DT Elastic_LagrangianStn_n_Elements_IPs;
    dArray2DT Elastic_LagrangianStn_Elements_IPs;
    dArray2DT Elastic_LagrangianStn_n_IPs;
    dArray2DT Elastic_LagrangianStn_IPs;

    dMatrixT Elastic_MicroStnTensor_n;
    dArray2DT Elastic_MicroStnTensor_n_Elements_IPs;
    dArray2DT Elastic_MicroStnTensor_Elements_IPs;
    dArray2DT Elastic_MicroStnTensor_n_IPs;
    dArray2DT Elastic_MicroStnTensor_IPs;

    dTensor3DT dfMKLMdDelgammanablachi;
    dMatrixT dmeanfMKLMdDelgammanablachi;
    dTensor3DT dfdevMKLMdDelgammanablachi;
    dMatrixT kc_nablachi_n;
    dMatrixT dfkc_nablachidDelgammanablachi;
    dMatrixT fDelkc_nablachi;
    dMatrixT cohesion_nablachi;
    dMatrixT fDeltaLbar_P;
    dMatrixT fDeltaLbarChai_P;

    dMatrixT fdfnablachidcohesion_nablachi;
    dMatrixT PSIe_n_inverseT;

    dMatrixT II13e_1;
    dTensor3DT Temp_II14p;
    dMatrixT II14p_1;
    dMatrixT II14p_2;
    dMatrixT II14p_3;
    dMatrixT II14p_4;
    dMatrixT II14p_5;
    dMatrixT II14p_6;
    dMatrixT II14p_7;
    dMatrixT II14p_8;
    dMatrixT II14p_9;
    dMatrixT II14p_10;
    dMatrixT II14p_11;
    dMatrixT II14p_12;
    dMatrixT II14p_13;
    dMatrixT II14p_14;
    dMatrixT II14p_15;
    dMatrixT II14p_16;


    dMatrixT II15e_1;

    dTensor3DT Temp_II16p;
    dMatrixT II16p_1;
    dMatrixT II16p_2;
    dMatrixT II16p_3;
    dMatrixT II16p_4;
    dMatrixT II16p_5;
    dMatrixT II16p_6;
    dMatrixT II16p_7;
    dMatrixT II16p_8;
    dMatrixT II16p_9;
    dMatrixT II16p_10;
    dMatrixT II16p_11;
    dMatrixT II16p_12;
    dMatrixT II16p_13;
    dMatrixT II16p_14;
    dMatrixT II16p_15;
    dMatrixT II16p_16;

    dTensor3DT Temp_II17p;
    dMatrixT II17p_1;
    dMatrixT II17p_2;
    dMatrixT II17p_3;
    dMatrixT II17p_4;
    dMatrixT II17p_5;
    dMatrixT II17p_6;
    dMatrixT II17p_7;
    dMatrixT II17p_8;
    dMatrixT II17p_9;
    dMatrixT II17p_10;
    dMatrixT II17p_11;
    dMatrixT II17p_12;
    dMatrixT II17p_13;
    dMatrixT II17p_14;
    dMatrixT II17p_15;
    dMatrixT II17p_16;

    dTensor3DT Temp_II18p;
    dMatrixT II18p_1;
    dMatrixT II18p_2;
    dMatrixT II18p_3;
    dMatrixT II18p_4;
    dMatrixT II18p_5;
    dMatrixT II18p_6;
    dMatrixT II18p_7;
    dMatrixT II18p_8;
    dMatrixT II18p_9;
    dMatrixT II18p_10;
    dMatrixT II18p_11;
    dMatrixT II18p_12;
    dMatrixT II18p_13;
    dMatrixT II18p_14;
    dMatrixT II18p_15;
    dMatrixT II18p_16;

    dTensor3DT Temp_II19p;
    dMatrixT II19p_1;
    dMatrixT II19p_2;
    dMatrixT II19p_3;
    dMatrixT II19p_4;
    dMatrixT II19p_5;
    dMatrixT II19p_6;
    dMatrixT II19p_7;
    dMatrixT II19p_8;
    dMatrixT II19p_9;
    dMatrixT II19p_10;
    dMatrixT II19p_11;
    dMatrixT II19p_12;
    dMatrixT II19p_13;
    dMatrixT II19p_14;
    dMatrixT II19p_15;
    dMatrixT II19p_16;

    dTensor3DT Temp_II20p;
    dMatrixT II20p_1;
    dMatrixT II20p_2;
    dMatrixT II20p_3;
    dMatrixT II20p_4;
    dMatrixT II20p_5;
    dMatrixT II20p_6;
    dMatrixT II20p_7;
    dMatrixT II20p_8;
    dMatrixT II20p_9;
    dMatrixT II20p_10;
    dMatrixT II20p_11;
    dMatrixT II20p_12;
    dMatrixT II20p_13;
    dMatrixT II20p_14;
    dMatrixT II20p_15;
    dMatrixT II20p_16;


    dTensor3DT Temp_II21p;
    dMatrixT II21e_1;
    dMatrixT II21p_1;
    dMatrixT II21p_2;
    dMatrixT II21p_3;
    dMatrixT II21p_4;
    dMatrixT II21p_5;
    dMatrixT II21p_6;
    dMatrixT II21p_7;
    dMatrixT II21p_8;
    dMatrixT II21p_1a;
    dMatrixT II21p_2a;
    dMatrixT II21p_3a;
    dMatrixT II21p_4a;
    dMatrixT II21p_5a;
    dMatrixT II21p_6a;
    dMatrixT II21p_7a;
    dMatrixT II21p_8a;
    dMatrixT II21p_9;

    dMatrixT II21p_1_1;
    dMatrixT II21p_2_1;
    dMatrixT II21p_3_1;
    dMatrixT II21p_4_1;
    dMatrixT II21p_5_1;
    dMatrixT II21p_6_1;
    dMatrixT II21p_7_1;
    dMatrixT II21p_8_1;
    dMatrixT II21p_1a_1;
    dMatrixT II21p_2a_1;
    dMatrixT II21p_3a_1;
    dMatrixT II21p_4a_1;
    dMatrixT II21p_5a_1;
    dMatrixT II21p_6a_1;
    dMatrixT II21p_7a_1;
    dMatrixT II21p_8a_1;

    dMatrixT II21p_10;
    dMatrixT II21p_11;
    dMatrixT II21p_12;
    dMatrixT II21p_13;
    dMatrixT II21p_14;
    dMatrixT II21p_15;
    dMatrixT II21p_16;
    dMatrixT II21p_17;
    dMatrixT II21p_10a;
    dMatrixT II21p_11a;
    dMatrixT II21p_12a;
    dMatrixT II21p_13a;
    dMatrixT II21p_14a;
    dMatrixT II21p_15a;
	dMatrixT II21p_16a;
	dMatrixT II21p_17a;

	dMatrixT II21p_10_1;
	dMatrixT II21p_11_1;
	dMatrixT II21p_12_1;
	dMatrixT II21p_13_1;
	dMatrixT II21p_14_1;
	dMatrixT II21p_15_1;
	dMatrixT II21p_16_1;
	dMatrixT II21p_17_1;
	dMatrixT II21p_10a_1;
	dMatrixT II21p_11a_1;
	dMatrixT II21p_12a_1;
	dMatrixT II21p_13a_1;
	dMatrixT II21p_14a_1;
	dMatrixT II21p_15a_1;
	dMatrixT II21p_16a_1;
	dMatrixT II21p_17a_1;

	dMatrixT II21p_18;
	dMatrixT II21p_19;
	dMatrixT II21p_20;
	dMatrixT II21p_21;
	dMatrixT II21p_22;
	dMatrixT II21p_23;
	dMatrixT II21p_24;
	dMatrixT II21p_25;
	dMatrixT II21p_18a;
	dMatrixT II21p_19a;
	dMatrixT II21p_20a;
	dMatrixT II21p_21a;
	dMatrixT II21p_22a;
	dMatrixT II21p_23a;
	dMatrixT II21p_24a;
	dMatrixT II21p_25a;
	dMatrixT II21p_26;

	dMatrixT II21p_27;
	dMatrixT II21p_28;
	dMatrixT II21p_29;
	dMatrixT II21p_30;
	dMatrixT II21p_31;
	dMatrixT II21p_32;
	dMatrixT II21p_33;
	dMatrixT II21p_34;
	dMatrixT II21p_35;
	dMatrixT II21p_27a;
	dMatrixT II21p_28a;
	dMatrixT II21p_29a;
	dMatrixT II21p_30a;
	dMatrixT II21p_31a;
	dMatrixT II21p_32a;
	dMatrixT II21p_33a;
	dMatrixT II21p_34a;

	dMatrixT II21p_27_1;
	dMatrixT II21p_28_1;
	dMatrixT II21p_29_1;
	dMatrixT II21p_30_1;
	dMatrixT II21p_31_1;
	dMatrixT II21p_32_1;
	dMatrixT II21p_33_1;
	dMatrixT II21p_34_1;
	dMatrixT II21p_27a_1;
	dMatrixT II21p_28a_1;
	dMatrixT II21p_29a_1;
	dMatrixT II21p_30a_1;
	dMatrixT II21p_31a_1;
	dMatrixT II21p_32a_1;
	dMatrixT II21p_33a_1;
	dMatrixT II21p_34a_1;

	dMatrixT II21p_36;
	dMatrixT II21p_37;
	dMatrixT II21p_38;
	dMatrixT II21p_39;
	dMatrixT II21p_40;
	dMatrixT II21p_41;
	dMatrixT II21p_42;
	dMatrixT II21p_43;
	dMatrixT II21p_36a;
	dMatrixT II21p_37a;
	dMatrixT II21p_38a;
	dMatrixT II21p_39a;
	dMatrixT II21p_40a;
	dMatrixT II21p_41a;
	dMatrixT II21p_42a;
	dMatrixT II21p_43a;



    double Temp_II21p_2_8_tau7;
    double Temp_II21p_2_8_tau8_1;
    double Temp_II21p_2_8_tau8_2;
    double Temp_II21p_2_8_tau9;
    double Temp_II21p_2_8_tau10;
    double Temp_II21p_2_8_tau11;

    double Temp_II21p_2_8_tau7_1;
    double Temp_II21p_2_8_tau8_1_1;
    double Temp_II21p_2_8_tau8_2_1;
    double Temp_II21p_2_8_tau9_1;
    double Temp_II21p_2_8_tau10_1;
    double Temp_II21p_2_8_tau11_1;

    double Temp_II21p_10_17_tau7;
    double Temp_II21p_10_17_tau8_1;
    double Temp_II21p_10_17_tau8_2;
    double Temp_II21p_10_17_tau9;
    double Temp_II21p_10_17_tau10;
    double Temp_II21p_10_17_tau11;

    double Temp_II21p_10_17_tau7_1;
    double Temp_II21p_10_17_tau8_1_1;
    double Temp_II21p_10_17_tau8_2_1;
    double Temp_II21p_10_17_tau9_1;
    double Temp_II21p_10_17_tau10_1;
    double Temp_II21p_10_17_tau11_1;

    double Temp_II21p_18_25_tau7;
    double Temp_II21p_18_25_tau8_1;
    double Temp_II21p_18_25_tau8_2;
    double Temp_II21p_18_25_tau9;
    double Temp_II21p_18_25_tau10;
    double Temp_II21p_18_25_tau11;

    double Temp_II21p_27_34_tau7;
    double Temp_II21p_27_34_tau8_1;
    double Temp_II21p_27_34_tau8_2;
    double Temp_II21p_27_34_tau9;
    double Temp_II21p_27_34_tau10;
    double Temp_II21p_27_34_tau11;

    double Temp_II21p_27_34_tau7_1;
    double Temp_II21p_27_34_tau8_1_1;
    double Temp_II21p_27_34_tau8_2_1;
    double Temp_II21p_27_34_tau9_1;
    double Temp_II21p_27_34_tau10_1;
    double Temp_II21p_27_34_tau11_1;

    double Temp_II21p_36_43_tau7;
    double Temp_II21p_36_43_tau8;
    double Temp_II21p_36_43_tau8_1;
    double Temp_II21p_36_43_tau9;
    double Temp_II21p_36_43_tau10;
    double Temp_II21p_36_43_tau11;

    double Temp_II21p_44_51_tau7;
    double Temp_II21p_44_51_tau8;
    double Temp_II21p_44_51_tau8_1;
    double Temp_II21p_44_51_tau9;
    double Temp_II21p_44_51_tau10;
    double Temp_II21p_44_51_tau11;

    double Temp_II21p_52_59_tau7;
    double Temp_II21p_52_59_tau8;
    double Temp_II21p_52_59_tau8_1;
    double Temp_II21p_52_59_tau9;
    double Temp_II21p_52_59_tau10;
    double Temp_II21p_52_59_tau11;

    double Temp_II21p_60_67_tau7;
    double Temp_II21p_60_67_tau8_1;
    double Temp_II21p_60_67_tau8_2;
    double Temp_II21p_60_67_tau9;
    double Temp_II21p_60_67_tau10;
    double Temp_II21p_60_67_tau11;

    double Temp_II21p_60_67_tau7_1;
    double Temp_II21p_60_67_tau8_1_1;
    double Temp_II21p_60_67_tau8_2_1;
    double Temp_II21p_60_67_tau9_1;
    double Temp_II21p_60_67_tau10_1;
    double Temp_II21p_60_67_tau11_1;

    double Temp_II21p_68_75_tau7;
    double Temp_II21p_68_75_tau8;
    double Temp_II21p_68_75_tau8_1;
    double Temp_II21p_68_75_tau9;
    double Temp_II21p_68_75_tau10;
    double Temp_II21p_68_75_tau11;

    double Temp_II21p_2_8;
    double Temp_II21p_10_17;
    double Temp_II21p_18_25;
    double Temp_II21p_27_34;
    double Temp_II21p_36_43;
    double Temp_II21p_44_51;

    dMatrixT II21p_44;
    dMatrixT II21p_45;
    dMatrixT II21p_46;
    dMatrixT II21p_47;
    dMatrixT II21p_48;
    dMatrixT II21p_49;
    dMatrixT II21p_50;
    dMatrixT II21p_51;
    dMatrixT II21p_44a;
    dMatrixT II21p_45a;
    dMatrixT II21p_46a;
    dMatrixT II21p_47a;
    dMatrixT II21p_48a;
    dMatrixT II21p_49a;
    dMatrixT II21p_50a;
    dMatrixT II21p_51a;

    double Temp_II21p_52_59;
    dMatrixT II21p_52;
    dMatrixT II21p_53;
    dMatrixT II21p_54;
    dMatrixT II21p_55;
    dMatrixT II21p_56;
    dMatrixT II21p_57;
    dMatrixT II21p_58;
    dMatrixT II21p_59;
    dMatrixT II21p_52a;
    dMatrixT II21p_53a;
    dMatrixT II21p_54a;
    dMatrixT II21p_55a;
    dMatrixT II21p_56a;
    dMatrixT II21p_57a;
    dMatrixT II21p_58a;
    dMatrixT II21p_59a;

    double Temp_II21p_60_67;
    dMatrixT II21p_60;
    dMatrixT II21p_61;
    dMatrixT II21p_62;
    dMatrixT II21p_63;
    dMatrixT II21p_64;
    dMatrixT II21p_65;
    dMatrixT II21p_66;
    dMatrixT II21p_67;
    dMatrixT II21p_60a;
    dMatrixT II21p_61a;
    dMatrixT II21p_62a;
    dMatrixT II21p_63a;
    dMatrixT II21p_64a;
    dMatrixT II21p_65a;
    dMatrixT II21p_66a;
    dMatrixT II21p_67a;

    dMatrixT II21p_60_1;
    dMatrixT II21p_61_1;
    dMatrixT II21p_62_1;
    dMatrixT II21p_63_1;
    dMatrixT II21p_64_1;
    dMatrixT II21p_65_1;
    dMatrixT II21p_66_1;
    dMatrixT II21p_67_1;
    dMatrixT II21p_60a_1;
    dMatrixT II21p_61a_1;
    dMatrixT II21p_62a_1;
    dMatrixT II21p_63a_1;
    dMatrixT II21p_64a_1;
    dMatrixT II21p_65a_1;
    dMatrixT II21p_66a_1;
    dMatrixT II21p_67a_1;

    double Temp_II21p_68_75;
    dMatrixT II21p_68;
    dMatrixT II21p_69;
    dMatrixT II21p_70;
    dMatrixT II21p_71;
    dMatrixT II21p_72;
    dMatrixT II21p_73;
    dMatrixT II21p_74;
    dMatrixT II21p_75;
    dMatrixT II21p_68a;
    dMatrixT II21p_69a;
    dMatrixT II21p_70a;
    dMatrixT II21p_71a;
    dMatrixT II21p_72a;
    dMatrixT II21p_73a;
    dMatrixT II21p_74a;
    dMatrixT II21p_75a;



    dMatrixT fKMphiu_II13e_1;


    dMatrixT fKMphiu_II14p_1;
    dMatrixT fKMphiu_II14p_2;
    dMatrixT fKMphiu_II14p_3;
    dMatrixT fKMphiu_II14p_4;
    dMatrixT fKMphiu_II14p_5;
    dMatrixT fKMphiphi_II14p_6;
    dMatrixT fKMphiphi_II14p_7;
    dMatrixT fKMphiphi_II14p_8;
    dMatrixT fKMphiu_II14p_9;
    dMatrixT fKMphiu_II14p_10;
    dMatrixT fKMphiu_II14p_11;
    dMatrixT fKMphiu_II14p_12;
    dMatrixT fKMphiu_II14p_13;
    dMatrixT fKMphiphi_II14p_14;
    dMatrixT fKMphiphi_II14p_15;
    dMatrixT fKMphiphi_II14p_16;

    dMatrixT fKMphiphi_II15e_1;

    dMatrixT fKMphiu_II16p_1;
    dMatrixT fKMphiu_II16p_2;
    dMatrixT fKMphiu_II16p_3;
    dMatrixT fKMphiu_II16p_4;
    dMatrixT fKMphiu_II16p_5;
    dMatrixT fKMphiphi_II16p_6;
    dMatrixT fKMphiphi_II16p_7;
    dMatrixT fKMphiphi_II16p_8;
    dMatrixT fKMphiu_II16p_9;
    dMatrixT fKMphiu_II16p_10;
    dMatrixT fKMphiu_II16p_11;
    dMatrixT fKMphiu_II16p_12;
    dMatrixT fKMphiu_II16p_13;
    dMatrixT fKMphiphi_II16p_14;
    dMatrixT fKMphiphi_II16p_15;
    dMatrixT fKMphiphi_II16p_16;


    dMatrixT fKMphiu_II17p_1;
    dMatrixT fKMphiu_II17p_2;
    dMatrixT fKMphiu_II17p_3;
    dMatrixT fKMphiu_II17p_4;
    dMatrixT fKMphiu_II17p_5;
    dMatrixT fKMphiphi_II17p_6;
    dMatrixT fKMphiphi_II17p_7;
    dMatrixT fKMphiphi_II17p_8;
    dMatrixT fKMphiu_II17p_9;
    dMatrixT fKMphiu_II17p_10;
    dMatrixT fKMphiu_II17p_11;
    dMatrixT fKMphiu_II17p_12;
    dMatrixT fKMphiu_II17p_13;
    dMatrixT fKMphiphi_II17p_14;
    dMatrixT fKMphiphi_II17p_15;
    dMatrixT fKMphiphi_II17p_16;


    dMatrixT fKMphiu_II18p_1;
    dMatrixT fKMphiu_II18p_2;
    dMatrixT fKMphiu_II18p_3;
    dMatrixT fKMphiu_II18p_4;
    dMatrixT fKMphiu_II18p_5;
    dMatrixT fKMphiphi_II18p_6;
    dMatrixT fKMphiphi_II18p_7;
    dMatrixT fKMphiphi_II18p_8;
    dMatrixT fKMphiu_II18p_9;
    dMatrixT fKMphiu_II18p_10;
    dMatrixT fKMphiu_II18p_11;
    dMatrixT fKMphiu_II18p_12;
    dMatrixT fKMphiu_II18p_13;
    dMatrixT fKMphiphi_II18p_14;
    dMatrixT fKMphiphi_II18p_15;
    dMatrixT fKMphiphi_II18p_16;


    dMatrixT fKMphiu_II19p_1;
    dMatrixT fKMphiu_II19p_2;
    dMatrixT fKMphiu_II19p_3;
    dMatrixT fKMphiu_II19p_4;
    dMatrixT fKMphiu_II19p_5;
    dMatrixT fKMphiphi_II19p_6;
    dMatrixT fKMphiphi_II19p_7;
    dMatrixT fKMphiphi_II19p_8;
    dMatrixT fKMphiu_II19p_9;
    dMatrixT fKMphiu_II19p_10;
    dMatrixT fKMphiu_II19p_11;
    dMatrixT fKMphiu_II19p_12;
    dMatrixT fKMphiu_II19p_13;
    dMatrixT fKMphiphi_II19p_14;
    dMatrixT fKMphiphi_II19p_15;
    dMatrixT fKMphiphi_II19p_16;



    dMatrixT fKMphiu_II20p_1;
    dMatrixT fKMphiu_II20p_2;
    dMatrixT fKMphiu_II20p_3;
    dMatrixT fKMphiu_II20p_4;
    dMatrixT fKMphiu_II20p_5;
    dMatrixT fKMphiphi_II20p_6;
    dMatrixT fKMphiphi_II20p_7;
    dMatrixT fKMphiphi_II20p_8;
    dMatrixT fKMphiu_II20p_9;
    dMatrixT fKMphiu_II20p_10;
    dMatrixT fKMphiu_II20p_11;
    dMatrixT fKMphiu_II20p_12;
    dMatrixT fKMphiu_II20p_13;
    dMatrixT fKMphiphi_II20p_14;
    dMatrixT fKMphiphi_II20p_15;
    dMatrixT fKMphiphi_II20p_16;



    dMatrixT fKMphiu_II21e_1;
    dMatrixT fKMphiu_II21p_1;
    dMatrixT fKMphiu_II21p_2;
    dMatrixT fKMphiu_II21p_3;
    dMatrixT fKMphiu_II21p_4;
    dMatrixT fKMphiu_II21p_5;
    dMatrixT fKMphiphi_II21p_6;
    dMatrixT fKMphiphi_II21p_7;
    dMatrixT fKMphiphi_II21p_8;
    dMatrixT fKMphiu_II21p_1a;
    dMatrixT fKMphiu_II21p_2a;
    dMatrixT fKMphiu_II21p_3a;
    dMatrixT fKMphiu_II21p_4a;
    dMatrixT fKMphiu_II21p_5a;
    dMatrixT fKMphiphi_II21p_6a;
    dMatrixT fKMphiphi_II21p_7a;
    dMatrixT fKMphiphi_II21p_8a;

    dMatrixT fKMphiphi_II21p_9;



    dMatrixT fKMphiu_II21p_10;
    dMatrixT fKMphiu_II21p_11;
    dMatrixT fKMphiu_II21p_12;
    dMatrixT fKMphiu_II21p_13;
    dMatrixT fKMphiu_II21p_14;
    dMatrixT fKMphiphi_II21p_15;
    dMatrixT fKMphiphi_II21p_16;
    dMatrixT fKMphiphi_II21p_17;
    dMatrixT fKMphiu_II21p_10a;
    dMatrixT fKMphiu_II21p_11a;
    dMatrixT fKMphiu_II21p_12a;
    dMatrixT fKMphiu_II21p_13a;
    dMatrixT fKMphiu_II21p_14a;
    dMatrixT fKMphiphi_II21p_15a;
    dMatrixT fKMphiphi_II21p_16a;
    dMatrixT fKMphiphi_II21p_17a;


    dMatrixT fKMphiu_II21p_18;
    dMatrixT fKMphiu_II21p_19;
    dMatrixT fKMphiu_II21p_20;
    dMatrixT fKMphiu_II21p_21;
    dMatrixT fKMphiu_II21p_22;
    dMatrixT fKMphiphi_II21p_23;
    dMatrixT fKMphiphi_II21p_24;
    dMatrixT fKMphiphi_II21p_25;
    dMatrixT fKMphiu_II21p_18a;
    dMatrixT fKMphiu_II21p_19a;
    dMatrixT fKMphiu_II21p_20a;
    dMatrixT fKMphiu_II21p_21a;
    dMatrixT fKMphiu_II21p_22a;
    dMatrixT fKMphiphi_II21p_23a;
    dMatrixT fKMphiphi_II21p_24a;
    dMatrixT fKMphiphi_II21p_25a;


    dMatrixT fKMphiu_II21p_26;

    dMatrixT fKMphiu_II21p_27;
    dMatrixT fKMphiu_II21p_28;
    dMatrixT fKMphiu_II21p_29;
    dMatrixT fKMphiu_II21p_30;
    dMatrixT fKMphiu_II21p_31;
    dMatrixT fKMphiphi_II21p_32;
    dMatrixT fKMphiphi_II21p_33;
    dMatrixT fKMphiphi_II21p_34;
    dMatrixT fKMphiu_II21p_27a;
    dMatrixT fKMphiu_II21p_28a;
    dMatrixT fKMphiu_II21p_29a;
    dMatrixT fKMphiu_II21p_30a;
    dMatrixT fKMphiu_II21p_31a;
    dMatrixT fKMphiphi_II21p_32a;
    dMatrixT fKMphiphi_II21p_33a;
    dMatrixT fKMphiphi_II21p_34a;

    dMatrixT fKMphiphi_II21p_35;


    dMatrixT fKMphiu_II21p_36;
    dMatrixT fKMphiu_II21p_37;
    dMatrixT fKMphiu_II21p_38;
    dMatrixT fKMphiu_II21p_39;
    dMatrixT fKMphiu_II21p_40;
    dMatrixT fKMphiphi_II21p_41;
    dMatrixT fKMphiphi_II21p_42;
    dMatrixT fKMphiphi_II21p_43;
    dMatrixT fKMphiu_II21p_36a;
    dMatrixT fKMphiu_II21p_37a;
    dMatrixT fKMphiu_II21p_38a;
    dMatrixT fKMphiu_II21p_39a;
    dMatrixT fKMphiu_II21p_40a;
    dMatrixT fKMphiphi_II21p_41a;
    dMatrixT fKMphiphi_II21p_42a;
    dMatrixT fKMphiphi_II21p_43a;



    dMatrixT fKMphiu_II21p_44;
    dMatrixT fKMphiu_II21p_45;
    dMatrixT fKMphiu_II21p_46;
    dMatrixT fKMphiu_II21p_47;
    dMatrixT fKMphiu_II21p_48;
    dMatrixT fKMphiphi_II21p_49;
    dMatrixT fKMphiphi_II21p_50;
    dMatrixT fKMphiphi_II21p_51;
    dMatrixT fKMphiu_II21p_44a;
    dMatrixT fKMphiu_II21p_45a;
    dMatrixT fKMphiu_II21p_46a;
    dMatrixT fKMphiu_II21p_47a;
    dMatrixT fKMphiu_II21p_48a;
    dMatrixT fKMphiphi_II21p_49a;
    dMatrixT fKMphiphi_II21p_50a;
    dMatrixT fKMphiphi_II21p_51a;



    dMatrixT fKMphiu_II21p_52;
    dMatrixT fKMphiu_II21p_53;
    dMatrixT fKMphiu_II21p_54;
    dMatrixT fKMphiu_II21p_55;
    dMatrixT fKMphiu_II21p_56;
    dMatrixT fKMphiphi_II21p_57;
    dMatrixT fKMphiphi_II21p_58;
    dMatrixT fKMphiphi_II21p_59;
    dMatrixT fKMphiu_II21p_52a;
    dMatrixT fKMphiu_II21p_53a;
    dMatrixT fKMphiu_II21p_54a;
    dMatrixT fKMphiu_II21p_55a;
    dMatrixT fKMphiu_II21p_56a;
    dMatrixT fKMphiphi_II21p_57a;
    dMatrixT fKMphiphi_II21p_58a;
    dMatrixT fKMphiphi_II21p_59a;



    dMatrixT fKMphiu_II21p_60;
    dMatrixT fKMphiu_II21p_61;
    dMatrixT fKMphiu_II21p_62;
    dMatrixT fKMphiu_II21p_63;
    dMatrixT fKMphiu_II21p_64;
    dMatrixT fKMphiphi_II21p_65;
    dMatrixT fKMphiphi_II21p_66;
    dMatrixT fKMphiphi_II21p_67;
    dMatrixT fKMphiu_II21p_60a;
    dMatrixT fKMphiu_II21p_61a;
    dMatrixT fKMphiu_II21p_62a;
    dMatrixT fKMphiu_II21p_63a;
    dMatrixT fKMphiu_II21p_64a;
    dMatrixT fKMphiphi_II21p_65a;
    dMatrixT fKMphiphi_II21p_66a;
    dMatrixT fKMphiphi_II21p_67a;



    dMatrixT fKMphiu_II21p_68;
    dMatrixT fKMphiu_II21p_69;
    dMatrixT fKMphiu_II21p_70;
    dMatrixT fKMphiu_II21p_71;
    dMatrixT fKMphiu_II21p_72;
    dMatrixT fKMphiphi_II21p_73;
    dMatrixT fKMphiphi_II21p_74;
    dMatrixT fKMphiphi_II21p_75;
    dMatrixT fKMphiu_II21p_68a;
    dMatrixT fKMphiu_II21p_69a;
    dMatrixT fKMphiu_II21p_70a;
    dMatrixT fKMphiu_II21p_71a;
    dMatrixT fKMphiu_II21p_72a;
    dMatrixT fKMphiphi_II21p_73a;
    dMatrixT fKMphiphi_II21p_74a;
    dMatrixT fKMphiphi_II21p_75a;




    double fNormdevMKLM_tr;
    double fNormdevMeKLM;
    double fMicro_gradient_Yield_function_tr;
    double fMicro_gradient_Yield_function;
    double Micro_gradient_Plasticity_Occurrence;
    double dNormdevfMKLMdDelgammanablachi;
    double Norm_Mean_fMKLM_tr;
    double Norm_Mean_fMeKLM;
    double Norm_kc_nablachi_n;
    double dNorm_meanfMKLMdDelgammanablachi;
    double dNorm_kc_nablachidDelgammanablachi;
    double dfNorm_devMKLMdDelgammanablachi;
    double dFyield_nablachidDelgammanablachi;
    double fdelDelgammanablachi;
    double fDelgammanablachi;
    double Norm_cohesion_nablachi;
    double Coeff_delDelgamma_nablachi;
    double fdFYdS_trace;
    double fdFYchidSIGMA_S_trace;



    dMatrixT fKEtaM;
    dMatrixT fKMFphiu;
    dMatrixT fKMchiphiphi;
    dMatrixT fKMphiu_1;
    dMatrixT fKMphiphi_1;
    dMatrixT fKMphiu_2;
    dMatrixT fKMphiphi_2;
    /* Plasticity Matrices*/
    dMatrixT PHIMATRIX;
    dTensor3DT GPHIMATRIX;

    dArrayT Vintp_1;
    dArrayT Vintp_1_temp;
    dArrayT fV1p;
    dArrayT Vintp_2;
    dArrayT Vintp_2_temp;
    dArrayT fV2p;
    dArrayT Vintp_3;
    dArrayT Vintp_3_temp;
    dArrayT fV3p;



    dMatrixT fFp;
    dMatrixT fFp_inverse;
    dMatrixT fFp_n;
    dMatrixT fFp_n_inverse;

    dMatrixT fChip;
    dMatrixT fChip_inverse;
    dMatrixT fChip_n;
    dMatrixT fChip_n_inverse;

    dMatrixT fFe;
    dMatrixT fFe_tr;
    dMatrixT fFe_Inverse;
    dMatrixT fChie;
    dMatrixT fChie_tr;
    dMatrixT fdGdS_n;
    dMatrixT fdGdS;
    dMatrixT fdFYdS;
    dMatrixT fdFYdS_n;

    /***************************************************/
    /*****Micro-scale plasticity matrices **************/
    /***************************************************/
    dMatrixT PSIe_n_inverse;
    dMatrixT PSIe_n;
    dMatrixT PSIe_inverse;
    dMatrixT PSIe_tr;
    dMatrixT PSIe;
    dMatrixT fCchie;
    dMatrixT fCchie_tr;
    dMatrixT fCchi;
    dMatrixT fCchie_n;
    dMatrixT fCchie_n_inverse;

    dMatrixT dChipdDelgamma;//For combined plasticity
    dMatrixT dChipdDelgammachi;
    dMatrixT dChiedDelgamma;
    dMatrixT dChiedDelgammachi;
    dMatrixT dEpsilonedDelgammachi;
    dMatrixT dSdDelgammachi;
    dMatrixT ddevSdDelgammachi;


    //For line search algorithm
    dMatrixT dSds;
    dMatrixT ddevSds;
    dMatrixT dEeds;
    dMatrixT dEpsiloneds;
    dMatrixT dFeds;

    // For line search algortihm for combined plasticity
    dMatrixT dChipds;
    dMatrixT dChieds;
    dMatrixT dSIGMA_Sds;
    dMatrixT ddevSIGMA_Sds;



    dMatrixT LocalConsistentTangent;
    dMatrixT LocalConsistentTangentInverse;
    dArrayT LocalRHSVector;
    dArrayT fdelDelgammaVector;

    dArrayT Temp_vec1;
    dArrayT Temp_vec2;
    dArrayT Temp_vec3;
    dArrayT Temp_vec4;

    double alphai;
    double alpha1;
    double alpha2;
    double s1,s2,incr,incrx;
    /***************************************************/
    /************Micro Scale Gradient Plasticity *******/
    dArrayT PGchivar;
    dArrayT PGchivar_tr;
    dTensor3DT fMeKLM_Tr;
    dTensor3DT devMeKLM_tr;
    dTensor3DT fmeklm;

    /***************************************************/
    /***************************************************/
    /***************************************************/
    //////////////////Variables Defined due to the trial plasticity flow direction//////
    double			fNorm_devSPK_tr;
    double			Comp33;
    double			Comp44;
    dMatrixT		fdGdS_tr;
    dMatrixT		fdGdS_tr_transpose;
    double			fdGdS_tr_trace;


    dMatrixT		I2p_trial_1;
    dMatrixT		I2p_trial_2;
    dMatrixT		I2p_trial_3;
    dMatrixT		I2p_trial_4;
    dMatrixT		I2p_trial_5;

    dMatrixT		I3p_trial_1;
    dMatrixT		I3p_trial_2;
    dMatrixT		I3p_trial_3;
    dMatrixT		I3p_trial_4;
    dMatrixT		I3p_trial_5;

    dMatrixT		I4p_trial_1;
    dMatrixT		I4p_trial_2;
    dMatrixT		I4p_trial_3;
    dMatrixT		I4p_trial_4;
    dMatrixT		I4p_trial_5;
    dMatrixT		I4p_trial_6;
    dMatrixT		I4p_trial_7;
    dMatrixT		I4p_trial_8;
    dMatrixT		I4p_trial_9;
    dMatrixT		I4p_trial_10;
    dMatrixT		I4p_trial_11;
    dMatrixT		I4p_trial_12;
    dMatrixT		I4p_trial_13;
    dMatrixT		I4p_trial_14;
    dMatrixT		I4p_trial_15;

    dMatrixT		I5p_trial_1;
    dMatrixT		I5p_trial_2;
    dMatrixT		I5p_trial_3;
    dMatrixT		I5p_trial_4;
    dMatrixT		I5p_trial_5;



    dMatrixT		fKu_I2p_trial_1;
    dMatrixT		fKu_I2p_trial_2;
    dMatrixT		fKu_I2p_trial_3;
    dMatrixT		fKu_I2p_trial_4;
    dMatrixT		fKu_I2p_trial_5;

    dMatrixT		fKu_I3p_trial_1;
    dMatrixT		fKu_I3p_trial_2;
    dMatrixT		fKu_I3p_trial_3;
    dMatrixT		fKu_I3p_trial_4;
    dMatrixT		fKu_I3p_trial_5;

    dMatrixT		fKu_I4p_trial_1;
    dMatrixT		fKu_I4p_trial_2;
    dMatrixT		fKu_I4p_trial_3;
    dMatrixT		fKu_I4p_trial_4;
    dMatrixT		fKu_I4p_trial_5;
    dMatrixT		fKu_I4p_trial_6;
    dMatrixT		fKu_I4p_trial_7;
    dMatrixT		fKu_I4p_trial_8;
    dMatrixT		fKu_I4p_trial_9;
    dMatrixT		fKu_I4p_trial_10;
    dMatrixT		fKu_I4p_trial_11;
    dMatrixT		fKu_I4p_trial_12;
    dMatrixT		fKu_I4p_trial_13;
    dMatrixT		fKu_I4p_trial_14;
    dMatrixT		fKu_I4p_trial_15;

    dMatrixT		fKu_I5p_trial_1;
    dMatrixT		fKu_I5p_trial_2;
    dMatrixT		fKu_I5p_trial_3;
    dMatrixT		fKu_I5p_trial_4;
    dMatrixT		fKu_I5p_trial_5;

    dMatrixT		I_temp_DelGamma_dGdStrial_1;
    dMatrixT		I_temp_DelGamma_dGdStrial_2;

    dMatrixT		I6p_trial_1;
    dMatrixT		I6p_trial_2;

    dMatrixT		I7p_trial_1;
    dMatrixT		I7p_trial_2;

    dMatrixT		I8p_trial_1;
    dMatrixT		I8p_trial_2;

    dMatrixT		I9p_trial_1;
    dMatrixT		I9p_trial_2;

    dMatrixT		I10p_trial_1;
    dMatrixT		I10p_trial_2;

    dMatrixT		I11p_trial_1;
    dMatrixT		I11p_trial_2;

    dMatrixT		I12p_trial_1;
    dMatrixT		I12p_trial_2;
    dMatrixT		I12p_trial_3;
    dMatrixT		I12p_trial_4;
    dMatrixT		I12p_trial_5;
    dMatrixT		I12p_trial_6;
    dMatrixT		I12p_trial_7;
    dMatrixT		I12p_trial_8;
    dMatrixT		I12p_trial_9;
    dMatrixT		I12p_trial_10;


    dMatrixT		I13p_trial_1;
    dMatrixT		I13p_trial_2;
    dMatrixT		I13p_trial_3;
    dMatrixT		I13p_trial_4;
    dMatrixT		I13p_trial_5;
    dMatrixT		I13p_trial_6;
    dMatrixT		I13p_trial_7;
    dMatrixT		I13p_trial_8;
    dMatrixT		I13p_trial_9;
    dMatrixT		I13p_trial_10;


    dMatrixT		I14p_trial_1;
    dMatrixT		I14p_trial_2;
    dMatrixT		I14p_trial_3;
    dMatrixT		I14p_trial_4;
    dMatrixT		I14p_trial_5;
    dMatrixT		I14p_trial_6;
    dMatrixT		I14p_trial_7;
    dMatrixT		I14p_trial_8;
    dMatrixT		I14p_trial_9;
    dMatrixT		I14p_trial_10;


    dMatrixT		I15p_trial_1;
    dMatrixT		I15p_trial_2;
    dMatrixT		I15p_trial_3;
    dMatrixT		I15p_trial_4;
    dMatrixT		I15p_trial_5;
    dMatrixT		I15p_trial_6;
    dMatrixT		I15p_trial_7;
    dMatrixT		I15p_trial_8;
    dMatrixT		I15p_trial_9;
    dMatrixT		I15p_trial_10;


    dMatrixT		I16p_trial_1;
    dMatrixT		I16p_trial_2;
    dMatrixT		I16p_trial_3;
    dMatrixT		I16p_trial_4;
    dMatrixT		I16p_trial_5;
    dMatrixT		I16p_trial_6;
    dMatrixT		I16p_trial_7;
    dMatrixT		I16p_trial_8;
    dMatrixT		I16p_trial_9;
    dMatrixT		I16p_trial_10;


    dMatrixT		I17p_trial_1;
    dMatrixT		I17p_trial_2;
    dMatrixT		I17p_trial_3;
    dMatrixT		I17p_trial_4;
    dMatrixT		I17p_trial_5;
    dMatrixT		I17p_trial_6;
    dMatrixT		I17p_trial_7;
    dMatrixT		I17p_trial_8;
    dMatrixT		I17p_trial_9;
    dMatrixT		I17p_trial_10;




    dMatrixT		fKu_I6p_trial_1;
    dMatrixT		fKu_I6p_trial_2;

    dMatrixT		fKu_I7p_trial_1;
    dMatrixT		fKu_I7p_trial_2;

    dMatrixT		fKu_I8p_trial_1;
    dMatrixT		fKu_I8p_trial_2;

    dMatrixT		fKu_I9p_trial_1;
    dMatrixT		fKu_I9p_trial_2;

    dMatrixT		fKu_I10p_trial_1;
    dMatrixT		fKu_I10p_trial_2;

    dMatrixT		fKu_I11p_trial_1;
    dMatrixT		fKu_I11p_trial_2;


    dMatrixT		fKu_I12p_trial_1;
    dMatrixT		fKuphi_I12p_trial_2;
    dMatrixT		fKu_I12p_trial_3;
    dMatrixT		fKuphi_I12p_trial_4;
    dMatrixT		fKu_I12p_trial_5;
    dMatrixT		fKuphi_I12p_trial_6;
    dMatrixT		fKu_I12p_trial_7;
    dMatrixT		fKuphi_I12p_trial_8;
    dMatrixT		fKu_I12p_trial_9;
    dMatrixT		fKuphi_I12p_trial_10;


    dMatrixT		fKu_I13p_trial_1;
    dMatrixT		fKuphi_I13p_trial_2;
    dMatrixT		fKu_I13p_trial_3;
    dMatrixT		fKuphi_I13p_trial_4;
    dMatrixT		fKu_I13p_trial_5;
    dMatrixT		fKuphi_I13p_trial_6;
    dMatrixT		fKu_I13p_trial_7;
    dMatrixT		fKuphi_I13p_trial_8;
    dMatrixT		fKu_I13p_trial_9;
    dMatrixT		fKuphi_I13p_trial_10;


    dMatrixT		fKu_I14p_trial_1;
    dMatrixT		fKuphi_I14p_trial_2;
    dMatrixT		fKu_I14p_trial_3;
    dMatrixT		fKuphi_I14p_trial_4;
    dMatrixT		fKu_I14p_trial_5;
    dMatrixT		fKuphi_I14p_trial_6;
    dMatrixT		fKu_I14p_trial_7;
    dMatrixT		fKuphi_I14p_trial_8;
    dMatrixT		fKu_I14p_trial_9;
    dMatrixT		fKuphi_I14p_trial_10;


    dMatrixT		fKu_I15p_trial_1;
    dMatrixT		fKuphi_I15p_trial_2;
    dMatrixT		fKu_I15p_trial_3;
    dMatrixT		fKuphi_I15p_trial_4;
    dMatrixT		fKu_I15p_trial_5;
    dMatrixT		fKuphi_I15p_trial_6;
    dMatrixT		fKu_I15p_trial_7;
    dMatrixT		fKuphi_I15p_trial_8;
    dMatrixT		fKu_I15p_trial_9;
    dMatrixT		fKuphi_I15p_trial_10;


    dMatrixT		fKu_I16p_trial_1;
    dMatrixT		fKuphi_I16p_trial_2;
    dMatrixT		fKu_I16p_trial_3;
    dMatrixT		fKuphi_I16p_trial_4;
    dMatrixT		fKu_I16p_trial_5;
    dMatrixT		fKuphi_I16p_trial_6;
    dMatrixT		fKu_I16p_trial_7;
    dMatrixT		fKuphi_I16p_trial_8;
    dMatrixT		fKu_I16p_trial_9;
    dMatrixT		fKuphi_I16p_trial_10;


    dMatrixT		fKu_I17p_trial_1;
    dMatrixT		fKuphi_I17p_trial_2;
    dMatrixT		fKu_I17p_trial_3;
    dMatrixT		fKuphi_I17p_trial_4;
    dMatrixT		fKu_I17p_trial_5;
    dMatrixT		fKuphi_I17p_trial_6;
    dMatrixT		fKu_I17p_trial_7;
    dMatrixT		fKuphi_I17p_trial_8;
    dMatrixT		fKu_I17p_trial_9;
    dMatrixT		fKuphi_I17p_trial_10;


    void Form_I2p_trial_1(void);
    void Form_I2p_trial_2(void);
    void Form_I2p_trial_3(void);
    void Form_I2p_trial_4(void);
    void Form_I2p_trial_5(void);

    void Form_I3p_trial_1(void);
    void Form_I3p_trial_2(void);
    void Form_I3p_trial_3(void);
    void Form_I3p_trial_4(void);
    void Form_I3p_trial_5(void);

    void Form_I4p_trial_1(void);
    void Form_I4p_trial_2(void);
    void Form_I4p_trial_3(void);
    void Form_I4p_trial_4(void);
    void Form_I4p_trial_5(void);
    void Form_I4p_trial_6(void);
    void Form_I4p_trial_7(void);
    void Form_I4p_trial_8(void);
    void Form_I4p_trial_9(void);
    void Form_I4p_trial_10(void);
    void Form_I4p_trial_11(void);
    void Form_I4p_trial_12(void);
    void Form_I4p_trial_13(void);
    void Form_I4p_trial_14(void);
    void Form_I4p_trial_15(void);

    void Form_I5p_trial_1(void);
    void Form_I5p_trial_2(void);
    void Form_I5p_trial_3(void);
    void Form_I5p_trial_4(void);
    void Form_I5p_trial_5(void);

    void Form_I_temp_DelGamma_dGdStrial(void);
    void Form_I_temp_DelGamma_dGdStrial_transpose(void);

    void Form_I6p_trial_1(void);
    void Form_I6p_trial_2(void);
    void Form_I7p_trial_1(void);
    void Form_I7p_trial_2(void);
    void Form_I8p_trial_1(void);
    void Form_I8p_trial_2(void);
    void Form_I9p_trial_1(void);
    void Form_I9p_trial_2(void);
    void Form_I10p_trial_1(void);
    void Form_I10p_trial_2(void);
    void Form_I11p_trial_1(void);
    void Form_I11p_trial_2(void);


    void Form_I2p_trial_1_5(void);
    void Form_I3p_trial_1_5(void);
    void Form_I4p_trial_1_5(void);
    void Form_I4p_trial_6_10(void);
    void Form_I4p_trial_11_15(void);
    void Form_I5p_trial_1_5(void);


    void Form_I6p_trial_1_2(void);
    void Form_I7p_trial_1_2(void);
    void Form_I8p_trial_1_2(void);
    void Form_I9p_trial_1_2(void);
    void Form_I10p_trial_1_2(void);
    void Form_I11p_trial_1_2(void);


    void Form_I12p_trial_1_10(void);
    void Form_I13p_trial_1_10(void);
    void Form_I14p_trial_1_10(void);
    void Form_I15p_trial_1_10(void);
    void Form_I16p_trial_1_10(void);
    void Form_I17p_trial_1_10(void);


    ///////////////////////////////////////////////////////////////////////////////////////////



    dMatrixT fRight_Cauchy_Green_tensor_tr;
    dMatrixT fLagrangian_strain_tensor_tr;
    dMatrixT Elastic_MicroStnTensor_tr;
    dMatrixT Elastic_MicroStnTensor;
    dMatrixT fMicroRight_Cauchy_Green_tensor;
    dMatrixT fMicroRight_Cauchy_Green_tensor_tr;
    dMatrixT fMicroRight_Elastic_Cauchy_Green_tensor;
    dMatrixT fMicroRight_Elastic_Cauchy_Green_tensor_tr;
    //dMatrixT fMicroRight_Cauchy_Green_tensor;
    //dMatrixT fMicroRight_Cauchy_Green_tensor_tr;

    dMatrixT fSPK_tr;
    dMatrixT fdevSPK_tr;
    //dMatrixT fSPK;
    dMatrixT dSdDelgamma;
    dMatrixT ddevSdDelgamma;
    dMatrixT dEedDelgamma;
    dMatrixT dEpsilonedDelgamma;

    //dMatrixT fSIGMA_S;
    dMatrixT SIGMA_S_tr;
    dMatrixT devSIGMA_S_tr;

    dMatrixT fFeT;


    dMatrixT fA1;
    dMatrixT fA2;
    dMatrixT fA4;
    double fdFYdS_fA2;
    double fA3;
    double fN2;
    double fD2;

    dMatrixT fN1;
    dMatrixT fD1;

    dMatrixT Predictor;
    dMatrixT fCn1;
    dMatrixT fCn1_inv;
    dMatrixT fFp_tr;
    dMatrixT fdGdS_n1;

    int PlasticityCheck,MicroScaleGradient_check;
    double fF_tr_fact;
    double fCombinedYield_function,fCombinedYield_function_tr,Stress_Norm,Stress_Norm_tr,dFCYdDelGamma;
    double fMicroScaleGradientYield_function,fMicroScaleGradientYield_function_tr,invGc,invGc_n,invPGchivar,invPGchivar_tr;
    double invdevMeKLM,invdevMeKLM_tr,AGphi_chi,BGphi_chi,AGpsi_chi,BGpsi_chi;
    double fdelDelgammaGchi,fDelgammaGchi,dInvddevMKLMdDelgammaGchi,dFGYdDelgammaGchi,dInvcGchidDelgammaGchi,dinvPGchivardDelgammaGchi;
    double Pbar,Pbar_tr,Pchibar,Pchibar_tr,dcchidDelgamma;
    double dFCYdDelgamma;
    double fYield_function,fYield_function_tr,dFYdc;
    double fMicroYield_function,fMicroYield_function_tr;
    double devfSPKinv,devfSPKinv_tr;
    double devSIGMA_S_inv,devSIGMA_S_inv_tr;
    double fDelgamma, fdelDelgamma,dFYdDelgamma,dFYdDelgammachi;
    double kgamma,kcc,cohesion,cohesion_chi;
    double fDelgammachi, fdelDelgammachi,dFYchidDelgammachi,dFYchidDelgamma;
    double dPdDelgamma,dcdDelgamma,Temp_inv,press,InvddevSdDelgamma;
    double dPdDelgammachi,ddevSdDelgamma_inv,ddevSdDelgammachi_inv;
    double dPchidDelgammachi,dcchidDelgammachi,ddevSIGMA_SdDelgammachi_inv;
    double dPchidDelgamma,ddevSIGMA_SdDelgamma_inv;
    int iter_count, global_iteration;
    int iteration_num;
    double predictor_norm, Fp_norm;
    double Aphi,Bphi,Apsi,Bpsi;
    double Aphi_chi,Bphi_chi,Apsi_chi,Bpsi_chi,Aphi_nablachi,Bphi_nablachi,Apsi_nablachi,Bpsi_nablachi;
    double Beta;

    /* for local Newton-Raphson iteration */
   int iIterationMax;
   double dRelTol, dAbsTol,fConst1;

   int PlasticityCondition,MacroPlasticityCondition,MicroPlasticityCondition;
   int iPlasticityCheck;

   /* for local trial yield check */
   double dYieldTrialTol;
     /* some scalars used in calculations */
   double dFYdScol1,fdFYdS_fA1,fdFYdS_fA1T,trfA1,fdFYdc,dFYdc_delc;
   double trfN1,fdFYdS_fN1,fdFYdS_fN1T;
   double fdFYchidSIGMA_fD1T,fdFYchidSIGMA_fD1,dFYchidSIGMA_Scol1,trfD1;
   double fdFYchidSIGMA_fA1,fdFYchidSIGMA_fA1T,fdFYchidSIGMA_fN1,fdFYchidSIGMA_fN1T;
   double fConst4,fdFYchidcchi;
   double mean_stress_tr,mean_stress;
   double fdFYchidSIGMA_S_fA2;


   /* for coupled solution*/
   double fdFYdS_fD1,fdFYdS_fD1T;
   double fConst2,fConst3;
   double Comp11,Comp12,Comp21,Comp22;
   //double Comp1,Comp2,Comp3,Comp4;

   // For line search algorithm
   double dcds;
   double dPds;
   double InvddevSds;
   double dPchids;
   double dcchids;
   double dFYds;
   double Gsi;
   double Gs0;
   ///////////

    dMatrixT IJp_1;
    dMatrixT fKu_IJp_1;
    dMatrixT IJp_2;
    dMatrixT fKu_IJp_2;
    dMatrixT IJp_3;
    dMatrixT fKu_IJp_3;
    dMatrixT IJp_4;
    dMatrixT fKu_IJp_4;
    dMatrixT IJp_5;
    dMatrixT fKu_IJp_5;
    dMatrixT IJp_6;
    dMatrixT fKu_IJp_6;
    dMatrixT IJp_7;
    dMatrixT fKuphi_IJp_7;
    dMatrixT IJp_8;
    dMatrixT fKuphi_IJp_8;
    dMatrixT IJp_9;
    dMatrixT fKuphi_IJp_9;
    /* Matrices from coupling */
    dMatrixT IJp_10;
    dMatrixT fKu_IJp_10;
    dMatrixT IJp_11;
    dMatrixT fKu_IJp_11;
    dMatrixT IJp_12;
    dMatrixT fKu_IJp_12;
    dMatrixT IJp_13;
    dMatrixT fKuphi_IJp_13;
    dMatrixT IJp_14;
    dMatrixT fKu_IJp_14;
    dMatrixT IJp_15;
    dMatrixT fKuphi_IJp_15;
    dMatrixT IJp_16;
    dMatrixT fKu_IJp_16;
    dMatrixT IJp_17;
    dMatrixT fKuphi_IJp_17;

    dMatrixT I1e_1;
    dMatrixT fKu_I1e_1;

    dMatrixT I2e_1;
    dMatrixT fKu_I2e_1;
    dMatrixT I2p_2;
    dMatrixT I2p_2_1;
    dMatrixT fKu_I2p_2;
    dMatrixT I2p_3;
    dMatrixT I2p_3_1;
    dMatrixT fKu_I2p_3;
    dMatrixT I2p_4;
    dMatrixT I2p_4_1;
    dMatrixT fKu_I2p_4;
    dMatrixT I2p_5;
    dMatrixT I2p_5_1;
    dMatrixT fKu_I2p_5;
    dMatrixT I2p_6;
    dMatrixT I2p_6_1;
    dMatrixT fKu_I2p_6;
    dMatrixT I2p_7;
    dMatrixT I2p_7_1;
    dMatrixT fKu_I2p_7;
    dMatrixT I2p_8;
    dMatrixT I2p_8_1;
    dMatrixT fKuphi_I2p_8;
    dMatrixT I2p_9;
    dMatrixT I2p_9_1;
    dMatrixT fKuphi_I2p_9;
    dMatrixT I2p_10;
    dMatrixT I2p_10_1;
    dMatrixT fKuphi_I2p_10;
    /* Matrices from couplinb */
    dMatrixT I2p_11;
    dMatrixT fKu_I2p_11;
    dMatrixT I2p_12;
    dMatrixT fKu_I2p_12;
    dMatrixT I2p_13;
    dMatrixT fKu_I2p_13;
    dMatrixT I2p_14;
    dMatrixT fKuphi_I2p_14;
    dMatrixT I2p_15;
    dMatrixT fKu_I2p_15;
    dMatrixT I2p_16;
    dMatrixT fKuphi_I2p_16;
    dMatrixT I2p_17;
    dMatrixT fKu_I2p_17;
    dMatrixT I2p_18;
    dMatrixT fKuphi_I2p_18;




    dMatrixT I2p_11_1;
    dMatrixT I2p_12_1;
    dMatrixT I2p_13_1;
    dMatrixT I2p_14_1;
    dMatrixT I2p_15_1;
    dMatrixT I2p_16_1;
    dMatrixT I2p_17_1;
    dMatrixT I2p_18_1;


    dMatrixT I3p_7_1;
    dMatrixT I3p_8_1;
    dMatrixT I3p_9_1;
    dMatrixT I3p_10_1;
    dMatrixT I3p_11_1;
    dMatrixT I3p_12_1;
    dMatrixT I3p_19_1;
    dMatrixT I3p_20_1;
    dMatrixT I3p_21_1;
    dMatrixT I3p_27_1;
    dMatrixT I3p_28_1;
    dMatrixT I3p_29_1;
    dMatrixT I3p_30_1;
    dMatrixT I3p_31_1;
    dMatrixT I3p_32_1;
    dMatrixT I3p_33_1;
    dMatrixT I3p_34_1;
    dMatrixT I3p_35_1;
    dMatrixT I3p_36_1;
    dMatrixT I3p_43_1;
    dMatrixT I3p_44_1;
    dMatrixT I3p_45_1;
    dMatrixT I3p_49_1;
    dMatrixT I3p_50_1;
    dMatrixT I3p_51_1;
    dMatrixT I3p_52_1;
    dMatrixT I3p_53_1;
    dMatrixT I3p_54_1;
    dMatrixT I3p_87_1;
    dMatrixT I3p_88_1;
    dMatrixT I3p_89_1;
    dMatrixT I3p_90_1;
    dMatrixT I3p_91_1;
    dMatrixT I3p_92_1;
    dMatrixT I3p_93_1;
    dMatrixT I3p_94_1;
    dMatrixT I3p_111_1;
    dMatrixT I3p_112_1;
    dMatrixT I3p_113_1;
    dMatrixT I3p_114_1;
    dMatrixT I3p_115_1;
    dMatrixT I3p_116_1;
    dMatrixT I3p_117_1;
    dMatrixT I3p_118_1;
    dMatrixT I3p_119_1;
    dMatrixT I3p_120_1;
    dMatrixT I3p_121_1;
    dMatrixT I3p_122_1;
    dMatrixT I3p_123_1;
    dMatrixT I3p_124_1;
    dMatrixT I3p_125_1;
    dMatrixT I3p_126_1;


    dMatrixT I4p_2_1;
    dMatrixT I4p_3_1;
    dMatrixT I4p_4_1;
    dMatrixT I4p_5_1;
    dMatrixT I4p_6_1;
    dMatrixT I4p_7_1;
    dMatrixT I4p_8_1;
    dMatrixT I4p_9_1;
    dMatrixT I4p_10_1;
    dMatrixT I4p_11_1;
    dMatrixT I4p_12_1;
    dMatrixT I4p_13_1;
    dMatrixT I4p_14_1;
    dMatrixT I4p_15_1;
    dMatrixT I4p_16_1;
    dMatrixT I4p_17_1;
    dMatrixT I4p_18_1;

    dMatrixT II2p_2_1;
    dMatrixT II2p_3_1;
    dMatrixT II2p_4_1;
    dMatrixT II2p_5_1;
    dMatrixT II2p_6_1;
    dMatrixT II2p_7_1;
    dMatrixT II2p_8_1;
    dMatrixT II2p_9_1;
    dMatrixT II2p_10_1;
    dMatrixT II2p_11_1;
    dMatrixT II2p_12_1;
    dMatrixT II2p_13_1;
    dMatrixT II2p_14_1;
    dMatrixT II2p_15_1;
    dMatrixT II2p_16_1;
    dMatrixT II2p_17_1;
    dMatrixT II3p_11_1;
    dMatrixT II3p_12_1;
    dMatrixT II3p_13_1;
    dMatrixT II3p_14_1;
    dMatrixT II3p_15_1;
    dMatrixT II3p_21_1;
    dMatrixT II3p_22_1;
    dMatrixT II3p_23_1;
    dMatrixT II3p_24_1;
    dMatrixT II3p_25_1;
    dMatrixT II3p_26_1;
    dMatrixT II3p_27_1;
    dMatrixT II3p_28_1;
    dMatrixT II3p_29_1;
    dMatrixT II3p_30_1;
    dMatrixT II3p_37_1;
    dMatrixT II3p_38_1;
    dMatrixT II3p_39_1;
    dMatrixT II3p_43_1;
    dMatrixT II3p_44_1;
    dMatrixT II3p_45_1;
    dMatrixT II3p_46_1;
    dMatrixT II3p_47_1;
    dMatrixT II3p_48_1;
    dMatrixT II3p_81_1;
    dMatrixT II3p_82_1;
    dMatrixT II3p_83_1;
    dMatrixT II3p_84_1;
    dMatrixT II3p_85_1;
    dMatrixT II3p_86_1;
    dMatrixT II3p_87_1;
    dMatrixT II3p_88_1;
    dMatrixT II3p_97_1;
    dMatrixT II3p_98_1;
    dMatrixT II3p_99_1;
    dMatrixT II3p_100_1;
    dMatrixT II3p_101_1;
    dMatrixT II3p_102_1;
    dMatrixT II3p_103_1;
    dMatrixT II3p_104_1;
    dMatrixT II3p_105_1;
    dMatrixT II3p_106_1;
    dMatrixT II3p_107_1;
    dMatrixT II3p_108_1;
    dMatrixT II3p_109_1;
    dMatrixT II3p_110_1;
    dMatrixT II3p_111_1;
    dMatrixT II3p_112_1;
    dMatrixT II4p_2_1;
    dMatrixT II4p_3_1;
    dMatrixT II4p_4_1;
    dMatrixT II4p_5_1;
    dMatrixT II4p_6_1;
    dMatrixT II4p_7_1;
    dMatrixT II4p_8_1;
    dMatrixT II4p_9_1;
    dMatrixT II4p_10_1;
    dMatrixT II4p_11_1;
    dMatrixT II4p_12_1;
    dMatrixT II4p_13_1;
    dMatrixT II4p_14_1;
    dMatrixT II4p_15_1;
    dMatrixT II4p_16_1;
    dMatrixT II4p_17_1;
    dMatrixT II7p_1_1;
    dMatrixT II7p_2_1;
    dMatrixT II7p_3_1;
    dMatrixT II7p_4_1;
    dMatrixT II7p_5_1;
    dMatrixT II7p_6_1;
    dMatrixT II7p_7_1;
    dMatrixT II7p_8_1;
    dMatrixT II7p_9_1;
    dMatrixT II7p_10_1;
    dMatrixT II7p_11_1;
    dMatrixT II7p_12_1;
    dMatrixT II7p_13_1;
    dMatrixT II7p_14_1;
    dMatrixT II7p_15_1;
    dMatrixT II7p_16_1;
    dMatrixT II8p_1_1;
    dMatrixT II8p_2_1;
    dMatrixT II8p_3_1;
    dMatrixT II8p_4_1;
    dMatrixT II8p_5_1;
    dMatrixT II8p_6_1;
    dMatrixT II8p_7_1;
    dMatrixT II8p_8_1;
    dMatrixT II8p_9_1;
    dMatrixT II8p_10_1;
    dMatrixT II8p_11_1;
    dMatrixT II8p_12_1;
    dMatrixT II8p_13_1;
    dMatrixT II8p_14_1;
    dMatrixT II8p_15_1;
    dMatrixT II8p_16_1;
    dMatrixT II9p_1_1;
    dMatrixT II9p_2_1;
    dMatrixT II9p_3_1;
    dMatrixT II9p_4_1;
    dMatrixT II9p_5_1;
    dMatrixT II9p_6_1;
    dMatrixT II9p_7_1;
    dMatrixT II9p_8_1;
    dMatrixT II9p_9_1;
    dMatrixT II9p_10_1;
    dMatrixT II9p_11_1;
    dMatrixT II9p_12_1;
    dMatrixT II9p_13_1;
    dMatrixT II9p_14_1;
    dMatrixT II9p_15_1;
    dMatrixT II9p_16_1;

    dMatrixT II10p_1_1;
    dMatrixT II10p_2_1;
    dMatrixT II10p_3_1;
    dMatrixT II10p_4_1;
    dMatrixT II10p_5_1;
    dMatrixT II10p_6_1;
    dMatrixT II10p_7_1;
    dMatrixT II10p_8_1;
    dMatrixT II10p_9_1;
    dMatrixT II10p_10_1;
    dMatrixT II10p_11_1;
    dMatrixT II10p_12_1;
    dMatrixT II10p_13_1;
    dMatrixT II10p_14_1;
    dMatrixT II10p_15_1;
    dMatrixT II10p_16_1;

    dMatrixT II14p_1_1;
    dMatrixT II14p_2_1;
    dMatrixT II14p_3_1;
    dMatrixT II14p_4_1;
    dMatrixT II14p_5_1;
    dMatrixT II14p_6_1;
    dMatrixT II14p_7_1;
    dMatrixT II14p_8_1;
    dMatrixT II14p_9_1;
    dMatrixT II14p_10_1;
    dMatrixT II14p_11_1;
    dMatrixT II14p_12_1;
    dMatrixT II14p_13_1;
    dMatrixT II14p_14_1;
    dMatrixT II14p_15_1;
    dMatrixT II14p_16_1;

    dTensor3DT Temp_II18p_1;
    dMatrixT II18p_1_1;
    dMatrixT II18p_2_1;
    dMatrixT II18p_3_1;
    dMatrixT II18p_4_1;
    dMatrixT II18p_5_1;
    dMatrixT II18p_6_1;
    dMatrixT II18p_7_1;
    dMatrixT II18p_8_1;
    dMatrixT II18p_9_1;
    dMatrixT II18p_10_1;
    dMatrixT II18p_11_1;
    dMatrixT II18p_12_1;
    dMatrixT II18p_13_1;
    dMatrixT II18p_14_1;
    dMatrixT II18p_15_1;
    dMatrixT II18p_16_1;





    dMatrixT I3e_1;
    dMatrixT fKu_I3e_1;
    dMatrixT I3e_2;
    dMatrixT fKu_I3e_2;
    dMatrixT I3e_3;
    dMatrixT fKu_I3e_3;
    dMatrixT I3p_4;
    dMatrixT fKu_I3p_4;
    dMatrixT I3p_5;
    dMatrixT fKu_I3p_5;
    dMatrixT I3p_6;
    dMatrixT fKu_I3p_6;
    dMatrixT I3p_7;
    dMatrixT fKu_I3p_7;
    dMatrixT I3p_8;
    dMatrixT fKu_I3p_8;
    dMatrixT I3p_9;
    dMatrixT fKu_I3p_9;
    dMatrixT I3p_10;
    dMatrixT fKu_I3p_10;
    dMatrixT I3p_11;
    dMatrixT fKu_I3p_11;
    dMatrixT I3p_12;
    dMatrixT fKu_I3p_12;
    dMatrixT I3e_13;
    dMatrixT fKu_I3e_13;
    dMatrixT I3e_14;
    dMatrixT fKu_I3e_14;
    dMatrixT I3e_15;
    dMatrixT fKu_I3e_15;
    dMatrixT I3p_16;
    dMatrixT fKu_I3p_16;
    dMatrixT I3p_17;
    dMatrixT fKu_I3p_17;
    dMatrixT I3p_18;
    dMatrixT fKu_I3p_18;
    dMatrixT I3p_19;
    dMatrixT fKu_I3p_19;
    dMatrixT I3p_20;
    dMatrixT fKu_I3p_20;
    dMatrixT I3p_21;
    dMatrixT fKu_I3p_21;
    dMatrixT I3p_22;
    dMatrixT fKu_I3p_22;
    dMatrixT I3p_23;
    dMatrixT fKu_I3p_23;
    dMatrixT I3p_24;
    dMatrixT fKu_I3p_24;
    dMatrixT I3p_25;
    dMatrixT fKu_I3p_25;
    dMatrixT I3p_26;
    dMatrixT fKu_I3p_26;
    dMatrixT I3p_27;
    dMatrixT fKu_I3p_27;
    dMatrixT I3p_28;
    dMatrixT fKu_I3p_28;
    dMatrixT I3p_29;
    dMatrixT fKu_I3p_29;
    dMatrixT I3p_30;
    dMatrixT fKu_I3p_30;
    dMatrixT I3p_31;
    dMatrixT fKu_I3p_31;
    dMatrixT I3p_32;
    dMatrixT fKu_I3p_32;
    dMatrixT I3p_33;
    dMatrixT fKu_I3p_33;
    dMatrixT I3p_34;
    dMatrixT fKu_I3p_34;
    dMatrixT I3p_35;
    dMatrixT fKu_I3p_35;
    dMatrixT I3p_36;
    dMatrixT fKu_I3p_36;
    dMatrixT I3e_37;
    dMatrixT fKuphi_I3e_37;
    dMatrixT I3e_38;
    dMatrixT fKuphi_I3e_38;
    dMatrixT I3e_39;
    dMatrixT fKuphi_I3e_39;
    dMatrixT I3p_40;
    dMatrixT fKuphi_I3p_40;
    dMatrixT I3p_41;
    dMatrixT fKuphi_I3p_41;
    dMatrixT I3p_42;
    dMatrixT fKuphi_I3p_42;
    dMatrixT I3p_43;
    dMatrixT fKuphi_I3p_43;
    dMatrixT I3p_44;
    dMatrixT fKuphi_I3p_44;
    dMatrixT I3p_45;
    dMatrixT fKuphi_I3p_45;
    dMatrixT I3p_46;
    dMatrixT fKuphi_I3p_46;
    dMatrixT I3p_47;
    dMatrixT fKuphi_I3p_47;
    dMatrixT I3p_48;
    dMatrixT fKuphi_I3p_48;
    dMatrixT I3p_49;
    dMatrixT fKuphi_I3p_49;
    dMatrixT I3p_50;
    dMatrixT fKuphi_I3p_50;
    dMatrixT I3p_51;
    dMatrixT fKuphi_I3p_51;
    dMatrixT I3p_52;
    dMatrixT fKuphi_I3p_52;
    dMatrixT I3p_53;
    dMatrixT fKuphi_I3p_53;
    dMatrixT I3p_54;
    dMatrixT fKuphi_I3p_54;
    /* Matrices from Del(delgammachi) */
    dMatrixT I3p_55;
    dMatrixT fKu_I3p_55;
    dMatrixT I3p_56;
    dMatrixT fKu_I3p_56;
    dMatrixT I3p_57;
    dMatrixT fKu_I3p_57;
    dMatrixT I3p_58;
    dMatrixT fKuphi_I3p_58;
    dMatrixT I3p_59;
    dMatrixT fKu_I3p_59;
    dMatrixT I3p_60;
    dMatrixT fKuphi_I3p_60;
    dMatrixT I3p_61;
    dMatrixT fKu_I3p_61;
    dMatrixT I3p_62;
    dMatrixT fKuphi_I3p_62;
    dMatrixT I3p_63;
    dMatrixT fKu_I3p_63;
    dMatrixT I3p_64;
    dMatrixT fKu_I3p_64;
    dMatrixT I3p_65;
    dMatrixT fKu_I3p_65;
    dMatrixT I3p_66;
    dMatrixT fKuphi_I3p_66;
    dMatrixT I3p_67;
    dMatrixT fKu_I3p_67;
    dMatrixT I3p_68;
    dMatrixT fKuphi_I3p_68;
    dMatrixT I3p_69;
    dMatrixT fKu_I3p_69;
    dMatrixT I3p_70;
    dMatrixT fKuphi_I3p_70;
    dMatrixT I3p_71;
    dMatrixT fKu_I3p_71;
    dMatrixT I3p_72;
    dMatrixT fKu_I3p_72;
    dMatrixT I3p_73;
    dMatrixT fKu_I3p_73;
    dMatrixT I3p_74;
    dMatrixT fKuphi_I3p_74;
    dMatrixT I3p_75;
    dMatrixT fKu_I3p_75;
    dMatrixT I3p_76;
    dMatrixT fKuphi_I3p_76;
    dMatrixT I3p_77;
    dMatrixT fKu_I3p_77;
    dMatrixT I3p_78;
    dMatrixT fKuphi_I3p_78;
    /* Matrices from coupling*/
    dMatrixT I3p_79;
    dMatrixT fKu_I3p_79;
    dMatrixT I3p_80;
    dMatrixT fKu_I3p_80;
    dMatrixT I3p_81;
    dMatrixT fKu_I3p_81;
    dMatrixT I3p_82;
    dMatrixT fKuphi_I3p_82;
    dMatrixT I3p_83;
    dMatrixT fKu_I3p_83;
    dMatrixT I3p_84;
    dMatrixT fKuphi_I3p_84;
    dMatrixT I3p_85;
    dMatrixT fKu_I3p_85;
    dMatrixT I3p_86;
    dMatrixT fKuphi_I3p_86;
    dMatrixT I3p_87;
    dMatrixT fKu_I3p_87;
    dMatrixT I3p_88;
    dMatrixT fKu_I3p_88;
    dMatrixT I3p_89;
    dMatrixT fKu_I3p_89;
    dMatrixT I3p_90;
    dMatrixT fKuphi_I3p_90;
    dMatrixT I3p_91;
    dMatrixT fKu_I3p_91;
    dMatrixT I3p_92;
    dMatrixT fKuphi_I3p_92;
    dMatrixT I3p_93;
    dMatrixT fKu_I3p_93;
    dMatrixT I3p_94;
    dMatrixT fKuphi_I3p_94;
    dMatrixT I3p_95;
    dMatrixT fKu_I3p_95;
    dMatrixT I3p_96;
    dMatrixT fKu_I3p_96;
    dMatrixT I3p_97;
    dMatrixT fKu_I3p_97;
    dMatrixT I3p_98;
    dMatrixT fKuphi_I3p_98;
    dMatrixT I3p_99;
    dMatrixT fKu_I3p_99;
    dMatrixT I3p_100;
    dMatrixT fKuphi_I3p_100;
    dMatrixT I3p_101;
    dMatrixT fKu_I3p_101;
    dMatrixT I3p_102;
    dMatrixT fKuphi_I3p_102;
    dMatrixT I3p_103;
    dMatrixT fKu_I3p_103;
    dMatrixT I3p_104;
    dMatrixT fKu_I3p_104;
    dMatrixT I3p_105;
    dMatrixT fKu_I3p_105;
    dMatrixT I3p_106;
    dMatrixT fKuphi_I3p_106;
    dMatrixT I3p_107;
    dMatrixT fKu_I3p_107;
    dMatrixT I3p_108;
    dMatrixT fKuphi_I3p_108;
    dMatrixT I3p_109;
    dMatrixT fKu_I3p_109;
    dMatrixT I3p_110;
    dMatrixT fKuphi_I3p_110;
    dMatrixT I3p_111;
    dMatrixT fKu_I3p_111;
    dMatrixT I3p_112;
    dMatrixT fKu_I3p_112;
    dMatrixT I3p_113;
    dMatrixT fKu_I3p_113;
    dMatrixT I3p_114;
    dMatrixT fKuphi_I3p_114;
    dMatrixT I3p_115;
    dMatrixT fKu_I3p_115;
    dMatrixT I3p_116;
    dMatrixT fKuphi_I3p_116;
    dMatrixT I3p_117;
    dMatrixT fKu_I3p_117;
    dMatrixT I3p_118;
    dMatrixT fKuphi_I3p_118;
    dMatrixT I3p_119;
    dMatrixT fKu_I3p_119;
    dMatrixT I3p_120;
    dMatrixT fKu_I3p_120;
    dMatrixT I3p_121;
    dMatrixT fKu_I3p_121;
    dMatrixT I3p_122;
    dMatrixT fKuphi_I3p_122;
    dMatrixT I3p_123;
    dMatrixT fKu_I3p_123;
    dMatrixT I3p_124;
    dMatrixT fKuphi_I3p_124;
    dMatrixT I3p_125;
    dMatrixT fKu_I3p_125;
    dMatrixT I3p_126;
    dMatrixT fKuphi_I3p_126;
    dMatrixT I3p_127;
    dMatrixT fKu_I3p_127;
    dMatrixT I3p_128;
    dMatrixT fKu_I3p_128;
    dMatrixT I3p_129;
    dMatrixT fKu_I3p_129;
    dMatrixT I3p_130;
    dMatrixT fKuphi_I3p_130;
    dMatrixT I3p_131;
    dMatrixT fKu_I3p_131;
    dMatrixT I3p_132;
    dMatrixT fKuphi_I3p_132;
    dMatrixT I3p_133;
    dMatrixT fKu_I3p_133;
    dMatrixT I3p_134;
    dMatrixT fKuphi_I3p_134;
    dMatrixT I3p_135;
    dMatrixT fKu_I3p_135;
    dMatrixT I3p_136;
    dMatrixT fKu_I3p_136;
    dMatrixT I3p_137;
    dMatrixT fKu_I3p_137;
    dMatrixT I3p_138;
    dMatrixT fKuphi_I3p_138;
    dMatrixT I3p_139;
    dMatrixT fKu_I3p_139;
    dMatrixT I3p_140;
    dMatrixT fKuphi_I3p_140;
    dMatrixT I3p_141;
    dMatrixT fKu_I3p_141;
    dMatrixT I3p_142;
    dMatrixT fKuphi_I3p_142;



    dMatrixT I4e_1;
    dMatrixT fKu_I4e_1;
    dMatrixT I4p_2;
    dMatrixT fKu_I4p_2;
    dMatrixT I4p_3;
    dMatrixT fKu_I4p_3;
    dMatrixT I4p_4;
    dMatrixT fKu_I4p_4;
    dMatrixT I4p_5;
    dMatrixT fKu_I4p_5;
    dMatrixT I4p_6;
    dMatrixT fKu_I4p_6;
    dMatrixT I4p_7;
    dMatrixT fKu_I4p_7;
    dMatrixT I4p_8;
    dMatrixT fKuphi_I4p_8;
    dMatrixT I4p_9;
    dMatrixT fKuphi_I4p_9;
    dMatrixT I4p_10;
    dMatrixT fKuphi_I4p_10;
    /* Matrices from coupling*/
    dMatrixT I4p_11;
    dMatrixT fKu_I4p_11;
    dMatrixT I4p_12;
    dMatrixT fKu_I4p_12;
    dMatrixT I4p_13;
    dMatrixT fKu_I4p_13;
    dMatrixT I4p_14;
    dMatrixT fKuphi_I4p_14;
    dMatrixT I4p_15;
    dMatrixT fKu_I4p_15;
    dMatrixT I4p_16;
    dMatrixT fKuphi_I4p_16;
    dMatrixT I4p_17;
    dMatrixT fKu_I4p_17;
    dMatrixT I4p_18;
    dMatrixT fKuphi_I4p_18;

    dMatrixT IIJp_1;
    dMatrixT fKphiu_IIJp_1;
    dMatrixT IIJp_2;
    dMatrixT fKphiu_IIJp_2;
    dMatrixT IIJp_3;
    dMatrixT fKphiu_IIJp_3;
    dMatrixT IIJp_4;
    dMatrixT fKphiu_IIJp_4;
    dMatrixT IIJp_5;
    dMatrixT fKphiu_IIJp_5;
    dMatrixT IIJp_6;
    dMatrixT fKphiphi_IIJp_6;
    dMatrixT IIJp_7;
    dMatrixT fKphiphi_IIJp_7;
    dMatrixT IIJp_8;
    dMatrixT fKphiphi_IIJp_8;
    /* Matrices from coupling*/
    dMatrixT IIJp_9;
    dMatrixT fKphiu_IIJp_9;
    dMatrixT IIJp_10;
    dMatrixT fKphiu_IIJp_10;
    dMatrixT IIJp_11;
    dMatrixT fKphiu_IIJp_11;
    dMatrixT IIJp_12;
    dMatrixT fKphiphi_IIJp_12;
    dMatrixT IIJp_13;
    dMatrixT fKphiu_IIJp_13;
    dMatrixT IIJp_14;
    dMatrixT fKphiphi_IIJp_14;
    dMatrixT IIJp_15;
    dMatrixT fKphiu_IIJp_15;
    dMatrixT IIJp_16;
    dMatrixT fKphiphi_IIJp_16;

    dMatrixT II2e_1;
    dMatrixT fKphiu_II2e_1;
    dMatrixT II2p_2;
    dMatrixT fKphiu_II2p_2;
    dMatrixT II2p_3;
    dMatrixT fKphiu_II2p_3;
    dMatrixT II2p_4;
    dMatrixT fKphiu_II2p_4;
    dMatrixT II2p_5;
    dMatrixT fKphiu_II2p_5;
    dMatrixT II2p_6;
    dMatrixT fKphiu_II2p_6;
    dMatrixT II2p_7;
    dMatrixT fKphiphi_II2p_7;
    dMatrixT II2p_8;
    dMatrixT fKphiphi_II2p_8;
    dMatrixT II2p_9;
    dMatrixT fKphiphi_II2p_9;
    /* Matrices from coupling*/
    dMatrixT II2p_10;
    dMatrixT fKphiu_II2p_10;
    dMatrixT II2p_11;
    dMatrixT fKphiu_II2p_11;
    dMatrixT II2p_12;
    dMatrixT fKphiu_II2p_12;
    dMatrixT II2p_13;
    dMatrixT fKphiphi_II2p_13;
    dMatrixT II2p_14;
    dMatrixT fKphiu_II2p_14;
    dMatrixT II2p_15;
    dMatrixT fKphiphi_II2p_15;
    dMatrixT II2p_16;
    dMatrixT fKphiu_II2p_16;
    dMatrixT II2p_17;
    dMatrixT fKphiphi_II2p_17;



    dMatrixT II3e_1;
    dMatrixT fKphiu_II3e_1;
    dMatrixT II3e_2;
    dMatrixT fKphiu_II3e_2;
    dMatrixT II3e_3;
    dMatrixT fKphiu_II3e_3;
    dMatrixT II3e_4;
    dMatrixT fKphiu_II3e_4;
    dMatrixT II3e_5;
    dMatrixT fKphiu_II3e_5;
    dMatrixT II3p_6;
    dMatrixT fKphiu_II3p_6;
    dMatrixT II3p_7;
    dMatrixT fKphiu_II3p_7;
    dMatrixT II3p_8;
    dMatrixT fKphiu_II3p_8;
    dMatrixT II3p_9;
    dMatrixT fKphiu_II3p_9;
    dMatrixT II3p_10;
    dMatrixT fKphiu_II3p_10;
    dMatrixT II3p_11;
    dMatrixT fKphiu_II3p_11;
    dMatrixT II3p_12;
    dMatrixT fKphiu_II3p_12;
    dMatrixT II3p_13;
    dMatrixT fKphiu_II3p_13;
    dMatrixT II3p_14;
    dMatrixT fKphiu_II3p_14;
    dMatrixT II3p_15;
    dMatrixT fKphiu_II3p_15;
    dMatrixT II3p_16;
    dMatrixT fKphiu_II3p_16;
    dMatrixT II3p_17;
    dMatrixT fKphiu_II3p_17;
    dMatrixT II3p_18;
    dMatrixT fKphiu_II3p_18;
    dMatrixT II3p_19;
    dMatrixT fKphiu_II3p_19;
    dMatrixT II3p_20;
    dMatrixT fKphiu_II3p_20;
    dMatrixT II3p_21;
    dMatrixT fKphiu_II3p_21;
    dMatrixT II3p_22;
    dMatrixT fKphiu_II3p_22;
    dMatrixT II3p_23;
    dMatrixT fKphiu_II3p_23;
    dMatrixT II3p_24;
    dMatrixT fKphiu_II3p_24;
    dMatrixT II3p_25;
    dMatrixT fKphiu_II3p_25;
    dMatrixT II3p_26;
    dMatrixT fKphiu_II3p_26;
    dMatrixT II3p_27;
    dMatrixT fKphiu_II3p_27;
    dMatrixT II3p_28;
    dMatrixT fKphiu_II3p_28;
    dMatrixT II3p_29;
    dMatrixT fKphiu_II3p_29;
    dMatrixT II3p_30;
    dMatrixT fKphiu_II3p_30;
    dMatrixT II3e_31;
    dMatrixT fKphiphi_II3e_31;
    dMatrixT II3e_32;
    dMatrixT fKphiphi_II3e_32;
    dMatrixT II3e_33;
    dMatrixT fKphiphi_II3e_33;
    dMatrixT II3p_34;
    dMatrixT fKphiphi_II3p_34;
    dMatrixT II3p_35;
    dMatrixT fKphiphi_II3p_35;
    dMatrixT II3p_36;
    dMatrixT fKphiphi_II3p_36;
    dMatrixT II3p_37;
    dMatrixT fKphiphi_II3p_37;
    dMatrixT II3p_38;
    dMatrixT fKphiphi_II3p_38;
    dMatrixT II3p_39;
    dMatrixT fKphiphi_II3p_39;
    dMatrixT II3p_40;
    dMatrixT fKphiphi_II3p_40;
    dMatrixT II3p_41;
    dMatrixT fKphiphi_II3p_41;
    dMatrixT II3p_42;
    dMatrixT fKphiphi_II3p_42;
    dMatrixT II3p_43;
    dMatrixT fKphiphi_II3p_43;
    dMatrixT II3p_44;
    dMatrixT fKphiphi_II3p_44;
    dMatrixT II3p_45;
    dMatrixT fKphiphi_II3p_45;
    dMatrixT II3p_46;
    dMatrixT fKphiphi_II3p_46;
    dMatrixT II3p_47;
    dMatrixT fKphiphi_II3p_47;
    dMatrixT II3p_48;
    dMatrixT fKphiphi_II3p_48;
    /* Matrices from Del(delgammachi) */
    dMatrixT II3p_49;
    dMatrixT fKphiu_II3p_49;
    dMatrixT II3p_50;
    dMatrixT fKphiu_II3p_50;
    dMatrixT II3p_51;
    dMatrixT fKphiu_II3p_51;
    dMatrixT II3p_52;
    dMatrixT fKphiphi_II3p_52;
    dMatrixT II3p_53;
    dMatrixT fKphiu_II3p_53;
    dMatrixT II3p_54;
    dMatrixT fKphiphi_II3p_54;
    dMatrixT II3p_55;
    dMatrixT fKphiu_II3p_55;
    dMatrixT II3p_56;
    dMatrixT fKphiphi_II3p_56;
    dMatrixT II3p_57;
    dMatrixT fKphiu_II3p_57;
    dMatrixT II3p_58;
    dMatrixT fKphiu_II3p_58;
    dMatrixT II3p_59;
    dMatrixT fKphiu_II3p_59;
    dMatrixT II3p_60;
    dMatrixT fKphiphi_II3p_60;
    dMatrixT II3p_61;
    dMatrixT fKphiu_II3p_61;
    dMatrixT II3p_62;
    dMatrixT fKphiphi_II3p_62;
    dMatrixT II3p_63;
    dMatrixT fKphiu_II3p_63;
    dMatrixT II3p_64;
    dMatrixT fKphiphi_II3p_64;
    dMatrixT II3p_65;
    dMatrixT fKphiu_II3p_65;
    dMatrixT II3p_66;
    dMatrixT fKphiu_II3p_66;
    dMatrixT II3p_67;
    dMatrixT fKphiu_II3p_67;
    dMatrixT II3p_68;
    dMatrixT fKphiphi_II3p_68;
    dMatrixT II3p_69;
    dMatrixT fKphiu_II3p_69;
    dMatrixT II3p_70;
    dMatrixT fKphiphi_II3p_70;
    dMatrixT II3p_71;
    dMatrixT fKphiu_II3p_71;
    dMatrixT II3p_72;
    dMatrixT fKphiphi_II3p_72;
    /* Matrices from coupling */
    dMatrixT II3p_73;
    dMatrixT fKphiu_II3p_73;
    dMatrixT II3p_74;
    dMatrixT fKphiu_II3p_74;
    dMatrixT II3p_75;
    dMatrixT fKphiu_II3p_75;
    dMatrixT II3p_76;
    dMatrixT fKphiphi_II3p_76;
    dMatrixT II3p_77;
    dMatrixT fKphiu_II3p_77;
    dMatrixT II3p_78;
    dMatrixT fKphiphi_II3p_78;
    dMatrixT II3p_79;
    dMatrixT fKphiu_II3p_79;
    dMatrixT II3p_80;
    dMatrixT fKphiphi_II3p_80;
    dMatrixT II3p_81;
    dMatrixT fKphiu_II3p_81;
    dMatrixT II3p_82;
    dMatrixT fKphiu_II3p_82;
    dMatrixT II3p_83;
    dMatrixT fKphiu_II3p_83;
    dMatrixT II3p_84;
    dMatrixT fKphiphi_II3p_84;
    dMatrixT II3p_85;
    dMatrixT fKphiu_II3p_85;
    dMatrixT II3p_86;
    dMatrixT fKphiphi_II3p_86;
    dMatrixT II3p_87;
    dMatrixT fKphiu_II3p_87;
    dMatrixT II3p_88;
    dMatrixT fKphiphi_II3p_88;
    dMatrixT II3p_89;
    dMatrixT fKphiu_II3p_89;
    dMatrixT II3p_90;
    dMatrixT fKphiu_II3p_90;
    dMatrixT II3p_91;
    dMatrixT fKphiu_II3p_91;
    dMatrixT II3p_92;
    dMatrixT fKphiphi_II3p_92;
    dMatrixT II3p_93;
    dMatrixT fKphiu_II3p_93;
    dMatrixT II3p_94;
    dMatrixT fKphiphi_II3p_94;
    dMatrixT II3p_95;
    dMatrixT fKphiu_II3p_95;
    dMatrixT II3p_96;
    dMatrixT fKphiphi_II3p_96;
    dMatrixT II3p_97;
    dMatrixT fKphiu_II3p_97;
    dMatrixT II3p_98;
    dMatrixT fKphiu_II3p_98;
    dMatrixT II3p_99;
    dMatrixT fKphiu_II3p_99;
    dMatrixT II3p_100;
    dMatrixT fKphiphi_II3p_100;
    dMatrixT II3p_101;
    dMatrixT fKphiu_II3p_101;
    dMatrixT II3p_102;
    dMatrixT fKphiphi_II3p_102;
    dMatrixT II3p_103;
    dMatrixT fKphiu_II3p_103;
    dMatrixT II3p_104;
    dMatrixT fKphiphi_II3p_104;
    dMatrixT II3p_105;
    dMatrixT fKphiu_II3p_105;
    dMatrixT II3p_106;
    dMatrixT fKphiu_II3p_106;
    dMatrixT II3p_107;
    dMatrixT fKphiu_II3p_107;
    dMatrixT II3p_108;
    dMatrixT fKphiphi_II3p_108;
    dMatrixT II3p_109;
    dMatrixT fKphiu_II3p_109;
    dMatrixT II3p_110;
    dMatrixT fKphiphi_II3p_110;
    dMatrixT II3p_111;
    dMatrixT fKphiu_II3p_111;
    dMatrixT II3p_112;
    dMatrixT fKphiphi_II3p_112;
    dMatrixT II3p_113;
    dMatrixT fKphiu_II3p_113;
    dMatrixT II3p_114;
    dMatrixT fKphiu_II3p_114;
    dMatrixT II3p_115;
    dMatrixT fKphiu_II3p_115;
    dMatrixT II3p_116;
    dMatrixT fKphiphi_II3p_116;
    dMatrixT II3p_117;
    dMatrixT fKphiu_II3p_117;
    dMatrixT II3p_118;
    dMatrixT fKphiphi_II3p_118;
    dMatrixT II3p_119;
    dMatrixT fKphiu_II3p_119;
    dMatrixT II3p_120;
    dMatrixT fKphiphi_II3p_120;
    dMatrixT II3p_121;
    dMatrixT fKphiu_II3p_121;
    dMatrixT II3p_122;
    dMatrixT fKphiu_II3p_122;
    dMatrixT II3p_123;
    dMatrixT fKphiu_II3p_123;
    dMatrixT II3p_124;
    dMatrixT fKphiphi_II3p_124;
    dMatrixT II3p_125;
    dMatrixT fKphiu_II3p_125;
    dMatrixT II3p_126;
    dMatrixT fKphiphi_II3p_126;
    dMatrixT II3p_127;
    dMatrixT fKphiu_II3p_127;
    dMatrixT II3p_128;
    dMatrixT fKphiphi_II3p_128;
    dMatrixT II3p_129;
    dMatrixT fKphiu_II3p_129;
    dMatrixT II3p_130;
    dMatrixT fKphiu_II3p_130;
    dMatrixT II3p_131;
    dMatrixT fKphiu_II3p_131;
    dMatrixT II3p_132;
    dMatrixT fKphiphi_II3p_132;
    dMatrixT II3p_133;
    dMatrixT fKphiu_II3p_133;
    dMatrixT II3p_134;
    dMatrixT fKphiphi_II3p_134;
    dMatrixT II3p_135;
    dMatrixT fKphiu_II3p_135;
    dMatrixT II3p_136;
    dMatrixT fKphiphi_II3p_136;





    dMatrixT II4e_1;
    dMatrixT fKphiu_II4e_1;
    dMatrixT II4p_2;
    dMatrixT fKphiu_II4p_2;
    dMatrixT II4p_3;
    dMatrixT fKphiu_II4p_3;
    dMatrixT II4p_4;
    dMatrixT fKphiu_II4p_4;
    dMatrixT II4p_5;
    dMatrixT fKphiu_II4p_5;
    dMatrixT II4p_6;
    dMatrixT fKphiu_II4p_6;
    dMatrixT II4p_7;
    dMatrixT fKphiphi_II4p_7;
    dMatrixT II4p_8;
    dMatrixT fKphiphi_II4p_8;
    dMatrixT II4p_9;
    dMatrixT fKphiphi_II4p_9;
    /* Matrices from coupling*/
    dMatrixT II4p_10;
    dMatrixT fKphiu_II4p_10;
    dMatrixT II4p_11;
    dMatrixT fKphiu_II4p_11;
    dMatrixT II4p_12;
    dMatrixT fKphiu_II4p_12;
    dMatrixT II4p_13;
    dMatrixT fKphiphi_II4p_13;
    dMatrixT II4p_14;
    dMatrixT fKphiu_II4p_14;
    dMatrixT II4p_15;
    dMatrixT fKphiphi_II4p_15;
    dMatrixT II4p_16;
    dMatrixT fKphiu_II4p_16;
    dMatrixT II4p_17;
    dMatrixT fKphiphi_II4p_17;

    /* Matrices from higher order couple stress tensor*/
    dMatrixT II5Jp_1;
    dMatrixT fKMphiu_II5Jp_1;
    dMatrixT II5Jp_2;
    dMatrixT fKMphiu_II5Jp_2;
    dMatrixT II5Jp_3;
    dMatrixT fKMphiu_II5Jp_3;
    dMatrixT II5Jp_4;
    dMatrixT fKMphiu_II5Jp_4;
    dMatrixT II5Jp_5;
    dMatrixT fKMphiu_II5Jp_5;
    dMatrixT II5Jp_6;
    dMatrixT fKMphiphi_II5Jp_6;
    dMatrixT II5Jp_7;
    dMatrixT fKMphiphi_II5Jp_7;
    dMatrixT II5Jp_8;
    dMatrixT fKMphiphi_II5Jp_8;
    dMatrixT II5Jp_9;
    dMatrixT fKMphiu_II5Jp_9;
    dMatrixT II5Jp_10;
    dMatrixT fKMphiu_II5Jp_10;
    dMatrixT II5Jp_11;
    dMatrixT fKMphiu_II5Jp_11;
    dMatrixT II5Jp_12;
    dMatrixT fKMphiphi_II5Jp_12;
    dMatrixT II5Jp_13;
    dMatrixT fKMphiu_II5Jp_13;
    dMatrixT II5Jp_14;
    dMatrixT fKMphiphi_II5Jp_14;
    dMatrixT II5Jp_15;
    dMatrixT fKMphiu_II5Jp_15;
    dMatrixT II5Jp_16;
    dMatrixT fKMphiphi_II5Jp_16;

    dMatrixT II6e_1;
    dMatrixT fKMphiu_II6e_1;

    dMatrixT II7e_1;
    dMatrixT fKMphiu_II7e_1;

    dMatrixT II7p_1;
    dMatrixT fKMphiu_II7p_1;
    dMatrixT II7p_2;
    dMatrixT fKMphiu_II7p_2;
    dMatrixT II7p_3;
    dMatrixT fKMphiu_II7p_3;
    dMatrixT II7p_4;
    dMatrixT fKMphiu_II7p_4;
    dMatrixT II7p_5;
    dMatrixT fKMphiu_II7p_5;
    dMatrixT II7p_6;
    dMatrixT fKMphiphi_II7p_6;
    dMatrixT II7p_7;
    dMatrixT fKMphiphi_II7p_7;
    dMatrixT II7p_8;
    dMatrixT fKMphiphi_II7p_8;
    dMatrixT II7p_9;
    dMatrixT fKMphiu_II7p_9;
    dMatrixT II7p_10;
    dMatrixT fKMphiu_II7p_10;
    dMatrixT II7p_11;
    dMatrixT fKMphiu_II7p_11;
    dMatrixT II7p_12;
    dMatrixT fKMphiphi_II7p_12;
    dMatrixT II7p_13;
    dMatrixT fKMphiu_II7p_13;
    dMatrixT II7p_14;
    dMatrixT fKMphiphi_II7p_14;
    dMatrixT II7p_15;
    dMatrixT fKMphiu_II7p_15;
    dMatrixT II7p_16;
    dMatrixT fKMphiphi_II7p_16;


    dMatrixT II8e_1;
    dMatrixT fKMphiu_II8e_1;

    dMatrixT II8p_1;
    dMatrixT fKMphiu_II8p_1;
    dMatrixT II8p_2;
    dMatrixT fKMphiu_II8p_2;
    dMatrixT II8p_3;
    dMatrixT fKMphiu_II8p_3;
    dMatrixT II8p_4;
    dMatrixT fKMphiu_II8p_4;
    dMatrixT II8p_5;
    dMatrixT fKMphiu_II8p_5;
    dMatrixT II8p_6;
    dMatrixT fKMphiphi_II8p_6;
    dMatrixT II8p_7;
    dMatrixT fKMphiphi_II8p_7;
    dMatrixT II8p_8;
    dMatrixT fKMphiphi_II8p_8;
    dMatrixT II8p_9;
    dMatrixT fKMphiu_II8p_9;
    dMatrixT II8p_10;
    dMatrixT fKMphiu_II8p_10;
    dMatrixT II8p_11;
    dMatrixT fKMphiu_II8p_11;
    dMatrixT II8p_12;
    dMatrixT fKMphiphi_II8p_12;
    dMatrixT II8p_13;
    dMatrixT fKMphiu_II8p_13;
    dMatrixT II8p_14;
    dMatrixT fKMphiphi_II8p_14;
    dMatrixT II8p_15;
    dMatrixT fKMphiu_II8p_15;
    dMatrixT II8p_16;
    dMatrixT fKMphiphi_II8p_16;

    dMatrixT II9e_1;
    dMatrixT fKMphiu_II9e_1;

    dMatrixT II9p_1;
    dMatrixT fKMphiu_II9p_1;
    dMatrixT II9p_2;
    dMatrixT fKMphiu_II9p_2;
    dMatrixT II9p_3;
    dMatrixT fKMphiu_II9p_3;
    dMatrixT II9p_4;
    dMatrixT fKMphiu_II9p_4;
    dMatrixT II9p_5;
    dMatrixT fKMphiu_II9p_5;
    dMatrixT II9p_6;
    dMatrixT fKMphiphi_II9p_6;
    dMatrixT II9p_7;
    dMatrixT fKMphiphi_II9p_7;
    dMatrixT II9p_8;
    dMatrixT fKMphiphi_II9p_8;
    dMatrixT II9p_9;
    dMatrixT fKMphiu_II9p_9;
    dMatrixT II9p_10;
    dMatrixT fKMphiu_II9p_10;
    dMatrixT II9p_11;
    dMatrixT fKMphiu_II9p_11;
    dMatrixT II9p_12;
    dMatrixT fKMphiphi_II9p_12;
    dMatrixT II9p_13;
    dMatrixT fKMphiu_II9p_13;
    dMatrixT II9p_14;
    dMatrixT fKMphiphi_II9p_14;
    dMatrixT II9p_15;
    dMatrixT fKMphiu_II9p_15;
    dMatrixT II9p_16;
    dMatrixT fKMphiphi_II9p_16;


    dMatrixT II10e_1;
    dMatrixT fKMphiphi_II10e_1;

    dMatrixT II10p_1;
    dMatrixT fKMphiu_II10p_1;
    dMatrixT II10p_2;
    dMatrixT fKMphiu_II10p_2;
    dMatrixT II10p_3;
    dMatrixT fKMphiu_II10p_3;
    dMatrixT II10p_4;
    dMatrixT fKMphiu_II10p_4;
    dMatrixT II10p_5;
    dMatrixT fKMphiu_II10p_5;
    dMatrixT II10p_6;
    dMatrixT fKMphiphi_II10p_6;
    dMatrixT II10p_7;
    dMatrixT fKMphiphi_II10p_7;
    dMatrixT II10p_8;
    dMatrixT fKMphiphi_II10p_8;
    dMatrixT II10p_9;
    dMatrixT fKMphiu_II10p_9;
    dMatrixT II10p_10;
    dMatrixT fKMphiu_II10p_10;
    dMatrixT II10p_11;
    dMatrixT fKMphiu_II10p_11;
    dMatrixT II10p_12;
    dMatrixT fKMphiphi_II10p_12;
    dMatrixT II10p_13;
    dMatrixT fKMphiu_II10p_13;
    dMatrixT II10p_14;
    dMatrixT fKMphiphi_II10p_14;
    dMatrixT II10p_15;
    dMatrixT fKMphiu_II10p_15;
    dMatrixT II10p_16;
    dMatrixT fKMphiphi_II10p_16;

    dMatrixT II11p_1;
    dMatrixT fKMphiu_II11p_1;
    dMatrixT II11p_2;
    dMatrixT fKMphiu_II11p_2;
    dMatrixT II11p_3;
    dMatrixT fKMphiu_II11p_3;
    dMatrixT II11p_4;
    dMatrixT fKMphiu_II11p_4;
    dMatrixT II11p_5;
    dMatrixT fKMphiu_II11p_5;
    dMatrixT II11p_6;
    dMatrixT fKMphiphi_II11p_6;
    dMatrixT II11p_7;
    dMatrixT fKMphiphi_II11p_7;
    dMatrixT II11p_8;
    dMatrixT fKMphiphi_II11p_8;
    dMatrixT II11p_9;
    dMatrixT fKMphiu_II11p_9;
    dMatrixT II11p_10;
    dMatrixT fKMphiu_II11p_10;
    dMatrixT II11p_11;
    dMatrixT fKMphiu_II11p_11;
    dMatrixT II11p_12;
    dMatrixT fKMphiphi_II11p_12;
    dMatrixT II11p_13;
    dMatrixT fKMphiu_II11p_13;
    dMatrixT II11p_14;
    dMatrixT fKMphiphi_II11p_14;
    dMatrixT II11p_15;
    dMatrixT fKMphiu_II11p_15;
    dMatrixT II11p_16;
    dMatrixT fKMphiphi_II11p_16;


    dMatrixT II12e_1;
    dMatrixT fKMphiphi_II12e_1;

    dMatrixT II12p_1;
    dMatrixT fKMphiu_II12p_1;
    dMatrixT II12p_2;
    dMatrixT fKMphiu_II12p_2;
    dMatrixT II12p_3;
    dMatrixT fKMphiu_II12p_3;
    dMatrixT II12p_4;
    dMatrixT fKMphiu_II12p_4;
    dMatrixT II12p_5;
    dMatrixT fKMphiu_II12p_5;
    dMatrixT II12p_6;
    dMatrixT fKMphiphi_II12p_6;
    dMatrixT II12p_7;
    dMatrixT fKMphiphi_II12p_7;
    dMatrixT II12p_8;
    dMatrixT fKMphiphi_II12p_8;
    dMatrixT II12p_9;
    dMatrixT fKMphiu_II12p_9;
    dMatrixT II12p_10;
    dMatrixT fKMphiu_II12p_10;
    dMatrixT II12p_11;
    dMatrixT fKMphiu_II12p_11;
    dMatrixT II12p_12;
    dMatrixT fKMphiphi_II12p_12;
    dMatrixT II12p_13;
    dMatrixT fKMphiu_II12p_13;
    dMatrixT II12p_14;
    dMatrixT fKMphiphi_II12p_14;
    dMatrixT II12p_15;
    dMatrixT fKMphiu_II12p_15;
    dMatrixT II12p_16;
    dMatrixT fKMphiphi_II12p_16;



    dMatrixT dFedDelgamma;


    dArray2DT   fState_variables_IPs;
    dArray2DT   fState_variables_Elements_IPs;
    dArray2DT   fState_variables_n_IPs;
    dArray2DT   fState_variables_n_Elements_IPs;

    dArray2DT   fFp_IPs;
    dArray2DT   fFp_Elements_IPs;
    dArray2DT   fFp_n_IPs;
    dArray2DT   fFp_n_Elements_IPs;

    //for implicit local iteration
    dMatrixT    fCe;
    dMatrixT    fCe_inverse;
    //
    dArray2DT   fCe_IPs;
    dArray2DT   fCe_Elements_IPs;

    dMatrixT    fCe_n;
    dMatrixT    fCe_n_inverse;
    dMatrixT    fdGdS_n_transpose;
    dArray2DT   fCe_n_IPs;
    dArray2DT   fCe_n_Elements_IPs;

    //for implicit local iteration
    dMatrixT    fdGdS_transpose;
    dMatrixT 	flocalnewTangent;
    dMatrixT    flocalnewTangentInverse;
    dArrayT     flocalRHS;
    dArrayT     flocalSol;
    //

    dArray2DT   fdGdS_IPs;
    dArray2DT   fdGdS_n_IPs;


    dArray2DT   fdGdS_Elements_IPs;
    dArray2DT   fdGdS_n_Elements_IPs;


    dArray2DT   fdFYdS_IPs;
    dArray2DT   fdFYdS_n_IPs;


    dArray2DT   fdFYdS_Elements_IPs;
    dArray2DT   fdFYdS_n_Elements_IPs;


    /* Micromorphic additions */
    dArray2DT   fdGchidSIGMA_IPs;
    dArray2DT   fdGchidSIGMA_Elements_IPs;
    dArray2DT   fdGchidSIGMA_n_IPs;
    dArray2DT   fdGchidSIGMA_n_Elements_IPs;


    dArray2DT   fdFYchidSIGMA_IPs;
    dArray2DT   fdFYchidSIGMA_n_IPs;
    dArray2DT   fdFYchidSIGMA_Elements_IPs;
    dArray2DT   fdFYchidSIGMA_n_Elements_IPs;

    dArrayT 	fcG_chi;
    dArrayT 	fcG_chi_n;
    dArrayT 	dcGchidDelgammaGchi;
    dArrayT 	dPGchivardDelgammaGchi;

    dMatrixT 	DLXp;
    dMatrixT 	DLp;


    dTensor3DT  dGGchidMKLM;
    dArray2DT   dGGchidMKLM_IPs;
    dArray2DT   dGGchidMKLM_Element_IPs;
    dTensor3DT  dGGchidMKLM_n;
    dArray2DT   dGGchidMKLM_n_IPs;
    dArray2DT   dGGchidMKLM_n_Element_IPs;


    dTensor3DT  dGXpdDelgammaGchi;
    dTensor3DT  dGXedDelgammaGchi;
    dTensor3DT  dGammaedDelgammaGchi;
    dTensor3DT  dMKLMdDelgammaGchi;
    dTensor3DT  ddevMKLMdDelgammaGchi;

    dArray2DT 	GXe_IPs;
    dArray2DT   GXe_Elements_IPs;

    dArray2DT   GXp_IPs;
    dArray2DT   GXp_Elements_IPs;

    dArray2DT   GXp_n_IPs;
    dArray2DT   GXp_n_Elements_IPs;

    dArray2DT   PSIe_IPs;
    dArray2DT   PSIe_Elements_IPs;
    dArray2DT   PSIe_n_IPs;
    dArray2DT   PSIe_n_Elements_IPs;

    dArray2DT   fChip_IPs;
    dArray2DT   fChip_n_IPs;

    dArray2DT   fChip_Elements_IPs;
    dArray2DT   fChip_n_Elements_IPs;

    dArray2DT   fCchie_IPs;
    dArray2DT   fCchie_Elements_IPs;
    dArray2DT   fCchie_n_IPs;
    dArray2DT   fCchie_n_Elements_IPs;


/////stress invariants variables////////
 double Cauchy_inv;
 double trS;//trace of SPK
 double invdevS;// Norm of dev. part of SPK
 double Rel_strs_inv;
 double trSIGMA_S;
 double invdevSIGMA_S;// Norm of dev. part of SIGMA_S
 double Higher_orderT_inv;
 double invtrM;//||tr(M)|| trM is a vector
 double invdevMKLM;//||dev(M)||
 double temp_inv;
 double invPhi;
 double invGPhi;
 double deveps; // ||deveps||
 double treps;//trace of epsilon
 double invtrgammastn;
 double invdevgammastn;

 dMatrixT devsigma;
 dMatrixT devRelsts;
 dTensor3DT  devmklm;
 dMatrixT s_sigma_temp;
 dTensor3DT fmklm;
 dTensor3DT gammastn;
 dTensor3DT devgammastn;
 dArrayT trgammastn;


//////////////////////////////////////////////////
    double invJ;

    double trsigma;
    double trs_sigma;
    double trmklm;
    double invtrMKLM;
    dArrayT trvecmklm;
    dArrayT ftemp_u_element;
    dArrayT fState_variables;

    //////////////////////////////////////////////////////////
    /////DEFINITIONS FINISH HERE FOR MICROMORPHIC MATRICES////
    //////////////////////////////////////////////////////////

        /* to store fEulerian_strain_tensor_current_IP */
        dMatrixT    fEulerian_strain_tensor_current_IP;
        /* to store fEulerian_strain_IPs for each of the 27 IPs of each element */
        dArray2DT   fEulerian_strain_IPs;
        dArray2DT   fEulerian_strain_n_IPs;
        /* to store fCauchy_stress_tensor_current_IP */
//        dMatrixT    fCauchy_stress_tensor_current_IP;
        /* to store fCauchy_stress_IPs for each of the 27 IPs of each element */
//        dArray2DT   fCauchy_stress_IPs;
        ////////////////////////////////////
        dArray2DT  fE_values_IPs;
        dArray2DT  fVarepsilon_IPs;
        ////////////////////////////////////

        dArray2DT   fEulerian_strain_Elements_IPs;
        dArray2DT fEulerian_strain_Elements_n_IPs;
//        dArray2DT   fCauchy_stress_Elements_IPs;

        /////////////////////////////////////////
        dArray2DT  fE_values_Element_IPs;
        dArray2DT  fVarepsilon_Element_IPs;
        ////////////////////////////////////////

        dArray2DT fDisplacement_Element_IPs;
        dArrayT   fDisplacements_current_IPs;
        /* to store displacements for each of 27  IPs of each element
         ( probably it is better to do it only  for certain nodes but I do not know how to extract the coordinates of the nodes and calculate
         the shape functions a nodes)
         */
       // dArray2DT  fDisplacement_IPs;


    /** the solid displacement field */
    const FieldT* fDispl;

    /** the micro-displacement-gradient field */
    const FieldT* fMicro;

    /** \name state variable storage *
     * State variables are handled ABAQUS-style. For every iteration, the state
     * variables from the previous increment are passed to the element, which
     * updates the values in place. Each row in the array is the state variable
     * storage for all integration points for an element */
    /*@{*/
    dArray2DT fdState_new;
    dArray2DT fdState;

    iArray2DT fiState_new;
    iArray2DT fiState;
    /*@}*/

    /** \name connectivities */
    /*@{*/
    ArrayT<const iArray2DT*> fConnectivities_displ;
    ArrayT<const iArray2DT*> fConnectivities_micro;
    ArrayT<iArray2DT> fConnectivities_reduced;
    /*@}*/

    /** \name equations */
    /*@{*/
    ArrayT<iArray2DT> fEqnos_displ;
    ArrayT<iArray2DT> fEqnos_micro;
    /*@}*/

    /** \name element cards */
    /*@{*/
    AutoArrayT<ElementCardT> fElementCards_displ;
    AutoArrayT<ElementCardT> fElementCards_micro;
    /*@}*/

    /** \name output */
    /*@{*/
    /** output ID */
    int fOutputID;

    /** integration point stresses. Calculated and stored during
     * FSMicromorphic2_3DT::RHSDriver */
    dArray2DT fIPVariable;
    /*@}*/

    /** \name prescribed plastic gradient side set ID */
    /*@{*/
    ArrayT<StringT> fSideSetID;

    /** prescribed micro-displacement-gradient weight over the side set;
        the direction is defined by {n1,n2,n3} ?? */
    //ArrayT<iArray2DT> fMicroWght;

    /** for each side set, the global nodes on the faces in the set */
    ArrayT<iArray2DT> fMicroFaces;

    /** equation numbers for the nodes on each face */
    ArrayT<iArray2DT> fMicroFaceEqnos;

    /** side set elements */
    ArrayT<iArrayT> fSideSetElements;

    /** side set faces */
    ArrayT<iArrayT> fSideSetFaces;
    /*@}*/

    /** write output for debugging */
    /*@{*/
    /** output file stream */
    ofstreamT fs_micromorph3D_out;
    ofstreamT fs_micromorph3DMn_out;

    /** line output formating variables */
    int outputPrecision, outputFileWidth;
    /*@}*/

    void Form_solid_shape_functions(const double* &shapes_displ_X);
    void Form_Gradient_of_solid_shape_functions(const dMatrixT &fShapeDisplGrad_temp);
    void Form_micro_shape_functions(const double* &shapes_micro_X);
    void Form_deformation_gradient_tensor(void);
    void Form_micro_deformation_gradient_tensor(void);
    void Form_Grad_grad_transformation_matrix(void);
    void Form_deformation_gradient_inv_vector(void);
    void Extract_six_values_from_symmetric_tensor(const dMatrixT &fTensor,dArrayT& fTemp_six_values);
    void Put_values_In_dArrayT_vector(const dArray2DT &f2DArrayT,const int& e,const int& IP,dArrayT& fArrayT);
    void Put_values_In_Array(const dArray2DT &f2DArrayT,const int& e,const int& IP,dArrayT& fArrayT);

    //////////////////////////////////////////////////////////
    /////FUNCTIONS  FOR MICROMORPHIC MATRICES////////////////
    //////////////////////////////////////////////////////////
    //Forming the Matrices coming from the Balance of Linear Momentum
    void Form_Gamma_tensor3D(void);
    void Form_micro_deformation_tensor_Chi(void);
    void Form_Chi_inv_matrix(void);
    void Form_GRAD_Chi_matrix(void);
    void Form_double_Finv_from_Deformation_tensor_inverse(void);
    void Form_GRAD_Nuw_matrix(const dMatrixT &fShapeDisplGrad_temp);
    void Form_Finv_w_matrix(void);

    void Mapping_double_and_Array(const int condition);//
//  void Mapping_double_and_Array(double& dmat, dArrayT& fArrayT,const int& dim,const int& condition);
    void Form_deformation_tensors_arrays(const int condition);//

    void Form_KroneckerDelta_matrix(void);
    void Form_Var_F_tensor(void);
    void Form_Tsigma_1_matrix(void);
    void Form_fG1_1_matrix(void);//not defined yet
    void Form_Tsigma_2_matrix(void);
    void Form_fG1_2_matrix(void);
    void Form_Tsigma_3_matrix(void);
    void Form_fG1_3_matrix(void);
    void Form_TFn_1_matrix(void);
    void Form_fG1_4_matrix(void);
    void Form_TFn_2_matrix(void);
    void Form_fG1_5_matrix(void);
    void Form_TFn_3_matrix(void);
    void Form_fG1_6_matrix(void);
    void Form_TChi_1_matrix(void);
    void Form_fG1_7_matrix(void);
    void Form_TFn_4_matrix(void);
    void Form_fG1_8_matrix(void);
    void Form_TChi_2_matrix(void);
    void Form_fG1_9_matrix(void);
    void Form_TFn_5_matrix(void);
    void Form_fG1_10_matrix(void);
    void Form_TChi_3_matrix(void);
    void Form_fG1_11_matrix(void);
    void Form_TFn_6_matrix(void);
    void Form_fG1_12_matrix(void);
    void Form_SigCurr_matrix(void);
    void Form_fG1_13_matrix(void);
    //Forming the Matrices coming from the Balance of First Moment of Momentum
    void Form_Gradient_of_micro_shape_eta_functions(const dMatrixT &fShapeMicroGrad);
//  void Form_NCHI_eta_matrix(const dMatrixT &fShapeMicro_row_matrix);//same with the one below
    void Form_NCHI_matrix(const dMatrixT &fShapeMicro_row_matrix); //shape function matrice for micro deformatin {ETA}=[NCHI].{alpha}

    void Form_Etagrad_matrix(void);
    void Form_Mm_1_matrix(void);// needs to be multiplied by "-" and J
    void Form_Mm_2_matrix(void);// needs to be multiplied by J
    void Form_Mm_3_matrix(void);// needs to be multiplied by J
    void Form_Mm_4_matrix(void);// needs to be multiplied by J
    void Form_Mm_5_matrix(void);// needs to be multiplied by J
    void Form_Mm_6_matrix(void);// needs to be multiplied by J
    void Form_Mm_7_matrix(void);
    void Form_Mm_71_matrix(void);
    void Form_Mm_72_matrix(void);
    void Form_Mm_73_matrix(void);
    void Form_Mm_74_matrix(void);
    void Form_Mm_75_matrix(void);
    void Form_Mm_76_matrix(void);
    void Form_Mm_77_matrix(void);
    void Form_Mm_78_matrix(void);
    void Form_Mm_8_matrix(void);
    void Form_Mm_9_matrix(void);
    void Form_Mm_10_matrix(void);
    void Form_Mm_11_matrix(void);
    void Form_Mm_12_matrix(void);
    void Form_Mm_13_matrix(void);
    void Form_Mm_14_matrix(void);//something should be changed in the loop due to div(du)!!! changed done ok!
    void Form_Ru_1_matrix(void);
    void Form_Ru_2_matrix(void);
    void Form_Ru_3_matrix(void);
    void Form_RChi_1_matrix(void);// needs to be multiplied by Kappa and J
    void Form_Ru_4_matrix(void);// needs to be multiplied by Kappa and J
    void Form_RChi_2_matrix(void);//needs to be multiplied by Nu and J
    void Form_Ru_5_matrix(void);// needs to be multiplied by Nu and J
    void Form_Ru_6_matrix(void);
    void Form_Ru_7_matrix(void);
    void Form_Ru_8_matrix(void);
    void Form_Ru_9_matrix(void);
    void Form_RChi_3_matrix(void);
    void Form_Rs_sigma_matrix(void);
    void Form_R_Capital_Lambda_Chi_matrix(void);// DO NOT multiply with J !!!
    void Form_Finv_eta_matrix(void);
    void Form_CapitalLambda_matrix(void);
    void Form_CCof_tensor(void);
    void Form_SPiola_matrix(void);

//    void Form_GRAD_NCHI_Phi_matrix(const  dMatrixT &fShapeMicroGrad); no need for this

    void Form_G1_matrix(void);
    void Form_H1_matrix(void);
    void Form_H2_matrix(void);
    void Form_H3_matrix(void);


    //////////////////////////////////////////////////////////
    /////FINITE STRAIN ELASTICITY MATRICES START /////////////
    //////////////////////////////////////////////////////////
    void Form_fV1(void);
    void Form_fV2(void);
    void Form_fV3(void);
    void Form_I1_1_matrix(void);
    void Form_Second_Piola_Kirchhoff_SPK(const dMatrixT& LagStn, const dMatrixT& MicroStn);
    void Form_ChiM(void);
    void Form_I1_1(void);
    void Form_I1_2(void);
    void Form_I1_3(void);
    void Form_I1_4(void);
    void Form_I1_5(void);
    void Form_I1_6(void);
    void Form_I1_7(void);
    void Form_I2_1(void);
    void Form_I1_8(void);
    void Form_I2_2(void);
    void Form_I1_9(void);
    void Form_I2_3(void);
    void Form_SIGMA_S(void);
    void Form_fFJ(void);
    void Form_fJF(void);
    void Form_fJ1_1(void);
    void Form_fJ1_2(void);
    void Form_fJ1_3(void);
    void Form_fJ1_4(void);
    void Form_fJ2_1(void);
    void Form_fJ1_5(void);
    void Form_fJ2_2(void);
    void Form_fJ1_6(void);
    void Form_fJ2_3(void);
    void Form_fFM(void);
    void Form_fMF(void);
    void Form_fMKLM(void);
    void Form_GAMMA(void);
    void Form_fEtaM(void);
    void Form_fMchi(void);
    void Form_fMpu_1_1(void);
    void Form_fMpp_1_1(void);
    void Form_fMpu_1_2(void);
    void Form_fMpp_1_2(void);
    void Form_fMpu_2_1(void);
    void Form_fMpp_2_1(void);
    void Form_fMpu_2_2(void);
    void Form_fMpp_2_2(void);
    void Form_fMpu_3(void);
    void Form_fMpp_3(void);
    void Form_fMpu_4(void);
    void Form_fMpp_4(void);
    void Form_fMpu_5_1(void);
    void Form_fMpp_5_1(void);
    void Form_fMpu_5_2(void);
    void Form_fMpp_5_2(void);
    void Form_fMpu_6(void);
    void Form_fMpp_6(void);
    void Form_fMpu_7(void);
    void Form_fMpp_7(void);
    void Form_fMpu_8_1(void);
    void Form_fMpp_8_1(void);
    void Form_fMpu_8_2(void);
    void Form_fMpp_8_2(void);
    void Form_fMpu_9(void);
    void Form_fMpp_9(void);
    void Form_fMpu_10(void);
    void Form_fMpp_10(void);
    void Form_fMpu_11(void);
    void Form_fMpp_11(void);


    void Form_Jmat(void);


    ////////////////////////////////////////////////////////
    /////////// functions to calculate stress measures ////
    void Calculate_Cauchy_INV(void);
    void Calculate_stress_diff_INV(void);
    void Calculate_higher_order_tensor_INV(void);
    void Calculate_fmklm(void);
    void Calculate_PHI_GPHI_matrices_INV(void);
    void Caculate_invdevpart_of_Matrix(const dMatrixT &fMatrix,dMatrixT &fdevfMatrix,double devinvariant);
    void Calculate_relative_strain_INV(void);
    void Calculate_HOST_INV(void);

/* Plasticity functions */
    void  Form_fV1p(void);
    void  Form_fV2p(void);
    void  Form_fV3p(void);



    void Form_GAMMAe(void);
    void Form_fMeKLM_tr(void);
    void Form_GAMMAe_tr(void);
    void Form_GXe(void);
    void Form_GXe_tr(void);
    void Form_devMeKLM(void);
    void Form_PGchivar_tr(void);
    void Form_devMeKLM_tr(void);

    void Form_dGGchidMKLM(void);
    void Form_dGXpdDelgammaGchi(void);
    void Form_dGXedDelgammaGchi(void);
    void Form_dGammaedDelgammaGchi(void);
    void Form_dMKLMdDelgammaGchi(void);
    void Form_dPGchivardDelgammaGchi(void);
    void Form_ddevMKLMdDelgammaGchi(void);
    void Calculate_dInvddevMKLMdDelgammaGchi(void);
	void Form_PGchivar(void);


	void Form_GXp(void);
	void Form_GXp_Macro_plasticity(void);
	void Form_GXp_Micro_plasticity(void);
	void Form_GXp_Coupled_plasticity(void);
	void Form_fMKLM_tr(void);
	void Form_Mean_fMKLM_tr(void);
	void Form_fdevMKLM_tr(void);
	void Form_fNormdevMKLM_tr(void);
	void Form_Norm_Mean_fMKLM_tr(void);
	void Form_dGnablachidMKLM_tr(void);
	void Form_dGnablachidMKLM(void);
	void Form_dFnablachidMKLM(void);
	void Form_dfMKLMdDelgammanablachi(void);
	void Form_dmeanfMKLMdDelgammanablachi(void);
	void Form_dfdevMKLMdDelgammanablachi(void);
	void Form_dfNorm_devMKLMdDelgammanablachi(void);
    void Form_fMeKLM(void);
	void Form_fdevMeKLM(void);
	void Form_Mean_fMeKLM(void);
	void Form_Norm_Mean_fMeKLM(void);
	void Form_fNormdevMeKLM(void);
	void Form_dNormdevfMKLMdDelgammanablachi(void);
	void Form_kc_nablachi_n(const int a);
	void Form_Norm_kc_nablachi_n(void);
	void Form_Norm_cohesion_nablachi(void);
	void Form_dNorm_meanfMKLMdDelgammanablachi(void);
	void Form_dfkc_nablachidDelgammanablachi(const int a);
	void Form_dNorm_kc_nablachidDelgammanablachi(void);


	void Form_Coeff_delDelgamma_nablachi(const int a);
	void Form_II13e_1(void);
	void Form_II14p_1_16(void);
	void Form_II15e_1(void);
	void Form_II16p_1_16(void);
	void Form_II17p_1_16(void);
	void Form_II18p_1_16(void);
	void Form_II19p_1_16(void);
	void Form_II20p_1_16(void);
	void Form_II21p_1_75(void);

//////////////////////////////////////////////////////
////////////////// Capped model functions ////////////
	void Form_Kappa_nablachi_n(const int a);
	void Form_Norm_Kappa_nablachi_n(void);
	void Form_Kappa_nablachi(const int a);
	void Form_Norm_Kappa_nablachi(void);
	void Form_dNorm_Kappa_nablachidDelgammanablachi(void);
	void Form_dfKappa_nablachidDelgammanablachi(const int a);


//////////////////////////////////////////////////////

    void Calculate_fmeklm(void);


// The first and the second terms cancel each other, we start from the thirh term which has 12 matrices
// all the matrices having "e" are the ones which direclty include the term  d(deltau)/dX
// matrices having "p" are the one which include delta(gamma)
    void Form_IJp_1(void);
    void Form_IJp_2(void);
    void Form_IJp_3(void);
    void Form_IJp_4(void);
    void Form_IJp_5(void);
    void Form_IJp_6(void);
    void Form_IJp_7(void);
    void Form_IJp_8(void);
    void Form_IJp_9(void);
    /* Matrices from coupling */
    void Form_IJp_10(void);
    void Form_IJp_11(void);
    void Form_IJp_12(void);
    void Form_IJp_13(void);
    void Form_IJp_14(void);
    void Form_IJp_15(void);
    void Form_IJp_16(void);
    void Form_IJp_17(void);


    void Form_I1e_1(void); // the first term first matrix

    void Form_I2e_1(void); // the  second term first matrix
    void Form_I2p_2(void);
//    void Form_I2p_3(void);
//    void Form_I2p_4(void);
//    void Form_I2p_5(void);
//    void Form_I2p_6(void);
//    void Form_I2p_7(void);
//    void Form_I2p_8(void);
//    void Form_I2p_9(void);
//    void Form_I2p_10(void);
    /* Matrices from coupling */
//    void Form_I2p_11(void);
//    void Form_I2p_12(void);
//    void Form_I2p_13(void);
//    void Form_I2p_14(void);
//    void Form_I2p_15(void);
//    void Form_I2p_16(void);
//    void Form_I2p_17(void);
//    void Form_I2p_18(void);



    void Form_I3e_1(void); // the third term first matrix
    void Form_I3e_2(void); //
    void Form_I3e_3(void); //
    void Form_I3p_4(void); //
    void Form_I3p_5(void); //
    void Form_I3p_6(void); //
    void Form_I3p_7(void); //
//    void Form_I3p_8(void); //
//    void Form_I3p_9(void); //
//    void Form_I3p_10(void); //
//    void Form_I3p_11(void); //
//    void Form_I3p_12(void); //
    void Form_I3e_13(void); //
    void Form_I3e_14(void); //
    void Form_I3e_15(void); //
    void Form_I3p_16(void); //
//    void Form_I3p_17(void); //
//    void Form_I3p_18(void); //
    void Form_I3p_19(void); //
//    void Form_I3p_20(void); //
//    void Form_I3p_21(void); //
    void Form_I3p_22(void); //
//    void Form_I3p_23(void); //
//    void Form_I3p_24(void); //
    void Form_I3p_25(void); //
    void Form_I3p_26(void); //
    void Form_I3p_27(void); //
//    void Form_I3p_28(void); //
//    void Form_I3p_29(void); //
//    void Form_I3p_30(void); //
//    void Form_I3p_31(void); //
    void Form_I3p_32(void); //
//    void Form_I3p_33(void); //
//    void Form_I3p_34(void); //
//    void Form_I3p_35(void); //
//    void Form_I3p_36(void); //
    void Form_I3e_37(void); //
    void Form_I3e_38(void); //
    void Form_I3e_39(void); //
    void Form_I3p_40(void); //
//    void Form_I3p_41(void); //
//    void Form_I3p_42(void); //
//    void Form_I3p_43(void); //
//    void Form_I3p_44(void); //
//    void Form_I3p_45(void); //
//    void Form_I3p_46(void); //
//    void Form_I3p_47(void); //
//    void Form_I3p_48(void); //
    void Form_I3p_49(void); //
//    void Form_I3p_50(void); //
//    void Form_I3p_51(void); //
    void Form_I3p_52(void); //
//    void Form_I3p_53(void); //
//    void Form_I3p_54(void); //
/* Matrices from Del(delgammachi) */
    void Form_I3p_55(void); //
//    void Form_I3p_56(void); //
//    void Form_I3p_57(void); //
//    void Form_I3p_58(void); //
//    void Form_I3p_59(void); //
//    void Form_I3p_60(void); //
//    void Form_I3p_61(void); //
//    void Form_I3p_62(void); //
    void Form_I3p_63(void); //
//    void Form_I3p_64(void); //
//    void Form_I3p_65(void); //
//    void Form_I3p_66(void); //
//    void Form_I3p_67(void); //
//    void Form_I3p_68(void); //
//    void Form_I3p_69(void); //
//    void Form_I3p_70(void); //
    void Form_I3p_71(void); //
//    void Form_I3p_72(void); //
//    void Form_I3p_73(void); //
//    void Form_I3p_74(void); //
//    void Form_I3p_75(void); //
//    void Form_I3p_76(void); //
//    void Form_I3p_77(void); //
//    void Form_I3p_78(void); //
    /* Matrices from coupling*/
    void Form_I3p_79(void); //
//    void Form_I3p_80(void); //
//    void Form_I3p_81(void); //
//    void Form_I3p_82(void); //
//    void Form_I3p_83(void); //
//    void Form_I3p_84(void); //
//    void Form_I3p_85(void); //
//    void Form_I3p_86(void); //
//    void Form_I3p_87(void); //
//    void Form_I3p_88(void); //
//    void Form_I3p_89(void); //
//    void Form_I3p_90(void); //
//    void Form_I3p_91(void); //
//    void Form_I3p_92(void); //
//    void Form_I3p_93(void); //
//    void Form_I3p_94(void); //
//    void Form_I3p_95(void); //
//    void Form_I3p_96(void); //
//    void Form_I3p_97(void); //
//    void Form_I3p_98(void); //
//    void Form_I3p_99(void); //
//    void Form_I3p_100(void); //

//    void Form_I3p_101(void); //
//    void Form_I3p_102(void); //
//    void Form_I3p_103(void); //
//    void Form_I3p_104(void); //
//    void Form_I3p_105(void); //
//    void Form_I3p_106(void); //
//    void Form_I3p_107(void); //
//    void Form_I3p_108(void); //
//    void Form_I3p_109(void); //
//    void Form_I3p_110(void); //
    void Form_I3p_111(void); //
//    void Form_I3p_112(void); //
//    void Form_I3p_113(void); //
//    void Form_I3p_114(void); //
//    void Form_I3p_115(void); //
//    void Form_I3p_116(void); //
//    void Form_I3p_117(void); //
//    void Form_I3p_118(void); //
    void Form_I3p_119(void); //
//    void Form_I3p_120(void); //
//    void Form_I3p_121(void); //
//    void Form_I3p_122(void); //
//    void Form_I3p_123(void); //
//    void Form_I3p_124(void); //
//    void Form_I3p_125(void); //
//    void Form_I3p_126(void); //
    void Form_I3p_127(void); //
//    void Form_I3p_128(void); //
//    void Form_I3p_129(void); //
//    void Form_I3p_130(void); //
//    void Form_I3p_131(void); //
//    void Form_I3p_132(void); //
//    void Form_I3p_133(void); //
//    void Form_I3p_134(void); //
    void Form_I3p_135(void); //
//    void Form_I3p_136(void); //
//    void Form_I3p_137(void); //
//    void Form_I3p_138(void); //
//    void Form_I3p_139(void); //
//    void Form_I3p_140(void); //
//    void Form_I3p_141(void); //
//    void Form_I3p_142(void); //











    void Form_I4e_1(void); // the fourth term first matrix
    void Form_I4p_2(void); //
//    void Form_I4p_3(void); //
//    void Form_I4p_4(void); //
//    void Form_I4p_5(void); //
//    void Form_I4p_6(void); //
//    void Form_I4p_7(void); //
//    void Form_I4p_8(void); //
//    void Form_I4p_9(void); //
//    void Form_I4p_10(void); //
//    void Form_I4p_11(void); //
//    void Form_I4p_12(void); //
//    void Form_I4p_13(void); //
//    void Form_I4p_14(void); //
//    void Form_I4p_15(void); //
//    void Form_I4p_16(void); //
//    void Form_I4p_17(void); //
//    void Form_I4p_18(void); //


    /* Functions for the second balance equation */
    void Form_IIJp_1(void);
    void Form_IIJp_2(void);
    void Form_IIJp_3(void);
    void Form_IIJp_4(void);
    void Form_IIJp_5(void);
    void Form_IIJp_6(void);
    void Form_IIJp_7(void);
    void Form_IIJp_8(void);
    /* Matrices from coupling*/
    void Form_IIJp_9(void);
    void Form_IIJp_10(void);
    void Form_IIJp_11(void);
    void Form_IIJp_12(void);
    void Form_IIJp_13(void);
    void Form_IIJp_14(void);
    void Form_IIJp_15(void);
    void Form_IIJp_16(void);


    void Form_II2e_1(void);
    void Form_II2p_2(void);
//    void Form_II2p_3(void);
//    void Form_II2p_4(void);
//    void Form_II2p_5(void);
//    void Form_II2p_6(void);
//    void Form_II2p_7(void);
//    void Form_II2p_8(void);
//    void Form_II2p_9(void);
    /* Matrices from coupling*/
//    void Form_II2p_10(void);
//    void Form_II2p_11(void);
//    void Form_II2p_12(void);
//    void Form_II2p_13(void);
//    void Form_II2p_14(void);
//    void Form_II2p_15(void);
//    void Form_II2p_16(void);
//    void Form_II2p_17(void);



    void Form_II3e_1(void);
    void Form_II3e_2(void);
    void Form_II3e_3(void);
    void Form_II3e_4(void);
    void Form_II3e_5(void);
    void Form_II3p_6(void);
//    void Form_II3p_7(void);
//    void Form_II3p_8(void);
//    void Form_II3p_9(void);
//    void Form_II3p_10(void);
    void Form_II3p_11(void);
//    void Form_II3p_12(void);
//    void Form_II3p_13(void);
//    void Form_II3p_14(void);
//    void Form_II3p_15(void);
    void Form_II3p_16(void);
//    void Form_II3p_17(void);
//    void Form_II3p_18(void);
//    void Form_II3p_19(void);
//    void Form_II3p_20(void);
    void Form_II3p_21(void);
//    void Form_II3p_22(void);
//    void Form_II3p_23(void);
//    void Form_II3p_24(void);
//    void Form_II3p_25(void);
    void Form_II3p_26(void);
//    void Form_II3p_27(void);
//    void Form_II3p_28(void);
//    void Form_II3p_29(void);
//    void Form_II3p_30(void);
    void Form_II3e_31(void);
    void Form_II3e_32(void);
    void Form_II3e_33(void);
    void Form_II3p_34(void);
//    void Form_II3p_35(void);
//    void Form_II3p_36(void);
    void Form_II3p_37(void);
//    void Form_II3p_38(void);
//    void Form_II3p_39(void);
    void Form_II3p_40(void);
//    void Form_II3p_41(void);
//    void Form_II3p_42(void);
    void Form_II3p_43(void);
//    void Form_II3p_44(void);
//    void Form_II3p_45(void);
    void Form_II3p_46(void);
//    void Form_II3p_47(void);
//    void Form_II3p_48(void);
    /* Matrices from Del(delgammachi) */
    void Form_II3p_49(void);
//    void Form_II3p_50(void);
//    void Form_II3p_51(void);
    void Form_II3p_52(void);
//    void Form_II3p_53(void);
//    void Form_II3p_54(void);
//    void Form_II3p_55(void);
//    void Form_II3p_56(void);
    void Form_II3p_57(void);
//    void Form_II3p_58(void);
//    void Form_II3p_59(void);
//    void Form_II3p_60(void);
//    void Form_II3p_61(void);
//    void Form_II3p_62(void);
//    void Form_II3p_63(void);
//    void Form_II3p_64(void);
    void Form_II3p_65(void);
//    void Form_II3p_66(void);
//    void Form_II3p_67(void);
//    void Form_II3p_68(void);
//    void Form_II3p_69(void);
//    void Form_II3p_70(void);
//    void Form_II3p_71(void);
//    void Form_II3p_72(void);
    /* Matrices from coupling */
    void Form_II3p_73(void);
//    void Form_II3p_74(void);
//    void Form_II3p_75(void);
//    void Form_II3p_76(void);
//    void Form_II3p_77(void);
//    void Form_II3p_78(void);
//    void Form_II3p_79(void);
//    void Form_II3p_80(void);
    void Form_II3p_81(void);
//    void Form_II3p_82(void);
//    void Form_II3p_83(void);
//    void Form_II3p_84(void);
//    void Form_II3p_85(void);
//    void Form_II3p_86(void);
//    void Form_II3p_87(void);
//    void Form_II3p_88(void);
    void Form_II3p_89(void);
//    void Form_II3p_90(void);
//    void Form_II3p_91(void);
//    void Form_II3p_92(void);
//    void Form_II3p_93(void);
//    void Form_II3p_94(void);
//    void Form_II3p_95(void);
//    void Form_II3p_96(void);
    void Form_II3p_97(void);
//    void Form_II3p_98(void);
//    void Form_II3p_99(void);
//    void Form_II3p_100(void);
//    void Form_II3p_101(void);
//    void Form_II3p_102(void);
//    void Form_II3p_103(void);
//    void Form_II3p_104(void);
    void Form_II3p_105(void);
//    void Form_II3p_106(void);
//    void Form_II3p_107(void);
//    void Form_II3p_108(void);
//    void Form_II3p_109(void);
//    void Form_II3p_110(void);
//    void Form_II3p_111(void);
//    void Form_II3p_112(void);
    void Form_II3p_113(void);
//    void Form_II3p_114(void);
//    void Form_II3p_115(void);
//    void Form_II3p_116(void);
//    void Form_II3p_117(void);
//    void Form_II3p_118(void);
//    void Form_II3p_119(void);
//    void Form_II3p_120(void);
    void Form_II3p_121(void);
//    void Form_II3p_122(void);
//    void Form_II3p_123(void);
//    void Form_II3p_124(void);
//    void Form_II3p_125(void);
//    void Form_II3p_126(void);
//    void Form_II3p_127(void);
//    void Form_II3p_128(void);
    void Form_II3p_129(void);
//    void Form_II3p_130(void);
//    void Form_II3p_131(void);
//    void Form_II3p_132(void);
//    void Form_II3p_133(void);
//    void Form_II3p_134(void);
//    void Form_II3p_135(void);
//    void Form_II3p_136(void);







    void Form_II4e_1(void);
    void Form_II4p_2(void);
//    void Form_II4p_3(void);
//    void Form_II4p_4(void);
//    void Form_II4p_5(void);
//    void Form_II4p_6(void);
//    void Form_II4p_7(void);
//    void Form_II4p_8(void);
//    void Form_II4p_9(void);
    /* Matrices from coupling*/
//    void Form_II4p_10(void);
//    void Form_II4p_11(void);
//    void Form_II4p_12(void);
//    void Form_II4p_13(void);
//    void Form_II4p_14(void);
//    void Form_II4p_15(void);
//    void Form_II4p_16(void);
//    void Form_II4p_17(void);

    /*Functions from higher orde couple stress tensor */
    void Form_Temp_tensor_for_II5Jp(void);
    void Form_II5Jp_1(void);
    void Form_II5Jp_2(void);
    void Form_II5Jp_3(void);
    void Form_II5Jp_4(void);
    void Form_II5Jp_5(void);
    void Form_II5Jp_6(void);
    void Form_II5Jp_7(void);
    void Form_II5Jp_8(void);
    void Form_II5Jp_9(void);
    void Form_II5Jp_10(void);
    void Form_II5Jp_11(void);
    void Form_II5Jp_12(void);
    void Form_II5Jp_13(void);
    void Form_II5Jp_14(void);
    void Form_II5Jp_15(void);
    void Form_II5Jp_16(void);

    void Form_II6e_1(void);
    void Form_II7e_1(void);

	void Form_Temp_tensor_for_II7(void);

    void Form_II7p_1(void);
    void Form_II7p_2(void);
    void Form_II7p_3(void);
    void Form_II7p_4(void);
    void Form_II7p_5(void);
    void Form_II7p_6(void);
    void Form_II7p_7(void);
    void Form_II7p_8(void);
    void Form_II7p_9(void);
    void Form_II7p_10(void);
    void Form_II7p_11(void);
    void Form_II7p_12(void);
    void Form_II7p_13(void);
    void Form_II7p_14(void);
    void Form_II7p_15(void);
    void Form_II7p_16(void);


    void Form_II8e_1(void);

	void Form_Temp_tensor_for_II8(void);

    void Form_II8p_1(void);
    void Form_II8p_2(void);
    void Form_II8p_3(void);
    void Form_II8p_4(void);
    void Form_II8p_5(void);
    void Form_II8p_6(void);
    void Form_II8p_7(void);
    void Form_II8p_8(void);
    void Form_II8p_9(void);
    void Form_II8p_10(void);
    void Form_II8p_11(void);
    void Form_II8p_12(void);
    void Form_II8p_13(void);
    void Form_II8p_14(void);
    void Form_II8p_15(void);
    void Form_II8p_16(void);


    void Form_II9e_1(void);

	void Form_Temp_tensor_for_II9(void);

    void Form_II9p_1(void);
    void Form_II9p_2(void);
    void Form_II9p_3(void);
    void Form_II9p_4(void);
    void Form_II9p_5(void);
    void Form_II9p_6(void);
    void Form_II9p_7(void);
    void Form_II9p_8(void);
    void Form_II9p_9(void);
    void Form_II9p_10(void);
    void Form_II9p_11(void);
    void Form_II9p_12(void);
    void Form_II9p_13(void);
    void Form_II9p_14(void);
    void Form_II9p_15(void);
    void Form_II9p_16(void);


    void Form_II10e_1(void);

	void Form_Temp_tensor_for_II10(void);

    void Form_II10p_1(void);
    void Form_II10p_2(void);
    void Form_II10p_3(void);
    void Form_II10p_4(void);
    void Form_II10p_5(void);
    void Form_II10p_6(void);
    void Form_II10p_7(void);
    void Form_II10p_8(void);
    void Form_II10p_9(void);
    void Form_II10p_10(void);
    void Form_II10p_11(void);
    void Form_II10p_12(void);
    void Form_II10p_13(void);
    void Form_II10p_14(void);
    void Form_II10p_15(void);
    void Form_II10p_16(void);



	void Form_Temp_tensor_for_II11(void);

    void Form_II11p_1(void);
    void Form_II11p_2(void);
    void Form_II11p_3(void);
    void Form_II11p_4(void);
    void Form_II11p_5(void);
    void Form_II11p_6(void);
    void Form_II11p_7(void);
    void Form_II11p_8(void);
    void Form_II11p_9(void);
    void Form_II11p_10(void);
    void Form_II11p_11(void);
    void Form_II11p_12(void);
    void Form_II11p_13(void);
    void Form_II11p_14(void);
    void Form_II11p_15(void);
    void Form_II11p_16(void);


    void Form_II12e_1(void);

	void Form_Temp_tensor_for_II12(void);

    void Form_II12p_1(void);
    void Form_II12p_2(void);
    void Form_II12p_3(void);
    void Form_II12p_4(void);
    void Form_II12p_5(void);
    void Form_II12p_6(void);
    void Form_II12p_7(void);
    void Form_II12p_8(void);
    void Form_II12p_9(void);
    void Form_II12p_10(void);
    void Form_II12p_11(void);
    void Form_II12p_12(void);
    void Form_II12p_13(void);
    void Form_II12p_14(void);
    void Form_II12p_15(void);
    void Form_II12p_16(void);


    /////////////////////////////////////////////////////////
    void FindYieldFunctionValue();

    void Form_fA4(void);
    //////////////////////////////////////////////////////////
    /////FUNCTIONS FINISH HERE FOR MICROMORPHIC MATRICES////
    //////////////////////////////////////////////////////////

protected:

    /** extract natural boundary condition information */
    void TakeNaturalBC(const ParameterListT& list);

    /** apply traction boundary conditions to displacement equations */
    void ApplyTractionBC(void);

    /** update traction BC data for displacement equations */
    void SetTractionBC(void);

    /* traction data */
    ArrayT<Traction_CardT> fTractionList;
    int fTractionBCSet;

    /** \name arrays with local ordering */
    /*@{*/
    LocalArrayT fLocInitCoords;   /**< initial coords with local ordering */
    LocalArrayT fLocDisp;              /**< solid displacements with local ordering  */
    /*@}*/

    /** \name work space */
    /*@{*/
    dArrayT fNEEvec; /**< work space vector: [element DOF] */
    dArrayT fDOFvec; /**< work space vector: [nodal DOF]   */
    /*@}*/

};


inline const ShapeFunctionT& FSMicromorphic2_3DT::ShapeFunctionDispl(void) const
{
#if __option(extended_errorcheck)
    if (!fShapes_displ)
        ExceptionT::GeneralFail("FSMicromorphic2_3DT::ShapeFunctionDispl", "no displ shape functions");
#endif
    return *fShapes_displ;
}

inline const ShapeFunctionT& FSMicromorphic2_3DT::ShapeFunctionMicro(void) const
{
#if __option(extended_errorcheck)
    if (!fShapes_micro)
        ExceptionT::GeneralFail("FSMicromorphic2_3DT::ShapeFunctionMicro", "no micro shape functions");
#endif
    return *fShapes_micro;
}

/* return the geometry code */
inline GeometryT::CodeT FSMicromorphic2_3DT::GeometryCode(void) const
{ return fGeometryCode_displ; }


} // namespace Tahoe
#endif /* _FS_MICROMORPHIC2_3D_T_H_ */



