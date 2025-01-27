/* $Id: ZBdUdC.c,v 1.1 2007-11-09 21:32:14 hspark Exp $ */
#include "ZB_inc.h"

#include <math.h>

static double z[300];

/* function to compute derivatives of the potential function wrt to the
 * internal degrees of freedom */
void ZBget_dUdC(const double* params, const double *Xsi, const double *Xa, const double *Ya, const double *Za, const double* Cmat, double* dUdC) {

/* common definitions */
#include "ZB_common_defines.h"

	z[1] = 1./sqrt(2.);
	z[2] = sqrt(2.);
	z[3] = c*c;
	z[4] = 1./(d*d);
	z[5] = d*d;
	z[6] = -rzero;
	z[7] = -1. + s;
	z[8] = 1./s;
	z[9] = sqrt(s);
	z[10] = -X2;
	z[11] = -X3;
	z[12] = -X4;
	z[13] = -X5;
	z[14] = -Y2;
	z[15] = -Y3;
	z[16] = -Y4;
	z[17] = -Y5;
	z[18] = -Z2;
	z[19] = -Z3;
	z[20] = -Z4;
	z[21] = -Z5;
	z[22] = X3 + z[10];
	z[23] = X4 + z[10];
	z[24] = X5 + z[10];
	z[10] = X1 + Xs1 + z[10];
	z[25] = X2 + z[11];
	z[26] = X4 + z[11];
	z[27] = X5 + z[11];
	z[11] = X1 + Xs1 + z[11];
	z[28] = X2 + z[12];
	z[29] = X3 + z[12];
	z[30] = X5 + z[12];
	z[12] = X1 + Xs1 + z[12];
	z[31] = X2 + z[13];
	z[32] = X3 + z[13];
	z[33] = X4 + z[13];
	z[13] = X1 + Xs1 + z[13];
	z[34] = Y3 + z[14];
	z[35] = Y4 + z[14];
	z[36] = Y5 + z[14];
	z[14] = Y1 + Ys1 + z[14];
	z[37] = Y2 + z[15];
	z[38] = Y4 + z[15];
	z[39] = Y5 + z[15];
	z[15] = Y1 + Ys1 + z[15];
	z[40] = Y2 + z[16];
	z[41] = Y3 + z[16];
	z[42] = Y5 + z[16];
	z[16] = Y1 + Ys1 + z[16];
	z[43] = Y2 + z[17];
	z[44] = Y3 + z[17];
	z[45] = Y4 + z[17];
	z[17] = Y1 + Ys1 + z[17];
	z[46] = z[19] + Z2;
	z[47] = Z2 + z[20];
	z[48] = Z2 + z[21];
	z[49] = z[18] + Z3;
	z[50] = z[20] + Z3;
	z[51] = z[21] + Z3;
	z[4] = z[3]*z[4];
	z[52] = z[18] + Z4;
	z[53] = z[19] + Z4;
	z[54] = z[21] + Z4;
	z[55] = z[18] + Z5;
	z[56] = z[19] + Z5;
	z[57] = z[20] + Z5;
	z[7] = 1./z[7];
	z[58] = 1./sqrt(z[8]);
	z[8] = sqrt(z[8]);
	z[18] = Z1 + z[18] + Zs1;
	z[19] = Z1 + z[19] + Zs1;
	z[20] = Z1 + z[20] + Zs1;
	z[21] = Z1 + z[21] + Zs1;
	z[59] = C11*z[22];
	z[60] = C12*z[22];
	z[61] = C13*z[22];
	z[62] = z[22]*z[22];
	z[63] = C11*z[23];
	z[64] = C12*z[23];
	z[65] = C13*z[23];
	z[66] = z[23]*z[23];
	z[67] = C11*z[24];
	z[68] = C12*z[24];
	z[69] = C13*z[24];
	z[70] = z[24]*z[24];
	z[71] = C11*z[10];
	z[72] = C12*z[10];
	z[73] = C13*z[10];
	z[74] = z[10]*z[10];
	z[75] = C11*z[25];
	z[76] = C12*z[25];
	z[77] = C13*z[25];
	z[78] = z[25]*z[25];
	z[79] = C11*z[26];
	z[80] = C12*z[26];
	z[81] = C13*z[26];
	z[82] = z[26]*z[26];
	z[83] = C11*z[27];
	z[84] = C12*z[27];
	z[85] = C13*z[27];
	z[86] = z[27]*z[27];
	z[87] = C11*z[11];
	z[88] = C12*z[11];
	z[89] = C13*z[11];
	z[90] = z[11]*z[11];
	z[91] = C11*z[28];
	z[92] = C12*z[28];
	z[93] = C13*z[28];
	z[94] = z[28]*z[28];
	z[95] = C11*z[29];
	z[96] = C12*z[29];
	z[97] = C13*z[29];
	z[98] = z[29]*z[29];
	z[99] = C11*z[30];
	z[100] = C12*z[30];
	z[101] = C13*z[30];
	z[102] = z[30]*z[30];
	z[103] = C11*z[12];
	z[104] = C12*z[12];
	z[105] = C13*z[12];
	z[106] = z[12]*z[12];
	z[107] = C11*z[31];
	z[108] = C12*z[31];
	z[109] = C13*z[31];
	z[110] = z[31]*z[31];
	z[111] = C11*z[32];
	z[112] = C12*z[32];
	z[113] = C13*z[32];
	z[114] = z[32]*z[32];
	z[115] = C11*z[33];
	z[116] = C12*z[33];
	z[117] = C13*z[33];
	z[118] = z[33]*z[33];
	z[119] = C11*z[13];
	z[120] = C12*z[13];
	z[121] = C13*z[13];
	z[122] = z[13]*z[13];
	z[123] = C21*z[34];
	z[124] = C22*z[34];
	z[125] = C23*z[34];
	z[126] = -z[22]*z[34];
	z[127] = z[34]*z[34];
	z[128] = C21*z[35];
	z[129] = C22*z[35];
	z[130] = C23*z[35];
	z[131] = -z[23]*z[35];
	z[132] = z[35]*z[35];
	z[133] = C21*z[36];
	z[134] = C22*z[36];
	z[135] = C23*z[36];
	z[136] = -z[24]*z[36];
	z[137] = z[36]*z[36];
	z[138] = C21*z[14];
	z[139] = C22*z[14];
	z[140] = C23*z[14];
	z[141] = z[10]*z[14];
	z[142] = z[14]*z[14];
	z[143] = C21*z[37];
	z[144] = C22*z[37];
	z[145] = C23*z[37];
	z[146] = -z[25]*z[37];
	z[147] = z[37]*z[37];
	z[148] = C21*z[38];
	z[149] = C22*z[38];
	z[150] = C23*z[38];
	z[151] = -z[26]*z[38];
	z[152] = z[38]*z[38];
	z[153] = C21*z[39];
	z[154] = C22*z[39];
	z[155] = C23*z[39];
	z[156] = -z[27]*z[39];
	z[157] = z[39]*z[39];
	z[158] = C21*z[15];
	z[159] = C22*z[15];
	z[160] = C23*z[15];
	z[161] = z[11]*z[15];
	z[162] = z[15]*z[15];
	z[163] = C21*z[40];
	z[164] = C22*z[40];
	z[165] = C23*z[40];
	z[166] = -z[28]*z[40];
	z[167] = z[40]*z[40];
	z[168] = C21*z[41];
	z[169] = C22*z[41];
	z[170] = C23*z[41];
	z[171] = -z[29]*z[41];
	z[172] = z[41]*z[41];
	z[173] = C21*z[42];
	z[174] = C22*z[42];
	z[175] = C23*z[42];
	z[176] = -z[30]*z[42];
	z[177] = z[42]*z[42];
	z[178] = C21*z[16];
	z[179] = C22*z[16];
	z[180] = C23*z[16];
	z[181] = z[12]*z[16];
	z[182] = z[16]*z[16];
	z[183] = C21*z[43];
	z[184] = C22*z[43];
	z[185] = C23*z[43];
	z[186] = -z[31]*z[43];
	z[187] = z[43]*z[43];
	z[188] = C21*z[44];
	z[189] = C22*z[44];
	z[190] = C23*z[44];
	z[191] = -z[32]*z[44];
	z[192] = z[44]*z[44];
	z[193] = C21*z[45];
	z[194] = C22*z[45];
	z[195] = C23*z[45];
	z[196] = -z[33]*z[45];
	z[197] = z[45]*z[45];
	z[198] = C21*z[17];
	z[199] = C22*z[17];
	z[200] = C23*z[17];
	z[201] = z[13]*z[17];
	z[202] = z[17]*z[17];
	z[203] = C31*z[46];
	z[204] = C32*z[46];
	z[205] = C33*z[46];
	z[206] = -z[25]*z[46];
	z[207] = -z[37]*z[46];
	z[208] = z[46]*z[46];
	z[209] = C31*z[47];
	z[210] = C32*z[47];
	z[211] = C33*z[47];
	z[212] = -z[28]*z[47];
	z[213] = -z[40]*z[47];
	z[214] = z[47]*z[47];
	z[215] = C31*z[48];
	z[216] = C32*z[48];
	z[217] = C33*z[48];
	z[218] = -z[31]*z[48];
	z[219] = -z[43]*z[48];
	z[220] = z[48]*z[48];
	z[221] = C31*z[49];
	z[222] = C32*z[49];
	z[223] = C33*z[49];
	z[224] = -z[22]*z[49];
	z[225] = -z[34]*z[49];
	z[226] = z[49]*z[49];
	z[227] = C31*z[50];
	z[228] = C32*z[50];
	z[229] = C33*z[50];
	z[230] = -z[29]*z[50];
	z[231] = -z[41]*z[50];
	z[232] = z[50]*z[50];
	z[233] = C31*z[51];
	z[234] = C32*z[51];
	z[235] = C33*z[51];
	z[236] = -z[32]*z[51];
	z[237] = -z[44]*z[51];
	z[238] = z[51]*z[51];
	z[239] = C31*z[52];
	z[240] = C32*z[52];
	z[241] = C33*z[52];
	z[242] = -z[23]*z[52];
	z[243] = -z[35]*z[52];
	z[244] = z[52]*z[52];
	z[245] = C31*z[53];
	z[246] = C32*z[53];
	z[247] = C33*z[53];
	z[248] = -z[26]*z[53];
	z[249] = -z[38]*z[53];
	z[250] = z[53]*z[53];
	z[251] = C31*z[54];
	z[252] = C32*z[54];
	z[253] = C33*z[54];
	z[254] = -z[33]*z[54];
	z[255] = -z[45]*z[54];
	z[256] = z[54]*z[54];
	z[257] = C31*z[55];
	z[258] = C32*z[55];
	z[259] = C33*z[55];
	z[260] = -z[24]*z[55];
	z[261] = -z[36]*z[55];
	z[262] = z[55]*z[55];
	z[263] = C31*z[56];
	z[264] = C32*z[56];
	z[265] = C33*z[56];
	z[266] = -z[27]*z[56];
	z[267] = -z[39]*z[56];
	z[268] = z[56]*z[56];
	z[269] = C31*z[57];
	z[270] = C32*z[57];
	z[271] = C33*z[57];
	z[272] = -z[30]*z[57];
	z[273] = -z[42]*z[57];
	z[274] = z[57]*z[57];
	z[275] = C31*z[18];
	z[276] = C32*z[18];
	z[277] = C33*z[18];
	z[278] = z[10]*z[18];
	z[279] = z[14]*z[18];
	z[280] = z[18]*z[18];
	z[281] = C31*z[19];
	z[282] = C32*z[19];
	z[283] = C33*z[19];
	z[284] = z[11]*z[19];
	z[285] = z[15]*z[19];
	z[286] = z[19]*z[19];
	z[287] = C31*z[20];
	z[288] = C32*z[20];
	z[289] = C33*z[20];
	z[290] = z[12]*z[20];
	z[291] = z[16]*z[20];
	z[292] = z[20]*z[20];
	z[293] = C31*z[21];
	z[294] = C32*z[21];
	z[295] = C33*z[21];
	z[296] = z[13]*z[21];
	z[297] = z[17]*z[21];
	z[298] = z[21]*z[21];
	z[86] = -z[86];
	z[94] = -z[94];
	z[98] = -z[98];
	z[102] = -z[102];
	z[110] = -z[110];
	z[114] = -z[114];
	z[118] = -z[118];
	z[127] = -z[127];
	z[132] = -z[132];
	z[137] = -z[137];
	z[147] = -z[147];
	z[152] = -z[152];
	z[157] = -z[157];
	z[299] = z[141] + z[161];
	z[167] = -z[167];
	z[172] = -z[172];
	z[177] = -z[177];
	z[131] = z[131] + z[141] + z[181];
	z[151] = z[151] + z[161] + z[181];
	z[166] = z[141] + z[166] + z[181];
	z[171] = z[161] + z[171] + z[181];
	z[187] = -z[187];
	z[192] = -z[192];
	z[197] = -z[197];
	z[136] = z[136] + z[141] + z[201];
	z[156] = z[156] + z[161] + z[201];
	z[176] = z[176] + z[181] + z[201];
	z[186] = z[141] + z[186] + z[201];
	z[191] = z[161] + z[191] + z[201];
	z[196] = z[181] + z[196] + z[201];
	z[208] = -z[208];
	z[91] = z[163] + z[209] + z[91];
	z[92] = z[164] + z[210] + z[92];
	z[93] = z[165] + z[211] + z[93];
	z[163] = -z[214];
	z[107] = z[107] + z[183] + z[215];
	z[108] = z[108] + z[184] + z[216];
	z[109] = z[109] + z[185] + z[217];
	z[164] = -z[220];
	z[165] = -z[226];
	z[95] = z[168] + z[227] + z[95];
	z[96] = z[169] + z[228] + z[96];
	z[97] = z[170] + z[229] + z[97];
	z[168] = -z[232];
	z[111] = z[111] + z[188] + z[233];
	z[112] = z[112] + z[189] + z[234];
	z[113] = z[113] + z[190] + z[235];
	z[169] = -z[238];
	z[170] = -z[244];
	z[183] = -z[250];
	z[115] = z[115] + z[193] + z[251];
	z[116] = z[116] + z[194] + z[252];
	z[117] = z[117] + z[195] + z[253];
	z[184] = -z[256];
	z[185] = -z[262];
	z[85] = z[155] + z[265] + z[85];
	z[155] = -z[268];
	z[99] = z[173] + z[269] + z[99];
	z[100] = z[100] + z[174] + z[270];
	z[101] = z[101] + z[175] + z[271];
	z[173] = -z[274];
	z[87] = z[158] + z[281] + z[87];
	z[88] = z[159] + z[282] + z[88];
	z[89] = z[160] + z[283] + z[89];
	z[158] = z[206] + z[278] + z[284];
	z[159] = z[224] + z[278] + z[284];
	z[160] = z[207] + z[279] + z[285];
	z[174] = z[225] + z[279] + z[285];
	z[103] = z[103] + z[178] + z[287];
	z[104] = z[104] + z[179] + z[288];
	z[105] = z[105] + z[180] + z[289];
	z[175] = z[212] + z[278] + z[290];
	z[178] = z[242] + z[278] + z[290];
	z[179] = z[230] + z[284] + z[290];
	z[180] = z[248] + z[284] + z[290];
	z[188] = z[213] + z[279] + z[291];
	z[189] = z[243] + z[279] + z[291];
	z[190] = z[231] + z[285] + z[291];
	z[193] = z[249] + z[285] + z[291];
	z[119] = z[119] + z[198] + z[293];
	z[120] = z[120] + z[199] + z[294];
	z[121] = z[121] + z[200] + z[295];
	z[194] = z[218] + z[278] + z[296];
	z[195] = z[260] + z[278] + z[296];
	z[198] = z[236] + z[284] + z[296];
	z[199] = z[266] + z[284] + z[296];
	z[200] = z[254] + z[290] + z[296];
	z[206] = z[272] + z[290] + z[296];
	z[207] = z[219] + z[279] + z[297];
	z[209] = z[261] + z[279] + z[297];
	z[210] = z[237] + z[285] + z[297];
	z[211] = z[267] + z[285] + z[297];
	z[212] = z[255] + z[291] + z[297];
	z[213] = z[273] + z[291] + z[297];
	z[126] = z[126] + z[299];
	z[146] = z[146] + z[299];
	z[59] = z[123] + z[221] + z[59];
	z[60] = z[124] + z[222] + z[60];
	z[61] = z[125] + z[223] + z[61];
	z[62] = -z[62];
	z[63] = z[128] + z[239] + z[63];
	z[64] = z[129] + z[240] + z[64];
	z[65] = z[130] + z[241] + z[65];
	z[66] = -z[66];
	z[67] = z[133] + z[257] + z[67];
	z[68] = z[134] + z[258] + z[68];
	z[69] = z[135] + z[259] + z[69];
	z[70] = -z[70];
	z[71] = z[138] + z[275] + z[71];
	z[72] = z[139] + z[276] + z[72];
	z[73] = z[140] + z[277] + z[73];
	z[75] = z[143] + z[203] + z[75];
	z[76] = z[144] + z[204] + z[76];
	z[77] = z[145] + z[205] + z[77];
	z[78] = -z[78];
	z[79] = z[148] + z[245] + z[79];
	z[80] = z[149] + z[246] + z[80];
	z[81] = z[150] + z[247] + z[81];
	z[82] = -z[82];
	z[83] = z[153] + z[263] + z[83];
	z[84] = z[154] + z[264] + z[84];
	z[123] = z[122] + z[90];
	z[98] = z[106] + z[90] + z[98];
	z[102] = z[102] + z[106] + z[122];
	z[118] = z[106] + z[118] + z[122];
	z[124] = z[127] + z[142] + z[162];
	z[125] = z[132] + z[142] + z[182];
	z[127] = z[137] + z[142] + z[202];
	z[128] = z[142] + z[147] + z[162];
	z[129] = z[152] + z[162] + z[182];
	z[130] = z[157] + z[162] + z[202];
	z[132] = z[142] + z[167] + z[182];
	z[133] = z[162] + z[172] + z[182];
	z[134] = z[177] + z[182] + z[202];
	z[135] = z[142] + z[187] + z[202];
	z[137] = z[162] + z[192] + z[202];
	z[138] = z[182] + z[197] + z[202];
	z[139] = z[208] + z[280] + z[286];
	z[28] = -z[28]*z[91];
	z[91] = z[163] + z[280] + z[292];
	z[31] = -z[107]*z[31];
	z[107] = z[164] + z[280] + z[298];
	z[140] = z[165] + z[280] + z[286];
	z[29] = -z[29]*z[95];
	z[95] = z[168] + z[286] + z[292];
	z[32] = -z[111]*z[32];
	z[111] = z[169] + z[286] + z[298];
	z[143] = z[170] + z[280] + z[292];
	z[144] = z[183] + z[286] + z[292];
	z[33] = -z[115]*z[33];
	z[115] = z[184] + z[292] + z[298];
	z[145] = z[185] + z[280] + z[298];
	z[147] = z[155] + z[286] + z[298];
	z[30] = -z[30]*z[99];
	z[99] = z[173] + z[292] + z[298];
	z[11] = z[11]*z[87];
	z[12] = z[103]*z[12];
	z[13] = z[119]*z[13];
	z[22] = -z[22]*z[59];
	z[34] = -z[34]*z[60];
	z[23] = -z[23]*z[63];
	z[35] = -z[35]*z[64];
	z[24] = -z[24]*z[67];
	z[36] = -z[36]*z[68];
	z[10] = z[10]*z[71];
	z[14] = z[14]*z[72];
	z[25] = -z[25]*z[75];
	z[37] = -z[37]*z[76];
	z[26] = -z[26]*z[79];
	z[38] = -z[38]*z[80];
	z[59] = z[106] + z[82] + z[90];
	z[27] = -z[27]*z[83];
	z[60] = z[123] + z[86];
	z[63] = z[114] + z[123];
	z[39] = -z[39]*z[84];
	z[15] = z[15]*z[88];
	z[40] = -z[40]*z[92];
	z[41] = -z[41]*z[96];
	z[42] = -z[100]*z[42];
	z[16] = z[104]*z[16];
	z[43] = -z[108]*z[43];
	z[44] = -z[112]*z[44];
	z[45] = -z[116]*z[45];
	z[17] = z[120]*z[17];
	z[46] = -z[46]*z[77];
	z[47] = -z[47]*z[93];
	z[48] = -z[109]*z[48];
	z[49] = -z[49]*z[61];
	z[50] = -z[50]*z[97];
	z[51] = -z[113]*z[51];
	z[52] = -z[52]*z[65];
	z[53] = -z[53]*z[81];
	z[54] = -z[117]*z[54];
	z[55] = -z[55]*z[69];
	z[56] = -z[56]*z[85];
	z[57] = -z[101]*z[57];
	z[18] = z[18]*z[73];
	z[19] = z[19]*z[89];
	z[20] = z[105]*z[20];
	z[21] = z[121]*z[21];
	z[61] = z[106] + z[74] + z[94];
	z[64] = z[110] + z[122] + z[74];
	z[62] = z[62] + z[74] + z[90];
	z[65] = z[106] + z[66] + z[74];
	z[66] = z[122] + z[70] + z[74];
	z[67] = z[74] + z[78] + z[90];
	z[10] = z[10] + z[14] + z[18];
	z[11] = z[11] + z[15] + z[19];
	z[12] = z[12] + z[16] + z[20];
	z[13] = z[13] + z[17] + z[21];
	z[14] = z[10] + z[11] + z[25] + z[37] + z[46];
	z[15] = z[10] + z[11] + z[22] + z[34] + z[49];
	z[16] = z[10] + z[12] + z[28] + z[40] + z[47];
	z[17] = z[10] + z[12] + z[23] + z[35] + z[52];
	z[18] = z[11] + z[12] + z[29] + z[41] + z[50];
	z[19] = z[11] + z[12] + z[26] + z[38] + z[53];
	z[20] = z[10] + z[13] + z[31] + z[43] + z[48];
	z[21] = z[10] + z[13] + z[24] + z[36] + z[55];
	z[22] = z[11] + z[13] + z[32] + z[44] + z[51];
	z[23] = z[11] + z[13] + z[27] + z[39] + z[56];
	z[24] = z[12] + z[13] + z[33] + z[45] + z[54];
	z[25] = z[12] + z[13] + z[30] + z[42] + z[57];
	z[26] = pow(z[10],-1.5);
	z[27] = 1./sqrt(z[10]);
	z[10] = sqrt(z[10]);
	z[28] = pow(z[11],-1.5);
	z[29] = 1./sqrt(z[11]);
	z[11] = sqrt(z[11]);
	z[30] = pow(z[12],-1.5);
	z[31] = 1./sqrt(z[12]);
	z[12] = sqrt(z[12]);
	z[32] = pow(z[13],-1.5);
	z[33] = 1./sqrt(z[13]);
	z[13] = sqrt(z[13]);
	z[34] = -0.25*z[14];
	z[35] = -0.25*z[27]*z[28];
	z[36] = -0.25*z[141]*z[15]*z[26]*z[29];
	z[37] = -0.25*z[142]*z[15]*z[26]*z[29];
	z[38] = -0.25*z[15]*z[26]*z[278]*z[29];
	z[39] = -0.25*z[15]*z[26]*z[279]*z[29];
	z[40] = -0.25*z[15]*z[26]*z[280]*z[29];
	z[41] = 0.5*z[158]*z[27]*z[29];
	z[42] = 0.5*z[159]*z[27]*z[29];
	z[43] = 0.5*z[160]*z[27]*z[29];
	z[44] = 0.5*z[174]*z[27]*z[29];
	z[45] = 0.5*z[126]*z[27]*z[29];
	z[46] = 0.5*z[146]*z[27]*z[29];
	z[47] = 0.5*z[124]*z[27]*z[29];
	z[48] = 0.5*z[128]*z[27]*z[29];
	z[49] = 0.5*z[139]*z[27]*z[29];
	z[50] = 0.5*z[140]*z[27]*z[29];
	z[51] = 0.5*z[27]*z[29]*z[62];
	z[52] = 0.5*z[27]*z[29]*z[67];
	z[14] = 0.5*z[14]*z[27]*z[29];
	z[53] = 0.5*z[15]*z[27]*z[29];
	z[54] = -0.25*z[106]*z[16]*z[27]*z[30];
	z[55] = -0.25*z[16]*z[181]*z[27]*z[30];
	z[56] = -0.25*z[16]*z[182]*z[27]*z[30];
	z[57] = -0.25*z[16]*z[27]*z[290]*z[30];
	z[62] = -0.25*z[16]*z[27]*z[291]*z[30];
	z[67] = -0.25*z[16]*z[27]*z[292]*z[30];
	z[68] = -0.25*z[106]*z[17]*z[27]*z[30];
	z[69] = -0.25*z[17]*z[181]*z[27]*z[30];
	z[70] = -0.25*z[17]*z[182]*z[27]*z[30];
	z[71] = -0.25*z[17]*z[27]*z[290]*z[30];
	z[72] = -0.25*z[17]*z[27]*z[291]*z[30];
	z[73] = -0.25*z[17]*z[27]*z[292]*z[30];
	z[75] = -0.25*z[106]*z[18]*z[29]*z[30];
	z[76] = -0.25*z[18]*z[181]*z[29]*z[30];
	z[77] = -0.25*z[18]*z[182]*z[29]*z[30];
	z[78] = -0.25*z[18]*z[29]*z[290]*z[30];
	z[79] = -0.25*z[18]*z[29]*z[291]*z[30];
	z[80] = -0.25*z[18]*z[29]*z[292]*z[30];
	z[81] = -0.25*z[106]*z[19]*z[29]*z[30];
	z[82] = -0.25*z[181]*z[19]*z[29]*z[30];
	z[83] = -0.25*z[182]*z[19]*z[29]*z[30];
	z[84] = -0.25*z[19]*z[29]*z[290]*z[30];
	z[85] = -0.25*z[19]*z[29]*z[291]*z[30];
	z[86] = -0.25*z[19]*z[29]*z[292]*z[30];
	z[87] = -0.25*z[141]*z[16]*z[26]*z[31];
	z[88] = -0.25*z[142]*z[16]*z[26]*z[31];
	z[89] = -0.25*z[16]*z[26]*z[278]*z[31];
	z[92] = -0.25*z[16]*z[26]*z[279]*z[31];
	z[93] = -0.25*z[16]*z[26]*z[280]*z[31];
	z[94] = -0.25*z[141]*z[17]*z[26]*z[31];
	z[96] = -0.25*z[142]*z[17]*z[26]*z[31];
	z[97] = -0.25*z[17]*z[26]*z[278]*z[31];
	z[100] = -0.25*z[17]*z[26]*z[279]*z[31];
	z[101] = -0.25*z[17]*z[26]*z[280]*z[31];
	z[103] = 0.5*z[131]*z[27]*z[31];
	z[104] = 0.5*z[166]*z[27]*z[31];
	z[105] = 0.5*z[175]*z[27]*z[31];
	z[108] = 0.5*z[178]*z[27]*z[31];
	z[109] = 0.5*z[188]*z[27]*z[31];
	z[110] = 0.5*z[189]*z[27]*z[31];
	z[112] = 0.5*z[125]*z[27]*z[31];
	z[113] = 0.5*z[132]*z[27]*z[31];
	z[91] = 0.5*z[27]*z[31]*z[91];
	z[114] = 0.5*z[143]*z[27]*z[31];
	z[61] = 0.5*z[27]*z[31]*z[61];
	z[65] = 0.5*z[27]*z[31]*z[65];
	z[116] = 0.5*z[16]*z[27]*z[31];
	z[117] = 0.5*z[17]*z[27]*z[31];
	z[119] = -0.25*z[18]*z[28]*z[31]*z[90];
	z[120] = -0.25*z[161]*z[18]*z[28]*z[31];
	z[121] = -0.25*z[162]*z[18]*z[28]*z[31];
	z[123] = -0.25*z[18]*z[28]*z[284]*z[31];
	z[124] = -0.25*z[18]*z[28]*z[285]*z[31];
	z[125] = -0.25*z[18]*z[28]*z[286]*z[31];
	z[126] = -0.25*z[19]*z[28]*z[31]*z[90];
	z[128] = -0.25*z[161]*z[19]*z[28]*z[31];
	z[131] = -0.25*z[162]*z[19]*z[28]*z[31];
	z[132] = -0.25*z[19]*z[28]*z[284]*z[31];
	z[139] = -0.25*z[19]*z[28]*z[285]*z[31];
	z[140] = -0.25*z[19]*z[28]*z[286]*z[31];
	z[143] = 0.5*z[151]*z[29]*z[31];
	z[146] = 0.5*z[171]*z[29]*z[31];
	z[148] = 0.5*z[179]*z[29]*z[31];
	z[149] = 0.5*z[180]*z[29]*z[31];
	z[150] = 0.5*z[190]*z[29]*z[31];
	z[151] = 0.5*z[193]*z[29]*z[31];
	z[98] = 0.5*z[29]*z[31]*z[98];
	z[129] = 0.5*z[129]*z[29]*z[31];
	z[133] = 0.5*z[133]*z[29]*z[31];
	z[95] = 0.5*z[29]*z[31]*z[95];
	z[144] = 0.5*z[144]*z[29]*z[31];
	z[59] = 0.5*z[29]*z[31]*z[59];
	z[18] = 0.5*z[18]*z[29]*z[31];
	z[19] = 0.5*z[19]*z[29]*z[31];
	z[152] = -0.25*z[122]*z[20]*z[27]*z[32];
	z[153] = -0.25*z[20]*z[201]*z[27]*z[32];
	z[154] = -0.25*z[20]*z[202]*z[27]*z[32];
	z[155] = -0.25*z[20]*z[27]*z[296]*z[32];
	z[157] = -0.25*z[20]*z[27]*z[297]*z[32];
	z[158] = -0.25*z[20]*z[27]*z[298]*z[32];
	z[159] = -0.25*z[122]*z[21]*z[27]*z[32];
	z[160] = -0.25*z[201]*z[21]*z[27]*z[32];
	z[163] = -0.25*z[202]*z[21]*z[27]*z[32];
	z[164] = -0.25*z[21]*z[27]*z[296]*z[32];
	z[165] = -0.25*z[21]*z[27]*z[297]*z[32];
	z[166] = -0.25*z[21]*z[27]*z[298]*z[32];
	z[167] = -0.25*z[122]*z[22]*z[29]*z[32];
	z[168] = -0.25*z[201]*z[22]*z[29]*z[32];
	z[169] = -0.25*z[202]*z[22]*z[29]*z[32];
	z[170] = -0.25*z[22]*z[29]*z[296]*z[32];
	z[171] = -0.25*z[22]*z[29]*z[297]*z[32];
	z[172] = -0.25*z[22]*z[29]*z[298]*z[32];
	z[173] = -0.25*z[122]*z[23]*z[29]*z[32];
	z[174] = -0.25*z[201]*z[23]*z[29]*z[32];
	z[175] = -0.25*z[202]*z[23]*z[29]*z[32];
	z[177] = -0.25*z[23]*z[29]*z[296]*z[32];
	z[178] = -0.25*z[23]*z[29]*z[297]*z[32];
	z[179] = -0.25*z[23]*z[29]*z[298]*z[32];
	z[180] = -0.25*z[122]*z[24]*z[31]*z[32];
	z[183] = -0.25*z[201]*z[24]*z[31]*z[32];
	z[184] = -0.25*z[202]*z[24]*z[31]*z[32];
	z[185] = -0.25*z[24]*z[296]*z[31]*z[32];
	z[187] = -0.25*z[24]*z[297]*z[31]*z[32];
	z[188] = -0.25*z[24]*z[298]*z[31]*z[32];
	z[189] = -0.25*z[122]*z[25]*z[31]*z[32];
	z[190] = -0.25*z[201]*z[25]*z[31]*z[32];
	z[192] = -0.25*z[202]*z[25]*z[31]*z[32];
	z[193] = -0.25*z[25]*z[296]*z[31]*z[32];
	z[197] = -0.25*z[25]*z[297]*z[31]*z[32];
	z[32] = -0.25*z[25]*z[298]*z[31]*z[32];
	z[203] = -0.25*z[141]*z[20]*z[26]*z[33];
	z[204] = -0.25*z[142]*z[20]*z[26]*z[33];
	z[205] = -0.25*z[20]*z[26]*z[278]*z[33];
	z[208] = -0.25*z[20]*z[26]*z[279]*z[33];
	z[214] = -0.25*z[20]*z[26]*z[280]*z[33];
	z[215] = -0.25*z[141]*z[21]*z[26]*z[33];
	z[216] = -0.25*z[142]*z[21]*z[26]*z[33];
	z[217] = -0.25*z[21]*z[26]*z[278]*z[33];
	z[218] = -0.25*z[21]*z[26]*z[279]*z[33];
	z[219] = -0.25*z[21]*z[26]*z[280]*z[33];
	z[136] = 0.5*z[136]*z[27]*z[33];
	z[186] = 0.5*z[186]*z[27]*z[33];
	z[194] = 0.5*z[194]*z[27]*z[33];
	z[195] = 0.5*z[195]*z[27]*z[33];
	z[207] = 0.5*z[207]*z[27]*z[33];
	z[209] = 0.5*z[209]*z[27]*z[33];
	z[127] = 0.5*z[127]*z[27]*z[33];
	z[135] = 0.5*z[135]*z[27]*z[33];
	z[107] = 0.5*z[107]*z[27]*z[33];
	z[145] = 0.5*z[145]*z[27]*z[33];
	z[64] = 0.5*z[27]*z[33]*z[64];
	z[66] = 0.5*z[27]*z[33]*z[66];
	z[220] = 0.5*z[20]*z[27]*z[33];
	z[221] = 0.5*z[21]*z[27]*z[33];
	z[222] = -0.25*z[22]*z[28]*z[33]*z[90];
	z[223] = -0.25*z[161]*z[22]*z[28]*z[33];
	z[224] = -0.25*z[162]*z[22]*z[28]*z[33];
	z[225] = -0.25*z[22]*z[28]*z[284]*z[33];
	z[226] = -0.25*z[22]*z[28]*z[285]*z[33];
	z[227] = -0.25*z[22]*z[28]*z[286]*z[33];
	z[228] = -0.25*z[23]*z[28]*z[33]*z[90];
	z[229] = -0.25*z[161]*z[23]*z[28]*z[33];
	z[230] = -0.25*z[162]*z[23]*z[28]*z[33];
	z[231] = -0.25*z[23]*z[28]*z[284]*z[33];
	z[232] = -0.25*z[23]*z[28]*z[285]*z[33];
	z[233] = -0.25*z[23]*z[28]*z[286]*z[33];
	z[156] = 0.5*z[156]*z[29]*z[33];
	z[191] = 0.5*z[191]*z[29]*z[33];
	z[198] = 0.5*z[198]*z[29]*z[33];
	z[199] = 0.5*z[199]*z[29]*z[33];
	z[210] = 0.5*z[210]*z[29]*z[33];
	z[211] = 0.5*z[211]*z[29]*z[33];
	z[130] = 0.5*z[130]*z[29]*z[33];
	z[137] = 0.5*z[137]*z[29]*z[33];
	z[111] = 0.5*z[111]*z[29]*z[33];
	z[147] = 0.5*z[147]*z[29]*z[33];
	z[60] = 0.5*z[29]*z[33]*z[60];
	z[63] = 0.5*z[29]*z[33]*z[63];
	z[22] = 0.5*z[22]*z[29]*z[33];
	z[23] = 0.5*z[23]*z[29]*z[33];
	z[234] = -0.25*z[106]*z[24]*z[30]*z[33];
	z[235] = -0.25*z[181]*z[24]*z[30]*z[33];
	z[236] = -0.25*z[182]*z[24]*z[30]*z[33];
	z[237] = -0.25*z[24]*z[290]*z[30]*z[33];
	z[238] = -0.25*z[24]*z[291]*z[30]*z[33];
	z[239] = -0.25*z[24]*z[292]*z[30]*z[33];
	z[240] = -0.25*z[106]*z[25]*z[30]*z[33];
	z[241] = -0.25*z[181]*z[25]*z[30]*z[33];
	z[242] = -0.25*z[182]*z[25]*z[30]*z[33];
	z[243] = -0.25*z[25]*z[290]*z[30]*z[33];
	z[244] = -0.25*z[25]*z[291]*z[30]*z[33];
	z[30] = -0.25*z[25]*z[292]*z[30]*z[33];
	z[176] = 0.5*z[176]*z[31]*z[33];
	z[196] = 0.5*z[196]*z[31]*z[33];
	z[200] = 0.5*z[200]*z[31]*z[33];
	z[206] = 0.5*z[206]*z[31]*z[33];
	z[212] = 0.5*z[212]*z[31]*z[33];
	z[213] = 0.5*z[213]*z[31]*z[33];
	z[102] = 0.5*z[102]*z[31]*z[33];
	z[118] = 0.5*z[118]*z[31]*z[33];
	z[134] = 0.5*z[134]*z[31]*z[33];
	z[138] = 0.5*z[138]*z[31]*z[33];
	z[115] = 0.5*z[115]*z[31]*z[33];
	z[99] = 0.5*z[31]*z[33]*z[99];
	z[24] = 0.5*z[24]*z[31]*z[33];
	z[25] = 0.5*z[25]*z[31]*z[33];
	z[28] = z[27]*z[28]*z[34];
	z[245] = z[141]*z[26]*z[29]*z[34];
	z[246] = z[142]*z[26]*z[29]*z[34];
	z[247] = z[26]*z[278]*z[29]*z[34];
	z[248] = z[26]*z[279]*z[29]*z[34];
	z[249] = z[26]*z[280]*z[29]*z[34];
	z[250] = z[35]*z[90];
	z[251] = z[15]*z[161]*z[35];
	z[252] = z[15]*z[162]*z[35];
	z[253] = z[15]*z[284]*z[35];
	z[254] = z[15]*z[285]*z[35];
	z[35] = z[15]*z[286]*z[35];
	z[10] = z[10] + z[6];
	z[11] = z[11] + z[6];
	z[12] = z[12] + z[6];
	z[6] = z[13] + z[6];
	z[13] = z[28]*z[90];
	z[255] = z[161]*z[28];
	z[256] = z[162]*z[28];
	z[257] = z[28]*z[284];
	z[258] = z[28]*z[285];
	z[28] = z[28]*z[286];
	z[250] = z[15]*z[250];
	z[15] = -0.25*z[15]*z[26]*z[29]*z[74];
	z[16] = -0.25*z[16]*z[26]*z[31]*z[74];
	z[17] = -0.25*z[17]*z[26]*z[31]*z[74];
	z[20] = -0.25*z[20]*z[26]*z[33]*z[74];
	z[21] = -0.25*z[21]*z[26]*z[33]*z[74];
	z[26] = z[26]*z[29]*z[34]*z[74];
	z[2] = -beta*z[2];
	z[14] = h + z[14];
	z[34] = h + z[53];
	z[53] = z[103] + z[69] + z[94];
	z[55] = z[104] + z[55] + z[87];
	z[57] = z[105] + z[57] + z[89];
	z[69] = z[108] + z[71] + z[97];
	z[62] = z[109] + z[62] + z[92];
	z[71] = z[100] + z[110] + z[72];
	z[70] = z[112] + z[70] + z[96];
	z[56] = z[113] + z[56] + z[88];
	z[67] = z[67] + z[91] + z[93];
	z[72] = z[101] + z[114] + z[73];
	z[73] = h + z[116];
	z[87] = h + z[117];
	z[82] = z[128] + z[143] + z[82];
	z[76] = z[120] + z[146] + z[76];
	z[78] = z[123] + z[148] + z[78];
	z[84] = z[132] + z[149] + z[84];
	z[79] = z[124] + z[150] + z[79];
	z[85] = z[139] + z[151] + z[85];
	z[75] = z[119] + z[75] + z[98];
	z[83] = z[129] + z[131] + z[83];
	z[77] = z[121] + z[133] + z[77];
	z[80] = z[125] + z[80] + z[95];
	z[86] = z[140] + z[144] + z[86];
	z[59] = z[126] + z[59] + z[81];
	z[18] = h + z[18];
	z[19] = h + z[19];
	z[81] = z[136] + z[160] + z[215];
	z[88] = z[153] + z[186] + z[203];
	z[89] = z[155] + z[194] + z[205];
	z[91] = z[164] + z[195] + z[217];
	z[92] = z[157] + z[207] + z[208];
	z[93] = z[165] + z[209] + z[218];
	z[94] = z[127] + z[163] + z[216];
	z[95] = z[135] + z[154] + z[204];
	z[96] = z[107] + z[158] + z[214];
	z[97] = z[145] + z[166] + z[219];
	z[98] = h + z[220];
	z[100] = h + z[221];
	z[101] = z[156] + z[174] + z[229];
	z[103] = z[168] + z[191] + z[223];
	z[104] = z[170] + z[198] + z[225];
	z[105] = z[177] + z[199] + z[231];
	z[107] = z[171] + z[210] + z[226];
	z[108] = z[178] + z[211] + z[232];
	z[109] = z[130] + z[175] + z[230];
	z[110] = z[137] + z[169] + z[224];
	z[111] = z[111] + z[172] + z[227];
	z[112] = z[147] + z[179] + z[233];
	z[60] = z[173] + z[228] + z[60];
	z[63] = z[167] + z[222] + z[63];
	z[22] = h + z[22];
	z[23] = h + z[23];
	z[113] = z[176] + z[190] + z[241];
	z[114] = z[183] + z[196] + z[235];
	z[116] = z[185] + z[200] + z[237];
	z[117] = z[193] + z[206] + z[243];
	z[119] = z[187] + z[212] + z[238];
	z[120] = z[197] + z[213] + z[244];
	z[102] = z[102] + z[189] + z[240];
	z[118] = z[118] + z[180] + z[234];
	z[121] = z[134] + z[192] + z[242];
	z[123] = z[138] + z[184] + z[236];
	z[115] = z[115] + z[188] + z[239];
	z[30] = z[30] + z[32] + z[99];
	z[24] = h + z[24];
	z[25] = h + z[25];
	z[32] = z[251] + z[36] + z[45];
	z[36] = z[252] + z[37] + z[47];
	z[37] = z[253] + z[38] + z[42];
	z[38] = z[254] + z[39] + z[44];
	z[35] = z[35] + z[40] + z[50];
	z[39] = z[245] + z[255] + z[46];
	z[40] = z[246] + z[256] + z[48];
	z[41] = z[247] + z[257] + z[41];
	z[42] = z[248] + z[258] + z[43];
	z[28] = z[249] + z[28] + z[49];
	z[15] = z[15] + z[250] + z[51];
	z[16] = z[16] + z[54] + z[61];
	z[17] = z[17] + z[65] + z[68];
	z[20] = z[152] + z[20] + z[64];
	z[21] = z[159] + z[21] + z[66];
	z[13] = z[13] + z[26] + z[52];
	z[8] = z[2]*z[8];
	z[26] = z[10]*z[2];
	z[43] = z[11]*z[2];
	z[44] = z[12]*z[2];
	z[2] = z[2]*z[6];
	z[10] = z[10]*z[8];
	z[11] = z[11]*z[8];
	z[12] = z[12]*z[8];
	z[6] = z[6]*z[8];
	z[8] = z[26]*z[9];
	z[26] = z[43]*z[9];
	z[43] = z[44]*z[9];
	z[2] = z[2]*z[9];
	z[10] = exp(z[10]);
	z[11] = exp(z[11]);
	z[12] = exp(z[12]);
	z[6] = exp(z[6]);
	z[8] = exp(z[8]);
	z[26] = exp(z[26]);
	z[43] = exp(z[43]);
	z[2] = exp(z[2]);
	z[44] = z[14]*z[14];
	z[45] = z[34]*z[34];
	z[46] = z[73]*z[73];
	z[47] = z[87]*z[87];
	z[48] = z[18]*z[18];
	z[49] = z[19]*z[19];
	z[50] = z[98]*z[98];
	z[51] = z[100]*z[100];
	z[52] = z[22]*z[22];
	z[54] = z[23]*z[23];
	z[61] = z[24]*z[24];
	z[64] = z[25]*z[25];
	z[44] = z[44] + z[5];
	z[45] = z[45] + z[5];
	z[46] = z[46] + z[5];
	z[47] = z[47] + z[5];
	z[48] = z[48] + z[5];
	z[49] = z[49] + z[5];
	z[50] = z[5] + z[50];
	z[51] = z[5] + z[51];
	z[52] = z[5] + z[52];
	z[9] = -beta*dzero*z[1]*z[7]*z[9];
	z[54] = z[5] + z[54];
	z[61] = z[5] + z[61];
	z[5] = z[5] + z[64];
	z[8] = z[27]*z[8]*z[9];
	z[26] = z[26]*z[29]*z[9];
	z[43] = z[31]*z[43]*z[9];
	z[2] = z[2]*z[33]*z[9];
	z[9] = z[141]*z[8];
	z[64] = z[142]*z[8];
	z[65] = z[278]*z[8];
	z[66] = z[279]*z[8];
	z[68] = z[280]*z[8];
	z[8] = z[74]*z[8];
	z[99] = z[26]*z[90];
	z[124] = z[161]*z[26];
	z[125] = z[162]*z[26];
	z[126] = z[26]*z[284];
	z[127] = z[26]*z[285];
	z[26] = z[26]*z[286];
	z[128] = z[106]*z[43];
	z[129] = z[181]*z[43];
	z[130] = z[182]*z[43];
	z[131] = z[290]*z[43];
	z[132] = z[291]*z[43];
	z[43] = z[292]*z[43];
	z[133] = z[122]*z[2];
	z[134] = z[2]*z[201];
	z[135] = z[2]*z[202];
	z[136] = z[2]*z[296];
	z[137] = z[2]*z[297];
	z[2] = z[2]*z[298];
	z[138] = 1./(z[44]*z[44]);
	z[44] = 1./z[44];
	z[139] = 1./(z[45]*z[45]);
	z[45] = 1./z[45];
	z[140] = 1./(z[46]*z[46]);
	z[46] = 1./z[46];
	z[143] = 1./(z[47]*z[47]);
	z[47] = 1./z[47];
	z[144] = 1./(z[48]*z[48]);
	z[48] = 1./z[48];
	z[145] = 1./(z[49]*z[49]);
	z[49] = 1./z[49];
	z[146] = 1./(z[50]*z[50]);
	z[50] = 1./z[50];
	z[147] = 1./(z[51]*z[51]);
	z[51] = 1./z[51];
	z[148] = 1./(z[52]*z[52]);
	z[52] = 1./z[52];
	z[149] = 1./(z[54]*z[54]);
	z[54] = 1./z[54];
	z[150] = 1./(z[61]*z[61]);
	z[61] = 1./z[61];
	z[151] = 1./(z[5]*z[5]);
	z[5] = 1./z[5];
	z[14] = 2.*gamma*z[138]*z[14]*z[3];
	z[44] = -z[3]*z[44];
	z[32] = 2.*gamma*z[139]*z[3]*z[32]*z[34];
	z[36] = 2.*gamma*z[139]*z[3]*z[34]*z[36];
	z[37] = 2.*gamma*z[139]*z[3]*z[34]*z[37];
	z[38] = 2.*gamma*z[139]*z[3]*z[34]*z[38];
	z[35] = 2.*gamma*z[139]*z[3]*z[34]*z[35];
	z[15] = 2.*gamma*z[139]*z[15]*z[3]*z[34];
	z[34] = -z[3]*z[45];
	z[45] = 2.*gamma*z[140]*z[3]*z[55]*z[73];
	z[55] = 2.*gamma*z[140]*z[3]*z[57]*z[73];
	z[57] = 2.*gamma*z[140]*z[3]*z[62]*z[73];
	z[56] = 2.*gamma*z[140]*z[3]*z[56]*z[73];
	z[62] = 2.*gamma*z[140]*z[3]*z[67]*z[73];
	z[16] = 2.*gamma*z[140]*z[16]*z[3]*z[73];
	z[46] = -z[3]*z[46];
	z[53] = 2.*gamma*z[143]*z[3]*z[53]*z[87];
	z[67] = 2.*gamma*z[143]*z[3]*z[69]*z[87];
	z[69] = 2.*gamma*z[143]*z[3]*z[71]*z[87];
	z[70] = 2.*gamma*z[143]*z[3]*z[70]*z[87];
	z[71] = 2.*gamma*z[143]*z[3]*z[72]*z[87];
	z[17] = 2.*gamma*z[143]*z[17]*z[3]*z[87];
	z[47] = -z[3]*z[47];
	z[72] = 2.*gamma*z[144]*z[18]*z[3]*z[76];
	z[73] = 2.*gamma*z[144]*z[18]*z[3]*z[78];
	z[76] = 2.*gamma*z[144]*z[18]*z[3]*z[79];
	z[75] = 2.*gamma*z[144]*z[18]*z[3]*z[75];
	z[77] = 2.*gamma*z[144]*z[18]*z[3]*z[77];
	z[18] = 2.*gamma*z[144]*z[18]*z[3]*z[80];
	z[48] = -z[3]*z[48];
	z[78] = 2.*gamma*z[145]*z[19]*z[3]*z[82];
	z[79] = 2.*gamma*z[145]*z[19]*z[3]*z[84];
	z[80] = 2.*gamma*z[145]*z[19]*z[3]*z[85];
	z[82] = 2.*gamma*z[145]*z[19]*z[3]*z[83];
	z[83] = 2.*gamma*z[145]*z[19]*z[3]*z[86];
	z[19] = 2.*gamma*z[145]*z[19]*z[3]*z[59];
	z[49] = -z[3]*z[49];
	z[59] = 2.*gamma*z[146]*z[3]*z[88]*z[98];
	z[84] = 2.*gamma*z[146]*z[3]*z[89]*z[98];
	z[85] = 2.*gamma*z[146]*z[3]*z[92]*z[98];
	z[86] = 2.*gamma*z[146]*z[3]*z[95]*z[98];
	z[87] = 2.*gamma*z[146]*z[3]*z[96]*z[98];
	z[20] = 2.*gamma*z[146]*z[20]*z[3]*z[98];
	z[50] = -z[3]*z[50];
	z[81] = 2.*gamma*z[100]*z[147]*z[3]*z[81];
	z[88] = 2.*gamma*z[100]*z[147]*z[3]*z[91];
	z[89] = 2.*gamma*z[100]*z[147]*z[3]*z[93];
	z[91] = 2.*gamma*z[100]*z[147]*z[3]*z[94];
	z[92] = 2.*gamma*z[100]*z[147]*z[3]*z[97];
	z[21] = 2.*gamma*z[100]*z[147]*z[21]*z[3];
	z[51] = -z[3]*z[51];
	z[93] = 2.*gamma*z[103]*z[148]*z[22]*z[3];
	z[94] = 2.*gamma*z[104]*z[148]*z[22]*z[3];
	z[95] = 2.*gamma*z[107]*z[148]*z[22]*z[3];
	z[96] = 2.*gamma*z[110]*z[148]*z[22]*z[3];
	z[97] = 2.*gamma*z[111]*z[148]*z[22]*z[3];
	z[22] = 2.*gamma*z[148]*z[22]*z[3]*z[63];
	z[52] = -z[3]*z[52];
	z[63] = 2.*gamma*z[101]*z[149]*z[23]*z[3];
	z[98] = 2.*gamma*z[105]*z[149]*z[23]*z[3];
	z[100] = 2.*gamma*z[108]*z[149]*z[23]*z[3];
	z[101] = 2.*gamma*z[109]*z[149]*z[23]*z[3];
	z[103] = 2.*gamma*z[112]*z[149]*z[23]*z[3];
	z[23] = 2.*gamma*z[149]*z[23]*z[3]*z[60];
	z[54] = -z[3]*z[54];
	z[60] = 2.*gamma*z[114]*z[150]*z[24]*z[3];
	z[104] = 2.*gamma*z[116]*z[150]*z[24]*z[3];
	z[105] = 2.*gamma*z[119]*z[150]*z[24]*z[3];
	z[107] = 2.*gamma*z[118]*z[150]*z[24]*z[3];
	z[108] = 2.*gamma*z[123]*z[150]*z[24]*z[3];
	z[24] = 2.*gamma*z[115]*z[150]*z[24]*z[3];
	z[61] = -z[3]*z[61];
	z[109] = 2.*gamma*z[113]*z[151]*z[25]*z[3];
	z[110] = 2.*gamma*z[117]*z[151]*z[25]*z[3];
	z[111] = 2.*gamma*z[120]*z[151]*z[25]*z[3];
	z[102] = 2.*gamma*z[102]*z[151]*z[25]*z[3];
	z[112] = 2.*gamma*z[121]*z[151]*z[25]*z[3];
	z[25] = 2.*gamma*z[151]*z[25]*z[3]*z[30];
	z[3] = -z[3]*z[5];
	z[5] = z[14]*z[39];
	z[30] = z[14]*z[40];
	z[39] = z[14]*z[41];
	z[40] = z[14]*z[42];
	z[28] = z[14]*z[28];
	z[13] = z[13]*z[14];
	z[14] = z[109] + z[63] + z[81];
	z[41] = z[110] + z[88] + z[98];
	z[42] = z[100] + z[111] + z[89];
	z[21] = z[102] + z[21] + z[23];
	z[23] = z[101] + z[112] + z[91];
	z[25] = z[103] + z[25] + z[92];
	z[49] = 1. + z[4] + z[49];
	z[50] = 1. + z[4] + z[50];
	z[51] = 1. + z[4] + z[51];
	z[52] = 1. + z[4] + z[52];
	z[54] = 1. + z[4] + z[54];
	z[61] = 1. + z[4] + z[61];
	z[3] = 1. + z[3] + z[4];
	z[44] = 1. + z[4] + z[44];
	z[34] = 1. + z[34] + z[4];
	z[5] = z[45] + z[5] + z[59];
	z[39] = z[39] + z[55] + z[84];
	z[40] = z[40] + z[57] + z[85];
	z[30] = z[30] + z[56] + z[86];
	z[28] = z[28] + z[62] + z[87];
	z[13] = z[13] + z[16] + z[20];
	z[16] = 1. + z[4] + z[46];
	z[20] = z[105] + z[69] + z[80];
	z[45] = z[108] + z[70] + z[82];
	z[24] = z[24] + z[71] + z[83];
	z[17] = z[107] + z[17] + z[19];
	z[19] = 1. + z[4] + z[47];
	z[32] = z[32] + z[72] + z[93];
	z[37] = z[37] + z[73] + z[94];
	z[38] = z[38] + z[76] + z[95];
	z[15] = z[15] + z[22] + z[75];
	z[22] = z[36] + z[77] + z[96];
	z[18] = z[18] + z[35] + z[97];
	z[4] = 1. + z[4] + z[48];
	z[35] = z[53] + z[60] + z[78];
	z[36] = z[104] + z[67] + z[79];
	z[46] = gamma*z[49];
	z[47] = gamma*z[50];
	z[48] = gamma*z[51];
	z[49] = gamma*z[52];
	z[50] = gamma*z[54];
	z[51] = gamma*z[61];
	z[3] = gamma*z[3];
	z[44] = gamma*z[44];
	z[34] = gamma*z[34];
	z[16] = gamma*z[16];
	z[19] = gamma*z[19];
	z[4] = gamma*z[4];
	z[3] = 1. + z[3] + z[48] + z[50];
	z[16] = 1. + z[16] + z[44] + z[47];
	z[19] = 1. + z[19] + z[46] + z[51];
	z[4] = 1. + z[34] + z[4] + z[49];
	z[34] = pow(z[3],-1.5);
	z[3] = 1./sqrt(z[3]);
	z[44] = pow(z[16],-1.5);
	z[16] = 1./sqrt(z[16]);
	z[46] = pow(z[19],-1.5);
	z[19] = 1./sqrt(z[19]);
	z[47] = pow(z[4],-1.5);
	z[4] = 1./sqrt(z[4]);
	z[7] = dzero*z[7];
	z[48] = 0.5*s*z[7];
	z[49] = z[10]*z[7];
	z[50] = beta*z[1]*z[141]*z[16]*z[27]*z[49]*z[58];
	z[51] = beta*z[1]*z[142]*z[16]*z[27]*z[49]*z[58];
	z[52] = beta*z[1]*z[16]*z[27]*z[278]*z[49]*z[58];
	z[53] = beta*z[1]*z[16]*z[27]*z[279]*z[49]*z[58];
	z[54] = beta*z[1]*z[16]*z[27]*z[280]*z[49]*z[58];
	z[10] = z[10]*z[44]*z[48];
	z[5] = z[10]*z[5];
	z[39] = z[10]*z[39];
	z[40] = z[10]*z[40];
	z[30] = z[10]*z[30];
	z[28] = z[10]*z[28];
	z[10] = z[10]*z[13];
	z[13] = z[11]*z[32]*z[47]*z[48];
	z[32] = z[11]*z[37]*z[47]*z[48];
	z[37] = z[11]*z[38]*z[47]*z[48];
	z[15] = z[11]*z[15]*z[47]*z[48];
	z[22] = z[11]*z[22]*z[47]*z[48];
	z[18] = z[11]*z[18]*z[47]*z[48];
	z[38] = beta*z[1]*z[11]*z[29]*z[4]*z[58]*z[7]*z[90];
	z[44] = beta*z[1]*z[11]*z[161]*z[29]*z[4]*z[58]*z[7];
	z[47] = beta*z[1]*z[11]*z[162]*z[29]*z[4]*z[58]*z[7];
	z[55] = beta*z[1]*z[11]*z[284]*z[29]*z[4]*z[58]*z[7];
	z[56] = beta*z[1]*z[11]*z[285]*z[29]*z[4]*z[58]*z[7];
	z[4] = beta*z[1]*z[11]*z[286]*z[29]*z[4]*z[58]*z[7];
	z[11] = z[12]*z[20]*z[46]*z[48];
	z[20] = z[12]*z[45]*z[46]*z[48];
	z[24] = z[12]*z[24]*z[46]*z[48];
	z[17] = z[12]*z[17]*z[46]*z[48];
	z[29] = z[12]*z[35]*z[46]*z[48];
	z[35] = z[12]*z[36]*z[46]*z[48];
	z[36] = beta*z[1]*z[106]*z[12]*z[19]*z[31]*z[58]*z[7];
	z[45] = beta*z[1]*z[12]*z[181]*z[19]*z[31]*z[58]*z[7];
	z[46] = beta*z[1]*z[12]*z[182]*z[19]*z[31]*z[58]*z[7];
	z[57] = beta*z[1]*z[12]*z[19]*z[290]*z[31]*z[58]*z[7];
	z[59] = beta*z[1]*z[12]*z[19]*z[291]*z[31]*z[58]*z[7];
	z[12] = beta*z[1]*z[12]*z[19]*z[292]*z[31]*z[58]*z[7];
	z[14] = z[14]*z[34]*z[48]*z[6];
	z[19] = z[34]*z[41]*z[48]*z[6];
	z[31] = z[34]*z[42]*z[48]*z[6];
	z[21] = z[21]*z[34]*z[48]*z[6];
	z[23] = z[23]*z[34]*z[48]*z[6];
	z[25] = z[25]*z[34]*z[48]*z[6];
	z[34] = beta*z[1]*z[122]*z[3]*z[33]*z[58]*z[6]*z[7];
	z[41] = beta*z[1]*z[201]*z[3]*z[33]*z[58]*z[6]*z[7];
	z[42] = beta*z[1]*z[202]*z[3]*z[33]*z[58]*z[6]*z[7];
	z[48] = beta*z[1]*z[296]*z[3]*z[33]*z[58]*z[6]*z[7];
	z[60] = beta*z[1]*z[297]*z[3]*z[33]*z[58]*z[6]*z[7];
	z[3] = beta*z[1]*z[298]*z[3]*z[33]*z[58]*z[6]*z[7];
	z[1] = beta*z[1]*z[16]*z[27]*z[49]*z[58]*z[74];
	z[1] = z[1] + z[10] + z[15] + z[17] + z[21] + z[34] + z[36] + z[38] + z[8] + z[99];
	z[1] = z[1] + z[128] + z[133];
	z[5] = z[124] + z[13] + z[14] + z[29] + z[41] + z[44] + z[45] + z[5] + z[50] + z[9];
	z[5] = z[129] + z[134] + z[5];
	z[6] = z[125] + z[20] + z[22] + z[23] + z[30] + z[42] + z[46] + z[47] + z[51] + z[64];
	z[6] = z[130] + z[135] + z[6];
	z[7] = z[126] + z[19] + z[32] + z[35] + z[39] + z[48] + z[52] + z[55] + z[57] + z[65];
	z[7] = z[131] + z[136] + z[7];
	z[8] = z[11] + z[127] + z[31] + z[37] + z[40] + z[53] + z[56] + z[59] + z[60] + z[66];
	z[8] = z[132] + z[137] + z[8];
	z[3] = z[12] + z[18] + z[24] + z[25] + z[26] + z[28] + z[3] + z[4] + z[54] + z[68];
	z[2] = z[2] + z[3] + z[43];
	z[1] = 0.5*z[1];
	z[3] = 0.5*z[5];
	z[4] = 0.5*z[6];
	z[5] = 0.5*z[7];
	z[6] = 0.5*z[8];
	z[2] = 0.5*z[2];

	/* output */
	/* {{z1, z3, z5},
	 *  {z3, z4, z6},
	 *  {z5, z6, z2}}
	 */
	 
	/* return values */
	dUdC[0] = z[1];
	dUdC[1] = z[3];
	dUdC[2] = z[5];
	dUdC[3] = z[3];
	dUdC[4] = z[4];
	dUdC[5] = z[6];
	dUdC[6] = z[5];
	dUdC[7] = z[6];
	dUdC[8] = z[2];
}
