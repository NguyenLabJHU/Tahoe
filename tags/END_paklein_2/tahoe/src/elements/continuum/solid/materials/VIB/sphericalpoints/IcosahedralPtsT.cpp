/* $Id: IcosahedralPtsT.cpp,v 1.4 2002-10-20 22:48:59 paklein Exp $ */
/* created: paklein (10/31/1997)                                          */
/* Base class for spherical point generators.                             */

#include "IcosahedralPtsT.h"

#include <math.h>
#include <iostream.h>

#include "toolboxConstants.h"
#include "ExceptionT.h"
#include "fstreamT.h"


using namespace Tahoe;

const double Pi = acos(-1.0);

/*
* Constructor
*/
IcosahedralPtsT::IcosahedralPtsT(ifstreamT& in)
{
	/* number of integration points */
	in >> fN;
}

/*
* Print parameters.
*/
void IcosahedralPtsT::Print(ostream& out) const
{
	/* number of integration points */
	out << " Number of sampling points . . . . . . . . . . . = " << fN << '\n';
}

void IcosahedralPtsT::PrintName(ostream& out) const
{
	out << "    " << fN << " Icohesadral points\n";
}

/*
* Generate sphere points:
*
*   theta = angle about z from x
*   phi   = angle about x from z
*
* The final orientation is generated by applied the
* theta and phi rotations in succession about the local
* axes.
*
*/
const dArray2DT& IcosahedralPtsT::SpherePoints(double phi_tr, double theta_tr)
{	
	/* set jacobians */
	SetJacobians(fN);

	/* set coordinates */
	SetCoords(fN);
	
	/* reorient points */
	TransformPoints(phi_tr,theta_tr);
	
	return fPoints;
}

/*
* Returns the correct data pointer for the specified number of
* integration points
*/
void IcosahedralPtsT::SetCoords(int numint)
{
	/* vertex coordinate data */
	double p6[] =
{   0.00000000000000000,   0.00000000000000000,   1.00000000000000000
,   0.89442719099991600,   0.00000000000000000,   0.44721359549995800
,   0.27639320225002110,   0.85065080835204000,   0.44721359549995800
,  -0.72360679774997880,   0.52573111211913380,   0.44721359549995800
,  -0.72360679774997900,  -0.52573111211913340,   0.44721359549995780
,   0.27639320225002080,  -0.85065080835204000,   0.44721359549995780};

	double p10[]=
{   0.49112347318842310,   0.35682208977308990,   0.79465447229176620
,  -0.18759247408507980,   0.57735026918962580,   0.79465447229176620
,  -0.60706199820668630,   0.00000000000000000,   0.79465447229176620
,  -0.18759247408508000,  -0.57735026918962570,   0.79465447229176620
,   0.49112347318842290,  -0.35682208977308990,   0.79465447229176620
,   0.79465447229176620,   0.57735026918962580,   0.18759247408507990
,  -0.30353099910334300,   0.93417235896271600,   0.18759247408507990
,  -0.98224694637684600,   0.00000000000000000,   0.18759247408507980
,  -0.30353099910334320,  -0.93417235896271600,   0.18759247408507980
,   0.79465447229176620,  -0.57735026918962600,   0.18759247408507990};
	
	double p40[] =
{  0.24299902268858990,   0.17654912424700010,   0.95382486951222200
,   0.39795726611924130,   0.65346155900311130,   0.64390838265091940
,   0.74445443210413810,   0.17654912424700010,   0.64390838265091940
,   0.49112347318842320,   0.35682208977308990,   0.79465447229176620
,  -0.09281736743403440,   0.28566248371567440,   0.95382486951222200
,  -0.49850331557245990,   0.58041057805241790,   0.64390838265091950
,   0.06214087599661696,   0.76257491847178550,   0.64390838265091950
,  -0.18759247408507980,   0.57735026918962580,   0.79465447229176620
,  -0.30036331050911080,   0.00000000000000000,   0.95382486951222200
,  -0.70604925864753640,  -0.29474809433674310,   0.64390838265091940
,  -0.70604925864753610,   0.29474809433674350,   0.64390838265091940
,  -0.60706199820668620,   0.00000000000000000,   0.79465447229176610
,  -0.09281736743403460,  -0.28566248371567440,   0.95382486951222200
,   0.06214087599661670,  -0.76257491847178550,   0.64390838265091950
,  -0.49850331557246000,  -0.58041057805241770,   0.64390838265091950
,  -0.18759247408508000,  -0.57735026918962580,   0.79465447229176620
,   0.24299902268858990,  -0.17654912424700010,   0.95382486951222200
,   0.74445443210413790,  -0.17654912424700010,   0.64390838265091940
,   0.39795726611924120,  -0.65346155900311130,   0.64390838265091950
,   0.49112347318842290,  -0.35682208977308990,   0.79465447229176620
,   0.89463608735869300,   0.28566248371567440,   0.34354507214180850
,   0.54813892137379680,   0.76257491847178550,   0.34354507214180850
,   0.79465447229176620,   0.57735026918962570,   0.18759247408507990
,   0.00477658817609604,   0.93912404271878600,   0.34354507214180860
,  -0.55586760339298070,   0.75695970229941800,   0.34354507214180860
,  -0.30353099910334300,   0.93417235896271600,   0.18759247408507990
,  -0.89168399351560500,   0.29474809433674350,   0.34354507214180850
,  -0.89168399351560500,  -0.29474809433674300,   0.34354507214180840
,  -0.98224694637684600,   0.00000000000000000,   0.18759247408507980
,  -0.55586760339298100,  -0.75695970229941780,   0.34354507214180840
,   0.00477658817609572,  -0.93912404271878600,   0.34354507214180840
,  -0.30353099910334320,  -0.93417235896271500,   0.18759247408507980
,   0.54813892137379660,  -0.76257491847178560,   0.34354507214180840
,   0.89463608735869300,  -0.28566248371567450,   0.34354507214180850
,   0.79465447229176620,  -0.57735026918962590,   0.18759247408507990
,   0.30513989868520680,   0.93912404271878600,   0.15791033727373960
,  -0.79886662608157070,   0.58041057805241800,   0.15791033727373960
,  -0.79886662608157090,  -0.58041057805241760,   0.15791033727373940
,   0.30513989868520660,  -0.93912404271878600,   0.15791033727373940
,   0.98745345479272800,   0.00000000000000000,   0.15791033727373950};

	double p160[] =
{   0.12096586950816360,   0.08788684863482780,   0.98875839326459000
,   0.20584537264337180,   0.34586394837692130,   0.91542657366630900
,   0.39254588021568750,   0.08889274521159460,   0.91542657366630900
,   0.24346541215301140,   0.17688797602751600,   0.95364313924132700
,   0.28331120027381430,   0.58427925076879000,   0.76049491840542400
,   0.34015438663862470,   0.76247973934510340,   0.55038135900366770
,   0.55350671311921260,   0.58427925076879000,   0.59350490785925910
,   0.39816485086285720,   0.65300389175506010,   0.64424426182163580
,   0.64323056436108070,   0.08889274521159460,   0.76049491840542380
,   0.72672556963416320,   0.34586394837692130,   0.59350490785925900
,   0.83027481082971200,   0.08788684863482780,   0.55038135900366760
,   0.74408331189913470,   0.17688797602751600,   0.64424426182163560
,   0.32938699098659280,   0.43690900344903300,   0.83702731907266400
,   0.60135365323832690,   0.43690900344903300,   0.66894267799422930
,   0.51731133269910940,   0.17825333709947820,   0.83702731907266400
,   0.49112347318842310,   0.35682208977308980,   0.79465447229176620
,  -0.04620485067343395,   0.14220390825526870,   0.98875839326459000
,  -0.26532644349519590,   0.30264842079177110,   0.91542657366630900
,   0.03676132347366807,   0.40080268627097140,   0.91542657366630900
,  -0.09299551235744860,   0.28621075741369760,   0.95364313924132700
,  -0.46813461319833500,   0.44999718110803010,   0.76049491840542380
,  -0.62004783846466930,   0.55912524328326230,   0.55038135900366770
,  -0.38463960792525250,   0.70696838427335690,   0.59350490785925900
,  -0.49800390094035570,   0.58046657591810050,   0.64424426182163580
,   0.11422715110411050,   0.63921798866283990,   0.76049491840542380
,  -0.10436561059167100,   0.79803492634896640,   0.59350490785925900
,   0.17298366645702710,   0.81679679896554400,   0.55038135900366770
,   0.06170392635239709,   0.76232667314124170,   0.64424426182163580
,  -0.31373897681733970,   0.44827795122182570,   0.83702731907266400
,  -0.22969665627812250,   0.70693361757138060,   0.66894267799422930
,  -0.00967100461303820,   0.54707562438460320,   0.83702731907266400
,  -0.18759247408507980,   0.57735026918962580,   0.79465447229176620
,  -0.14952203766945930,   0.00000000000000000,   0.98875839326459000
,  -0.36982613283753130,  -0.15881693768612640,   0.91542657366630900
,  -0.36982613283753120,   0.15881693768612660,   0.91542657366630900
,  -0.30093979959112550,   0.00000000000000000,   0.95364313924132700
,  -0.57263430254067050,  -0.30616569800238520,   0.76049491840542380
,  -0.72336502546069480,  -0.41692133502799340,   0.55038135900366770
,  -0.79122706423645190,  -0.14734876031625860,   0.59350490785925880
,  -0.70594818817403290,  -0.29425581850440270,   0.64424426182163580
,  -0.57263430254067030,   0.30616569800238550,   0.76049491840542380
,  -0.79122706423645190,   0.14734876031625900,   0.59350490785925900
,  -0.72336502546069450,   0.41692133502799380,   0.55038135900366770
,  -0.70594818817403270,   0.29425581850440300,   0.64424426182163580
,  -0.52328834225532420,  -0.15985799318677710,   0.83702731907266400
,  -0.74331399392040840,   0.00000000000000000,   0.66894267799422930
,  -0.52328834225532410,   0.15985799318677740,   0.83702731907266400
,  -0.60706199820668620,   0.00000000000000000,   0.79465447229176620
,  -0.04620485067343401,  -0.14220390825526860,   0.98875839326459000
,   0.03676132347366792,  -0.40080268627097130,   0.91542657366630900
,  -0.26532644349519600,  -0.30264842079177100,   0.91542657366630900
,  -0.09299551235744870,  -0.28621075741369750,   0.95364313924132700
,   0.11422715110411030,  -0.63921798866284000,   0.76049491840542400
,   0.17298366645702680,  -0.81679679896554400,   0.55038135900366770
,  -0.10436561059167130,  -0.79803492634896640,   0.59350490785925910
,   0.06170392635239687,  -0.76232667314124170,   0.64424426182163580
,  -0.46813461319833500,  -0.44999718110802990,   0.76049491840542380
,  -0.38463960792525270,  -0.70696838427335680,   0.59350490785925910
,  -0.62004783846466940,  -0.55912524328326210,   0.55038135900366760
,  -0.49800390094035590,  -0.58046657591810030,   0.64424426182163580
,  -0.00967100461303838,  -0.54707562438460320,   0.83702731907266400
,  -0.22969665627812270,  -0.70693361757138050,   0.66894267799422930
,  -0.31373897681733980,  -0.44827795122182560,   0.83702731907266400
,  -0.18759247408508000,  -0.57735026918962580,   0.79465447229176620
,   0.12096586950816360,  -0.08788684863482780,   0.98875839326459000
,   0.39254588021568750,  -0.08889274521159460,   0.91542657366630900
,   0.20584537264337170,  -0.34586394837692130,   0.91542657366630900
,   0.24346541215301140,  -0.17688797602751600,   0.95364313924132700
,   0.64323056436108060,  -0.08889274521159460,   0.76049491840542380
,   0.83027481082971200,  -0.08788684863482780,   0.55038135900366760
,   0.72672556963416310,  -0.34586394837692140,   0.59350490785925900
,   0.74408331189913440,  -0.17688797602751600,   0.64424426182163580
,   0.28331120027381410,  -0.58427925076879000,   0.76049491840542400
,   0.55350671311921230,  -0.58427925076879000,   0.59350490785925910
,   0.34015438663862440,  -0.76247973934510350,   0.55038135900366770
,   0.39816485086285700,  -0.65300389175506010,   0.64424426182163590
,   0.51731133269910940,  -0.17825333709947840,   0.83702731907266400
,   0.60135365323832660,  -0.43690900344903310,   0.66894267799422930
,   0.32938699098659260,  -0.43690900344903300,   0.83702731907266400
,   0.49112347318842290,  -0.35682208977309000,   0.79465447229176620
,   0.90503582966444200,   0.14220390825526860,   0.40085932133420840
,   0.80234225520503800,   0.40080268627097120,   0.44227153671750910
,   0.93744001162773700,   0.30264842079177100,   0.17207602387211100
,   0.89455321169469700,   0.28621075741369740,   0.34330446223051010
,   0.62912339869008740,   0.63921798866283990,   0.44227153671750920
,   0.41491540547335440,   0.81679679896554400,   0.40085932133420840
,   0.57752064754047080,   0.79803492634896640,   0.17207602387211100
,   0.54863475065841990,   0.76232667314124160,   0.34330446223051010
,   0.75298499853344670,   0.54707562438460310,   0.36567998740398940
,   0.70104398794679690,   0.70693361757138050,   0.09371332515225560
,   0.88896832965931400,   0.44827795122182560,   0.09371332515225560
,   0.79465447229176630,   0.57735026918962580,   0.18759247408507990
,   0.14442749829573140,   0.90468364760037200,   0.40085932133420840
,  -0.13324861436313140,   0.88692767156056100,   0.44227153671750930
,   0.00184914205956765,   0.98508193703976100,   0.17207602387211110
,   0.00422953891428299,   0.93921464916875800,   0.34330446223051020
,  -0.41352261169671290,   0.79586112948495140,   0.44227153671750940
,  -0.64860400662596490,   0.64701209191809020,   0.40085932133420840
,  -0.58051262224287780,   0.79586112948495150,   0.17207602387211110
,  -0.55547828837846990,   0.75735455194561660,   0.34330446223051020
,  -0.28761467642098680,   0.88518695467085900,   0.36567998740398960
,  -0.45569931749942140,   0.88518695467085900,   0.09371332515225560
,  -0.15163134529511980,   0.98398462783363600,   0.09371332515225560
,  -0.30353099910334300,   0.93417235896271600,   0.18759247408508000
,  -0.81577472680756200,   0.41692133502799380,   0.40085932133420840
,  -0.88469442783528100,   0.14734876031625910,   0.44227153671750920
,  -0.93629717898489700,   0.30616569800238550,   0.17207602387211100
,  -0.89193921288893000,   0.29425581850440310,   0.34330446223051010
,  -0.88469442783528100,  -0.14734876031625860,   0.44227153671750920
,  -0.81577472680756300,  -0.41692133502799340,   0.40085932133420830
,  -0.93629717898489800,  -0.30616569800238520,   0.17207602387211090
,  -0.89193921288893000,  -0.29425581850440270,   0.34330446223051000
,  -0.93074064422492000,   0.00000000000000000,   0.36567998740398940
,  -0.98268165481156900,  -0.15985799318677710,   0.09371332515225550
,  -0.98268165481156900,   0.15985799318677740,   0.09371332515225550
,  -0.98224694637684600,   0.00000000000000000,   0.18759247408507980
,  -0.64860400662596520,  -0.64701209191809000,   0.40085932133420830
,  -0.41352261169671320,  -0.79586112948495130,   0.44227153671750920
,  -0.58051262224287790,  -0.79586112948495130,   0.17207602387211090
,  -0.55547828837847000,  -0.75735455194561660,   0.34330446223051010
,  -0.13324861436313170,  -0.88692767156056100,   0.44227153671750920
,   0.14442749829573120,  -0.90468364760037200,   0.40085932133420840
,   0.00184914205956733,  -0.98508193703976100,   0.17207602387211100
,   0.00422953891428270,  -0.93921464916875800,   0.34330446223051010
,  -0.28761467642098700,  -0.88518695467085900,   0.36567998740398940
,  -0.15163134529512010,  -0.98398462783363600,   0.09371332515225550
,  -0.45569931749942160,  -0.88518695467085900,   0.09371332515225550
,  -0.30353099910334340,  -0.93417235896271600,   0.18759247408507990
,   0.41491540547335420,  -0.81679679896554500,   0.40085932133420840
,   0.62912339869008730,  -0.63921798866284000,   0.44227153671750930
,   0.57752064754047050,  -0.79803492634896660,   0.17207602387211100
,   0.54863475065841980,  -0.76232667314124180,   0.34330446223051010
,   0.80234225520503800,  -0.40080268627097150,   0.44227153671750930
,   0.90503582966444200,  -0.14220390825526870,   0.40085932133420840
,   0.93744001162773700,  -0.30264842079177120,   0.17207602387211100
,   0.89455321169469700,  -0.28621075741369760,   0.34330446223051010
,   0.75298499853344660,  -0.54707562438460330,   0.36567998740398940
,   0.88896832965931400,  -0.44827795122182600,   0.09371332515225550
,   0.70104398794679670,  -0.70693361757138060,   0.09371332515225550
,   0.79465447229176610,  -0.57735026918962590,   0.18759247408507980
,   0.45517028017018130,   0.88692767156056100,   0.07860866027328215
,   0.29394953596519070,   0.90468364760037200,   0.30844961998734050
,   0.15308251320131740,   0.98508193703976100,   0.07860866027328220
,   0.30516933850540850,   0.93921464916875800,   0.15731343751561290
,  -0.70286298961316710,   0.70696838427335690,   0.07860866027328220
,  -0.76956987613412850,   0.55912524328326230,   0.30844961998734050
,  -0.88956349718548300,   0.44999718110803010,   0.07860866027328216
,  -0.79894370053148140,   0.58046657591810060,   0.15731343751561290
,  -0.88956349718548300,  -0.44999718110802990,   0.07860866027328207
,  -0.76956987613412870,  -0.55912524328326210,   0.30844961998734030
,  -0.70286298961316730,  -0.70696838427335680,   0.07860866027328207
,  -0.79894370053148160,  -0.58046657591810040,   0.15731343751561280
,   0.15308251320131710,  -0.98508193703976100,   0.07860866027328211
,   0.29394953596519050,  -0.90468364760037200,   0.30844961998734050
,   0.45517028017018120,  -0.88692767156056100,   0.07860866027328211
,   0.30516933850540820,  -0.93921464916875800,   0.15731343751561280
,   0.98417369342715200,  -0.15881693768612660,   0.07860866027328216
,   0.95124068033787600,   0.00000000000000000,   0.30844961998734050
,   0.98417369342715200,   0.15881693768612640,   0.07860866027328213
,   0.98754872405214600,   0.00000000000000000,   0.15731343751561290};

	double *p;
	switch (numint)
	{
		case 6:
		
			p = p6;
			break;

		case 10:

			p = p10;
			break;

		case 40:
		
			p = p40;
			break;

		case 160:
		
			p = p160;
			break;
		
		default:
			
			throw ExceptionT::kGeneralFail;
	}
	
	/* temp vector */
	dArray2DT temp(numint, 3, p);
	
	/* copy in */
	fPoints = temp;
}

void IcosahedralPtsT::SetJacobians(int numint)
{
	/* vertex coordinate data */
	double p6[] =
	{2.09439510239319500,   2.09439510239319500,   2.09439510239319500,
	 2.09439510239319500,   2.09439510239319500,   2.09439510239319500};	

	double p10[]=
	{1.25663706143591700,   1.25663706143591700,
	 1.25663706143591700,   1.25663706143591700,
	 1.25663706143591700,   1.25663706143591700,
	 1.25663706143591700,   1.25663706143591700,
	 1.25663706143591700,   1.25663706143591700};
	
	double p40[] =
{0.30010415628625260,   0.30010415628625270,   0.30010415628625260,   0.35632459257715960,
	0.30010415628625260,   0.30010415628625260,   0.30010415628625270,   0.35632459257715960,
	0.30010415628625260,   0.30010415628625270,   0.30010415628625260,   0.35632459257715960,
	0.30010415628625260,   0.30010415628625270,   0.30010415628625270,   0.35632459257715960,
	0.30010415628625260,   0.30010415628625270,   0.30010415628625290,   0.35632459257715960,
	0.30010415628625260,   0.30010415628625270,   0.35632459257715960,   0.30010415628625270,
	0.30010415628625260,   0.35632459257715960,   0.30010415628625270,   0.30010415628625270,
	0.35632459257715960,   0.30010415628625270,   0.30010415628625270,   0.35632459257715960,
	0.30010415628625270,   0.30010415628625270,   0.35632459257715960,   0.30010415628625270,
	0.30010415628625270,   0.30010415628625270,   0.30010415628625270,   0.30010415628625270};

	double p160[] =
{   0.07344701572534853,   0.07427949923346247,   0.07427949923346247,   0.07726750598033433
,   0.07427949923346250,   0.07344701572534853,   0.07427949923346250,   0.07726750598033437
,   0.07427949923346246,   0.07427949923346242,   0.07344701572534851,   0.07726750598033428
,   0.08854463327822550,   0.08854463327822540,   0.08854463327822550,   0.09318260108341320
,   0.07344701572534853,   0.07427949923346250,   0.07427949923346247,   0.07726750598033437
,   0.07427949923346246,   0.07344701572534851,   0.07427949923346244,   0.07726750598033430
,   0.07427949923346247,   0.07427949923346244,   0.07344701572534853,   0.07726750598033430
,   0.08854463327822550,   0.08854463327822540,   0.08854463327822550,   0.09318260108341320
,   0.07344701572534853,   0.07427949923346242,   0.07427949923346247,   0.07726750598033430
,   0.07427949923346246,   0.07344701572534856,   0.07427949923346250,   0.07726750598033437
,   0.07427949923346250,   0.07427949923346247,   0.07344701572534851,   0.07726750598033432
,   0.08854463327822550,   0.08854463327822550,   0.08854463327822550,   0.09318260108341320
,   0.07344701572534851,   0.07427949923346243,   0.07427949923346242,   0.07726750598033430
,   0.07427949923346247,   0.07344701572534856,   0.07427949923346250,   0.07726750598033437
,   0.07427949923346246,   0.07427949923346246,   0.07344701572534853,   0.07726750598033437
,   0.08854463327822550,   0.08854463327822550,   0.08854463327822550,   0.09318260108341320
,   0.07344701572534853,   0.07427949923346247,   0.07427949923346244,   0.07726750598033432
,   0.07427949923346250,   0.07344701572534853,   0.07427949923346251,   0.07726750598033437
,   0.07427949923346247,   0.07427949923346250,   0.07344701572534860,   0.07726750598033437
,   0.08854463327822550,   0.08854463327822550,   0.08854463327822550,   0.09318260108341320
,   0.07344701572534849,   0.07427949923346244,   0.07427949923346244,   0.07726750598033430
,   0.07427949923346246,   0.07344701572534851,   0.07427949923346247,   0.07726750598033430
,   0.08854463327822550,   0.08854463327822550,   0.08854463327822540,   0.09318260108341320
,   0.07344701572534853,   0.07427949923346246,   0.07427949923346247,   0.07726750598033432
,   0.07427949923346243,   0.07344701572534853,   0.07427949923346247,   0.07726750598033430
,   0.08854463327822540,   0.08854463327822550,   0.08854463327822550,   0.09318260108341320
,   0.07344701572534850,   0.07427949923346247,   0.07427949923346247,   0.07726750598033432
,   0.07427949923346246,   0.07344701572534853,   0.07427949923346244,   0.07726750598033430
,   0.08854463327822550,   0.08854463327822540,   0.08854463327822540,   0.09318260108341320
,   0.07344701572534853,   0.07427949923346243,   0.07427949923346242,   0.07726750598033428
,   0.07427949923346246,   0.07344701572534856,   0.07427949923346247,   0.07726750598033433
,   0.08854463327822550,   0.08854463327822540,   0.08854463327822540,   0.09318260108341320
,   0.07344701572534860,   0.07427949923346247,   0.07427949923346250,   0.07726750598033437
,   0.07427949923346250,   0.07344701572534850,   0.07427949923346246,   0.07726750598033430
,   0.08854463327822550,   0.08854463327822550,   0.08854463327822550,   0.09318260108341320
,   0.07427949923346246,   0.07344701572534853,   0.07427949923346246,   0.07726750598033432
,   0.07427949923346250,   0.07344701572534851,   0.07427949923346246,   0.07726750598033430
,   0.07427949923346246,   0.07344701572534858,   0.07427949923346246,   0.07726750598033437
,   0.07427949923346250,   0.07344701572534856,   0.07427949923346247,   0.07726750598033437
,   0.07427949923346247,   0.07344701572534853,   0.07427949923346246,   0.07726750598033430};	

	double *p;
	switch (numint)
	{
		case 6:
		
			p = p6;
			break;

		case 10:

			p = p10;
			break;

		case 40:
		
			p = p40;
			break;

		case 160:
		
			p = p160;
			break;
		
		default:
			
			throw ExceptionT::kGeneralFail;
	}
	
	/* temp vector */
	dArrayT temp(numint,p);
	
	/* copy in */
	fJacobians = temp;
}
