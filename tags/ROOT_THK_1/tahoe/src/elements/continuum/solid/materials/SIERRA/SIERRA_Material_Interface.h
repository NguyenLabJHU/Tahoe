/* $Id: SIERRA_Material_Interface.h,v 1.5 2003-03-10 23:30:56 paklein Exp $ */
#ifndef __SIERRA_MAT_INTERFACE_H__
#define __SIERRA_MAT_INTERFACE_H__

/* Fortran to C/C++ name mangling */
#include "fortran_names.h"

#ifdef __cplusplus 
extern "C" {
#endif

/** \name retrieving parameter information */
/*@{*/ 
/** retrieve a named value */
extern void FORTRAN_NAME(get_real_constant)(double* destination, const int* mat_vals, 
	const char* value_name);

/** retrieve the index for the value for the given material. The numbering uses
 * Fortran conventions, so the first index is 1. */
extern void FORTRAN_NAME(get_var_index)(int* index, int* num_workset_elem, const char* variable_name, 
	const char* material_name);
/*@}*/ 

/** \name Sierra callback function types */
/*@{*/
/** function to do checking/registration of parameters. This function
 * is registered with a call to register_material.
 * \param parameters array of material parameters
 */
typedef void (*Sierra_function_param_check)(int* mat_vals);

/** function to do material computations
 * \param nelem number of elements in the workset
 * \param dt time step
 * \param vars_input array of driving input variables, i.e., strain, temperature, etc.
 *        The array is dimension [nelem] x [ivars_size]    
 * \param ivars_size number of input variables per stress point
 * \param stress_old (rotated) stress from the previous time increment
 * \param stress_new destination for current stress
 * \param nsv number of state variables per stress point
 * \param state_old state variables from the previous time increment
 * \param state_new destination for updated state variables
 * \param matvals array of material parameters
 * \param ncd
 */
typedef void (*Sierra_function_material_calc)(int* nelem, double* dt,
	double* vars_input, int* ivars_size, double* stress_old, double* stress_new, 
	int* nsv, double* state_old, double* state_new, int* matvals, int* ncd);

/** function to do material initialization
 * \param nelem number of elements in the workset
 * \param dt time step
 * \param nsv number of state variables
 * \param state_old state variables from the previous time increment
 * \param state_new destination for updated state variables
 * \param matvals array of material parameters
 * \param ncd
 */
typedef void (*Sierra_function_material_init)(int* nelem, double* dt, int* nsv, 
	double* state_old, double* state_new, int* matvals, int* ncd);
/*@}*/

/** \name registration functions */
/*@{*/
/** register the material model.
 * \param XML_command_id XML command id for the material parameter block
 * \param function to do checking/registration of parameters
 * \param modulus_flag  0/1 = dont/do complete elastic constants
 * \param material_name name for material model
*/
extern void FORTRAN_NAME(register_material)(int* XML_command_id, Sierra_function_param_check check_func, 
	int* modulus_flag, const char* material_name);

/** register function to do material computations */
extern void FORTRAN_NAME(register_process_func)(Sierra_function_material_calc calc_func, const char* material_name);

/** register function to do material initialization */
extern void FORTRAN_NAME(register_init_func)(Sierra_function_material_init init_func, const char* material_name);

/** register the number of state variables */
extern void FORTRAN_NAME(register_num_state_vars)(int* nsv, const char* material_name);

/** register the data that the material model needs from the element in
 * order to do its computations */
extern void FORTRAN_NAME(register_input_var)(const char* variable_name, const char* material_name);

/** register the XML commands that specify material parameters */
extern void FORTRAN_NAME(register_parser_line)(int* XML_command_id, const char* material_name);
/*@}*/

#ifdef __cplusplus 
}
#endif

#endif /* __SIERRA_MAT_INTERFACE_H__ */