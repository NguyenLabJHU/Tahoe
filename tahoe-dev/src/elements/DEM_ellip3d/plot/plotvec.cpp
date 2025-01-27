#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include <cmath>

using namespace std;

const long double PI = 3.1415927;
static const int OWID = 15;

int main(int argc, char *argv[])
{
  if(argc < 2) {
    cout << endl 
	 << "-- Plot Vectors such as Force or Velocity --"<<endl
	 << "Usage:" << endl
	 << "1) process a single file:  plotvec particle_file" << endl
	 << "   --example: plotvec iso_particle_008" << endl
	 << "2) process multiple files: plotvec particle_file_prefix  first_suffix  last_suffix  suffix_increment" << endl
	 << "   --example: plotvec iso_particle  0  100  5" << endl << endl;
    return -1;
  }	

  int first, last, incre;
  if(argc == 2) {
    first = 0;
    last  = 1;
    incre = 2;
  }
  else {
    first = atoi(argv[2]);
    last  = atoi(argv[3]);
    incre = atoi(argv[4]);
  }

  ifstream ifs;
  ofstream ofs;
  char filein[50];
  char fileout[50];
  char num[4], s[20];

  int id, type, totalNum;
  long double cx, cy, cz, rd, wd, lt, ht;
  long double a, b, c, x0, y0, z0, l1, l2, l3, m1, m2, m3, n1, n2, n3, v1, v2, v3, w1, w2, w3, f1, f2, f3, mt1, mt2, mt3;
  long double dx0, dy0, dz0;

  std::map<int, std::vector<long double> > centerInit; // map: the particle ID order varies in different files because of MPI gathering.
  for(int snapshot = first; snapshot <= last; snapshot += incre) {
    if(argc == 2)
      strcpy(filein, argv[1]);
    else {
      sprintf(num, "%03d", snapshot);
      strcpy(filein, argv[1]);
      strcat(filein, "_");
      strcat(filein, num);
    }

    strcpy(fileout, "vec_");
    strcat(fileout, filein);
    strcat(fileout, ".dat");
    cout << "generating file " << fileout << " ......" <<endl;

    ifs.open(filein);
    if(!ifs)  { cout<<"stream error!"<<endl; exit(-1);}
    ofs.open(fileout);
    if(!ofs)  { cout<<"stream error!"<<endl; exit(-1);}
    ofs.setf(ios::scientific, ios::floatfield);

    ifs >> totalNum;
    ifs>>s>>s>>s>>s>>s>>s>>s>>s>>s>>s>>s>>s>>s>>s
       >>s>>s>>s>>s>>s>>s>>s>>s>>s>>s>>s>>s>>s>>s>>s;

    ofs << "VARIABLES=" << endl
	<< setw(OWID) << "x"
	<< setw(OWID) << "y"
	<< setw(OWID) << "z"
	<< setw(OWID) << "a"
	<< setw(OWID) << "b"
	<< setw(OWID) << "c"
	<< setw(OWID) << "delta_x"
	<< setw(OWID) << "delta_y"
	<< setw(OWID) << "delta_z"
	<< setw(OWID) << "velocity_x"
	<< setw(OWID) << "velocity_y"
	<< setw(OWID) << "velocity_z"
	<< setw(OWID) << "force_x"
	<< setw(OWID) << "force_y"
	<< setw(OWID) << "force_z"
	<< setw(OWID) << "omga_x"
	<< setw(OWID) << "omga_y"
	<< setw(OWID) << "omga_z"
	<< setw(OWID) << "moment_x"
	<< setw(OWID) << "moment_y"
	<< setw(OWID) << "moment_z"
	<< setw(OWID) << "axis_a_x"
	<< setw(OWID) << "axis_a_y"
	<< setw(OWID) << "axis_a_z"
	<< setw(OWID) << "axis_b_x"
	<< setw(OWID) << "axis_b_y"
	<< setw(OWID) << "axis_b_z"
	<< setw(OWID) << "axis_c_x"
	<< setw(OWID) << "axis_c_y"
	<< setw(OWID) << "axis_c_z"
	<< endl;

    ofs << "ZONE I=" << totalNum << ", DATAPACKING=POINT, STRANDID=1, SOLUTIONTIME=" << snapshot << endl;
    for(int it = 0; it < totalNum; ++it) {
      ifs >> id >> type >> a >> b >> c >> x0 >> y0 >> z0 >> l1 >> m1 >> n1 >> l2 >> m2 >> n2 >> l3 >> m3 >> n3
	  >>v1>>v2>>v3>>w1>>w2>>w3>>f1>>f2>>f3>>mt1>>mt2>>mt3;

      if (snapshot == first) {
	std::vector<long double> triple(3);
	triple[0] = x0;
	triple[1] = y0;
	triple[2] = z0;
	centerInit[id] = triple;
      }
	    
      ofs << setw(OWID) << x0
	  << setw(OWID) << y0
	  << setw(OWID) << z0
	  << setw(OWID) << a
	  << setw(OWID) << b
	  << setw(OWID) << c;

      if (centerInit.count(id)) // ==1, make sure the key exist, for special cases like extracting particles.
	ofs << setw(OWID) << x0 - centerInit[id][0]	
	    << setw(OWID) << y0 - centerInit[id][1]
	    << setw(OWID) << z0 - centerInit[id][2];
      else	  
	ofs << setw(OWID) << 0
	    << setw(OWID) << 0
	    << setw(OWID) << 0;
      
      ofs << setw(OWID) << v1
	  << setw(OWID) << v2
	  << setw(OWID) << v3
	  << setw(OWID) << f1
	  << setw(OWID) << f2
	  << setw(OWID) << f3
	  << setw(OWID) << w1
	  << setw(OWID) << w2
	  << setw(OWID) << w3
	  << setw(OWID) << mt1
	  << setw(OWID) << mt2
	  << setw(OWID) << mt3
	  << setw(OWID) << cos(l1)
	  << setw(OWID) << cos(m1)
	  << setw(OWID) << cos(n1)
	  << setw(OWID) << cos(l2)
	  << setw(OWID) << cos(m2)
	  << setw(OWID) << cos(n2)
	  << setw(OWID) << cos(l3)
	  << setw(OWID) << cos(m3)
	  << setw(OWID) << cos(n3)
	  << endl;   
    }
	
    ifs.close();
    ofs.close();

  }

  return 0;
}
