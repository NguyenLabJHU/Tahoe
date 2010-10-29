#include "FSDE_inc.h"

#include <math.h>

static double z[117];

/* function to compute derivative of free energy wrt C (i.e. PK2 stress) */
void get_dUdC(const double* params, const double *Xsi, const double* Cmat, double* dUdC) {

/* common definitions */
#include "FSDE_common_defines.h"

	z[1] = -C12*C21;
	z[2] = C13*C21;
	z[3] = C11*C22;
	z[4] = -C13*C22;
	z[5] = -C11*C23;
	z[6] = C12*C23;
	z[7] = C12*C31;
	z[8] = -C13*C31;
	z[9] = -C22*C31;
	z[10] = C23*C31;
	z[11] = -C11*C32;
	z[12] = C13*C32;
	z[13] = C21*C32;
	z[14] = -C23*C32;
	z[15] = C11*C33;
	z[16] = -C12*C33;
	z[17] = -C21*C33;
	z[18] = C22*C33;
	z[19] = C11 + C22 + C33;
	z[20] = ex*ex;
	z[21] = ey*ey;
	z[22] = ez*ez;
	z[23] = 1./(Nrig*Nrig);
	z[24] = 1./Nrig;
	z[25] = C33*z[1];
	z[26] = C32*z[2];
	z[27] = C33*z[3];
	z[28] = C31*z[4];
	z[29] = C32*z[5];
	z[30] = C31*z[6];
	z[12] = z[12] + z[16];
	z[10] = z[10] + z[17];
	z[14] = z[14] + z[18];
	z[16] = z[19]*z[19];
	z[17] = 0.1*z[19]*z[24];
	z[1] = z[1] + z[3];
	z[3] = z[25] + z[26] + z[27] + z[28] + z[29] + z[30];
	z[2] = z[2] + z[5];
	z[4] = z[4] + z[6];
	z[5] = z[11] + z[7];
	z[6] = z[15] + z[8];
	z[7] = z[13] + z[9];
	z[8] = z[12]*z[12];
	z[9] = z[10]*z[10];
	z[11] = z[14]*z[14];
	z[13] = 0.03142857142857143*z[16]*z[23];
	z[15] = z[1]*z[1];
	z[16] = 1./(z[3]*z[3]);
	z[3] = 1./z[3];
	z[18] = z[2]*z[2];
	z[19] = z[4]*z[4];
	z[23] = z[5]*z[5];
	z[24] = z[6]*z[6];
	z[25] = z[7]*z[7];
	z[13] = 0.5 + z[13] + z[17];
	z[16] = -z[16];
	z[17] = -C12*ex*ey*z[3];
	z[26] = C13*ex*ey*z[3];
	z[27] = -C21*ex*ey*z[3];
	z[28] = C23*ex*ey*z[3];
	z[29] = C31*ex*ey*z[3];
	z[30] = C32*ex*ey*z[3];
	z[31] = -C33*ex*ey*z[3];
	z[32] = C12*ex*ez*z[3];
	z[33] = -C13*ex*ez*z[3];
	z[34] = C21*ex*ez*z[3];
	z[35] = -C22*ex*ez*z[3];
	z[36] = C23*ex*ez*z[3];
	z[37] = -C31*ex*ez*z[3];
	z[38] = C32*ex*ez*z[3];
	z[39] = -C11*ey*ez*z[3];
	z[40] = C12*ey*ez*z[3];
	z[41] = C13*ey*ez*z[3];
	z[42] = C21*ey*ez*z[3];
	z[43] = -C23*ey*ez*z[3];
	z[44] = C31*ey*ez*z[3];
	z[45] = -C32*ey*ez*z[3];
	z[46] = C22*z[20]*z[3];
	z[47] = -C23*z[20]*z[3];
	z[48] = -C32*z[20]*z[3];
	z[49] = C33*z[20]*z[3];
	z[50] = C11*z[21]*z[3];
	z[51] = -C13*z[21]*z[3];
	z[52] = -C31*z[21]*z[3];
	z[53] = C33*z[21]*z[3];
	z[54] = C11*z[22]*z[3];
	z[55] = -C12*z[22]*z[3];
	z[56] = -C21*z[22]*z[3];
	z[3] = C22*z[22]*z[3];
	z[12] = z[12]*z[16];
	z[57] = z[10]*z[16];
	z[58] = z[1]*z[14]*z[16]*z[20];
	z[59] = z[1]*z[14]*z[16]*z[22];
	z[60] = ey*ez*z[14]*z[16]*z[2];
	z[61] = z[14]*z[16]*z[2]*z[20];
	z[62] = ey*ez*z[1]*z[16]*z[2];
	z[63] = z[1]*z[16]*z[2]*z[22];
	z[64] = ex*ez*z[14]*z[16]*z[4];
	z[65] = z[14]*z[16]*z[20]*z[4];
	z[66] = ex*ez*z[1]*z[16]*z[4];
	z[67] = z[1]*z[16]*z[22]*z[4];
	z[68] = ex*ez*z[16]*z[2]*z[4];
	z[69] = ey*ez*z[16]*z[2]*z[4];
	z[70] = ey*ez*z[14]*z[16]*z[5];
	z[71] = z[14]*z[16]*z[20]*z[5];
	z[72] = ey*ez*z[1]*z[16]*z[5];
	z[73] = z[1]*z[16]*z[22]*z[5];
	z[74] = ey*ez*z[16]*z[2]*z[5];
	z[75] = ex*ez*z[16]*z[4]*z[5];
	z[76] = ey*ez*z[16]*z[4]*z[5];
	z[77] = z[14]*z[16]*z[20]*z[6];
	z[78] = z[14]*z[16]*z[21]*z[6];
	z[79] = z[1]*z[16]*z[21]*z[6];
	z[80] = z[1]*z[16]*z[22]*z[6];
	z[81] = ey*ez*z[16]*z[2]*z[6];
	z[82] = z[16]*z[2]*z[21]*z[6];
	z[83] = ex*ez*z[16]*z[4]*z[6];
	z[84] = z[16]*z[21]*z[4]*z[6];
	z[85] = ey*ez*z[16]*z[5]*z[6];
	z[86] = z[16]*z[21]*z[5]*z[6];
	z[87] = ex*ez*z[14]*z[16]*z[7];
	z[88] = z[14]*z[16]*z[20]*z[7];
	z[89] = ex*ez*z[1]*z[16]*z[7];
	z[90] = z[1]*z[16]*z[22]*z[7];
	z[91] = ex*ez*z[16]*z[2]*z[7];
	z[92] = ey*ez*z[16]*z[2]*z[7];
	z[93] = ex*ez*z[16]*z[4]*z[7];
	z[94] = ex*ez*z[16]*z[5]*z[7];
	z[95] = ey*ez*z[16]*z[5]*z[7];
	z[96] = ex*ez*z[16]*z[6]*z[7];
	z[97] = z[16]*z[21]*z[6]*z[7];
	z[8] = ex*ey*z[16]*z[8];
	z[9] = ex*ey*z[16]*z[9];
	z[11] = z[11]*z[16]*z[20];
	z[15] = z[15]*z[16]*z[22];
	z[18] = ey*ez*z[16]*z[18];
	z[19] = ex*ez*z[16]*z[19];
	z[23] = ey*ez*z[16]*z[23];
	z[24] = z[16]*z[21]*z[24];
	z[16] = ex*ez*z[16]*z[25];
	z[25] = ex*ey*z[12];
	z[10] = z[10]*z[25];
	z[98] = z[14]*z[25];
	z[99] = z[1]*z[25];
	z[100] = z[2]*z[25];
	z[101] = z[25]*z[4];
	z[102] = z[25]*z[5];
	z[103] = z[25]*z[6];
	z[25] = z[25]*z[7];
	z[104] = z[12]*z[14]*z[20];
	z[105] = z[1]*z[12]*z[22];
	z[106] = ey*ez*z[12]*z[2];
	z[107] = ex*ez*z[12]*z[4];
	z[108] = ey*ez*z[12]*z[5];
	z[109] = z[12]*z[21]*z[6];
	z[12] = ex*ez*z[12]*z[7];
	z[110] = ex*ey*z[57];
	z[111] = z[110]*z[14];
	z[112] = z[1]*z[110];
	z[113] = z[110]*z[2];
	z[114] = z[110]*z[4];
	z[115] = z[110]*z[5];
	z[116] = z[110]*z[6];
	z[110] = z[110]*z[7];
	z[14] = z[14]*z[20]*z[57];
	z[1] = z[1]*z[22]*z[57];
	z[2] = ey*ez*z[2]*z[57];
	z[4] = ex*ez*z[4]*z[57];
	z[5] = ey*ez*z[5]*z[57];
	z[6] = z[21]*z[57]*z[6];
	z[7] = ex*ez*z[57]*z[7];
	z[13] = mu*z[13];
	z[20] = z[102] + z[115] + z[23] + z[29] + z[71] + z[73] + z[74] + z[75] + z[86] + z[94];
	z[20] = z[20] + z[32] + z[39] + z[48];
	z[16] = z[110] + z[16] + z[25] + z[30] + z[88] + z[90] + z[92] + z[93] + z[95] + z[97];
	z[16] = z[16] + z[35] + z[42] + z[52];
	z[21] = z[103] + z[116] + z[24] + z[33] + z[77] + z[80] + z[81] + z[83] + z[85] + z[96];
	z[21] = z[21] + z[37] + z[49] + z[54];
	z[8] = z[10] + z[104] + z[105] + z[106] + z[107] + z[108] + z[109] + z[12] + z[31] + z[8];
	z[8] = z[38] + z[41] + z[55] + z[8];
	z[1] = z[1] + z[10] + z[14] + z[2] + z[31] + z[4] + z[5] + z[6] + z[7] + z[9];
	z[1] = z[1] + z[36] + z[44] + z[56];
	z[2] = z[112] + z[15] + z[17] + z[27] + z[46] + z[66] + z[72] + z[79] + z[89] + z[99];
	z[2] = z[2] + z[50] + z[58] + z[62];
	z[4] = z[100] + z[113] + z[18] + z[26] + z[34] + z[39] + z[68] + z[74] + z[82] + z[91];
	z[4] = z[4] + z[47] + z[61] + z[63];
	z[3] = z[11] + z[111] + z[3] + z[43] + z[45] + z[53] + z[70] + z[78] + z[87] + z[98];
	z[3] = z[3] + z[59] + z[60] + z[64];
	z[5] = z[101] + z[114] + z[19] + z[28] + z[35] + z[67] + z[69] + z[76] + z[84] + z[93];
	z[5] = z[40] + z[5] + z[51] + z[65];
	z[6] = 0.5*epsilon;
	z[7] = z[20]*z[6];
	z[9] = z[16]*z[6];
	z[10] = z[21]*z[6];
	z[8] = z[6]*z[8];
	z[1] = z[1]*z[6];
	z[2] = z[2]*z[6];
	z[4] = z[4]*z[6];
	z[3] = z[3]*z[6];
	z[5] = z[5]*z[6];
	z[6] = z[10] + z[13];
	z[2] = z[13] + z[2];
	z[3] = z[13] + z[3];

	/* output */
	/* {{z3, z1, z9},
	 *  {z8, z6, z7},
	 *  {z5, z4, z2}}
	 */
	 
	/* return values */
	dUdC[0] = z[3];
	dUdC[1] = z[8];
	dUdC[2] = z[5];
	dUdC[3] = z[1];
	dUdC[4] = z[6];
	dUdC[5] = z[4];
	dUdC[6] = z[9];
	dUdC[7] = z[7];
	dUdC[8] = z[2];
}