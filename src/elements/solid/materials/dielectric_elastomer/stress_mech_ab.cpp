// Arruda-Boyce 3D + ideal DE
#include "FSDE_inc.h"
#include <math.h>
static double z[135];

/* function to compute mechanical stress */
void get_dUdCmech_ab(const double* params, const double *Xsi, const double* Cmat, double* dUdC) { 

/* common definitions */
#include "FSDE_common_defines.h"
	
	/* Stress code */
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
	z[22] = pow(Nrig,-4.);
	z[23] = pow(Nrig,-3.);
	z[24] = 1./(Nrig*Nrig);
	z[25] = 1./Nrig;
	z[26] = C33*z[1];
	z[27] = C32*z[2];
	z[28] = C33*z[3];
	z[29] = C31*z[4];
	z[30] = C32*z[5];
	z[31] = C31*z[6];
	z[12] = z[12] + z[16];
	z[10] = z[10] + z[17];
	z[14] = z[14] + z[18];
	z[16] = z[19]*z[19];
	z[17] = pow(z[19],3.);
	z[18] = pow(z[19],4.);
	z[32] = pow(z[19],5.);
	z[1] = z[1] + z[3];
	z[3] = z[26] + z[27] + z[28] + z[29] + z[30] + z[31];
	z[2] = z[2] + z[5];
	z[4] = z[4] + z[6];
	z[5] = z[11] + z[7];
	z[6] = z[15] + z[8];
	z[7] = z[13] + z[9];
	z[8] = z[12]*z[12];
	z[9] = z[10]*z[10];
	z[11] = z[14]*z[14];
	z[13] = z[1]*z[1];
	z[15] = pow(z[3],-2.6666666666666665);
	z[26] = pow(z[3],-2.3333333333333335);
	z[27] = 1./(z[3]*z[3]);
	z[28] = pow(z[3],-1.6666666666666667);
	z[29] = pow(z[3],-1.3333333333333333);
	z[30] = 1./z[3];
	z[31] = pow(z[3],-0.6666666666666666);
	z[33] = 1./sqrt(z[3]);
	z[34] = pow(z[3],-0.3333333333333333);
	z[3] = sqrt(z[3]);
	z[35] = z[2]*z[2];
	z[36] = z[4]*z[4];
	z[37] = z[5]*z[5];
	z[38] = z[6]*z[6];
	z[39] = z[7]*z[7];
	z[15] = -0.0012838589981447124*z[15]*z[22]*z[32];
	z[26] = -0.003619047619047619*z[18]*z[23]*z[26];
	z[32] = -ex*ey*z[10]*z[12]*z[27];
	z[40] = -ex*ey*z[12]*z[14]*z[27];
	z[41] = -z[12]*z[14]*z[20]*z[27];
	z[42] = -ex*ey*z[10]*z[14]*z[27];
	z[43] = -z[10]*z[14]*z[20]*z[27];
	z[44] = -0.010476190476190476*z[17]*z[24]*z[27];
	z[45] = -ex*ey*z[1]*z[12]*z[27];
	z[46] = -z[1]*z[12]*z[20]*z[27];
	z[47] = -ex*ey*z[1]*z[10]*z[27];
	z[48] = -z[1]*z[10]*z[20]*z[27];
	z[49] = -z[1]*z[14]*z[20]*z[27];
	z[50] = -ex*ey*z[12]*z[2]*z[27];
	z[51] = -ex*ey*z[10]*z[2]*z[27];
	z[52] = -ex*ey*z[14]*z[2]*z[27];
	z[53] = -z[14]*z[2]*z[20]*z[27];
	z[54] = -ex*ey*z[1]*z[2]*z[27];
	z[55] = -z[1]*z[2]*z[20]*z[27];
	z[56] = -ex*ey*z[12]*z[27]*z[4];
	z[57] = -z[12]*z[20]*z[27]*z[4];
	z[58] = -ex*ey*z[10]*z[27]*z[4];
	z[59] = -z[10]*z[20]*z[27]*z[4];
	z[60] = -z[14]*z[20]*z[27]*z[4];
	z[61] = -z[1]*z[20]*z[27]*z[4];
	z[62] = -ex*ey*z[2]*z[27]*z[4];
	z[63] = -z[2]*z[20]*z[27]*z[4];
	z[64] = -ex*ey*z[12]*z[27]*z[5];
	z[65] = -ex*ey*z[10]*z[27]*z[5];
	z[66] = -ex*ey*z[14]*z[27]*z[5];
	z[67] = -z[14]*z[20]*z[27]*z[5];
	z[68] = -ex*ey*z[1]*z[27]*z[5];
	z[69] = -z[1]*z[20]*z[27]*z[5];
	z[70] = -ex*ey*z[2]*z[27]*z[5];
	z[71] = -ex*ey*z[27]*z[4]*z[5];
	z[72] = -z[20]*z[27]*z[4]*z[5];
	z[73] = -ex*ey*z[12]*z[27]*z[6];
	z[74] = -z[12]*z[21]*z[27]*z[6];
	z[75] = -ex*ey*z[10]*z[27]*z[6];
	z[76] = -z[10]*z[21]*z[27]*z[6];
	z[77] = -z[14]*z[20]*z[27]*z[6];
	z[78] = -z[14]*z[21]*z[27]*z[6];
	z[79] = -z[1]*z[20]*z[27]*z[6];
	z[80] = -z[1]*z[21]*z[27]*z[6];
	z[81] = -ex*ey*z[2]*z[27]*z[6];
	z[82] = -z[2]*z[21]*z[27]*z[6];
	z[83] = -z[20]*z[27]*z[4]*z[6];
	z[84] = -z[21]*z[27]*z[4]*z[6];
	z[85] = -ex*ey*z[27]*z[5]*z[6];
	z[86] = -z[21]*z[27]*z[5]*z[6];
	z[87] = -ex*ey*z[12]*z[27]*z[7];
	z[88] = -z[12]*z[20]*z[27]*z[7];
	z[89] = -ex*ey*z[10]*z[27]*z[7];
	z[90] = -z[10]*z[20]*z[27]*z[7];
	z[91] = -z[14]*z[20]*z[27]*z[7];
	z[92] = -z[1]*z[20]*z[27]*z[7];
	z[93] = -ex*ey*z[2]*z[27]*z[7];
	z[94] = -z[2]*z[20]*z[27]*z[7];
	z[95] = -z[20]*z[27]*z[4]*z[7];
	z[96] = -ex*ey*z[27]*z[5]*z[7];
	z[97] = -z[20]*z[27]*z[5]*z[7];
	z[98] = -z[20]*z[27]*z[6]*z[7];
	z[99] = -z[21]*z[27]*z[6]*z[7];
	z[8] = -ex*ey*z[27]*z[8];
	z[9] = -ex*ey*z[27]*z[9];
	z[11] = -z[11]*z[20]*z[27];
	z[13] = -z[13]*z[20]*z[27];
	z[100] = -0.03333333333333333*z[16]*z[25]*z[28];
	z[18] = 0.0038515769944341373*z[18]*z[22]*z[28];
	z[22] = -0.16666666666666666*z[19]*z[29];
	z[17] = 0.010857142857142857*z[17]*z[23]*z[29];
	z[23] = -C11*ex*ey*z[30];
	z[28] = -C12*ex*ey*z[30];
	z[29] = C12*ex*ey*z[30];
	z[101] = C13*ex*ey*z[30];
	z[102] = -C21*ex*ey*z[30];
	z[103] = C21*ex*ey*z[30];
	z[104] = -C23*ex*ey*z[30];
	z[105] = C23*ex*ey*z[30];
	z[106] = C31*ex*ey*z[30];
	z[107] = -C32*ex*ey*z[30];
	z[108] = C32*ex*ey*z[30];
	z[109] = -C33*ex*ey*z[30];
	z[110] = C11*z[20]*z[30];
	z[111] = -C12*z[20]*z[30];
	z[112] = C12*z[20]*z[30];
	z[113] = -C13*z[20]*z[30];
	z[114] = -C21*z[20]*z[30];
	z[115] = C21*z[20]*z[30];
	z[116] = -C22*z[20]*z[30];
	z[117] = C22*z[20]*z[30];
	z[118] = -C23*z[20]*z[30];
	z[119] = C23*z[20]*z[30];
	z[120] = -C31*z[20]*z[30];
	z[121] = -C32*z[20]*z[30];
	z[122] = C32*z[20]*z[30];
	z[123] = C33*z[20]*z[30];
	z[124] = C11*z[21]*z[30];
	z[125] = -C13*z[21]*z[30];
	z[126] = -C31*z[21]*z[30];
	z[127] = C33*z[21]*z[30];
	z[128] = ex*ey*z[12]*z[30];
	z[129] = ex*ey*z[10]*z[30];
	z[130] = z[14]*z[20]*z[30];
	z[16] = 0.03142857142857143*z[16]*z[24]*z[30];
	z[24] = z[1]*z[20]*z[30];
	z[131] = ex*ey*z[2]*z[30];
	z[132] = z[20]*z[30]*z[4];
	z[133] = ex*ey*z[30]*z[5];
	z[134] = z[21]*z[30]*z[6];
	z[30] = z[20]*z[30]*z[7];
	z[19] = 0.1*z[19]*z[25]*z[31];
	z[25] = 0.5*z[34];
	z[31] = -1. + z[3];
	z[34] = -ex*ey*z[27]*z[35];
	z[35] = -z[20]*z[27]*z[36];
	z[36] = -ex*ey*z[27]*z[37];
	z[21] = -z[21]*z[27]*z[38];
	z[20] = -z[20]*z[27]*z[39];
	z[24] = z[128] + z[129] + z[130] + z[131] + z[132] + z[133] + z[134] + z[24] + z[30];
	z[16] = z[16] + z[17] + z[18] + z[19] + z[25];
	z[17] = z[110] + z[113] + z[73] + z[75] + z[77] + z[79] + z[81] + z[83] + z[85] + z[98];
	z[18] = z[103] + z[108] + z[87] + z[89] + z[91] + z[92] + z[93] + z[95] + z[96] + z[99];
	z[17] = z[120] + z[123] + z[17] + z[21];
	z[18] = z[116] + z[126] + z[18] + z[20];
	z[19] = 0.5*kappa*z[31]*z[33];
	z[15] = z[100] + z[15] + z[22] + z[26] + z[44];
	z[8] = z[101] + z[109] + z[111] + z[122] + z[32] + z[41] + z[46] + z[74] + z[8] + z[88];
	z[8] = z[50] + z[57] + z[64] + z[8];
	z[9] = z[106] + z[109] + z[114] + z[119] + z[32] + z[43] + z[48] + z[76] + z[9] + z[90];
	z[9] = z[51] + z[59] + z[65] + z[9];
	z[11] = z[104] + z[107] + z[11] + z[117] + z[127] + z[40] + z[42] + z[49] + z[78] + z[91];
	z[11] = z[11] + z[52] + z[60] + z[66];
	z[13] = z[102] + z[117] + z[124] + z[13] + z[28] + z[45] + z[47] + z[49] + z[80] + z[92];
	z[13] = z[13] + z[54] + z[61] + z[68];
	z[20] = z[101] + z[115] + z[118] + z[23] + z[34] + z[50] + z[51] + z[53] + z[82] + z[94];
	z[20] = z[20] + z[55] + z[63] + z[70];
	z[21] = z[106] + z[112] + z[121] + z[23] + z[36] + z[64] + z[65] + z[72] + z[86] + z[97];
	z[21] = z[21] + z[67] + z[69] + z[70];
	z[22] = z[105] + z[116] + z[125] + z[29] + z[35] + z[56] + z[58] + z[60] + z[84] + z[95];
	z[22] = z[22] + z[61] + z[62] + z[71];
	z[16] = mu*z[16];
	z[15] = mu*z[15];
	z[23] = -0.25*epsilon*z[12]*z[24]*z[33];
	z[25] = -0.25*epsilon*z[10]*z[24]*z[33];
	z[26] = -0.25*epsilon*z[14]*z[24]*z[33];
	z[27] = -0.25*epsilon*z[1]*z[24]*z[33];
	z[28] = -0.25*epsilon*z[2]*z[24]*z[33];
	z[29] = -0.25*epsilon*z[24]*z[33]*z[4];
	z[30] = -0.25*epsilon*z[24]*z[33]*z[5];
	z[31] = -0.25*epsilon*z[24]*z[33]*z[6];
	z[24] = -0.25*epsilon*z[24]*z[33]*z[7];
	z[17] = -0.5*epsilon*z[17]*z[3];
	z[18] = -0.5*epsilon*z[18]*z[3];
	z[8] = -0.5*epsilon*z[3]*z[8];
	z[9] = -0.5*epsilon*z[3]*z[9];
	z[11] = -0.5*epsilon*z[11]*z[3];
	z[13] = -0.5*epsilon*z[13]*z[3];
	z[20] = -0.5*epsilon*z[20]*z[3];
	z[21] = -0.5*epsilon*z[21]*z[3];
	z[3] = -0.5*epsilon*z[22]*z[3];
	z[15] = z[15] + z[19];
	z[12] = z[12]*z[15];
	z[10] = z[10]*z[15];
	z[14] = z[14]*z[15];
	z[1] = z[1]*z[15];
	z[2] = z[15]*z[2];
	z[4] = z[15]*z[4];
	z[5] = z[15]*z[5];
	z[6] = z[15]*z[6];
	z[7] = z[15]*z[7];
	z[8] = z[12] + z[23] + z[8];
	z[9] = z[10] + z[25] + z[9];
	z[10] = z[11] + z[14] + z[16] + z[26];
	z[1] = z[1] + z[13] + z[16] + z[27];
	z[2] = z[2] + z[20] + z[28];
	z[3] = z[29] + z[3] + z[4];
	z[4] = z[21] + z[30] + z[5];
	z[5] = z[16] + z[17] + z[31] + z[6];
	z[6] = z[18] + z[24] + z[7];
	
	/* return values */
	dUdC[0] = z[10];
	dUdC[1] = z[9];
	dUdC[2] = z[6];
	dUdC[3] = z[8];
	dUdC[4] = z[5];
	dUdC[5] = z[4];
	dUdC[6] = z[3];
	dUdC[7] = z[2];
	dUdC[8] = z[1];
}