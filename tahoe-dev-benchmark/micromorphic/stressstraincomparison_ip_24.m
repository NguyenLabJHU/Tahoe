
close all
clear all

format short e


 sim=load('el_cube99M_721.txt');
% 
time=sim(:,1);
s11=sim(:,578);
s22=sim(:,579);
s33=sim(:,580);
s12=sim(:,581);
s13=sim(:,582);
s21=sim(:,583);
s23=sim(:,584);
s31=sim(:,585);
s32=sim(:,586);
e11=sim(:,587);
e22=sim(:,588);
e33=sim(:,589);
e12=sim(:,590);
e13=sim(:,591);
e21=sim(:,592);
e23=sim(:,593);
e31=sim(:,594);
e32=sim(:,595);
s_inv=sim(:,596);
rel_inv=sim(:,597);
hos_inv=sim(:,598);
trs=sim(:,599);
trrel=sim(:,600);
trm=sim(:,601);

% figure(6)
% plot(time,s_inv)

time2=[0;0.0250000000000000;0.0500000000000000;0.0750000000000000;0.100000000000000;0.125000000000000;0.150000000000000;0.175000000000000;0.200000000000000;0.225000000000000;0.250000000000000;0.275000000000000;0.300000000000000;0.325000000000000;0.350000000000000;0.375000000000000;0.400000000000000;0.425000000000000;0.450000000000000;0.475000000000000;0.500000000000000;0.525000000000000;0.550000000000000;0.575000000000000;0.600000000000000;0.625000000000000;0.650000000000000;0.675000000000000;0.700000000000000;0.725000000000000;0.750000000000000;0.775000000000000;0.800000000000000;0.825000000000000;0.850000000000000;0.875000000000000;0.900000000000000;0.925000000000000;0.950000000000000;0.975000000000000;1;];
time1=[0;0.0500000000000000;0.100000000000000;0.150000000000000;0.200000000000000;0.250000000000000;0.300000000000000;0.350000000000000;0.400000000000000;0.450000000000000;0.500000000000000;0.550000000000000;0.600000000000000;0.650000000000000;0.700000000000000;0.750000000000000;0.800000000000000;0.850000000000000;0.900000000000000;0.950000000000000;1;];
s_inv33=[0;12836.4500000000;21747.9000000000;29563.4600000000;40241.8300000000;57062.0300000000;81333.3500000000;113121.100000000;152147.200000000;198106.300000000;250717.600000000;309717.200000000;374845.200000000;445837.200000000;522417.900000000;604298.200000000;691173.300000000;782720.700000000;878600.200000000;978452.600000000;1081900;];
s_inv66=[0;42705.8300000000;82867.3600000000;120714.500000000;156512.400000000;190562.600000000;223201.300000000;254796.500000000;285741;316442.800000000;347311.400000000;378738.900000000;411079.500000000;444624.100000000;479575.600000000;516025.300000000;553934.600000000;593124.600000000;633276;673940.100000000;714559.700000000;];
s_inv99=[0;9630.07900000000;18113.7200000000;25464.1800000000;31696.0500000000;36825.2900000000;40869.2900000000;43847.0900000000;45779.6500000000;46690.4900000000;46606.6700000000;45560.5400000000;43592.8400000000;40758.4400000000;37137.7300000000;32860.8200000000;28162.9200000000;23516.7800000000;19913.8800000000;19057.3100000000;22222.0400000000;28805.8200000000;37569.3200000000;47717.2500000000;58825.2900000000;70643.7700000000;82998.6000000000;95748.4600000000;108766.100000000;121930.100000000;135121.900000000;148224.400000000;161122.800000000;173705.800000000;185866.700000000;197505.500000000;208530.300000000;218858.600000000;228418.500000000;237149.600000000;245003.700000000;];

%12x12x12x mesh 4th row element # 1720
 time3=[0;0.0250000000000000;0.0500000000000000;0.0750000000000000;0.100000000000000;0.125000000000000;0.150000000000000;0.175000000000000;0.200000000000000;0.225000000000000;0.250000000000000;0.275000000000000;0.300000000000000;0.325000000000000;0.350000000000000;0.375000000000000;0.400000000000000;0.425000000000000;];
 s_inv12=[0;51627.0900000000;101586.200000000;149756.200000000;196006.200000000;240195.300000000;282172.200000000;321774.800000000;358830.100000000;393153.700000000;424549.400000000;452809;477712.500000000;499027.500000000;516509.400000000;529902;538937.500000000;543337.700000000;];
 tr_sigma12=[0;-391539.700000000;-789300.400000000;-1193537;-1604509;-2022482;-2447723;-2880507;-3321111;-3769815;-4226899;-4692643;-5167322;-5651204;-6144543;-6647576;-7160518;-7683554;];
 
 time4=[0;0.0250000000000000;0.0500000000000000;0.0750000000000000;0.100000000000000;0.125000000000000;0.150000000000000;0.175000000000000;0.200000000000000;0.225000000000000;0.250000000000000;0.275000000000000;0.300000000000000;0.325000000000000;0.350000000000000;0.375000000000000;0.400000000000000;0.425000000000000;0.450000000000000;0.475000000000000;0.500000000000000;0.525000000000000;0.550000000000000;0.575000000000000;0.600000000000000;0.625000000000000;0.650000000000000;0.675000000000000;0.700000000000000;0.725000000000000;0.750000000000000;0.775000000000000;0.800000000000000;0.825000000000000;0.850000000000000;0.875000000000000;];
 s_inv12FSE=[0;46330.0800000000;92169.8500000000;137524.900000000;182400.400000000;226801;270730.900000000;314193.700000000;357192.500000000;399729.600000000;441806.800000000;483425.200000000;524585.100000000;565286.400000000;605528.100000000;645308.700000000;684626.100000000;723477.700000000;761860.700000000;799771.700000000;837207.300000000;874164.300000000;910639.600000000;946630.400000000;982135.200000000;1017153;1051687;1085738;1119317;1152434;1185113;1217384;1249300;1280944;1312456;1344098;];
 tr_sigma12FSE=[0;-151118.300000000;-300480.100000000;-448067.300000000;-593861.100000000;-737842.100000000;-879990.600000000;-1020287;-1158710;-1295239;-1429855;-1562537;-1693266;-1822020;-1948783;-2073536;-2196261;-2316942;-2435565;-2552115;-2666578;-2778944;-2889199;-2997334;-3103338;-3207198;-3308901;-3408430;-3505764;-3600873;-3693715;-3784229;-3872323;-3957852;-4040579;-4120067;];
 % 9x9x9 mesh 3rd row element # 723
 s_inv99_723=[0;43770.3100000000;85732.6900000000;125794.600000000;163859.100000000;199825;233587.200000000;265036.600000000;294060.400000000;320542.700000000;344364.700000000;365405.200000000;383542;398652.200000000;410613.900000000;419308.400000000;424622;426449.600000000;424699.500000000;419299.500000000;410206;397418.200000000;380999.200000000;361110.100000000;338067.600000000;312442.400000000;285231.800000000;258158.500000000;234129.800000000;217691.800000000;214650;229719.500000000;263753.500000000;314211.400000000;377765.100000000;451791.200000000;534539.600000000;624890;722115.400000000;825725.800000000;935374.400000000;];
 tr_sigma99_723=[0;-394452.600000000;-793918.200000000;-1198538;-1608456;-2023816;-2444767;-2871458;-3304041;-3742671;-4187502;-4638689;-5096382;-5560730;-6031870;-6509932;-6995028;-7487249;-7986663;-8493305;-9007176;-9528235;-10056390;-10591520;-11133410;-11681830;-12236480;-12797000;-13362990;-13933990;-14509510;-15089010;-15671950;-16257730;-16845790;-17435520;-18026350;-18617710;-19209090;-19799960;-20389900;];
 
 
 s_inv99FSE_723=[0;43467.8500000000;86438.6800000000;128918.200000000;170911.900000000;212424.700000000;253461.100000000;294025.300000000;334120.800000000;373750.700000000;412917.400000000;451622.900000000;489868.400000000;527654.600000000;564981.600000000;601848.800000000;638254.900000000;674198.100000000;709675.900000000;744685.400000000;779222.900000000;813284.600000000;846865.900000000;879962;912567.800000000;944678.200000000;976287.700000000;1007391;1037984;1068061;1097618;1126653;1155164;1183152;1210620;1237573;1264024;1289992;1315503;1340603;1365358;];
 tr_sigma99FSE_723=[0;-161693.100000000;-321780.900000000;-480246.700000000;-637073;-792241.900000000;-945734.700000000;-1097532;-1247616;-1395964;-1542558;-1687376;-1830399;-1971607;-2110979;-2248495;-2384137;-2517886;-2649724;-2779636;-2907605;-3033618;-3157662;-3279726;-3399800;-3517879;-3633954;-3748023;-3860083;-3970133;-4078174;-4184207;-4288234;-4390258;-4490279;-4588296;-4684304;-4778289;-4870229;-4960084;-5047786;];
 
 
 figure(7)
 plot(time1,s_inv33,'-k','LineWidth',1, 'MarkerSize',8)
 hold on
 plot(time2,s_inv99_723,'-+k','LineWidth',1, 'MarkerSize',8)
 plot(time3,s_inv12,'-^k','LineWidth',1, 'MarkerSize',8)
 hold off
  xlabel('time','FontSize',16)
 ylabel('||devsigma||    (MPa)','FontSize',16)
 h=legend('3x3x3 MFSE','9x9x9 MFSE','12x12x12 MFSE');
 

 
 
 
 % Principal Stresses
figure(1)
%figure('Name','Results at the closest Gauss point (GP-1) under the corner point ','NumberTitle','off')
plot(time1,s_inv33,'-k','LineWidth',1, 'MarkerSize',8)
 hold on
 plot(time1,s_inv66,'-+k','LineWidth',1, 'MarkerSize',8)
 plot(time2,s_inv99,'-^k','LineWidth',1, 'MarkerSize',8)
 hold off
 xlabel('time','FontSize',16)
 ylabel('||devsigma||    (MPa)','FontSize',16)
 h=legend('3x3x3 MFSE','6x6x6 MFSE','9x9x9 MFSE');
set(gca,'FontName','Helvetica','FontSize',16)


s_inv33FSE=[0;17128.0700000000;31899.6300000000;44519.1200000000;55217.7700000000;64257.1000000000;71931.5400000000;78568.9000000000;84526.9200000000;90183.9000000000;95922.0300000000;102104.100000000;109046.900000000;116998.500000000;126124.100000000;136505.900000000;148153.500000000;161022.500000000;175034.600000000;190097;206118.500000000;];
s_inv66FSE=[0;23584.2200000000;44967.9200000000;64300.8700000000;81740.4200000000;97451.1800000000;111604.400000000;124377.300000000;135951.900000000;146514;156251.500000000;165352.900000000;174005.400000000;182392.800000000;190693.800000000;199079.300000000;207710.400000000;216736.100000000;226291.200000000;236493.900000000;247444.100000000;];
s_inv99FSE=[0;11239.3200000000;21896.5900000000;31990.5900000000;41540.6400000000;50566.6600000000;59089.1100000000;67128.9800000000;74707.7600000000;81847.3900000000;88570.2100000000;94898.9100000000;100856.500000000;106466;111750.800000000;116734.200000000;121439.400000000;125889.300000000;130106.700000000;134113.800000000;137932.300000000;141583.100000000;145086.400000000;148461.300000000;151725.700000000;154896.500000000;157988.800000000;161016.400000000;163991.300000000;166923.300000000;169820.500000000;172688.200000000;175529.300000000;178343.700000000;181127.500000000;183873;186567.200000000;189191.300000000;191718;194109.700000000;196312.900000000;];
figure(2)
plot(time1,s_inv33FSE,'-k','LineWidth',1, 'MarkerSize',8)
hold on
plot(time1,s_inv66FSE,'-+k','LineWidth',1, 'MarkerSize',8)
plot(time2,s_inv99FSE,'-^k','LineWidth',1, 'MarkerSize',8)
hold off
xlabel('time','FontSize',16)
ylabel('||devsigma|| FSE','FontSize',16)
 h=legend('3x3x3 FSE','6x6x6 FSE','9x9x9 FSE');
set(gca,'FontName','Helvetica','FontSize',16)


  figure(8)
 plot(time1,s_inv33FSE,'-k','LineWidth',1, 'MarkerSize',8)
 hold on
 plot(time2,s_inv99FSE_723,'-+k','LineWidth',1, 'MarkerSize',8)
 plot(time4,s_inv12FSE,'-^k','LineWidth',1, 'MarkerSize',8)
 hold off
 xlabel('time','FontSize',16)
ylabel('||devsigma|| FSE','FontSize',16)
 h=legend('3x3x3 FSE','9x9x9 FSE','12x12x12 FSE');
 

 

tr_sigma33=[0;-861577.500000000;-1726169;-2593661;-3463961;-4337001;-5212741;-6091175;-6972335;-7856289;-8743153;-9633083;-10526280;-11423010;-12323570;-13228310;-14137630;-15051990;-15971870;-16897830;-17830450;];
tr_sigma66=[0;-534154.600000000;-1061671;-1581453;-2092257;-2592707;-3081326;-3556580;-4016938;-4460945;-4887318;-5295040;-5683475;-6052467;-6402429;-6734396;-7050037;-7351607;-7641839;-7923782;-8200614;];
tr_sigma99=[0;-298745.800000000;-596963.200000000;-894624.400000000;-1191703;-1488171;-1784005;-2079179;-2373668;-2667448;-2960493;-3252776;-3544265;-3834927;-4124718;-4413590;-4701479;-4988309;-5273987;-5558399;-5841409;-6122852;-6402540;-6680254;-6955747;-7228748;-7498963;-7766080;-8029775;-8289721;-8545598;-8797098;-9043937;-9285862;-9522660;-9754164;-9980254;-10200860;-10415980;-10625630;-10829890;];
figure(4)
plot(time1,tr_sigma33,'-k','LineWidth',1, 'MarkerSize',8)
hold on 
plot(time1,tr_sigma66,'-+k','LineWidth',1, 'MarkerSize',8)
plot(time2,tr_sigma99,'-^k','LineWidth',1, 'MarkerSize',8)
hold off
h=legend('3x3x3 tr(sigma) MFSE','6x6x6 tr(sigma) MFSE','9x9x9 tr(sigma) MFSE');
set(gca,'FontName','Helvetica','FontSize',16)

tr_sigma33FSE=[0;-483281.100000000;-966728.700000000;-1450336;-1934071;-2417871;-2901640;-3385247;-3868519;-4351237;-4833134;-5313884;-5793103;-6270340;-6745068;-7216679;-7684478;-8147670;-8605354;-9056509;-9499985;];
tr_sigma66FSE=[0;-406535.700000000;-807589.600000000;-1202856;-1591997;-1974639;-2350372;-2718753;-3079302;-3431503;-3774809;-4108639;-4432381;-4745395;-5047014;-5336545;-5613273;-5876453;-6125317;-6359058;-6576826;];
tr_sigma99FSE=[0;-206568.300000000;-411913.500000000;-616004.600000000;-818808.500000000;-1020290;-1220413;-1419138;-1616425;-1812231;-2006512;-2199223;-2390315;-2579740;-2767448;-2953388;-3137508;-3319754;-3500075;-3678416;-3854726;-4028952;-4201043;-4370949;-4538623;-4704017;-4867090;-5027802;-5186114;-5341997;-5495421;-5646365;-5794812;-5940756;-6084194;-6225140;-6363615;-6499662;-6633344;-6764756;-6894046;];
figure(5)
plot(time1,tr_sigma33FSE,'-k','LineWidth',1, 'MarkerSize',8)
hold on 
plot(time1,tr_sigma66FSE,'-+k','LineWidth',1, 'MarkerSize',8)
plot(time2,tr_sigma99FSE,'-^k','LineWidth',1, 'MarkerSize',8)
hold off
h=legend('3x3x3 tr(sigma) FSE','6x6x6 tr(sigma) FSE','9x9x9 tr(sigma) FSE');
set(gca,'FontName','Helvetica','FontSize',16)

   figure(9)
 plot(time1,tr_sigma33FSE,'-k','LineWidth',1, 'MarkerSize',8)
 hold on
 plot(time2,tr_sigma99FSE_723,'-+k','LineWidth',1, 'MarkerSize',8)
 plot(time4,tr_sigma12FSE,'-^k','LineWidth',1, 'MarkerSize',8)
 hold off
 xlabel('time','FontSize',16)
ylabel('trsigma FSE','FontSize',16)
 h=legend('3x3x3 FSE','9x9x9 FSE','12x12x12 FSE');
 
 
    figure(10)
 plot(time1,tr_sigma33,'-k','LineWidth',1, 'MarkerSize',8)
 hold on
 plot(time2,tr_sigma99_723,'-+k','LineWidth',1, 'MarkerSize',8)
 plot(time3,tr_sigma12,'-^k','LineWidth',1, 'MarkerSize',8)
 hold off
 xlabel('time','FontSize',16)
ylabel('trsigma MFSE','FontSize',16)
 h=legend('3x3x3 MFSE','9x9x9 MFSE','12x12x12 MFSE');


     figure(11)
 plot(time,trrel,'-^k','LineWidth',1, 'MarkerSize',8)

 xlabel('trrel','FontSize',16)
ylabel('trsigma MFSE','FontSize',16)
 h=legend('3x3x3 MFSE','9x9x9 MFSE','12x12x12 MFSE');
 
 

