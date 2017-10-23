//
//                                    x1(1)
//                          ---------------------
//                         /                    /|
//                        /                    / |
//                       /                    /  |
//                      /        z2(6)       /   |
//                     /                    /    |
//                    /                    /     |                    z
//                   /                    /      |                    |
//                  |---------------------       |                    |
//            y1(3) |                    | y2(4) |                    |____ y
//                  |                    |       /                   /
//                  |                    |      /                   /
//                  |        x2(2)       |     /                   x
//                  |                    |    /
//                  |                    |   /
//                  |                    |  /
//                  |                    | /
//                  |                    |/
//                  ----------------------
//                             z1(5)
//
//
//    It is preferable to use the description of surface x1, x2, y1, y2, z1, z2,
//    where x1 < x2, y1 < y2, z1 < z2. We also use surface 1, 2, 3, 4, 5, 6 accordingly.
//

#ifdef STRESS_STRAIN
// The Qhull path of header files may cause "No such file or directory" error by "g++ -MM", but it does not actually affect compiling or linking.
// It may affect compile file dependency, it that case do "make clean" before "make"
#include "libqhullcpp/RboxPoints.h"
#include "libqhullcpp/QhullError.h"
#include "libqhullcpp/QhullQh.h"
#include "libqhullcpp/QhullFacet.h"
#include "libqhullcpp/QhullFacetList.h"
#include "libqhullcpp/QhullLinkedList.h"
#include "libqhullcpp/QhullVertex.h"
#include "libqhullcpp/Qhull.h"
#endif

#include "Assembly.h"
#include "const.h"
#include "ran.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <ctime>
#include <cassert>
#include <utility>
#include <sys/time.h>
#include <omp.h>

#define MODULE_TIME
//#define DEM_PROFILE
//#define CFD_PROFILE
//#define BIGON

static time_t timeStamp; // for file timestamping
static struct timeval time_w1, time_w2; // for wall-clock time record
static struct timeval time_p1, time_p2; // for internal wall-clock time profiling, can be used on any piece of code
static struct timeval time_r1, time_r2; // for internal wall-clock time profiling for contact resolution only (excluding space search)

//#define PAPI 1
//#define PAPI_FLOPS 1
//#define PAPI_CACHE 1

#ifdef PAPI
#include "papi.h"
static double papi_gettime() {
  return (double)PAPI_get_virt_usec()/1000000.0;
}
#if defined PAPI_FLOPS
 #define NUM_EVENTS 3
 #define LLD " PAPI_FP_OPS: %15lld\n MFLOPS: 15%f\n PAPI_L3_TCM: %15lld\n PAPI_MEM_SCY: %15lld\n"
#elif defined PAPI_CACHE
 #define NUM_EVENTS 3
 #define LLD " PAPI_L3_TCM: %15lld\n PAPI_L2_TCM: %15lld\n PAPI_L1_TCM %15lld\n"
#endif
#endif

namespace dem {

  struct timeval timediff(const struct timeval &time1, const struct timeval &time2) {
    struct timeval diff;
    diff.tv_sec = time2.tv_sec - time1.tv_sec; 
    if( ( diff.tv_usec = time2.tv_usec - time1.tv_usec) < 0) {
      diff.tv_usec += 1000000;
      --diff.tv_sec;
    }
    return(diff); 
  }


  long int timediffmsec(const struct timeval &time1, const struct timeval &time2) {
    struct timeval diff = timediff(time1, time2);
    return(diff.tv_sec * 1000000 + diff.tv_usec); 
  }


  REAL timediffsec(const struct timeval &time1, const struct timeval &time2) {
    return( (REAL) timediffmsec(time1,time2) / 1.0e+6);
  }


  char *combineString(char *cstr, const char *str, std::size_t num, std::size_t width) {
    std::string obj(str);
    std::stringstream ss;
    ss << std::setw(width) << std::setfill('0') << std::right << num;
    obj += ss.str();
    return strcpy( cstr, obj.c_str() );  
  }


  REAL ref(const REAL in) {
    if (in < 0)
      return in;
    else 
      return 0;
  }

  // input:   number percentage smaller from data file
  // output:  mass percentage smaller to disk file debugInf
  // purpose: let mass percentage smaller satisfy particle size distribution curve
  // method:  use trial and error method on number percentage until mass percentage is satisfied
  void Assembly::tuneMassPercent()
  {
    if (mpiRank == 0) {
      REAL minX = dem::Parameter::getSingleton().parameter["minX"];
      REAL minY = dem::Parameter::getSingleton().parameter["minY"];
      REAL minZ = dem::Parameter::getSingleton().parameter["minZ"];
      REAL maxX = dem::Parameter::getSingleton().parameter["maxX"];
      REAL maxY = dem::Parameter::getSingleton().parameter["maxY"];
      REAL maxZ = dem::Parameter::getSingleton().parameter["maxZ"];
      std::size_t particleLayers = dem::Parameter::getSingleton().parameter["particleLayers"];

      setContainer(Rectangle(minX, minY, minZ, maxX, maxY, maxZ));

      buildBoundary(5, "deposit_boundary_ini");
    
      std::size_t sieveNum = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["sieveNum"]);
      std::vector<REAL> percent(sieveNum), size(sieveNum);
      std::vector<std::pair<REAL, REAL> > &grada = dem::Parameter::getSingleton().gradation;
      assert(grada.size() == sieveNum);
      for (std::size_t i = 0; i < sieveNum; ++i) {
	percent[i] = grada[i].first;
	size[i] = grada[i].second;
      }
      REAL ratioBA = dem::Parameter::getSingleton().parameter["ratioBA"];
      REAL ratioCA = dem::Parameter::getSingleton().parameter["ratioCA"];
      setGradation(Gradation(sieveNum, percent, size, ratioBA, ratioCA));

      generateParticle(particleLayers, "float_particle_ini"); 

      // statistics of mass distribution
      Gradation massGrad = gradation;
      std::vector<REAL> &massPercent = massGrad.getPercent();
      std::vector<REAL> &massSize    = massGrad.getSize();
      for (std::size_t i = 0; i < massPercent.size(); ++i) massPercent[i] = 0;

      for (std::vector<Particle*>::iterator itr = allParticleVec.begin(); itr != allParticleVec.end(); ++itr)
	for (int i = massPercent.size()-1; i >= 0 ; --i) { // do not use size_t for descending series
	  if ( (*itr)->getA() <= massSize[i] )
	    massPercent[i] += (*itr)->getMass();
	}
      REAL totalMass = massPercent[0];
      for (std::size_t i = 0; i < massPercent.size(); ++i) massPercent[i] /= totalMass;
      debugInf << std::endl << "mass percentage of particles:" << std::endl
	       << std::setw(OWID) << massPercent.size() << std::endl;
      for (std::size_t i = 0; i < massPercent.size(); ++i)
	debugInf << std::setw(OWID) << massPercent[i] << std::setw(OWID) << massSize[i] << std::endl;
    }
  }


  // input:   particle file with estimated gradation
  // output:  particle file with precise gradation
  void Assembly::calcMassPercent()  
  {
    if (mpiRank == 0) {
      readParticle(dem::Parameter::getSingleton().datafile["particleFile"].c_str());    
      readBoundary(dem::Parameter::getSingleton().datafile["boundaryFile"].c_str());

      // statistics of mass distribution
      Gradation massGrad = gradation;
      std::vector<REAL> &massPercent = massGrad.getPercent();
      std::vector<REAL> &massSize    = massGrad.getSize();
      for (std::size_t i = 0; i < massPercent.size(); ++i) massPercent[i] = 0;

      for (std::vector<Particle*>::iterator itr = allParticleVec.begin(); itr != allParticleVec.end(); ++itr)
	for (int i = massPercent.size()-1; i >= 0 ; --i) { // do not use size_t for descending series
	  if ( (*itr)->getA() <= massSize[i] )
	    massPercent[i] += (*itr)->getMass(); // += 1 for calculating number percentage
	}
      REAL totalMass = massPercent[0];
      for (std::size_t i = 0; i < massPercent.size(); ++i) massPercent[i] /= totalMass;
      debugInf << std::endl << "mass percentage of particles:" << std::endl
	       << std::setw(OWID) << massPercent.size() << std::endl;
      for (std::size_t i = 0; i < massPercent.size(); ++i)
	debugInf << std::setw(OWID) << massPercent[i] << std::setw(OWID) << massSize[i] << std::endl;
    }
  }


  void Assembly::depositIntoContainer() 
  {
    if (mpiRank == 0) {
      REAL minX = dem::Parameter::getSingleton().parameter["minX"];
      REAL minY = dem::Parameter::getSingleton().parameter["minY"];
      REAL minZ = dem::Parameter::getSingleton().parameter["minZ"];
      REAL maxX = dem::Parameter::getSingleton().parameter["maxX"];
      REAL maxY = dem::Parameter::getSingleton().parameter["maxY"];
      REAL maxZ = dem::Parameter::getSingleton().parameter["maxZ"];
      std::size_t particleLayers = dem::Parameter::getSingleton().parameter["particleLayers"];

      setContainer(Rectangle(minX, minY, minZ, maxX, maxY, maxZ));

      buildBoundary(5, "deposit_boundary_ini");
    
      std::size_t sieveNum = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["sieveNum"]);
      std::vector<REAL> percent(sieveNum), size(sieveNum);
      std::vector<std::pair<REAL, REAL> > &grada = dem::Parameter::getSingleton().gradation;
      assert(grada.size() == sieveNum);
      for (std::size_t i = 0; i < sieveNum; ++i) {
	percent[i] = grada[i].first;
	size[i] = grada[i].second;
      }
      REAL ratioBA = dem::Parameter::getSingleton().parameter["ratioBA"];
      REAL ratioCA = dem::Parameter::getSingleton().parameter["ratioCA"];
      setGradation(Gradation(sieveNum, percent, size, ratioBA, ratioCA));

      generateParticle(particleLayers, "float_particle_ini"); 
    }

    deposit("deposit_boundary_ini",
	    "float_particle_ini");

    if (mpiRank == 0) {
      setContainer(Rectangle(allContainer.getMinCorner().getX(),
			     allContainer.getMinCorner().getY(),
			     allContainer.getMinCorner().getZ(),
			     allContainer.getMaxCorner().getX(),
			     allContainer.getMaxCorner().getY(),
			     dem::Parameter::getSingleton().parameter["trimHeight"]));
      buildBoundary(6, "trim_boundary_ini");
      char cstr[50];
      std::size_t endSnap = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["endSnap"]);
      trim(false,
	   combineString(cstr, "deposit_particle_", endSnap, 3),
	   "trim_particle_ini");
    }
  }


  void Assembly::resumeDepositIntoContainer() 
  {
    deposit(dem::Parameter::getSingleton().datafile["boundaryFile"].c_str(),
	    dem::Parameter::getSingleton().datafile["particleFile"].c_str());
  
    if (mpiRank == 0) {
      setContainer(Rectangle(allContainer.getMinCorner().getX(),
			     allContainer.getMinCorner().getY(),
			     allContainer.getMinCorner().getZ(),
			     allContainer.getMaxCorner().getX(),
			     allContainer.getMaxCorner().getY(),
			     dem::Parameter::getSingleton().parameter["trimHeight"]));
      buildBoundary(6, "trim_boundary_ini");
      char cstr[50];
      std::size_t endSnap = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["endSnap"]);
      trim(false,
	   combineString(cstr, "deposit_particle_", endSnap, 3),
	   "trim_particle_ini");
    }
    
  }


  void Assembly::proceedFromPreset() 
  {
    deposit(dem::Parameter::getSingleton().datafile["boundaryFile"].c_str(),
	    dem::Parameter::getSingleton().datafile["particleFile"].c_str());
  }


  void Assembly::deposit(const char *inputBoundary,
			 const char *inputParticle) 
  {
    int gridUpdate = static_cast<int> (dem::Parameter::getSingleton().parameter["gridUpdate"]);

    REAL pretime0, pretime1, pretime2;
#ifdef MODULE_TIME
    pretime0=MPI_Wtime();
#endif
    if (mpiRank == 0) {
      readParticle(inputParticle);
      readBoundary(inputBoundary, gridUpdate); // need particle info and gridUpdate to determine initial compute grid
      openDepositProg(progressInf, "deposit_progress");
    }
#ifdef MODULE_TIME
    pretime1=MPI_Wtime();
#endif
    scatterParticle(); // scatter particles only once; also updates grid for the first time
#ifdef MODULE_TIME
    pretime2=MPI_Wtime();
    debugInf << std::setw(OWID) << "readFile" << std::setw(OWID) << pretime1-pretime0
	     << std::setw(OWID) << "scatterPtcl" << std::setw(OWID) << pretime2-pretime1 << std::endl;
#endif

    std::size_t startStep = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["startStep"]);
    std::size_t endStep   = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["endStep"]);
    std::size_t startSnap = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["startSnap"]);
    std::size_t endSnap   = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["endSnap"]);
    std::size_t netStep   = endStep - startStep + 1;
    std::size_t netSnap   = endSnap - startSnap + 1;
    timeStep = dem::Parameter::getSingleton().parameter["timeStep"];

    REAL time0, time1, time2, commuT, gridT, migraT, gatherT, totalT;
    iteration = startStep;
    std::size_t iterSnap = startSnap;
    char cstr0[50];
    /**/REAL timeCount = 0;
    /**/timeAccrued = static_cast<REAL> (dem::Parameter::getSingleton().parameter["timeAccrued"]);
    /**/REAL timeIncr  = timeStep * netStep;
    /**/REAL timeTotal = timeAccrued + timeStep * netStep;
    if (mpiRank == 0) {
      plotBoundary(strcat(combineString(cstr0, "deposit_bdryplot_", iterSnap - 1, 3), ".dat"));
      plotGrid(strcat(combineString(cstr0, "deposit_gridplot_", iterSnap - 1, 3), ".dat"));
      printParticle(combineString(cstr0, "deposit_particle_", iterSnap - 1, 3));
      printBdryContact(combineString(cstr0, "deposit_bdrycntc_", iterSnap -1, 3));
    }
    if (mpiRank == 0)
      debugInf << std::setw(OWID) << "iter" << std::setw(OWID) << "commuT" << std::setw(OWID) << "gridT" << std::setw(OWID) << "migraT"
	       << std::setw(OWID) << "compuT" << std::setw(OWID) << "totalT" << std::setw(OWID) << "overhead%" << std::endl; 

#ifdef PAPI
#if defined PAPI_FLOPS
    int Events[NUM_EVENTS] = {PAPI_FP_OPS, PAPI_L3_TCM, PAPI_MEM_SCY};
#elif defined PAPI_CACHE
    int Events[NUM_EVENTS] = {PAPI_L3_TCM, PAPI_L2_TCM, PAPI_L1_TCM};
#endif
    long long values[NUM_EVENTS];
    double papi_t0, papi_t1;
    papi_t0 = papi_gettime();
    if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) {
      printf("PAPI_library_init error. \n"); fflush(stdout);
      exit(1);
    }
    const size_t EVENT_MAX = PAPI_num_counters();
    printf("# Max counters = %zd\n", EVENT_MAX); fflush(stdout);
    if (PAPI_start_counters(Events, NUM_EVENTS) != PAPI_OK) {
      printf("PAPI_start_counters error. \n"); fflush(stdout);
      exit(1);
    }
#endif
    /**/while (timeAccrued < timeTotal) { 
      //while (iteration <= endStep) {
      bool toCheckTime = (iteration + 1) % (netStep / netSnap) == 0;

#ifdef MODULE_TIME
      if (toCheckTime) {commuT = gridT = migraT = gatherT = totalT = 0; time0 = MPI_Wtime();}
#endif
      commuParticle(); 
#ifdef MODULE_TIME
      if (toCheckTime) time2 = MPI_Wtime(); commuT = time2 - time0;
#endif

      /**/calcTimeStep(); // use values from last step, must call before findContact (which clears data)
      findContact();
      if (isBdryProcess() || gridUpdate == 1) findBdryContact();

      clearContactForce();
      internalForce();
      if (isBdryProcess() || gridUpdate == 1) boundaryForce();

      dragForce();

#ifdef STRESS_STRAIN
      if (timeCount + timeStep*2 >= timeIncr/netSnap && timeCount + timeStep <= timeIncr/netSnap )
	calcPrevGranularStress(); // compute stress in previous time step

      if (timeCount + timeStep >= timeIncr/netSnap) {
	gatherGranularStress(timeStep, timeIncr/netSnap); //ensure both contact forces and particle locations are in current step.
        snapParticlePos(); // snapshort particle positions
      }
#endif

      updateParticle();
#ifdef MODULE_TIME
      if (toCheckTime) time1 = MPI_Wtime();
#endif
      if (gridUpdate == 0)
	updateGridMaxZ();
      else if (gridUpdate == 1)
	updateGridExplosion();
      else if (gridUpdate == 2)
	updateGrid5();
      else if (gridUpdate == 3)
	updateGrid();

#ifdef MODULE_TIME
      if (toCheckTime) time2 = MPI_Wtime(); gridT = time2 - time1;
#endif

      /**/timeCount += timeStep;
      /**/timeAccrued += timeStep;
      /**/if (timeCount >= timeIncr/netSnap) { 
	//if (iteration % (netStep / netSnap) == 0) {
#ifdef MODULE_TIME
	if (toCheckTime) time1 = MPI_Wtime();
#endif
	gatherParticle();
	gatherBdryContact();
	gatherEnergy(); 
#ifdef MODULE_TIME
	if (toCheckTime) time2 = MPI_Wtime(); gatherT = time2 - time1;
#endif

	char cstr[50];
	if (mpiRank == 0) {
	  plotBoundary(strcat(combineString(cstr, "deposit_bdryplot_", iterSnap, 3), ".dat"));
	  plotGrid(strcat(combineString(cstr, "deposit_gridplot_", iterSnap, 3), ".dat"));
	  printParticle(combineString(cstr, "deposit_particle_", iterSnap, 3));
	  printBdryContact(combineString(cstr, "deposit_bdrycntc_", iterSnap, 3));
	  printDepositProg(progressInf);
#ifdef STRESS_STRAIN
	  printGranularStressFEM(strcat(combineString(cstr, "deposit_stress_plot_", iterSnap, 3), ".dat"));
	  printGranularStressOrdered(strcat(combineString(cstr, "deposit_stress_data_", iterSnap, 3), ".dat"));
#endif
	}
	printContact(combineString(cstr, "deposit_contact_", iterSnap, 3));
      
	/**/timeCount = 0;
	++iterSnap;
      }

      releaseRecvParticle(); // late release because printContact refers to received particles
#ifdef MODULE_TIME
      if (toCheckTime) time1 = MPI_Wtime();
#endif
      migrateParticle(); 
#ifdef MODULE_TIME
      if (toCheckTime) time2 = MPI_Wtime(); migraT = time2 - time1; totalT = time2 - time0;
      if (mpiRank == 0 && toCheckTime) // ignore gather and print time at this step
	debugInf << std::setw(OWID) << iteration << std::setw(OWID) << commuT  << std::setw(OWID) << gridT << std::setw(OWID) << migraT
		 << std::setw(OWID) << totalT - commuT - gridT - migraT << std::setw(OWID) << totalT 
		 << std::setw(OWID) << (commuT + gridT + migraT)/totalT*100 << std::endl;
#endif
      ++iteration;
    } 

#ifdef PAPI
    if (PAPI_read_counters(values, NUM_EVENTS) != PAPI_OK) {
      printf("PAPI_read_counters error. \n"); fflush(stdout);
      exit(1);
    } else {
      papi_t1 = papi_gettime();
#if defined PAPI_FLOPS
      printf(LLD, values[0], (double)values[0]/1000000.0/(papi_t1-papi_t0), values[1], values[2]); fflush(stdout);
#elif defined PAPI_CACHE
      printf(LLD, values[0], values[1], values[2]); fflush(stdout);
#endif
    }
#endif
  
    if (mpiRank == 0) closeProg(progressInf);
  }


  void Assembly::expandCavityParticle() 
  {
    if (mpiRank == 0) {
      const char *inputParticle = dem::Parameter::getSingleton().datafile["particleFile"].c_str();
      REAL percent = dem::Parameter::getSingleton().parameter["expandPercent"];
      readParticle(inputParticle); 
    
      REAL x1 = dem::Parameter::getSingleton().parameter["cavityMinX"];
      REAL y1 = dem::Parameter::getSingleton().parameter["cavityMinY"];
      REAL z1 = dem::Parameter::getSingleton().parameter["cavityMinZ"];
      REAL x2 = dem::Parameter::getSingleton().parameter["cavityMaxX"];
      REAL y2 = dem::Parameter::getSingleton().parameter["cavityMaxY"];
      REAL z2 = dem::Parameter::getSingleton().parameter["cavityMaxZ"];
    
      std::vector<Particle*> cavityParticleVec;
      std::vector<Particle*>::iterator it;
      Vec center;
    
      for (it = allParticleVec.begin(); it != allParticleVec.end(); ++it ){
	center=(*it)->getCurrPos();
	if(center.getX() > x1 && center.getX() < x2 &&
	   center.getY() > y1 && center.getY() < y2 &&
	   center.getZ() > z1 && center.getZ() < z2 ) {
	  (*it)->expand(percent);
	  cavityParticleVec.push_back(*it);
	}
      }
    
      printParticle("cavity_particle_ini", cavityParticleVec);
      printParticle("expand_particle_ini");
    }
  
    deposit(dem::Parameter::getSingleton().datafile["boundaryFile"].c_str(),
	    "expand_particle_ini");
  }


  void Assembly::resumeExpandCavityParticle() 
  {
    deposit(dem::Parameter::getSingleton().datafile["boundaryFile"].c_str(),
	    dem::Parameter::getSingleton().datafile["particleFile"].c_str());
  }


  void Assembly::isotropic() 
  {
    std::size_t isotropicType = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["isotropicType"]);
    if (mpiRank == 0) {
      readParticle(dem::Parameter::getSingleton().datafile["particleFile"].c_str());
      readBoundary(dem::Parameter::getSingleton().datafile["boundaryFile"].c_str());
      openCompressProg(progressInf, "isotropic_progress");
      openCompressProg(balancedInf, "isotropic_balanced");
    }
    scatterParticle();

    std::size_t startStep = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["startStep"]);
    std::size_t endStep   = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["endStep"]);
    std::size_t startSnap = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["startSnap"]);
    std::size_t endSnap   = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["endSnap"]);
    std::size_t netStep   = endStep - startStep + 1;
    std::size_t netSnap   = endSnap - startSnap + 1;
    timeStep = dem::Parameter::getSingleton().parameter["timeStep"];

    REAL sigmaEnd, sigmaInc, sigmaVar;
    std::size_t sigmaDiv;
  
    sigmaEnd = dem::Parameter::getSingleton().parameter["sigmaEnd"];
    sigmaDiv = dem::Parameter::getSingleton().parameter["sigmaDiv"];
    std::vector<REAL> &sigmaPath = dem::Parameter::getSingleton().sigmaPath;
    std::size_t sigma_i = 0;

    if (isotropicType == 1) 
      sigmaVar = sigmaEnd;
    else if (isotropicType == 2) {
      REAL sigmaStart = dem::Parameter::getSingleton().parameter["sigmaStart"];
      sigmaInc = (sigmaEnd - sigmaStart) / sigmaDiv;
      sigmaVar = sigmaStart;
    } else if (isotropicType == 3) {
      sigmaVar = sigmaPath[sigma_i];
      sigmaInc = (sigmaPath[sigma_i+1] -  sigmaPath[sigma_i])/sigmaDiv;
      sigmaEnd = sigmaPath[sigmaPath.size()-1];
    }

    REAL time0, time1, time2, commuT, migraT, gatherT, totalT;
    iteration = startStep;
    std::size_t iterSnap = startSnap;
    char cstr0[50];
    REAL distX, distY, distZ;
    if (mpiRank == 0) {
      plotBoundary(strcat(combineString(cstr0, "isotropic_bdryplot_", iterSnap - 1, 3), ".dat"));
      plotGrid(strcat(combineString(cstr0, "isotropic_gridplot_", iterSnap - 1, 3), ".dat"));
      printParticle(combineString(cstr0, "isotropic_particle_", iterSnap - 1, 3));
      printBdryContact(combineString(cstr0, "isotropic_bdrycntc_", iterSnap -1, 3));
      printBoundary(combineString(cstr0, "isotropic_boundary_", iterSnap - 1, 3));
      getStartDimension(distX, distY, distZ);
    }
    if (mpiRank == 0)
      debugInf << std::setw(OWID) << "iter" << std::setw(OWID) << "commuT" << std::setw(OWID) << "migraT"
	       << std::setw(OWID) << "totalT" << std::setw(OWID) << "overhead%" << std::endl;
    while (iteration <= endStep) {
      commuT = migraT = gatherT = totalT = 0; time0 = MPI_Wtime();
      commuParticle(); time2 = MPI_Wtime(); commuT = time2 - time0;

      calcTimeStep(); // use values from last step, must call before findContact
      findContact();
      if (isBdryProcess()) findBdryContact();

      clearContactForce();
      internalForce();
      if (isBdryProcess()) boundaryForce();

#ifdef STRESS_STRAIN
      if ((iteration + 2) % (netStep / netSnap) == 0)
	calcPrevGranularStress(); // compute stress in previous time step

      if ((iteration + 1) % (netStep / netSnap) == 0) {
	gatherGranularStress(timeStep, netStep / netSnap * timeStep); //ensure both contact forces and particle locations are in current step.
        snapParticlePos(); // snapshort particle positions
      }
#endif

      updateParticle();
      gatherBdryContact(); // must call before updateBoundary
   
      if (iteration % (netStep / netSnap) == 0) {
	time1 = MPI_Wtime();
	gatherParticle();
	gatherEnergy(); time2 = MPI_Wtime(); gatherT = time2 - time1;

	char cstr[50];
	if (mpiRank == 0) {
	  plotBoundary(strcat(combineString(cstr, "isotropic_bdryplot_", iterSnap, 3), ".dat"));
	  plotGrid(strcat(combineString(cstr, "isotropic_gridplot_", iterSnap, 3), ".dat"));
	  printParticle(combineString(cstr, "isotropic_particle_", iterSnap, 3));
	  printBdryContact(combineString(cstr, "isotropic_bdrycntc_", iterSnap, 3));
	  printBoundary(combineString(cstr, "isotropic_boundary_", iterSnap, 3));
	  printCompressProg(progressInf, distX, distY, distZ);
#ifdef STRESS_STRAIN
	  printGranularStressFEM(strcat(combineString(cstr, "isotropic_stress_plot_", iterSnap, 3), ".dat"));
	  printGranularStressOrdered(strcat(combineString(cstr, "isotropic_stress_data_", iterSnap, 3), ".dat"));
#endif
	}
	printContact(combineString(cstr, "isotropic_contact_", iterSnap, 3));      
	++iterSnap;
      }

      updateBoundary(sigmaVar, "isotropic"); // must call after printBdryContact
      updateGrid();

      releaseRecvParticle(); // late release because printContact refers to received particles
      time1 = MPI_Wtime();
      migrateParticle(); time2 = MPI_Wtime(); migraT = time2 - time1; totalT = time2 - time0;
      if (mpiRank == 0 && (iteration+1 ) % (netStep / netSnap) == 0) // ignore gather and print time at this step
	debugInf << std::setw(OWID) << iteration << std::setw(OWID) << commuT << std::setw(OWID) << migraT
		 << std::setw(OWID) << totalT << std::setw(OWID) << (commuT + migraT)/totalT*100 << std::endl;

      if (isotropicType == 1) {
	if (tractionErrorTol(sigmaVar, "isotropic")) {
	  if (mpiRank == 0) {
	    printParticle("isotropic_particle_end");
	    printBdryContact("isotropic_bdrycntc_end");
	    printBoundary("isotropic_boundary_end");
	    printCompressProg(balancedInf, distX, distY, distZ);
	  }
	  break;
	}
      } else if (isotropicType == 2) {
	if (tractionErrorTol(sigmaVar, "isotropic")) {
	  if (mpiRank == 0) printCompressProg(balancedInf, distX, distY, distZ);
	  sigmaVar += sigmaInc;
	}
	if (tractionErrorTol(sigmaEnd, "isotropic")) {
	  if (mpiRank == 0) {
	    printParticle("isotropic_particle_end");
	    printBdryContact("isotropic_bdrycntc_end");
	    printBoundary("isotropic_boundary_end");
	    printCompressProg(balancedInf, distX, distY, distZ);
	  }
	  break;
	}
      }

      if (isotropicType == 3) {
	if (tractionErrorTol(sigmaVar, "isotropic")) {
	  if (mpiRank == 0) printCompressProg(balancedInf, distX, distY, distZ);
	  sigmaVar += sigmaInc;
	  if (sigmaVar == sigmaPath[sigma_i+1]) {
	    sigmaVar = sigmaPath[++sigma_i];
	    sigmaInc = (sigmaPath[sigma_i+1] -  sigmaPath[sigma_i])/sigmaDiv;
	  }
	}
	if (tractionErrorTol(sigmaEnd, "isotropic")) {
	  if (mpiRank == 0) {
	    printParticle("isotropic_particle_end");
	    printBdryContact("isotropic_bdrycntc_end");
	    printBoundary("isotropic_boundary_end");
	    printCompressProg(balancedInf, distX, distY, distZ);
	  }
	  break;
	}
      }

      ++iteration;
    } 
  
    if (mpiRank == 0) {
      closeProg(progressInf);
      closeProg(balancedInf);
    }
  }


  void Assembly::oedometer() 
  {
    std::size_t oedometerType = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["oedometerType"]);
    if (mpiRank == 0) {
      readParticle(dem::Parameter::getSingleton().datafile["particleFile"].c_str());
      readBoundary(dem::Parameter::getSingleton().datafile["boundaryFile"].c_str());
      openCompressProg(progressInf, "oedometer_progress");
      openCompressProg(balancedInf, "oedometer_balanced");
    }
    scatterParticle();

    std::size_t startStep = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["startStep"]);
    std::size_t endStep   = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["endStep"]);
    std::size_t startSnap = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["startSnap"]);
    std::size_t endSnap   = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["endSnap"]);
    std::size_t netStep   = endStep - startStep + 1;
    std::size_t netSnap   = endSnap - startSnap + 1;
    timeStep = dem::Parameter::getSingleton().parameter["timeStep"];

    REAL sigmaEnd, sigmaInc, sigmaVar;
    std::size_t sigmaDiv;
  
    sigmaEnd = dem::Parameter::getSingleton().parameter["sigmaEnd"];
    sigmaDiv = dem::Parameter::getSingleton().parameter["sigmaDiv"];
    std::vector<REAL> &sigmaPath = dem::Parameter::getSingleton().sigmaPath;
    std::size_t sigma_i = 0;

    if (oedometerType == 1) {
      REAL sigmaStart = dem::Parameter::getSingleton().parameter["sigmaStart"];
      sigmaInc = (sigmaEnd - sigmaStart) / sigmaDiv;
      sigmaVar = sigmaStart;
    } else if (oedometerType == 2) {
      sigmaVar = sigmaPath[sigma_i];
      sigmaInc = (sigmaPath[sigma_i+1] -  sigmaPath[sigma_i])/sigmaDiv;
      sigmaEnd = sigmaPath[sigmaPath.size()-1];
    }

    REAL time0, time1, time2, commuT, migraT, gatherT, totalT;
    iteration = startStep;
    std::size_t iterSnap = startSnap;
    char cstr0[50];
    REAL distX, distY, distZ;
    if (mpiRank == 0) {
      plotBoundary(strcat(combineString(cstr0, "oedometer_bdryplot_", iterSnap - 1, 3), ".dat"));
      plotGrid(strcat(combineString(cstr0, "oedometer_gridplot_", iterSnap - 1, 3), ".dat"));
      printParticle(combineString(cstr0, "oedometer_particle_", iterSnap - 1, 3));
      printBdryContact(combineString(cstr0, "oedometer_bdrycntc_", iterSnap -1, 3));
      printBoundary(combineString(cstr0, "oedometer_boundary_", iterSnap - 1, 3));
      getStartDimension(distX, distY, distZ);
    }
    if (mpiRank == 0)
      debugInf << std::setw(OWID) << "iter" << std::setw(OWID) << "commuT" << std::setw(OWID) << "migraT"
	       << std::setw(OWID) << "totalT" << std::setw(OWID) << "overhead%" << std::endl;
    while (iteration <= endStep) {
      commuT = migraT = gatherT = totalT = 0; time0 = MPI_Wtime();
      commuParticle(); time2 = MPI_Wtime(); commuT = time2 - time0;

      calcTimeStep(); // use values from last step, must call before findContact
      findContact();
      if (isBdryProcess()) findBdryContact();

      clearContactForce();
      internalForce();
      if (isBdryProcess()) boundaryForce();

#ifdef STRESS_STRAIN
      if ((iteration + 2) % (netStep / netSnap) == 0)
	calcPrevGranularStress(); // compute stress in previous time step

      if ((iteration + 1) % (netStep / netSnap) == 0) {
	gatherGranularStress(timeStep, netStep / netSnap * timeStep); //ensure both contact forces and particle locations are in current step.
        snapParticlePos(); // snapshort particle positions
      }
#endif

      updateParticle();
      gatherBdryContact(); // must call before updateBoundary
   
      if (iteration % (netStep / netSnap) == 0) {
	time1 = MPI_Wtime();
	gatherParticle();
	gatherEnergy(); time2 = MPI_Wtime(); gatherT = time2 - time1;

	char cstr[50];
	if (mpiRank == 0) {
	  plotBoundary(strcat(combineString(cstr, "oedometer_bdryplot_", iterSnap, 3), ".dat"));
	  plotGrid(strcat(combineString(cstr, "oedometer_gridplot_", iterSnap, 3), ".dat"));
	  printParticle(combineString(cstr, "oedometer_particle_", iterSnap, 3));
	  printBdryContact(combineString(cstr, "oedometer_bdrycntc_", iterSnap, 3));
	  printBoundary(combineString(cstr, "oedometer_boundary_", iterSnap, 3));
	  printCompressProg(progressInf, distX, distY, distZ);
#ifdef STRESS_STRAIN
	  printGranularStressFEM(strcat(combineString(cstr, "oedometer_stress_plot_", iterSnap, 3), ".dat"));
	  printGranularStressOrdered(strcat(combineString(cstr, "oedometer_stress_data_", iterSnap, 3), ".dat"));
#endif
	}
	printContact(combineString(cstr, "oedometer_contact_", iterSnap, 3));      
	++iterSnap;
      }

      updateBoundary(sigmaVar, "oedometer"); // must call after printBdryContact
      updateGrid();

      releaseRecvParticle(); // late release because printContact refers to received particles
      time1 = MPI_Wtime();
      migrateParticle(); time2 = MPI_Wtime(); migraT = time2 - time1; totalT = time2 - time0;
      if (mpiRank == 0 && (iteration+1 ) % (netStep / netSnap) == 0) // ignore gather and print time at this step
	debugInf << std::setw(OWID) << iteration << std::setw(OWID) << commuT << std::setw(OWID) << migraT
		 << std::setw(OWID) << totalT << std::setw(OWID) << (commuT + migraT)/totalT*100 << std::endl;

      if (oedometerType == 1) {
	if (tractionErrorTol(sigmaVar, "oedometer")) {
	  if (mpiRank == 0) printCompressProg(balancedInf, distX, distY, distZ);
	  sigmaVar += sigmaInc;
	}
	if (tractionErrorTol(sigmaEnd, "oedometer")) {
	  if (mpiRank == 0) {
	    printParticle("oedometer_particle_end");
	    printBdryContact("oedometer_bdrycntc_end");
	    printBoundary("oedometer_boundary_end");
	    printCompressProg(balancedInf, distX, distY, distZ);
	  }
	  break;
	}
      } else if (oedometerType == 2) {
	if (tractionErrorTol(sigmaVar, "oedometer")) {
	  if (mpiRank == 0) printCompressProg(balancedInf, distX, distY, distZ);
	  sigmaVar += sigmaInc;
	  if (sigmaVar == sigmaPath[sigma_i+1]) {
	    sigmaVar = sigmaPath[++sigma_i];
	    sigmaInc = (sigmaPath[sigma_i+1] -  sigmaPath[sigma_i])/sigmaDiv;
	  }
	}
	if (tractionErrorTol(sigmaEnd, "oedometer")) {
	  if (mpiRank == 0) {
	    printParticle("oedometer_particle_end");
	    printBdryContact("oedometer_bdrycntc_end");
	    printBoundary("oedometer_boundary_end");
	    printCompressProg(balancedInf, distX, distY, distZ);
	  }
	  break;
	}
      }

      ++iteration;
    } 
  
    if (mpiRank == 0) {
      closeProg(progressInf);
      closeProg(balancedInf);
    }
  }


  void Assembly::triaxial() 
  {
    if (mpiRank == 0) {
      readParticle(dem::Parameter::getSingleton().datafile["particleFile"].c_str());
      readBoundary(dem::Parameter::getSingleton().datafile["boundaryFile"].c_str());
      openCompressProg(progressInf, "triaxial_progress");
    }
    scatterParticle();

    std::size_t startStep = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["startStep"]);
    std::size_t endStep   = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["endStep"]);
    std::size_t startSnap = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["startSnap"]);
    std::size_t endSnap   = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["endSnap"]);
    std::size_t netStep   = endStep - startStep + 1;
    std::size_t netSnap   = endSnap - startSnap + 1; 
    REAL sigmaConf = dem::Parameter::getSingleton().parameter["sigmaConf"];
    timeStep = dem::Parameter::getSingleton().parameter["timeStep"];

    REAL time0, time1, time2, commuT, migraT, gatherT, totalT;
    iteration = startStep;
    std::size_t iterSnap = startSnap;
    char cstr0[50];
    REAL distX, distY, distZ;
    if (mpiRank == 0) {
      plotBoundary(strcat(combineString(cstr0, "triaxial_bdryplot_", iterSnap - 1, 3), ".dat"));
      plotGrid(strcat(combineString(cstr0, "triaxial_gridplot_", iterSnap - 1, 3), ".dat"));
      printParticle(combineString(cstr0, "triaxial_particle_", iterSnap - 1, 3));
      printBdryContact(combineString(cstr0, "triaxial_bdrycntc_", iterSnap -1, 3));
      printBoundary(combineString(cstr0, "triaxial_boundary_", iterSnap - 1, 3));
      getStartDimension(distX, distY, distZ);
    }
    if (mpiRank == 0)
      debugInf << std::setw(OWID) << "iter" << std::setw(OWID) << "commuT" << std::setw(OWID) << "migraT"
	       << std::setw(OWID) << "totalT" << std::setw(OWID) << "overhead%" << std::endl;
    while (iteration <= endStep) {
      commuT = migraT = gatherT = totalT = 0; time0 = MPI_Wtime();
      commuParticle(); time2 = MPI_Wtime(); commuT = time2 - time0;

      // displacement control relies on constant time step, so do not call calcTimeStep().
      //calcTimeStep(); // use values from last step, must call before findContact
      findContact();
      if (isBdryProcess()) findBdryContact();

      clearContactForce();
      internalForce();
      if (isBdryProcess()) boundaryForce();

#ifdef STRESS_STRAIN
      if ((iteration + 2) % (netStep / netSnap) == 0)
	calcPrevGranularStress(); // compute stress in previous time step

      if ((iteration + 1) % (netStep / netSnap) == 0) {
	gatherGranularStress(timeStep, netStep / netSnap * timeStep); //ensure both contact forces and particle locations are in current step.
        snapParticlePos(); // snapshort particle positions
      }
#endif

      updateParticle();
      gatherBdryContact(); // must call before updateBoundary
   
      if (iteration % (netStep / netSnap) == 0) {
	time1 = MPI_Wtime();
	gatherParticle();
	gatherEnergy(); time2 = MPI_Wtime(); gatherT = time2 - time1;

	char cstr[50];
	if (mpiRank == 0) {
	  plotBoundary(strcat(combineString(cstr, "triaxial_bdryplot_", iterSnap, 3), ".dat"));
	  plotGrid(strcat(combineString(cstr, "triaxial_gridplot_", iterSnap, 3), ".dat"));
	  printParticle(combineString(cstr, "triaxial_particle_", iterSnap, 3));
	  printBdryContact(combineString(cstr, "triaxial_bdrycntc_", iterSnap, 3));
	  printBoundary(combineString(cstr, "triaxial_boundary_", iterSnap, 3));
	  //printCompressProg(progressInf, distX, distY, distZ); // redundant
#ifdef STRESS_STRAIN
	  printGranularStressFEM(strcat(combineString(cstr, "triaxial_stress_plot_", iterSnap, 3), ".dat"));
	  printGranularStressOrdered(strcat(combineString(cstr, "triaxial_stress_data_", iterSnap, 3), ".dat"));
#endif
	}
	printContact(combineString(cstr, "triaxial_contact_", iterSnap, 3));      
	++iterSnap;
      }

      updateBoundary(sigmaConf, "triaxial"); // must call after printBdryContact
      updateGrid();

      releaseRecvParticle(); // late release because printContact refers to received particles
      time1 = MPI_Wtime();
      migrateParticle(); time2 = MPI_Wtime(); migraT = time2 - time1; totalT = time2 - time0;
      if (mpiRank == 0 && (iteration+1 ) % (netStep / netSnap) == 0) // ignore gather and print time at this step
	debugInf << std::setw(OWID) << iteration << std::setw(OWID) << commuT << std::setw(OWID) << migraT
		 << std::setw(OWID) << totalT << std::setw(OWID) << (commuT + migraT)/totalT*100 << std::endl;

      if (mpiRank == 0 && iteration % 10 == 0)
	printCompressProg(progressInf, distX, distY, distZ);

      // no break condition, just through top/bottom displacement control
      ++iteration;
    } 

    if (mpiRank == 0) {
      printParticle("triaxial_particle_end");
      printBdryContact("triaxial_bdrycntc_end");
      printBoundary("triaxial_boundary_end");
      printCompressProg(progressInf, distX, distY, distZ);
    }
  
    if (mpiRank == 0) closeProg(progressInf);
  }


  void Assembly::planeStrain() 
  {
    if (mpiRank == 0) {
      readParticle(dem::Parameter::getSingleton().datafile["particleFile"].c_str());
      readBoundary(dem::Parameter::getSingleton().datafile["boundaryFile"].c_str());
      openCompressProg(progressInf, "plnstrn_progress");
    }
    scatterParticle();

    std::size_t startStep = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["startStep"]);
    std::size_t endStep   = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["endStep"]);
    std::size_t startSnap = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["startSnap"]);
    std::size_t endSnap   = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["endSnap"]);
    std::size_t netStep   = endStep - startStep + 1;
    std::size_t netSnap   = endSnap - startSnap + 1; 
    REAL sigmaConf = dem::Parameter::getSingleton().parameter["sigmaConf"];
    timeStep = dem::Parameter::getSingleton().parameter["timeStep"];

    REAL time0, time1, time2, commuT, migraT, gatherT, totalT;
    iteration = startStep;
    std::size_t iterSnap = startSnap;
    char cstr0[50];
    REAL distX, distY, distZ;
    if (mpiRank == 0) {
      plotBoundary(strcat(combineString(cstr0, "plnstrn_bdryplot_", iterSnap - 1, 3), ".dat"));
      plotGrid(strcat(combineString(cstr0, "plnstrn_gridplot_", iterSnap - 1, 3), ".dat"));
      printParticle(combineString(cstr0, "plnstrn_particle_", iterSnap - 1, 3));
      printBdryContact(combineString(cstr0, "plnstrn_bdrycntc_", iterSnap -1, 3));
      printBoundary(combineString(cstr0, "plnstrn_boundary_", iterSnap - 1, 3));
      getStartDimension(distX, distY, distZ);
    }
    if (mpiRank == 0)
      debugInf << std::setw(OWID) << "iter" << std::setw(OWID) << "commuT" << std::setw(OWID) << "migraT"
	       << std::setw(OWID) << "totalT" << std::setw(OWID) << "overhead%" << std::endl;
    while (iteration <= endStep) {
      commuT = migraT = gatherT = totalT = 0; time0 = MPI_Wtime();
      commuParticle(); time2 = MPI_Wtime(); commuT = time2 - time0;

      // displacement control relies on constant time step, so do not call calcTimeStep().
      //calcTimeStep(); // use values from last step, must call before findContact
      findContact();
      if (isBdryProcess()) findBdryContact();

      clearContactForce();
      internalForce();
      if (isBdryProcess()) boundaryForce();

#ifdef STRESS_STRAIN
      if ((iteration + 2) % (netStep / netSnap) == 0)
	calcPrevGranularStress(); // compute stress in previous time step

      if ((iteration + 1) % (netStep / netSnap) == 0) {
	gatherGranularStress(timeStep, netStep / netSnap * timeStep); //ensure both contact forces and particle locations are in current step.
        snapParticlePos(); // snapshort particle positions
      }
#endif

      updateParticle();
      gatherBdryContact(); // must call before updateBoundary
   
      if (iteration % (netStep / netSnap) == 0) {
	time1 = MPI_Wtime();
	gatherParticle();
	gatherEnergy(); time2 = MPI_Wtime(); gatherT = time2 - time1;

	char cstr[50];
	if (mpiRank == 0) {
	  plotBoundary(strcat(combineString(cstr, "plnstrn_bdryplot_", iterSnap, 3), ".dat"));
	  plotGrid(strcat(combineString(cstr, "plnstrn_gridplot_", iterSnap, 3), ".dat"));
	  printParticle(combineString(cstr, "plnstrn_particle_", iterSnap, 3));
	  printBdryContact(combineString(cstr, "plnstrn_bdrycntc_", iterSnap, 3));
	  printBoundary(combineString(cstr, "plnstrn_boundary_", iterSnap, 3));
	  //printCompressProg(progressInf, distX, distY, distZ); // redundant
#ifdef STRESS_STRAIN
	  printGranularStressFEM(strcat(combineString(cstr, "plnstrn_stress_plot_", iterSnap, 3), ".dat"));
	  printGranularStressOrdered(strcat(combineString(cstr, "plnstrn_stress_data_", iterSnap, 3), ".dat"));
#endif
	}
	printContact(combineString(cstr, "plnstrn_contact_", iterSnap, 3));      
	++iterSnap;
      }

      updateBoundary(sigmaConf, "plnstrn"); // must call after printBdryContact
      updateGrid();

      releaseRecvParticle(); // late release because printContact refers to received particles
      time1 = MPI_Wtime();
      migrateParticle(); time2 = MPI_Wtime(); migraT = time2 - time1; totalT = time2 - time0;
      if (mpiRank == 0 && (iteration+1 ) % (netStep / netSnap) == 0) // ignore gather and print time at this step
	debugInf << std::setw(OWID) << iteration << std::setw(OWID) << commuT << std::setw(OWID) << migraT
		 << std::setw(OWID) << totalT << std::setw(OWID) << (commuT + migraT)/totalT*100 << std::endl;

      if (mpiRank == 0 && iteration % 10 == 0)
	printCompressProg(progressInf, distX, distY, distZ);

      // no break condition, just through top/bottom displacement control
      ++iteration;
    } 

    if (mpiRank == 0) {
      printParticle("plnstrn_particle_end");
      printBdryContact("plnstrn_bdrycntc_end");
      printBoundary("plnstrn_boundary_end");
      printCompressProg(progressInf, distX, distY, distZ);
    }
  
    if (mpiRank == 0) closeProg(progressInf);
  }


  void Assembly::trueTriaxial() 
  {
    std::size_t trueTriaxialType = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["trueTriaxialType"]);
    if (mpiRank == 0) {
      readParticle(dem::Parameter::getSingleton().datafile["particleFile"].c_str());
      readBoundary(dem::Parameter::getSingleton().datafile["boundaryFile"].c_str());
      openCompressProg(progressInf, "trueTriaxial_progress");
      openCompressProg(balancedInf, "trueTriaxial_balanced");
    }
    scatterParticle();

    std::size_t startStep = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["startStep"]);
    std::size_t endStep   = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["endStep"]);
    std::size_t startSnap = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["startSnap"]);
    std::size_t endSnap   = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["endSnap"]);
    std::size_t netStep   = endStep - startStep + 1;
    std::size_t netSnap   = endSnap - startSnap + 1; 
    timeStep = dem::Parameter::getSingleton().parameter["timeStep"];

    REAL sigmaStart, sigmaEndZ, sigmaEndX, sigmaEndY;
    REAL sigmaDiv, sigmaIncZ, sigmaIncX, sigmaIncY, sigmaVarZ, sigmaVarX, sigmaVarY;
    REAL sigmaInit[3], sigmaEnd, sigmaInc, sigmaVar;
    std::size_t changeDirc;
    sigmaDiv = dem::Parameter::getSingleton().parameter["sigmaDiv"];

    if (trueTriaxialType == 1) {
      sigmaStart = dem::Parameter::getSingleton().parameter["sigmaStart"];
      sigmaEndZ  = dem::Parameter::getSingleton().parameter["sigmaEndZ"];
      sigmaEndX  = dem::Parameter::getSingleton().parameter["sigmaEndX"];
      sigmaEndY  = dem::Parameter::getSingleton().parameter["sigmaEndY"];
      sigmaIncZ  = (sigmaEndZ - sigmaStart) / sigmaDiv;
      sigmaIncX  = (sigmaEndX - sigmaStart) / sigmaDiv;
      sigmaIncY  = (sigmaEndY - sigmaStart) / sigmaDiv;
      sigmaVarZ  = sigmaStart;
      sigmaVarX  = sigmaStart;
      sigmaVarY  = sigmaStart;
    } else if (trueTriaxialType == 2) {
      sigmaInit[0] = dem::Parameter::getSingleton().parameter["sigmaStartX"];
      sigmaInit[1] = dem::Parameter::getSingleton().parameter["sigmaStartY"];
      sigmaInit[2] = dem::Parameter::getSingleton().parameter["sigmaStartZ"];
      sigmaEnd     = dem::Parameter::getSingleton().parameter["sigmaEnd"];
      changeDirc   = dem::Parameter::getSingleton().parameter["changeDirc"];
      sigmaInc     = (sigmaEnd - sigmaInit[changeDirc]) / sigmaDiv;
      sigmaVar     = sigmaInit[changeDirc];
    }

    REAL time0, time1, time2, commuT, migraT, gatherT, totalT;
    iteration = startStep;
    std::size_t iterSnap = startSnap;
    char cstr0[50];
    REAL distX, distY, distZ;
    if (mpiRank == 0) {
      plotBoundary(strcat(combineString(cstr0, "trueTriaxial_bdryplot_", iterSnap - 1, 3), ".dat"));
      plotGrid(strcat(combineString(cstr0, "trueTriaxial_gridplot_", iterSnap - 1, 3), ".dat"));
      printParticle(combineString(cstr0, "trueTriaxial_particle_", iterSnap - 1, 3));
      printBdryContact(combineString(cstr0, "trueTriaxial_bdrycntc_", iterSnap -1, 3));
      printBoundary(combineString(cstr0, "trueTriaxial_boundary_", iterSnap - 1, 3));
      getStartDimension(distX, distY, distZ);
    }
    if (mpiRank == 0)
      debugInf << std::setw(OWID) << "iter" << std::setw(OWID) << "commuT" << std::setw(OWID) << "migraT"
	       << std::setw(OWID) << "totalT" << std::setw(OWID) << "overhead%" << std::endl;
    while (iteration <= endStep) {
      commuT = migraT = gatherT = totalT = 0; time0 = MPI_Wtime();
      commuParticle(); time2 = MPI_Wtime(); commuT = time2 - time0;

      calcTimeStep(); // use values from last step, must call before findContact
      findContact();
      if (isBdryProcess()) findBdryContact();

      clearContactForce();
      internalForce();
      if (isBdryProcess()) boundaryForce();

#ifdef STRESS_STRAIN
      if ((iteration + 2) % (netStep / netSnap) == 0)
	calcPrevGranularStress(); // compute stress in previous time step

      if ((iteration + 1) % (netStep / netSnap) == 0) {
	gatherGranularStress(timeStep, netStep / netSnap * timeStep); //ensure both contact forces and particle locations are in current step.
        snapParticlePos(); // snapshort particle positions
      }
#endif

      updateParticle();
      gatherBdryContact(); // must call before updateBoundary
   
      if (iteration % (netStep / netSnap) == 0) {
	time1 = MPI_Wtime();
	gatherParticle();
	gatherEnergy(); time2 = MPI_Wtime(); gatherT = time2 - time1;

	char cstr[50];
	if (mpiRank == 0) {
	  plotBoundary(strcat(combineString(cstr, "trueTriaxial_bdryplot_", iterSnap, 3), ".dat"));
	  plotGrid(strcat(combineString(cstr, "trueTriaxial_gridplot_", iterSnap, 3), ".dat"));
	  printParticle(combineString(cstr, "trueTriaxial_particle_", iterSnap, 3));
	  printBdryContact(combineString(cstr, "trueTriaxial_bdrycntc_", iterSnap, 3));
	  printBoundary(combineString(cstr, "trueTriaxial_boundary_", iterSnap, 3));
	  printCompressProg(progressInf, distX, distY, distZ);
#ifdef STRESS_STRAIN
	  printGranularStressFEM(strcat(combineString(cstr, "trueTriaxial_stress_plot_", iterSnap, 3), ".dat"));
	  printGranularStressOrdered(strcat(combineString(cstr, "trueTriaxial_stress_data_", iterSnap, 3), ".dat"));
#endif
	}
	printContact(combineString(cstr, "trueTriaxial_contact_", iterSnap, 3));      
	++iterSnap;
      }

      if (trueTriaxialType == 1)
	updateBoundary(sigmaVarZ, "trueTriaxial", sigmaVarX, sigmaVarY); // must call after printBdryContact
      else if (trueTriaxialType == 2) {
	REAL sigmaX, sigmaY, sigmaZ;
	if (changeDirc == 0) {
	  sigmaX = sigmaVar;     sigmaY = sigmaInit[1]; sigmaZ = sigmaInit[2];
	} else if (changeDirc == 1) {
	  sigmaX = sigmaInit[0]; sigmaY = sigmaVar;     sigmaZ = sigmaInit[2];
	} else if (changeDirc == 2) {
	  sigmaX = sigmaInit[0]; sigmaY = sigmaInit[1]; sigmaZ = sigmaVar;
	}
	updateBoundary(sigmaZ, "trueTriaxial", sigmaX, sigmaY); // must call after printBdryContact
      }
      updateGrid();

      releaseRecvParticle(); // late release because printContact refers to received particles
      time1 = MPI_Wtime();
      migrateParticle(); time2 = MPI_Wtime(); migraT = time2 - time1; totalT = time2 - time0;
      if (mpiRank == 0 && (iteration+1 ) % (netStep / netSnap) == 0) // ignore gather and print time at this step
	debugInf << std::setw(OWID) << iteration << std::setw(OWID) << commuT << std::setw(OWID) << migraT
		 << std::setw(OWID) << totalT << std::setw(OWID) << (commuT + migraT)/totalT*100 << std::endl;

      if (trueTriaxialType == 1) {
	if (tractionErrorTol(sigmaVarZ, "trueTriaxial", sigmaVarX, sigmaVarY)) {
	  if (mpiRank == 0) printCompressProg(balancedInf, distX, distY, distZ);
	  sigmaVarZ += sigmaIncZ;
	  sigmaVarX += sigmaIncX;
	  sigmaVarY += sigmaIncY;
	}
	if (tractionErrorTol(sigmaEndZ, "trueTriaxial", sigmaEndX, sigmaEndY)) {
	  if (mpiRank == 0) {
	    printParticle("trueTriaxial_particle_end");
	    printBdryContact("trueTriaxial_bdrycntc_end");
	    printBoundary("trueTriaxial_boundary_end");
	    printCompressProg(balancedInf, distX, distY, distZ);
	  }
	  break;
	}
      } else if (trueTriaxialType == 2) {
	REAL sigmaX, sigmaY, sigmaZ;
	if (changeDirc == 0) {
	  sigmaX = sigmaVar;     sigmaY = sigmaInit[1]; sigmaZ = sigmaInit[2];
	} else if (changeDirc == 1) {
	  sigmaX = sigmaInit[0]; sigmaY = sigmaVar;     sigmaZ = sigmaInit[2];
	} else if (changeDirc == 2) {
	  sigmaX = sigmaInit[0]; sigmaY = sigmaInit[1]; sigmaZ = sigmaVar;
	}
	if (tractionErrorTol(sigmaZ, "trueTriaxial", sigmaX, sigmaY)) {
	  if (mpiRank == 0) printCompressProg(balancedInf, distX, distY, distZ);
	  sigmaVar += sigmaInc;
	} 
   
	if (changeDirc == 0) {
	  sigmaX = sigmaEnd;     sigmaY = sigmaInit[1]; sigmaZ = sigmaInit[2];
	} else if (changeDirc == 1) {
	  sigmaX = sigmaInit[0]; sigmaY = sigmaEnd;     sigmaZ = sigmaInit[2];
	} else if (changeDirc == 2) {
	  sigmaX = sigmaInit[0]; sigmaY = sigmaInit[1]; sigmaZ = sigmaEnd;
	}
	if (tractionErrorTol(sigmaZ, "trueTriaxial", sigmaX, sigmaY)) {
	  if (mpiRank == 0) {
	    printParticle("trueTriaxial_particle_end");
	    printBdryContact("trueTriaxial_bdrycntc_end");
	    printBoundary("trueTriaxial_boundary_end");
	    printCompressProg(balancedInf, distX, distY, distZ);
	  }
	  break;
	}
      }

      ++iteration;
    } 

    if (mpiRank == 0) {
      printParticle("trueTriaxial_particle_end");
      printBdryContact("trueTriaxial_bdrycntc_end");
      printBoundary("trueTriaxial_boundary_end");
      printCompressProg(progressInf, distX, distY, distZ);
    }

    if (mpiRank == 0) {
      closeProg(progressInf);
      closeProg(balancedInf);
    }
  }


  bool Assembly::tractionErrorTol(REAL sigma, std::string type, REAL sigmaX, REAL sigmaY) {
    // sigma implies sigmaZ
    REAL tol = dem::Parameter::getSingleton().parameter["tractionErrorTol"];

    std::map<std::string, REAL> normalForce;
    REAL x1, x2, y1, y2, z1, z2;
    // do not use mergeBoundaryVec because each process calls this function.
    for(std::vector<Boundary*>::const_iterator it = boundaryVec.begin(); it != boundaryVec.end(); ++it) {
      std::size_t id = (*it)->getId();
      Vec normal = (*it)->getNormalForce();
      Vec point  = (*it)->getPoint();
      switch (id) {
      case 1: 
	normalForce["x1"] = fabs(normal.getX());
	x1 = point.getX();
	break;
      case 2:
	normalForce["x2"] = normal.getX();
	x2 = point.getX();
	break;
      case 3:
	normalForce["y1"] = fabs(normal.getY());
	y1 = point.getY();
	break;
      case 4:
	normalForce["y2"] = normal.getY();
	y2 = point.getY();
	break;
      case 5:
	normalForce["z1"] = fabs(normal.getZ());
	z1 = point.getZ();
	break;
      case 6:
	normalForce["z2"] = normal.getZ();
	z2 = point.getZ();
	break;
      }
    }
    REAL areaX = (y2 - y1) * (z2 - z1);
    REAL areaY = (z2 - z1) * (x2 - x1);
    REAL areaZ = (x2 - x1) * (y2 - y1);

    if (type.compare("isotropic") == 0)
      return (    fabs(normalForce["x1"]/areaX - sigma) / sigma <= tol
		  && fabs(normalForce["x2"]/areaX - sigma) / sigma <= tol
		  && fabs(normalForce["y1"]/areaY - sigma) / sigma <= tol
		  && fabs(normalForce["y2"]/areaY - sigma) / sigma <= tol
		  && fabs(normalForce["z1"]/areaZ - sigma) / sigma <= tol
		  && fabs(normalForce["z2"]/areaZ - sigma) / sigma <= tol );

    else if (type.compare("oedometer") == 0)
      return ( fabs(normalForce["z1"]/areaZ - sigma) / sigma <= tol
	       && fabs(normalForce["z2"]/areaZ - sigma) / sigma <= tol );

    else if (type.compare("triaxial") == 0)
      return true; // always near equilibrium

    else if (type.compare("trueTriaxial") == 0)
      return (    fabs(normalForce["x1"]/areaX - sigmaX) / sigmaX <= tol
		  && fabs(normalForce["x2"]/areaX - sigmaX) / sigmaX <= tol
		  && fabs(normalForce["y1"]/areaY - sigmaY) / sigmaY <= tol
		  && fabs(normalForce["y2"]/areaY - sigmaY) / sigmaY <= tol
		  && fabs(normalForce["z1"]/areaZ - sigma)  / sigma  <= tol
		  && fabs(normalForce["z2"]/areaZ - sigma)  / sigma  <= tol );
  }


  void Assembly::coupleWithGas() 
  {
    if (mpiRank == 0) {
      readParticle(dem::Parameter::getSingleton().datafile["particleFile"].c_str());
      readBoundary(dem::Parameter::getSingleton().datafile["boundaryFile"].c_str());
      openDepositProg(progressInf, "couple_progress");
    }
    scatterParticle();
    /*1*/ gas.initParameter(allContainer, gradation);
    /*2*/ gas.initialize();

    std::size_t startStep = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["startStep"]);
    std::size_t endStep   = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["endStep"]);
    std::size_t startSnap = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["startSnap"]);
    std::size_t endSnap   = static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["endSnap"]);
    std::size_t netStep   = endStep - startStep + 1;
    std::size_t netSnap   = endSnap - startSnap + 1;
    timeStep = dem::Parameter::getSingleton().parameter["timeStep"];

    REAL time0, time1, time2, commuT, migraT, gatherT, totalT;
    iteration = startStep;
    std::size_t iterSnap = startSnap;
    char cstr0[50];
    REAL timeCount = 0;
    timeAccrued = static_cast<REAL> (dem::Parameter::getSingleton().parameter["timeAccrued"]);
    REAL timeIncr  = timeStep * netStep;
    REAL timeTotal = timeAccrued + timeIncr;
    if (mpiRank == 0) {
      plotBoundary(strcat(combineString(cstr0, "couple_bdryplot_", iterSnap - 1, 3), ".dat"));
      plotGrid(strcat(combineString(cstr0, "couple_gridplot_", iterSnap - 1, 3), ".dat"));
      printParticle(combineString(cstr0, "couple_particle_", iterSnap - 1, 3));
      printBdryContact(combineString(cstr0, "couple_bdrycntc_", iterSnap -1, 3));
      /*3*/ gas.plot(strcat(combineString(cstr0, "couple_fluidplot_", iterSnap -1, 3), ".dat")); 
    }
    /*
    if (mpiRank == 0)
      debugInf << std::setw(OWID) << "iter" << std::setw(OWID) << "commuT" << std::setw(OWID) << "migraT"
	       << std::setw(OWID) << "totalT" << std::setw(OWID) << "overhead%" << std::endl;
    */

    struct timeval time_1, time_2, time_3, time_4, time_5, time_6, time_7;
    while (timeAccrued < timeTotal) {
#ifdef CFD_PROFILE
      gettimeofday(&time_1, NULL);
#endif
      commuT = migraT = gatherT = totalT = 0; time0 = MPI_Wtime();
      commuParticle(); time2 = MPI_Wtime(); commuT = time2 - time0;

      calcTimeStep(); // use values from last step, must call before findContact

      findContact();
      if (isBdryProcess()) findBdryContact();

      clearContactForce();

#ifdef CFD_PROFILE
      gettimeofday(&time_2, NULL);
#endif
      /*4*/ gas.getPtclInfo(particleVec, gradation); // not allParticleVec
#ifdef CFD_PROFILE
      gettimeofday(&time_3, NULL);
#endif
      /*5*/ gas.runOneStep(particleVec);
#ifdef CFD_PROFILE
      gettimeofday(&time_4, NULL);
#endif
      /*6*/ gas.calcPtclForce(particleVec); // not allParticleVec
#ifdef CFD_PROFILE
      gettimeofday(&time_5, NULL);
#endif
      /*7*/ gas.penalize(particleVec);
#ifdef CFD_PROFILE
      gettimeofday(&time_6, NULL);
#endif
      //gas.checkMomentum(particleVec);

      internalForce();
      if (isBdryProcess()) boundaryForce();

      updateParticle();
      updateGridMaxZ();

      timeCount += timeStep;
      //timeAccrued += timeStep; // note gas.runOneStep() might change timeStep and print timeAccrued
      if (timeCount >= timeIncr/netSnap) { 
	time1 = MPI_Wtime();
	gatherParticle();
	gatherBdryContact();
	gatherEnergy(); time2 = MPI_Wtime(); gatherT = time2 - time1;

	char cstr[50];
	if (mpiRank == 0) {
	  plotBoundary(strcat(combineString(cstr, "couple_bdryplot_", iterSnap, 3), ".dat"));
	  plotGrid(strcat(combineString(cstr, "couple_gridplot_", iterSnap, 3), ".dat"));
	  printParticle(combineString(cstr, "couple_particle_", iterSnap, 3));
	  printBdryContact(combineString(cstr, "couple_bdrycntc_", iterSnap, 3));
	  printDepositProg(progressInf);
	  /*8*/ gas.plot(strcat(combineString(cstr, "couple_fluidplot_", iterSnap, 3), ".dat"));
	}
	printContact(combineString(cstr, "couple_contact_", iterSnap, 3));
      
	timeCount = 0;
	++iterSnap;
      }

      releaseRecvParticle(); // late release because printContact refers to received particles
      time1 = MPI_Wtime();
      migrateParticle(); time2 = MPI_Wtime(); migraT = time2 - time1; totalT = time2 - time0;
      /*
      if (mpiRank == 0 && (iteration+1 ) % (netStep / netSnap) == 0) // ignore gather and print time at this step
	debugInf << std::setw(OWID) << iteration << std::setw(OWID) << commuT << std::setw(OWID) << migraT
		 << std::setw(OWID) << totalT << std::setw(OWID) << (commuT + migraT)/totalT*100 << std::endl;
      */

#ifdef CFD_PROFILE
      gettimeofday(&time_7, NULL);
      debugInf << std::setw(OWID) << timediffsec(time_2, time_3)
	       << std::setw(OWID) << timediffsec(time_3, time_4)
	       << std::setw(OWID) << timediffsec(time_4, time_5)
	       << std::setw(OWID) << timediffsec(time_5, time_6)
	       << std::setw(OWID) << timediffsec(time_2, time_6)
	       << std::setw(OWID) << timediffsec(time_1, time_7) - timediffsec(time_2, time_6);
#endif

      ++iteration;
    } 
  
    if (mpiRank == 0)
      closeProg(progressInf);
  }


  // particleLayers:
  // 0 - one free particle
  // 1 - a horizontal layer of free particles
  // 2 - multiple layers of free particles
  void Assembly::generateParticle(std::size_t particleLayers,
				  const char *genParticle)
  {
    int bottomGap= dem::Parameter::getSingleton().parameter["bottomGap"];
    int sideGap  = dem::Parameter::getSingleton().parameter["sideGap"];
    int layerGap = dem::Parameter::getSingleton().parameter["layerGap"];
    int genMode  = dem::Parameter::getSingleton().parameter["genMode"];

    REAL young   = dem::Parameter::getSingleton().parameter["young"];
    REAL poisson = dem::Parameter::getSingleton().parameter["poisson"];
    REAL diaMax  = gradation.getPtclMaxRadius() * 2.0;
    REAL diaMin  = gradation.getPtclMinRadius(0) * 2.0;
    std::size_t particleNum = 0;   
    REAL x,y,z;
    Particle *newptcl = NULL;

    REAL edge   = diaMax * sideGap;
    REAL offset = diaMax * 0.25; // +- makes 0.5

    REAL x1 = allContainer.getMinCorner().getX() + edge;
    REAL y1 = allContainer.getMinCorner().getY() + edge;
    REAL z1 = allContainer.getMinCorner().getZ() + diaMax*bottomGap;
    REAL x2 = allContainer.getMaxCorner().getX() - edge;
    REAL y2 = allContainer.getMaxCorner().getY() - edge;
    REAL z2 = dem::Parameter::getSingleton().parameter["floatMaxZ"] - diaMax;
    REAL x0 = allContainer.getCenter().getX();
    REAL y0 = allContainer.getCenter().getY();
    REAL z0 = allContainer.getCenter().getZ();

    if (particleLayers == 0) {      // just one free particle
      newptcl = new Particle(particleNum+1, 0, Vec(x0,y0,z0), gradation, young, poisson);
      allParticleVec.push_back(newptcl);
      ++particleNum;
    }
    else if (particleLayers == 1) { // a horizontal layer of free particles
      for (x = x1; x - x2 < EPS; x += diaMax)
	for (y = y1; y - y2 < EPS; y += diaMax) {
	  newptcl = new Particle(particleNum+1, 0, Vec(x,y,z0), gradation, young, poisson);
	  allParticleVec.push_back(newptcl);
	  ++particleNum;
	}
    }
    else if (particleLayers == 2) { // multiple layers of free particles
      // small variety of particle sizes
      if (genMode == 0) { // from side to side
	REAL diaRelax = 5.0E-3;
	for (z = z1; z - z2 < EPS; z += diaMax * (layerGap + 1)) {
	  // from - to + direction
	  for (x = x1; x - x2 < EPS; x += diaMax * (diaRelax + 1)) {
	    for (y = y1; y - y2 < EPS; y += diaMax * (diaRelax + 1)) {
	      newptcl = new Particle(particleNum+1, 0, Vec(x,y,z), gradation, young, poisson);
	      newptcl->setCurrPos(Vec(x + offset, y + offset, z)); // z should not offset
	      allParticleVec.push_back(newptcl);
	      ++particleNum;
	    }
	  }
	  offset *= -1;
	}
      } // end of genMode == 0
      else if (genMode == 1) { // xy-symmetric
	for (z = z1; z - z2 < EPS; z += diaMax * (layerGap + 1)) {
	  // + + 
	  for (x = x0 + diaMax/2 + fabs(offset) + offset; x - (x2 + ref(offset)) < EPS; x += diaMax) {
	    for (y = y0 + diaMax/2 + fabs(offset) + offset; y - (y2 + ref(offset)) < EPS; y += diaMax) {
	      newptcl = new Particle(particleNum+1, 0, Vec(x,y,z), gradation, young, poisson);
	      allParticleVec.push_back(newptcl);
	      ++particleNum;
	    }	
	  }
	  // - +
	  for (x = x0 - diaMax/2 - fabs(offset) - offset; x - x1 > EPS; x -= diaMax) {
	    for (y = y0 + diaMax/2 + fabs(offset) + offset; y - (y2 + ref(offset)) < EPS; y += diaMax) {
	      newptcl = new Particle(particleNum+1, 0, Vec(x,y,z), gradation, young, poisson);
	      allParticleVec.push_back(newptcl);
	      ++particleNum;
	    }	
	  }
	  // + - 
	  for (x = x0 + diaMax/2 + fabs(offset) + offset; x - (x2 + ref(offset)) < EPS; x += diaMax) {
	    for (y = y0 - diaMax/2 - fabs(offset) - offset; y - y1 > EPS; y -= diaMax) {
	      newptcl = new Particle(particleNum+1, 0, Vec(x,y,z), gradation, young, poisson);
	      allParticleVec.push_back(newptcl);
	      ++particleNum;
	    }	
	  }
	  // - -
	  for (x = x0 - diaMax/2 - fabs(offset) - offset; x - x1 > EPS; x -= diaMax) {
	    for (y = y0 - diaMax/2 - fabs(offset) - offset; y - y1 > EPS; y -= diaMax) {
	      newptcl = new Particle(particleNum+1, 0, Vec(x,y,z), gradation, young, poisson);
	      allParticleVec.push_back(newptcl);
	      ++particleNum;
	    }	
	  }
	  offset *= -1;
	}
      } // end of genMode == 1
      else if (genMode == 2) { // large variety of particle sizes, such as 10 or 100 times

	// Choices:
	// (a) Move particles at y tail: it cuts particles even in y+ direction but results in more smaller particles at y+. Better for a small number of particles.
	// (b) Do not move particles at y tail: it cannot cut particle even in y+ direction, and requires larger side gap. Better for a large number of particles.
	bool moveYTail = false;

	// define the grids based on the minimum particle
	int  gridNx = floor(allContainer.getDimx()/diaMin);
	REAL gridDim= allContainer.getDimx()/gridNx; // gridDim == diaMin if divisible
	int  gridNy = floor(allContainer.getDimy()/gridDim);
	int  gridNz = floor((dem::Parameter::getSingleton().parameter["floatMaxZ"] - allContainer.getMinCorner().getZ())/gridDim);

	//debugInf << "bottomGap, sideGap, nx, ny, nz dim=" <<bottomGap << " " << sideGap << " " << gridNx << " " << gridNy << " " << gridNz << " " << gridDim << std::endl;
	// mask the grids as not occupied
	std::valarray< std::valarray< std::valarray <int> > > gridMask;
	gridMask.resize(gridNx);
	for (int i = 0; i < gridMask.size(); ++i) {
	  gridMask[i].resize(gridNy);
	  for (int j = 0; j < gridMask[i].size(); ++j)
	    gridMask[i][j].resize(gridNz);
	}
	for (int i = 0; i < gridNx; ++i)
	  for (int j = 0; j < gridNy; ++j)
	    for (int k = 0; k < gridNz; ++k)
	      gridMask[i][j][k] = 0;

	int iCount = sideGap;
	int jCount = sideGap;
	int kCount = bottomGap;
	REAL xCorner, yCorner, zCorner;
	bool yOutOfTail = false;
	int iRecord, jRecord, kRecord;
	while (iCount < gridNx - sideGap && jCount < gridNy - sideGap && kCount < gridNz) {
	  // generate a particle without x, y, z coordinates
	  newptcl = new Particle(particleNum+1, 0, Vec(0,0,0), gradation, young, poisson);
	  int nGrid = ceil(newptcl->getA()*2.0 / gridDim);

	  // locate the particle
	  bool maskOverlapped;
	label2: ;
	  int cueInc = 0;
	  do {
	    maskOverlapped = false;

	    // if the particle volume reaches outside the tail in x direction, move it to the x head.
	    if (iCount + nGrid - 1 + cueInc > gridNx - 1 - sideGap) {
	      //debugInf <<" ptcl="<<particleNum<< " nGrid="<<nGrid<< " i j kCount=" << iCount << " " << jCount << " " << kCount << " cueInc=" <<cueInc <<" xRange ("<< iCount + cueInc << ","<< gridNx -1 - sideGap << "), (" << iCount + nGrid - 1 + cueInc << " ," << gridNx -1 - sideGap << ")"<<std::endl;
	      iCount = sideGap;
	      ++jCount;
	      if (jCount >= gridNy - sideGap) {
		jCount = sideGap;
		kCount += (layerGap+1);
	      }
	      cueInc = 0;
	      goto label2;  // relocate
	    }

	    // if the particle volume reaches outside the tail in y direction, move it to an upper level.
	    if (moveYTail && jCount + nGrid - 1 > gridNy - 1 - sideGap) {
	      yOutOfTail = true;
	      iRecord = iCount;
	      jRecord = jCount;
	      kRecord = kCount;

	      jCount = sideGap;
	      kCount += (layerGap+1);
	      //debugInf<<" change ptcl="<<particleNum<< " nGrid="<<nGrid<< " before ("<<iRecord<<" "<<jRecord<<" "<<kRecord<<") after ("<<iCount<<" "<<jCount<<" "<<kCount<<")"<<std::endl;
	      goto label2;  // relocate
	    }

	    // if the particle volume reaches outside the tail in z direction, do not add this particle, because it leads to infinite loops.
	    if (kCount + nGrid - 1 > gridNz - 1) {
	      //debugInf<<" cancel ptcl="<<particleNum<< " nGrid="<<nGrid<< " before ("<<iRecord<<" "<<jRecord<<" "<<kRecord<<") after ("<<iCount<<" "<<jCount<<" "<<kCount<<")"<<std::endl;
	      goto label3;  // relocate
	    }

	    // if the particle volume overlaps with any mask
	    for (int ix = std::min(iCount + cueInc, gridNx - 1 - sideGap); ix <= std::min(iCount + nGrid - 1 + cueInc, gridNx - 1 - sideGap); ++ix) {
	      for (int iy = std::min(jCount, gridNy - 1 - sideGap); iy <= std::min(jCount + nGrid - 1, gridNy - 1 - sideGap); ++iy) {
		for (int iz = std::min(kCount, gridNz - 1); iz <= std::min(kCount + nGrid - 1, gridNz -1); ++iz) {
		  if (gridMask[ix][iy][iz] == 1) {
		    maskOverlapped = true;
		    ++cueInc;
		    //debugInf << " cueInc="<<cueInc;
		    goto label1;
		  }
		}
	      }
	    }
	  label1: ;	
	  } while (maskOverlapped);

	  xCorner = allContainer.getMinCorner().getX() + iCount*gridDim;
	  yCorner = allContainer.getMinCorner().getY() + jCount*gridDim;
	  zCorner = allContainer.getMinCorner().getZ() + kCount*gridDim;
	  newptcl->setCurrPos(Vec(xCorner + 0.5*nGrid*gridDim + cueInc*gridDim, yCorner + 0.5*nGrid*gridDim, zCorner + 0.5*nGrid*gridDim ));
	  //newptcl->getCurrPos().print(debugInf);debugInf << std::endl;
	  allParticleVec.push_back(newptcl);
	  ++particleNum;
	  //debugInf << " after overlap, particles= " << particleNum << " nGrid=" << nGrid << " i j k=" << iCount << " " << jCount << " " << kCount;

	label3: ;
	  // mask grids occupied by the particle
	  for (int ix = std::min(iCount + cueInc, gridNx -1 - sideGap); ix <= std::min(iCount + nGrid - 1 + cueInc, gridNx - 1 - sideGap); ++ix)
	    for (int iy = std::min(jCount, gridNy -1 - sideGap); iy <= std::min(jCount + nGrid - 1, gridNy - 1 - sideGap); ++iy)
	      for (int iz = std::min(kCount, gridNz -1); iz <= std::min(kCount + nGrid - 1, gridNz -1); ++iz)
		gridMask[ix][iy][iz] = 1;

	  // continue generating particles along and in the order of x, y, z direction.
	  if (cueInc == 0) // ? otherwise: the particle is moved away, and maintaining iCount increases the chances of accommodating more particles.
	    iCount += nGrid;

	  if (moveYTail && yOutOfTail) {
	    //debugInf<<" restor ptcl="<<particleNum-1<< " nGrid="<<nGrid<< " before ("<<iCount<<" "<<jCount<<" "<<kCount<<") after ("<<iRecord<<" "<<jRecord<<" "<<kRecord<<")"<<std::endl;
	    iCount = iRecord;
	    jCount = jRecord;
	    kCount = kRecord;
	    yOutOfTail = false;
	  }

	  if (iCount >= gridNx - sideGap) {
	    iCount = sideGap;
	    ++jCount;
	  }
	  if (jCount >= gridNy - sideGap) {
	    jCount = sideGap;
	    kCount += (layerGap+1);
	  }
	  
	} // end of while (iCount < gridNx && jCount < gridNy && kCount < gridNz)
      } // end of genMode == 2
    } // end of particleLayers == 2
    
    printParticle(genParticle); 
  }
  

  void Assembly::trimOnly() {
    if (mpiRank == 0) {
      trim(true,
	   dem::Parameter::getSingleton().datafile["particleFile"].c_str(),
	   "trim_particle_end");
      plotGrid("trim_gridplot_end");
      plotBoundary("trim_bdryplot_end");

      // print density, void ratio, etc 
      REAL distX, distY, distZ;
      getStartDimension(distX, distY, distZ);
      openCompressProg(progressInf, "trim_stats");
      gatherBdryContact();
      printCompressProg(progressInf, distX, distY, distZ);
    }
  }


  void Assembly::trim(bool toRebuildBoundary,
		      const char *inputParticle,
		      const char *trmParticle)
  {
    readParticle(inputParticle);
    if (toRebuildBoundary)
      readBoundary(dem::Parameter::getSingleton().datafile["boundaryFile"].c_str());
    trimHistoryNum = allParticleVec.size();

    Vec  v1 = allContainer.getMinCorner();
    Vec  v2 = allContainer.getMaxCorner();
    REAL x1 = v1.getX();
    REAL y1 = v1.getY();
    REAL z1 = v1.getZ();
    REAL x2 = v2.getX();
    REAL y2 = v2.getY();
    REAL z2 = v2.getZ();
    REAL maxR = gradation.getPtclMaxRadius();
 
    std::vector<Particle*>::iterator itr;
    Vec center;

    for (itr = allParticleVec.begin(); itr != allParticleVec.end(); ) {
      center=(*itr)->getCurrPos();
      if(center.getX() < x1 || center.getX() > x2 ||
	 center.getY() < y1 || center.getY() > y2 ||
	 center.getZ() < z1 || center.getZ() + maxR > z2)
	{
	  delete (*itr); // release memory
	  itr = allParticleVec.erase(itr); 
	}
      else
	++itr;
    }
  
    printParticle(trmParticle);
  }


  void Assembly::removeBySphere()
  {
   if (mpiRank == 0) {

    readParticle(dem::Parameter::getSingleton().datafile["particleFile"].c_str());
    readBoundary(dem::Parameter::getSingleton().datafile["boundaryFile"].c_str());
    REAL minR = gradation.getPtclMinRadius(0);
    
    REAL x0S = dem::Parameter::getSingleton().parameter["x0S"];
    REAL y0S = dem::Parameter::getSingleton().parameter["y0S"];
    REAL z0S = dem::Parameter::getSingleton().parameter["z0S"];
    REAL r0S = dem::Parameter::getSingleton().parameter["r0S"];
 
    std::vector<Particle*>::iterator itr;
    Vec center;
    REAL dist;

    for (itr = allParticleVec.begin(); itr != allParticleVec.end(); ) {
      center = (*itr)->getCurrPos();
      dist = sqrt(pow(center.getX()-x0S,2) + pow(center.getY()-y0S,2) + pow(center.getZ()-z0S,2));
      if(dist <= r0S + minR)
	{
	  delete (*itr); // release memory
	  itr = allParticleVec.erase(itr); 
	}
      else
	++itr;
    }
  
    printParticle("remove_particle_end");
   }
  }


  void Assembly::
  findParticleInRectangle(const Rectangle &container,
			  const std::vector<Particle*> &inputParticle,
			  std::vector<Particle*> &foundParticle) {
    Vec  v1 = container.getMinCorner();
    Vec  v2 = container.getMaxCorner();
    REAL x1 = v1.getX();
    REAL y1 = v1.getY();
    REAL z1 = v1.getZ();
    REAL x2 = v2.getX();
    REAL y2 = v2.getY();
    REAL z2 = v2.getZ();
    for (std::size_t pt = 0; pt < inputParticle.size(); ++pt) {
      Vec center = inputParticle[pt]->getCurrPos();
      // it is critical to use EPS
      if (center.getX() - x1 >= -EPS && center.getX() - x2 < -EPS &&
	  center.getY() - y1 >= -EPS && center.getY() - y2 < -EPS &&
	  center.getZ() - z1 >= -EPS && center.getZ() - z2 < -EPS)
	foundParticle.push_back(inputParticle[pt]);
    }
  }


  void Assembly::removeParticleOutRectangle() {
    // use updated container
    Vec  v1 = container.getMinCorner();
    Vec  v2 = container.getMaxCorner();
    REAL x1 = v1.getX();
    REAL y1 = v1.getY();
    REAL z1 = v1.getZ();
    REAL x2 = v2.getX();
    REAL y2 = v2.getY();
    REAL z2 = v2.getZ();

    std::vector<Particle*>::iterator itr;
    Vec center;
    //std::size_t flag = 0;

    for (itr = particleVec.begin(); itr != particleVec.end(); ) {
      center=(*itr)->getCurrPos();
      // it is critical to use EPS
      if ( !(center.getX() - x1 >= -EPS && center.getX() - x2 < -EPS &&
	     center.getY() - y1 >= -EPS && center.getY() - y2 < -EPS &&
	     center.getZ() - z1 >= -EPS && center.getZ() - z2 < -EPS) )
	{
	  /*
	    debugInf << "iter=" << std::setw(8) << iteration << " rank=" << std::setw(2) << mpiRank
	    << " removed=" << std::setw(3) << (*itr)->getId();	
	    flag = 1;
	  */
	  delete (*itr); // release memory
	  itr = particleVec.erase(itr); 
	}
      else
	++itr;
    }
    /*
      if (flag == 1) {
      debugInf << " now " << particleVec.size() << ": ";
      for (std::vector<Particle*>::const_iterator it = particleVec.begin(); it != particleVec.end(); ++it)
      debugInf << std::setw(3) << (*it)->getId();
      debugInf << std::endl;
      }
    */

  }


  REAL Assembly::getPtclMaxX(const std::vector<Particle*> &inputParticle) const {
    if (inputParticle.size() == 0)
      return -1/EPS;

    std::vector<Particle*>::const_iterator it = inputParticle.begin();
    REAL x0 = (*it)->getCurrPos().getX();
    for (; it != inputParticle.end(); ++it) {
      if ( (*it)->getCurrPos().getX() > x0 )
	x0 = (*it)->getCurrPos().getX();
    }
    return x0;
  }


  REAL Assembly::getPtclMinX(const std::vector<Particle*> &inputParticle) const {
    if (inputParticle.size() == 0)
      return 1/EPS;

    std::vector<Particle*>::const_iterator it = inputParticle.begin();
    REAL x0 = (*it)->getCurrPos().getX();
    for (; it != inputParticle.end(); ++it) {
      if ( (*it)->getCurrPos().getX() < x0 )
	x0 = (*it)->getCurrPos().getX();
    }
    return x0;
  }


  REAL Assembly::getPtclMaxY(const std::vector<Particle*> &inputParticle) const {
    if (inputParticle.size() == 0)
      return -1/EPS;

    std::vector<Particle*>::const_iterator it = inputParticle.begin();
    REAL y0 = (*it)->getCurrPos().getY();
    for (; it != inputParticle.end(); ++it) {
      if ( (*it)->getCurrPos().getY() > y0 )
	y0 = (*it)->getCurrPos().getY();
    }
    return y0;
  }


  REAL Assembly::getPtclMinY(const std::vector<Particle*> &inputParticle) const {
    if (inputParticle.size() == 0)
      return 1/EPS;

    std::vector<Particle*>::const_iterator it = inputParticle.begin();
    REAL y0 = (*it)->getCurrPos().getY();
    for (; it != inputParticle.end(); ++it) {
      if ( (*it)->getCurrPos().getY() < y0 )
	y0 = (*it)->getCurrPos().getY();
    }
    return y0;
  }


  REAL Assembly::getPtclMaxZ(const std::vector<Particle*> &inputParticle) const {
    if (inputParticle.size() == 0)
      return -1/EPS;

    std::vector<Particle*>::const_iterator it = inputParticle.begin();
    REAL z0 = (*it)->getCurrPos().getZ();
    for (; it != inputParticle.end(); ++it) {
      if ( (*it)->getCurrPos().getZ() > z0 )
	z0 = (*it)->getCurrPos().getZ();
    }
    return z0;
  }


  REAL Assembly::getPtclMinZ(const std::vector<Particle*> &inputParticle) const {
    if (inputParticle.size() == 0)
      return 1/EPS;

    std::vector<Particle*>::const_iterator it = inputParticle.begin();
    REAL z0 = (*it)->getCurrPos().getZ();
    for (; it != inputParticle.end(); ++it) {
      if ( (*it)->getCurrPos().getZ() < z0 )
	z0 = (*it)->getCurrPos().getZ();
    }
    return z0;
  }


  void Assembly::setCommunicator(boost::mpi::communicator &comm) {
    boostWorld = comm;
    mpiWorld = MPI_Comm(comm);
    mpiProcX = static_cast<int> (dem::Parameter::getSingleton().parameter["mpiProcX"]);
    mpiProcY = static_cast<int> (dem::Parameter::getSingleton().parameter["mpiProcY"]);
    mpiProcZ = static_cast<int> (dem::Parameter::getSingleton().parameter["mpiProcZ"]);
  
    // create Cartesian virtual topology (unavailable in boost.mpi) 
    int ndim = 3;
    int dims[3] = {mpiProcX, mpiProcY, mpiProcZ};
    int periods[3] = {0, 0, 0};
    int reorder = 0; // mpiRank not reordered
    MPI_Cart_create(mpiWorld, ndim, dims, periods, reorder, &cartComm);
    MPI_Comm_rank(cartComm, &mpiRank); 
    MPI_Comm_size(cartComm, &mpiSize);
    MPI_Cart_coords(cartComm, mpiRank, ndim, mpiCoords);
    mpiTag = 0;
    assert(mpiRank == boostWorld.rank());
    //debugInf << mpiRank << " " << mpiCoords[0] << " " << mpiCoords[1] << " " << mpiCoords[2] << std::endl;

    for (int iRank = 0; iRank < mpiSize; ++iRank) {
      int ndim = 3;
      int coords[3];
      MPI_Cart_coords(cartComm, iRank, ndim, coords);
      if (coords[0] == 0 || coords[0] == mpiProcX - 1 ||
	  coords[1] == 0 || coords[1] == mpiProcY - 1 ||
	  coords[2] == 0 || coords[2] == mpiProcZ - 1)
	bdryProcess.push_back(iRank);
    }
  }


  void Assembly::scatterParticle() {
    // partition particles and send to each process
    if (mpiRank == 0) { // process 0

      // grid initialized in readBoundary()
      Vec v1 = grid.getMinCorner();
      Vec v2 = grid.getMaxCorner();
      Vec vspan = v2 - v1;

      boost::mpi::request *reqs = new boost::mpi::request [mpiSize - 1];
      std::vector<Particle*> tmpParticleVec;
      for (int iRank = mpiSize - 1; iRank >= 0; --iRank) {
	tmpParticleVec.clear(); // do not release memory!
	int ndim = 3;
	int coords[3];
	MPI_Cart_coords(cartComm, iRank, ndim, coords);
	Rectangle container(v1.getX() + vspan.getX() / mpiProcX * coords[0],
			    v1.getY() + vspan.getY() / mpiProcY * coords[1],
			    v1.getZ() + vspan.getZ() / mpiProcZ * coords[2],
			    v1.getX() + vspan.getX() / mpiProcX * (coords[0] + 1),
			    v1.getY() + vspan.getY() / mpiProcY * (coords[1] + 1),
			    v1.getZ() + vspan.getZ() / mpiProcZ * (coords[2] + 1));
	findParticleInRectangle(container, allParticleVec, tmpParticleVec);
	if (iRank != 0)
	  reqs[iRank - 1] = boostWorld.isend(iRank, mpiTag, tmpParticleVec); // non-blocking send
	if (iRank == 0) {
	  particleVec.resize(tmpParticleVec.size());
	  for (int i = 0; i < particleVec.size(); ++i)
	    particleVec[i] = new Particle(*tmpParticleVec[i]); // default synthesized copy constructor
	} // now particleVec do not share memeory with allParticleVec
      }
      boost::mpi::wait_all(reqs, reqs + mpiSize - 1); // for non-blocking send
      delete [] reqs;

    } else { // other processes except 0
      boostWorld.recv(0, mpiTag, particleVec);
    }

    // content of allParticleVec may need to be printed, so do not clear it. 
    //if (mpiRank == 0) releaseGatheredParticle();

    // broadcast necessary info
    broadcast(boostWorld, gradation, 0);
    broadcast(boostWorld, boundaryVec, 0);
    broadcast(boostWorld, allContainer, 0);
    broadcast(boostWorld, grid, 0);
  }


  bool Assembly::isBdryProcess() {
    return (mpiCoords[0] == 0 || mpiCoords[0] == mpiProcX - 1 ||
	    mpiCoords[1] == 0 || mpiCoords[1] == mpiProcY - 1 ||
	    mpiCoords[2] == 0 || mpiCoords[2] == mpiProcZ - 1);
  }


  void Assembly::commuParticle() 
  {
    // determine container of each process
    Vec v1 = grid.getMinCorner();
    Vec v2 = grid.getMaxCorner();
    Vec vspan = v2 - v1;
    container = Rectangle(v1.getX() + vspan.getX() / mpiProcX * mpiCoords[0],
			  v1.getY() + vspan.getY() / mpiProcY * mpiCoords[1],
			  v1.getZ() + vspan.getZ() / mpiProcZ * mpiCoords[2],
			  v1.getX() + vspan.getX() / mpiProcX * (mpiCoords[0] + 1),
			  v1.getY() + vspan.getY() / mpiProcY * (mpiCoords[1] + 1),
			  v1.getZ() + vspan.getZ() / mpiProcZ * (mpiCoords[2] + 1));

    // find neighboring blocks
    rankX1 = -1; rankX2 = -1; rankY1 = -1; rankY2 = -1; rankZ1 = -1; rankZ2 = -1;
    rankX1Y1 = -1; rankX1Y2 = -1; rankX1Z1 = -1; rankX1Z2 = -1; 
    rankX2Y1 = -1; rankX2Y2 = -1; rankX2Z1 = -1; rankX2Z2 = -1; 
    rankY1Z1 = -1; rankY1Z2 = -1; rankY2Z1 = -1; rankY2Z2 = -1; 
    rankX1Y1Z1 = -1; rankX1Y1Z2 = -1; rankX1Y2Z1 = -1; rankX1Y2Z2 = -1; 
    rankX2Y1Z1 = -1; rankX2Y1Z2 = -1; rankX2Y2Z1 = -1; rankX2Y2Z2 = -1;
    // x1: -x direction
    int neighborCoords[3] = {mpiCoords[0], mpiCoords[1], mpiCoords[2]};
    --neighborCoords[0];
    MPI_Cart_rank(cartComm, neighborCoords, &rankX1);
    // x2: +x direction
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    ++neighborCoords[0];
    MPI_Cart_rank(cartComm, neighborCoords, &rankX2);
    // y1: -y direction
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    --neighborCoords[1];
    MPI_Cart_rank(cartComm, neighborCoords, &rankY1);
    // y2: +y direction
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    ++neighborCoords[1];
    MPI_Cart_rank(cartComm, neighborCoords, &rankY2);
    // z1: -z direction
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    --neighborCoords[2];
    MPI_Cart_rank(cartComm, neighborCoords, &rankZ1);
    // z2: +z direction
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    ++neighborCoords[2];
    MPI_Cart_rank(cartComm, neighborCoords, &rankZ2);
    // x1y1
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    --neighborCoords[0]; --neighborCoords[1];
    MPI_Cart_rank(cartComm, neighborCoords, &rankX1Y1);
    // x1y2
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    --neighborCoords[0]; ++neighborCoords[1];
    MPI_Cart_rank(cartComm, neighborCoords, &rankX1Y2);
    // x1z1
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    --neighborCoords[0]; --neighborCoords[2];
    MPI_Cart_rank(cartComm, neighborCoords, &rankX1Z1);
    // x1z2
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    --neighborCoords[0]; ++neighborCoords[2];
    MPI_Cart_rank(cartComm, neighborCoords, &rankX1Z2);
    // x2y1
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    ++neighborCoords[0]; --neighborCoords[1];
    MPI_Cart_rank(cartComm, neighborCoords, &rankX2Y1);
    // x2y2
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    ++neighborCoords[0]; ++neighborCoords[1];
    MPI_Cart_rank(cartComm, neighborCoords, &rankX2Y2);
    // x2z1
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    ++neighborCoords[0]; --neighborCoords[2];
    MPI_Cart_rank(cartComm, neighborCoords, &rankX2Z1);
    // x2z2
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    ++neighborCoords[0]; ++neighborCoords[2];
    MPI_Cart_rank(cartComm, neighborCoords, &rankX2Z2);
    // y1z1
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    --neighborCoords[1]; --neighborCoords[2];
    MPI_Cart_rank(cartComm, neighborCoords, &rankY1Z1);
    // y1z2
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    --neighborCoords[1]; ++neighborCoords[2];
    MPI_Cart_rank(cartComm, neighborCoords, &rankY1Z2);
    // y2z1
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    ++neighborCoords[1]; --neighborCoords[2];
    MPI_Cart_rank(cartComm, neighborCoords, &rankY2Z1);
    // y2z2
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    ++neighborCoords[1]; ++neighborCoords[2];
    MPI_Cart_rank(cartComm, neighborCoords, &rankY2Z2);
    // x1y1z1
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    --neighborCoords[0]; --neighborCoords[1]; --neighborCoords[2];
    MPI_Cart_rank(cartComm, neighborCoords, &rankX1Y1Z1);
    // x1y1z2
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    --neighborCoords[0]; --neighborCoords[1]; ++neighborCoords[2];
    MPI_Cart_rank(cartComm, neighborCoords, &rankX1Y1Z2);
    // x1y2z1
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    --neighborCoords[0]; ++neighborCoords[1]; --neighborCoords[2];
    MPI_Cart_rank(cartComm, neighborCoords, &rankX1Y2Z1);
    // x1y2z2
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    --neighborCoords[0]; ++neighborCoords[1]; ++neighborCoords[2];
    MPI_Cart_rank(cartComm, neighborCoords, &rankX1Y2Z2);
    // x2y1z1
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    ++neighborCoords[0]; --neighborCoords[1]; --neighborCoords[2];
    MPI_Cart_rank(cartComm, neighborCoords, &rankX2Y1Z1);
    // x2y1z2
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    ++neighborCoords[0]; --neighborCoords[1]; ++neighborCoords[2];
    MPI_Cart_rank(cartComm, neighborCoords, &rankX2Y1Z2);
    // x2y2z1
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    ++neighborCoords[0]; ++neighborCoords[1]; --neighborCoords[2];
    MPI_Cart_rank(cartComm, neighborCoords, &rankX2Y2Z1);
    // x2y2z2
    neighborCoords[0] = mpiCoords[0];
    neighborCoords[1] = mpiCoords[1];
    neighborCoords[2] = mpiCoords[2];
    ++neighborCoords[0]; ++neighborCoords[1]; ++neighborCoords[2];
    MPI_Cart_rank(cartComm, neighborCoords, &rankX2Y2Z2);

    // if found, communicate with neighboring blocks
    std::vector<Particle*> particleX1, particleX2;
    std::vector<Particle*> particleY1, particleY2;
    std::vector<Particle*> particleZ1, particleZ2;
    std::vector<Particle*> particleX1Y1, particleX1Y2, particleX1Z1, particleX1Z2; 
    std::vector<Particle*> particleX2Y1, particleX2Y2, particleX2Z1, particleX2Z2; 
    std::vector<Particle*> particleY1Z1, particleY1Z2, particleY2Z1, particleY2Z2; 
    std::vector<Particle*> particleX1Y1Z1, particleX1Y1Z2, particleX1Y2Z1, particleX1Y2Z2; 
    std::vector<Particle*> particleX2Y1Z1, particleX2Y1Z2, particleX2Y2Z1, particleX2Y2Z2; 
    boost::mpi::request reqX1[2], reqX2[2];
    boost::mpi::request reqY1[2], reqY2[2];
    boost::mpi::request reqZ1[2], reqZ2[2];
    boost::mpi::request reqX1Y1[2], reqX1Y2[2], reqX1Z1[2], reqX1Z2[2];
    boost::mpi::request reqX2Y1[2], reqX2Y2[2], reqX2Z1[2], reqX2Z2[2];
    boost::mpi::request reqY1Z1[2], reqY1Z2[2], reqY2Z1[2], reqY2Z2[2];
    boost::mpi::request reqX1Y1Z1[2], reqX1Y1Z2[2], reqX1Y2Z1[2], reqX1Y2Z2[2];
    boost::mpi::request reqX2Y1Z1[2], reqX2Y1Z2[2], reqX2Y2Z1[2], reqX2Y2Z2[2];
    v1 = container.getMinCorner(); // redefine v1, v2 in terms of process
    v2 = container.getMaxCorner();   
    //debugInf << "rank=" << mpiRank << ' ' << v1.getX() << ' ' << v1.getY() << ' ' << v1.getZ() << ' '  << v2.getX() << ' ' << v2.getY() << ' ' << v2.getZ() << std::endl;
    //REAL borderWidth = gradation.getPtclMaxRadius() * 2;
    REAL borderWidth = gradation.getPtclMeanRadius() * 2;

    // 6 surfaces
    if (rankX1 >= 0) { // surface x1
      Rectangle containerX1(v1.getX(), v1.getY(), v1.getZ(), 
			    v1.getX() + borderWidth, v2.getY(), v2.getZ());
      findParticleInRectangle(containerX1, particleVec, particleX1);
      reqX1[0] = boostWorld.isend(rankX1, mpiTag,  particleX1);
      reqX1[1] = boostWorld.irecv(rankX1, mpiTag, rParticleX1);
    }
    if (rankX2 >= 0) { // surface x2
      Rectangle containerX2(v2.getX() - borderWidth, v1.getY(), v1.getZ(),
			    v2.getX(), v2.getY(), v2.getZ());
      findParticleInRectangle(containerX2, particleVec, particleX2);
      reqX2[0] = boostWorld.isend(rankX2, mpiTag,  particleX2);
      reqX2[1] = boostWorld.irecv(rankX2, mpiTag, rParticleX2);
    }
    if (rankY1 >= 0) {  // surface y1
      Rectangle containerY1(v1.getX(), v1.getY(), v1.getZ(), 
			    v2.getX(), v1.getY() + borderWidth, v2.getZ());
      findParticleInRectangle(containerY1, particleVec, particleY1);
      reqY1[0] = boostWorld.isend(rankY1, mpiTag,  particleY1);
      reqY1[1] = boostWorld.irecv(rankY1, mpiTag, rParticleY1);
    }
    if (rankY2 >= 0) {  // surface y2
      Rectangle containerY2(v1.getX(), v2.getY() - borderWidth, v1.getZ(),
			    v2.getX(), v2.getY(), v2.getZ());
      findParticleInRectangle(containerY2, particleVec, particleY2);
      reqY2[0] = boostWorld.isend(rankY2, mpiTag,  particleY2);
      reqY2[1] = boostWorld.irecv(rankY2, mpiTag, rParticleY2);
    }
    if (rankZ1 >= 0) {  // surface z1
      Rectangle containerZ1(v1.getX(), v1.getY(), v1.getZ(),
			    v2.getX(), v2.getY(), v1.getZ() + borderWidth);
      findParticleInRectangle(containerZ1, particleVec, particleZ1);
      reqZ1[0] = boostWorld.isend(rankZ1, mpiTag,  particleZ1);
      reqZ1[1] = boostWorld.irecv(rankZ1, mpiTag, rParticleZ1);
    }
    if (rankZ2 >= 0) {  // surface z2
      Rectangle containerZ2(v1.getX(), v1.getY(), v2.getZ() - borderWidth,
			    v2.getX(), v2.getY(), v2.getZ());
      findParticleInRectangle(containerZ2, particleVec, particleZ2);
      reqZ2[0] = boostWorld.isend(rankZ2, mpiTag,  particleZ2);
      reqZ2[1] = boostWorld.irecv(rankZ2, mpiTag, rParticleZ2);
    }
    // 12 edges
    if (rankX1Y1 >= 0) { // edge x1y1
      Rectangle containerX1Y1(v1.getX(), v1.getY(), v1.getZ(),
			      v1.getX() + borderWidth, v1.getY() + borderWidth, v2.getZ());
      findParticleInRectangle(containerX1Y1, particleVec, particleX1Y1);
      reqX1Y1[0] = boostWorld.isend(rankX1Y1, mpiTag,  particleX1Y1);
      reqX1Y1[1] = boostWorld.irecv(rankX1Y1, mpiTag, rParticleX1Y1);
    }
    if (rankX1Y2 >= 0) { // edge x1y2
      Rectangle containerX1Y2(v1.getX(), v2.getY() - borderWidth, v1.getZ(),
			      v1.getX() + borderWidth, v2.getY(), v2.getZ());
      findParticleInRectangle(containerX1Y2, particleVec, particleX1Y2);
      reqX1Y2[0] = boostWorld.isend(rankX1Y2, mpiTag,  particleX1Y2);
      reqX1Y2[1] = boostWorld.irecv(rankX1Y2, mpiTag, rParticleX1Y2);
    }
    if (rankX1Z1 >= 0) { // edge x1z1
      Rectangle containerX1Z1(v1.getX(), v1.getY(), v1.getZ(),
			      v1.getX() + borderWidth, v2.getY(), v1.getZ() + borderWidth);
      findParticleInRectangle(containerX1Z1, particleVec, particleX1Z1);
      reqX1Z1[0] = boostWorld.isend(rankX1Z1, mpiTag,  particleX1Z1);
      reqX1Z1[1] = boostWorld.irecv(rankX1Z1, mpiTag, rParticleX1Z1);
    }
    if (rankX1Z2 >= 0) { // edge x1z2
      Rectangle containerX1Z2(v1.getX(), v1.getY(), v2.getZ() - borderWidth,
			      v1.getX() + borderWidth, v2.getY(), v2.getZ());
      findParticleInRectangle(containerX1Z2, particleVec, particleX1Z2);
      reqX1Z2[0] = boostWorld.isend(rankX1Z2, mpiTag,  particleX1Z2);
      reqX1Z2[1] = boostWorld.irecv(rankX1Z2, mpiTag, rParticleX1Z2);
    }
    if (rankX2Y1 >= 0) { // edge x2y1
      Rectangle containerX2Y1(v2.getX() - borderWidth, v1.getY(), v1.getZ(),
			      v2.getX(), v1.getY() + borderWidth, v2.getZ());
      findParticleInRectangle(containerX2Y1, particleVec, particleX2Y1);
      reqX2Y1[0] = boostWorld.isend(rankX2Y1, mpiTag,  particleX2Y1);
      reqX2Y1[1] = boostWorld.irecv(rankX2Y1, mpiTag, rParticleX2Y1);
    }
    if (rankX2Y2 >= 0) { // edge x2y2
      Rectangle containerX2Y2(v2.getX() - borderWidth, v2.getY() - borderWidth, v1.getZ(),
			      v2.getX(), v2.getY(), v2.getZ());
      findParticleInRectangle(containerX2Y2, particleVec, particleX2Y2);
      reqX2Y2[0] = boostWorld.isend(rankX2Y2, mpiTag,  particleX2Y2);
      reqX2Y2[1] = boostWorld.irecv(rankX2Y2, mpiTag, rParticleX2Y2);
    }
    if (rankX2Z1 >= 0) { // edge x2z1
      Rectangle containerX2Z1(v2.getX() - borderWidth, v1.getY(), v1.getZ(),
			      v2.getX(), v2.getY(), v1.getZ() + borderWidth);
      findParticleInRectangle(containerX2Z1, particleVec, particleX2Z1);
      reqX2Z1[0] = boostWorld.isend(rankX2Z1, mpiTag,  particleX2Z1);
      reqX2Z1[1] = boostWorld.irecv(rankX2Z1, mpiTag, rParticleX2Z1);
    }
    if (rankX2Z2 >= 0) { // edge x2z2
      Rectangle containerX2Z2(v2.getX() - borderWidth, v1.getY(), v2.getZ() - borderWidth,
			      v2.getX(), v2.getY(), v2.getZ());
      findParticleInRectangle(containerX2Z2, particleVec, particleX2Z2);
      reqX2Z2[0] = boostWorld.isend(rankX2Z2, mpiTag,  particleX2Z2);
      reqX2Z2[1] = boostWorld.irecv(rankX2Z2, mpiTag, rParticleX2Z2);
    }
    if (rankY1Z1 >= 0) { // edge y1z1
      Rectangle containerY1Z1(v1.getX(), v1.getY(), v1.getZ(),
			      v2.getX(), v1.getY() + borderWidth, v1.getZ() + borderWidth);
      findParticleInRectangle(containerY1Z1, particleVec, particleY1Z1);
      reqY1Z1[0] = boostWorld.isend(rankY1Z1, mpiTag,  particleY1Z1);
      reqY1Z1[1] = boostWorld.irecv(rankY1Z1, mpiTag, rParticleY1Z1);
    }
    if (rankY1Z2 >= 0) { // edge y1z2
      Rectangle containerY1Z2(v1.getX(), v1.getY(), v2.getZ() - borderWidth,
			      v2.getX(), v1.getY() + borderWidth, v2.getZ());
      findParticleInRectangle(containerY1Z2, particleVec, particleY1Z2);
      reqY1Z2[0] = boostWorld.isend(rankY1Z2, mpiTag,  particleY1Z2);
      reqY1Z2[1] = boostWorld.irecv(rankY1Z2, mpiTag, rParticleY1Z2);
    }
    if (rankY2Z1 >= 0) { // edge y2z1
      Rectangle containerY2Z1(v1.getX(), v2.getY() - borderWidth, v1.getZ(),
			      v2.getX(), v2.getY(), v1.getZ() + borderWidth);
      findParticleInRectangle(containerY2Z1, particleVec, particleY2Z1);
      reqY2Z1[0] = boostWorld.isend(rankY2Z1, mpiTag,  particleY2Z1);
      reqY2Z1[1] = boostWorld.irecv(rankY2Z1, mpiTag, rParticleY2Z1);
    }
    if (rankY2Z2 >= 0) { // edge y2z2
      Rectangle containerY2Z2(v1.getX(), v2.getY() - borderWidth, v2.getZ() - borderWidth,
			      v2.getX(), v2.getY(), v2.getZ());
      findParticleInRectangle(containerY2Z2, particleVec, particleY2Z2);
      reqY2Z2[0] = boostWorld.isend(rankY2Z2, mpiTag,  particleY2Z2);
      reqY2Z2[1] = boostWorld.irecv(rankY2Z2, mpiTag, rParticleY2Z2);
    }
    // 8 vertices
    if (rankX1Y1Z1 >= 0) { // edge x1y1z1
      Rectangle containerX1Y1Z1(v1.getX(), v1.getY(), v1.getZ(),
				v1.getX() + borderWidth, v1.getY() + borderWidth, v1.getZ() + borderWidth);
      findParticleInRectangle(containerX1Y1Z1, particleVec, particleX1Y1Z1);
      reqX1Y1Z1[0] = boostWorld.isend(rankX1Y1Z1, mpiTag,  particleX1Y1Z1);
      reqX1Y1Z1[1] = boostWorld.irecv(rankX1Y1Z1, mpiTag, rParticleX1Y1Z1);
    }
    if (rankX1Y1Z2 >= 0) { // edge x1y1z2
      Rectangle containerX1Y1Z2(v1.getX(), v1.getY(), v2.getZ() - borderWidth,
				v1.getX() + borderWidth, v1.getY() + borderWidth, v2.getZ());
      findParticleInRectangle(containerX1Y1Z2, particleVec, particleX1Y1Z2);
      reqX1Y1Z2[0] = boostWorld.isend(rankX1Y1Z2, mpiTag,  particleX1Y1Z2);
      reqX1Y1Z2[1] = boostWorld.irecv(rankX1Y1Z2, mpiTag, rParticleX1Y1Z2);
    }
    if (rankX1Y2Z1 >= 0) { // edge x1y2z1
      Rectangle containerX1Y2Z1(v1.getX(), v2.getY() - borderWidth, v1.getZ(),
				v1.getX() + borderWidth, v2.getY(), v1.getZ() + borderWidth);
      findParticleInRectangle(containerX1Y2Z1, particleVec, particleX1Y2Z1);
      reqX1Y2Z1[0] = boostWorld.isend(rankX1Y2Z1, mpiTag,  particleX1Y2Z1);
      reqX1Y2Z1[1] = boostWorld.irecv(rankX1Y2Z1, mpiTag, rParticleX1Y2Z1);
    }
    if (rankX1Y2Z2 >= 0) { // edge x1y2z2
      Rectangle containerX1Y2Z2(v1.getX(), v2.getY() - borderWidth, v2.getZ() - borderWidth,
				v1.getX() + borderWidth, v2.getY() + borderWidth, v2.getZ());
      findParticleInRectangle(containerX1Y2Z2, particleVec, particleX1Y2Z2);
      reqX1Y2Z2[0] = boostWorld.isend(rankX1Y2Z2, mpiTag,  particleX1Y2Z2);
      reqX1Y2Z2[1] = boostWorld.irecv(rankX1Y2Z2, mpiTag, rParticleX1Y2Z2);
    }
    if (rankX2Y1Z1 >= 0) { // edge x2y1z1
      Rectangle containerX2Y1Z1(v2.getX() - borderWidth, v1.getY(), v1.getZ(),
				v2.getX(), v1.getY() + borderWidth, v1.getZ() + borderWidth);
      findParticleInRectangle(containerX2Y1Z1, particleVec, particleX2Y1Z1);
      reqX2Y1Z1[0] = boostWorld.isend(rankX2Y1Z1, mpiTag,  particleX2Y1Z1);
      reqX2Y1Z1[1] = boostWorld.irecv(rankX2Y1Z1, mpiTag, rParticleX2Y1Z1);
    }
    if (rankX2Y1Z2 >= 0) { // edge x2y1z2
      Rectangle containerX2Y1Z2(v2.getX() - borderWidth, v1.getY(), v2.getZ() - borderWidth,
				v2.getX(), v1.getY() + borderWidth, v2.getZ());
      findParticleInRectangle(containerX2Y1Z2, particleVec, particleX2Y1Z2);
      reqX2Y1Z2[0] = boostWorld.isend(rankX2Y1Z2, mpiTag,  particleX2Y1Z2);
      reqX2Y1Z2[1] = boostWorld.irecv(rankX2Y1Z2, mpiTag, rParticleX2Y1Z2);
    }
    if (rankX2Y2Z1 >= 0) { // edge x2y2z1
      Rectangle containerX2Y2Z1(v2.getX() - borderWidth, v2.getY() - borderWidth, v1.getZ(),
				v2.getX(), v2.getY(), v1.getZ() + borderWidth);
      findParticleInRectangle(containerX2Y2Z1, particleVec, particleX2Y2Z1);
      reqX2Y2Z1[0] = boostWorld.isend(rankX2Y2Z1, mpiTag,  particleX2Y2Z1);
      reqX2Y2Z1[1] = boostWorld.irecv(rankX2Y2Z1, mpiTag, rParticleX2Y2Z1);
    }
    if (rankX2Y2Z2 >= 0) { // edge x2y2z2
      Rectangle containerX2Y2Z2(v2.getX() - borderWidth, v2.getY() - borderWidth, v2.getZ() - borderWidth,
				v2.getX(), v2.getY(), v2.getZ());
      findParticleInRectangle(containerX2Y2Z2, particleVec, particleX2Y2Z2);
      reqX2Y2Z2[0] = boostWorld.isend(rankX2Y2Z2, mpiTag,  particleX2Y2Z2);
      reqX2Y2Z2[1] = boostWorld.irecv(rankX2Y2Z2, mpiTag, rParticleX2Y2Z2);
    }

    // 6 surfaces
    if (rankX1 >= 0) boost::mpi::wait_all(reqX1, reqX1 + 2);
    if (rankX2 >= 0) boost::mpi::wait_all(reqX2, reqX2 + 2);
    if (rankY1 >= 0) boost::mpi::wait_all(reqY1, reqY1 + 2);
    if (rankY2 >= 0) boost::mpi::wait_all(reqY2, reqY2 + 2);
    if (rankZ1 >= 0) boost::mpi::wait_all(reqZ1, reqZ1 + 2);
    if (rankZ2 >= 0) boost::mpi::wait_all(reqZ2, reqZ2 + 2);
    // 12 edges
    if (rankX1Y1 >= 0) boost::mpi::wait_all(reqX1Y1, reqX1Y1 + 2);
    if (rankX1Y2 >= 0) boost::mpi::wait_all(reqX1Y2, reqX1Y2 + 2);  
    if (rankX1Z1 >= 0) boost::mpi::wait_all(reqX1Z1, reqX1Z1 + 2);
    if (rankX1Z2 >= 0) boost::mpi::wait_all(reqX1Z2, reqX1Z2 + 2);
    if (rankX2Y1 >= 0) boost::mpi::wait_all(reqX2Y1, reqX2Y1 + 2);
    if (rankX2Y2 >= 0) boost::mpi::wait_all(reqX2Y2, reqX2Y2 + 2);  
    if (rankX2Z1 >= 0) boost::mpi::wait_all(reqX2Z1, reqX2Z1 + 2);
    if (rankX2Z2 >= 0) boost::mpi::wait_all(reqX2Z2, reqX2Z2 + 2); 
    if (rankY1Z1 >= 0) boost::mpi::wait_all(reqY1Z1, reqY1Z1 + 2);
    if (rankY1Z2 >= 0) boost::mpi::wait_all(reqY1Z2, reqY1Z2 + 2);
    if (rankY2Z1 >= 0) boost::mpi::wait_all(reqY2Z1, reqY2Z1 + 2);
    if (rankY2Z2 >= 0) boost::mpi::wait_all(reqY2Z2, reqY2Z2 + 2); 
    // 8 vertices
    if (rankX1Y1Z1 >= 0) boost::mpi::wait_all(reqX1Y1Z1, reqX1Y1Z1 + 2);
    if (rankX1Y1Z2 >= 0) boost::mpi::wait_all(reqX1Y1Z2, reqX1Y1Z2 + 2);
    if (rankX1Y2Z1 >= 0) boost::mpi::wait_all(reqX1Y2Z1, reqX1Y2Z1 + 2);
    if (rankX1Y2Z2 >= 0) boost::mpi::wait_all(reqX1Y2Z2, reqX1Y2Z2 + 2);
    if (rankX2Y1Z1 >= 0) boost::mpi::wait_all(reqX2Y1Z1, reqX2Y1Z1 + 2);
    if (rankX2Y1Z2 >= 0) boost::mpi::wait_all(reqX2Y1Z2, reqX2Y1Z2 + 2);
    if (rankX2Y2Z1 >= 0) boost::mpi::wait_all(reqX2Y2Z1, reqX2Y2Z1 + 2);
    if (rankX2Y2Z2 >= 0) boost::mpi::wait_all(reqX2Y2Z2, reqX2Y2Z2 + 2);  

    // merge: particles inside container (at front) + particles from neighoring blocks (at end)
    recvParticleVec.clear();
    // 6 surfaces
    if (rankX1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX1.begin(), rParticleX1.end());
    if (rankX2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX2.begin(), rParticleX2.end());
    if (rankY1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleY1.begin(), rParticleY1.end());
    if (rankY2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleY2.begin(), rParticleY2.end());
    if (rankZ1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleZ1.begin(), rParticleZ1.end());
    if (rankZ2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleZ2.begin(), rParticleZ2.end());
    // 12 edges
    if (rankX1Y1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX1Y1.begin(), rParticleX1Y1.end());
    if (rankX1Y2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX1Y2.begin(), rParticleX1Y2.end());
    if (rankX1Z1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX1Z1.begin(), rParticleX1Z1.end());
    if (rankX1Z2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX1Z2.begin(), rParticleX1Z2.end());
    if (rankX2Y1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX2Y1.begin(), rParticleX2Y1.end());
    if (rankX2Y2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX2Y2.begin(), rParticleX2Y2.end());
    if (rankX2Z1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX2Z1.begin(), rParticleX2Z1.end());
    if (rankX2Z2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX2Z2.begin(), rParticleX2Z2.end());
    if (rankY1Z1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleY1Z1.begin(), rParticleY1Z1.end());
    if (rankY1Z2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleY1Z2.begin(), rParticleY1Z2.end());
    if (rankY2Z1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleY2Z1.begin(), rParticleY2Z1.end());
    if (rankY2Z2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleY2Z2.begin(), rParticleY2Z2.end());
    // 8 vertices
    if (rankX1Y1Z1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX1Y1Z1.begin(), rParticleX1Y1Z1.end());
    if (rankX1Y1Z2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX1Y1Z2.begin(), rParticleX1Y1Z2.end());
    if (rankX1Y2Z1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX1Y2Z1.begin(), rParticleX1Y2Z1.end());
    if (rankX1Y2Z2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX1Y2Z2.begin(), rParticleX1Y2Z2.end());
    if (rankX2Y1Z1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX2Y1Z1.begin(), rParticleX2Y1Z1.end());
    if (rankX2Y1Z2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX2Y1Z2.begin(), rParticleX2Y1Z2.end());
    if (rankX2Y2Z1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX2Y2Z1.begin(), rParticleX2Y2Z1.end());
    if (rankX2Y2Z2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX2Y2Z2.begin(), rParticleX2Y2Z2.end());

    mergeParticleVec.clear();
    mergeParticleVec = particleVec; // duplicate pointers, pointing to the same memory
    mergeParticleVec.insert(mergeParticleVec.end(), recvParticleVec.begin(), recvParticleVec.end());

    /*
      std::vector<Particle*> testParticleVec;
      testParticleVec.insert(testParticleVec.end(), rParticleX1.begin(), rParticleX1.end());
      testParticleVec.insert(testParticleVec.end(), rParticleX2.begin(), rParticleX2.end());
      testParticleVec.insert(testParticleVec.end(), rParticleY1.begin(), rParticleY1.end());
      testParticleVec.insert(testParticleVec.end(), rParticleY2.begin(), rParticleY2.end());
      testParticleVec.insert(testParticleVec.end(), rParticleZ1.begin(), rParticleZ1.end());
      testParticleVec.insert(testParticleVec.end(), rParticleZ2.begin(), rParticleZ2.end());
      debugInf << "iter=" << std::setw(4) << iteration << " rank=" << std::setw(4) << mpiRank 
      << " ptclNum=" << std::setw(4) << particleVec.size() 
      << " surface="
      << std::setw(4) << particleX1.size()  << std::setw(4) << particleX2.size()
      << std::setw(4) << particleY1.size()  << std::setw(4) << particleY2.size()
      << std::setw(4) << particleZ1.size()  << std::setw(4) << particleZ2.size()  
      << " recv="
      << std::setw(4) << rParticleX1.size() << std::setw(4) << rParticleX2.size()
      << std::setw(4) << rParticleY1.size() << std::setw(4) << rParticleY2.size()
      << std::setw(4) << rParticleZ1.size() << std::setw(4) << rParticleZ2.size() 
      << " rNum="    
      << std::setw(4) << recvParticleVec.size() << ": ";   

      for (std::vector<Particle*>::const_iterator it = testParticleVec.begin(); it != testParticleVec.end();++it)
      debugInf << (*it)->getId() << ' ';
      debugInf << std::endl;
      testParticleVec.clear();
    */
  }


  void Assembly::releaseRecvParticle() {
    // release memory of received particles
    for (std::vector<Particle*>::iterator it = recvParticleVec.begin(); it != recvParticleVec.end(); ++it)
      delete (*it);
    recvParticleVec.clear();
    // 6 surfaces
    rParticleX1.clear();
    rParticleX2.clear();
    rParticleY1.clear();
    rParticleY2.clear();
    rParticleZ1.clear();
    rParticleZ2.clear();
    // 12 edges
    rParticleX1Y1.clear();
    rParticleX1Y2.clear();
    rParticleX1Z1.clear();
    rParticleX1Z2.clear();
    rParticleX2Y1.clear();
    rParticleX2Y2.clear();
    rParticleX2Z1.clear();
    rParticleX2Z2.clear();
    rParticleY1Z1.clear();
    rParticleY1Z2.clear();
    rParticleY2Z1.clear();
    rParticleY2Z2.clear();
    // 8 vertices
    rParticleX1Y1Z1.clear();
    rParticleX1Y1Z2.clear();
    rParticleX1Y2Z1.clear();
    rParticleX1Y2Z2.clear();
    rParticleX2Y1Z1.clear();
    rParticleX2Y1Z2.clear();
    rParticleX2Y2Z1.clear();
    rParticleX2Y2Z2.clear();
  }


  void Assembly::updateGrid() {
    updateGridMinX();
    updateGridMaxX();
    updateGridMinY();
    updateGridMaxY();
    updateGridMinZ();
    updateGridMaxZ();
  }

  void Assembly::updateGrid5() {
    updateGridMinX();
    updateGridMaxX();
    updateGridMinY();
    updateGridMaxY();
    updateGridMaxZ();
  }

  void Assembly::updateGridExplosion() {
    updateGridMinXExplosion();
    updateGridMaxXExplosion();
    updateGridMinYExplosion();
    updateGridMaxYExplosion();
    updateGridMaxZ();
  }


  void Assembly::updateGridMinXExplosion() {
    Vec v1 = allContainer.getMinCorner();
    Vec v2 = allContainer.getMaxCorner();
    Vec vspan = v2 - v1;
    REAL minX = v1.getX() - 0.5*vspan.getX();

    setGrid(Rectangle(minX - gradation.getPtclMaxRadius(),
		      grid.getMinCorner().getY(),
		      grid.getMinCorner().getZ(),
		      grid.getMaxCorner().getX(),
		      grid.getMaxCorner().getY(),
		      grid.getMaxCorner().getZ() ));
  }


  void Assembly::updateGridMaxXExplosion() {
    Vec v1 = allContainer.getMinCorner();
    Vec v2 = allContainer.getMaxCorner();
    Vec vspan = v2 - v1;
    REAL maxX = v2.getX() + 0.5*vspan.getX();

    setGrid(Rectangle(grid.getMinCorner().getX(),
		      grid.getMinCorner().getY(),
		      grid.getMinCorner().getZ(),
		      maxX + gradation.getPtclMaxRadius(),
		      grid.getMaxCorner().getY(),
		      grid.getMaxCorner().getZ() ));
  }


  void Assembly::updateGridMinYExplosion() {
    Vec v1 = allContainer.getMinCorner();
    Vec v2 = allContainer.getMaxCorner();
    Vec vspan = v2 - v1;
    REAL minY = v1.getY() - 0.5*vspan.getY();	

    setGrid(Rectangle(grid.getMinCorner().getX(),
		      minY - gradation.getPtclMaxRadius(),
		      grid.getMinCorner().getZ(),
		      grid.getMaxCorner().getX(),
		      grid.getMaxCorner().getY(),
		      grid.getMaxCorner().getZ() ));
  }


  void Assembly::updateGridMaxYExplosion() {
    Vec v1 = allContainer.getMinCorner();
    Vec v2 = allContainer.getMaxCorner();
    Vec vspan = v2 - v1;
    REAL maxY = v2.getY() + 0.5*vspan.getY();	

    setGrid(Rectangle(grid.getMinCorner().getX(),
		      grid.getMinCorner().getY(),
		      grid.getMinCorner().getZ(),
		      grid.getMaxCorner().getX(),
		      maxY + gradation.getPtclMaxRadius(),
		      grid.getMaxCorner().getZ() ));
  }


  void Assembly::updateGridMinX() {
    REAL pMinX = getPtclMinX(particleVec);
    REAL minX = 0;
    MPI_Allreduce(&pMinX, &minX, 1, MPI_DOUBLE, MPI_MIN, mpiWorld);

    setGrid(Rectangle(minX - gradation.getPtclMaxRadius(),
		      grid.getMinCorner().getY(),
		      grid.getMinCorner().getZ(),
		      grid.getMaxCorner().getX(),
		      grid.getMaxCorner().getY(),
		      grid.getMaxCorner().getZ() ));
  }


  void Assembly::updateGridMaxX() {
    REAL pMaxX = getPtclMaxX(particleVec);
    REAL maxX = 0;
    MPI_Allreduce(&pMaxX, &maxX, 1, MPI_DOUBLE, MPI_MAX, mpiWorld);

    setGrid(Rectangle(grid.getMinCorner().getX(),
		      grid.getMinCorner().getY(),
		      grid.getMinCorner().getZ(),
		      maxX + gradation.getPtclMaxRadius(),
		      grid.getMaxCorner().getY(),
		      grid.getMaxCorner().getZ() ));
  }


  void Assembly::updateGridMinY() {
    REAL pMinY = getPtclMinY(particleVec);
    REAL minY = 0;
    MPI_Allreduce(&pMinY, &minY, 1, MPI_DOUBLE, MPI_MIN, mpiWorld);

    setGrid(Rectangle(grid.getMinCorner().getX(),
		      minY - gradation.getPtclMaxRadius(),
		      grid.getMinCorner().getZ(),
		      grid.getMaxCorner().getX(),
		      grid.getMaxCorner().getY(),
		      grid.getMaxCorner().getZ() ));
  }


  void Assembly::updateGridMaxY() {
    REAL pMaxY = getPtclMaxY(particleVec);
    REAL maxY = 0;
    MPI_Allreduce(&pMaxY, &maxY, 1, MPI_DOUBLE, MPI_MAX, mpiWorld);

    setGrid(Rectangle(grid.getMinCorner().getX(),
		      grid.getMinCorner().getY(),
		      grid.getMinCorner().getZ(),
		      grid.getMaxCorner().getX(),
		      maxY + gradation.getPtclMaxRadius(),
		      grid.getMaxCorner().getZ() ));
  }


  void Assembly::updateGridMinZ() {
    REAL pMinZ = getPtclMinZ(particleVec);
    REAL minZ = 0;
    MPI_Allreduce(&pMinZ, &minZ, 1, MPI_DOUBLE, MPI_MIN, mpiWorld);

    setGrid(Rectangle(grid.getMinCorner().getX(),
		      grid.getMinCorner().getY(),
		      minZ - gradation.getPtclMaxRadius(),
		      grid.getMaxCorner().getX(),
		      grid.getMaxCorner().getY(),
		      grid.getMaxCorner().getZ() ));
  }


  void Assembly::updateGridMaxZ() {
    // update compute grids adaptively due to particle motion
    REAL pMaxZ = getPtclMaxZ(particleVec);
    REAL maxZ = 0;
    MPI_Allreduce(&pMaxZ, &maxZ, 1, MPI_DOUBLE, MPI_MAX, mpiWorld);

    // no need to broadcast grid as it is updated in each process
    setGrid(Rectangle(grid.getMinCorner().getX(),
		      grid.getMinCorner().getY(),
		      grid.getMinCorner().getZ(),
		      grid.getMaxCorner().getX(),
		      grid.getMaxCorner().getY(),
		      maxZ + gradation.getPtclMaxRadius() ));
  }


  void Assembly::migrateParticle() 
  {
    // now use updated grids to determine the new container of each process
    Vec    v1 = grid.getMinCorner();
    Vec    v2 = grid.getMaxCorner();
    Vec vspan = v2 - v1;
    REAL segX = vspan.getX() / mpiProcX;
    REAL segY = vspan.getY() / mpiProcY;
    REAL segZ = vspan.getZ() / mpiProcZ;

    // new container
    container = Rectangle(v1.getX() + vspan.getX() / mpiProcX * mpiCoords[0],
			  v1.getY() + vspan.getY() / mpiProcY * mpiCoords[1],
			  v1.getZ() + vspan.getZ() / mpiProcZ * mpiCoords[2],
			  v1.getX() + vspan.getX() / mpiProcX * (mpiCoords[0] + 1),
			  v1.getY() + vspan.getY() / mpiProcY * (mpiCoords[1] + 1),
			  v1.getZ() + vspan.getZ() / mpiProcZ * (mpiCoords[2] + 1));

    v1 = container.getMinCorner(); // redefine v1, v2 in terms of process
    v2 = container.getMaxCorner();  

    // if a neighbor exists, transfer particles crossing the boundary in between.
    std::vector<Particle*> particleX1, particleX2;
    std::vector<Particle*> particleY1, particleY2;
    std::vector<Particle*> particleZ1, particleZ2;
    std::vector<Particle*> particleX1Y1, particleX1Y2, particleX1Z1, particleX1Z2; 
    std::vector<Particle*> particleX2Y1, particleX2Y2, particleX2Z1, particleX2Z2; 
    std::vector<Particle*> particleY1Z1, particleY1Z2, particleY2Z1, particleY2Z2; 
    std::vector<Particle*> particleX1Y1Z1, particleX1Y1Z2, particleX1Y2Z1, particleX1Y2Z2; 
    std::vector<Particle*> particleX2Y1Z1, particleX2Y1Z2, particleX2Y2Z1, particleX2Y2Z2; 
    boost::mpi::request reqX1[2], reqX2[2];
    boost::mpi::request reqY1[2], reqY2[2];
    boost::mpi::request reqZ1[2], reqZ2[2];
    boost::mpi::request reqX1Y1[2], reqX1Y2[2], reqX1Z1[2], reqX1Z2[2];
    boost::mpi::request reqX2Y1[2], reqX2Y2[2], reqX2Z1[2], reqX2Z2[2];
    boost::mpi::request reqY1Z1[2], reqY1Z2[2], reqY2Z1[2], reqY2Z2[2];
    boost::mpi::request reqX1Y1Z1[2], reqX1Y1Z2[2], reqX1Y2Z1[2], reqX1Y2Z2[2];
    boost::mpi::request reqX2Y1Z1[2], reqX2Y1Z2[2], reqX2Y2Z1[2], reqX2Y2Z2[2];

    // 6 surfaces
    if (rankX1 >= 0) { // surface x1
      Rectangle containerX1(v1.getX() - segX, v1.getY(), v1.getZ(), 
			    v1.getX(), v2.getY(), v2.getZ());
      findParticleInRectangle(containerX1, particleVec, particleX1);
      reqX1[0] = boostWorld.isend(rankX1, mpiTag,  particleX1);
      reqX1[1] = boostWorld.irecv(rankX1, mpiTag, rParticleX1);
    }
    if (rankX2 >= 0) { // surface x2
      Rectangle containerX2(v2.getX(), v1.getY(), v1.getZ(),
			    v2.getX() + segX, v2.getY(), v2.getZ());
      findParticleInRectangle(containerX2, particleVec, particleX2);
      reqX2[0] = boostWorld.isend(rankX2, mpiTag,  particleX2);
      reqX2[1] = boostWorld.irecv(rankX2, mpiTag, rParticleX2);
    }
    if (rankY1 >= 0) {  // surface y1
      Rectangle containerY1(v1.getX(), v1.getY() - segY, v1.getZ(), 
			    v2.getX(), v1.getY(), v2.getZ());
      findParticleInRectangle(containerY1, particleVec, particleY1);
      reqY1[0] = boostWorld.isend(rankY1, mpiTag,  particleY1);
      reqY1[1] = boostWorld.irecv(rankY1, mpiTag, rParticleY1);
    }
    if (rankY2 >= 0) {  // surface y2
      Rectangle containerY2(v1.getX(), v2.getY(), v1.getZ(),
			    v2.getX(), v2.getY() + segY, v2.getZ());
      findParticleInRectangle(containerY2, particleVec, particleY2);
      reqY2[0] = boostWorld.isend(rankY2, mpiTag,  particleY2);
      reqY2[1] = boostWorld.irecv(rankY2, mpiTag, rParticleY2);
    }
    if (rankZ1 >= 0) {  // surface z1
      Rectangle containerZ1(v1.getX(), v1.getY(), v1.getZ() - segZ,
			    v2.getX(), v2.getY(), v1.getZ());
      findParticleInRectangle(containerZ1, particleVec, particleZ1);
      reqZ1[0] = boostWorld.isend(rankZ1, mpiTag,  particleZ1);
      reqZ1[1] = boostWorld.irecv(rankZ1, mpiTag, rParticleZ1);
    }
    if (rankZ2 >= 0) {  // surface z2
      Rectangle containerZ2(v1.getX(), v1.getY(), v2.getZ(),
			    v2.getX(), v2.getY(), v2.getZ() + segZ);
      findParticleInRectangle(containerZ2, particleVec, particleZ2);
      reqZ2[0] = boostWorld.isend(rankZ2, mpiTag,  particleZ2);
      reqZ2[1] = boostWorld.irecv(rankZ2, mpiTag, rParticleZ2);
    }
    // 12 edges
    if (rankX1Y1 >= 0) { // edge x1y1
      Rectangle containerX1Y1(v1.getX() - segX, v1.getY() - segY, v1.getZ(),
			      v1.getX(), v1.getY(), v2.getZ());
      findParticleInRectangle(containerX1Y1, particleVec, particleX1Y1);
      reqX1Y1[0] = boostWorld.isend(rankX1Y1, mpiTag,  particleX1Y1);
      reqX1Y1[1] = boostWorld.irecv(rankX1Y1, mpiTag, rParticleX1Y1);
    }
    if (rankX1Y2 >= 0) { // edge x1y2
      Rectangle containerX1Y2(v1.getX() - segX, v2.getY(), v1.getZ(),
			      v1.getX(), v2.getY() + segY, v2.getZ());
      findParticleInRectangle(containerX1Y2, particleVec, particleX1Y2);
      reqX1Y2[0] = boostWorld.isend(rankX1Y2, mpiTag,  particleX1Y2);
      reqX1Y2[1] = boostWorld.irecv(rankX1Y2, mpiTag, rParticleX1Y2);
    }
    if (rankX1Z1 >= 0) { // edge x1z1
      Rectangle containerX1Z1(v1.getX() - segX, v1.getY(), v1.getZ() -segZ,
			      v1.getX(), v2.getY(), v1.getZ());
      findParticleInRectangle(containerX1Z1, particleVec, particleX1Z1);
      reqX1Z1[0] = boostWorld.isend(rankX1Z1, mpiTag,  particleX1Z1);
      reqX1Z1[1] = boostWorld.irecv(rankX1Z1, mpiTag, rParticleX1Z1);
    }
    if (rankX1Z2 >= 0) { // edge x1z2
      Rectangle containerX1Z2(v1.getX() - segX, v1.getY(), v2.getZ(),
			      v1.getX(), v2.getY(), v2.getZ() + segZ);
      findParticleInRectangle(containerX1Z2, particleVec, particleX1Z2);
      reqX1Z2[0] = boostWorld.isend(rankX1Z2, mpiTag,  particleX1Z2);
      reqX1Z2[1] = boostWorld.irecv(rankX1Z2, mpiTag, rParticleX1Z2);
    }
    if (rankX2Y1 >= 0) { // edge x2y1
      Rectangle containerX2Y1(v2.getX(), v1.getY() - segY, v1.getZ(),
			      v2.getX() + segX, v1.getY(), v2.getZ());
      findParticleInRectangle(containerX2Y1, particleVec, particleX2Y1);
      reqX2Y1[0] = boostWorld.isend(rankX2Y1, mpiTag,  particleX2Y1);
      reqX2Y1[1] = boostWorld.irecv(rankX2Y1, mpiTag, rParticleX2Y1);
    }
    if (rankX2Y2 >= 0) { // edge x2y2
      Rectangle containerX2Y2(v2.getX(), v2.getY(), v1.getZ(),
			      v2.getX() + segX, v2.getY() + segY, v2.getZ());
      findParticleInRectangle(containerX2Y2, particleVec, particleX2Y2);
      reqX2Y2[0] = boostWorld.isend(rankX2Y2, mpiTag,  particleX2Y2);
      reqX2Y2[1] = boostWorld.irecv(rankX2Y2, mpiTag, rParticleX2Y2);
    }
    if (rankX2Z1 >= 0) { // edge x2z1
      Rectangle containerX2Z1(v2.getX(), v1.getY(), v1.getZ() - segZ,
			      v2.getX() + segX, v2.getY(), v1.getZ());
      findParticleInRectangle(containerX2Z1, particleVec, particleX2Z1);
      reqX2Z1[0] = boostWorld.isend(rankX2Z1, mpiTag,  particleX2Z1);
      reqX2Z1[1] = boostWorld.irecv(rankX2Z1, mpiTag, rParticleX2Z1);
    }
    if (rankX2Z2 >= 0) { // edge x2z2
      Rectangle containerX2Z2(v2.getX(), v1.getY(), v2.getZ(),
			      v2.getX() + segX, v2.getY(), v2.getZ() + segZ);
      findParticleInRectangle(containerX2Z2, particleVec, particleX2Z2);
      reqX2Z2[0] = boostWorld.isend(rankX2Z2, mpiTag,  particleX2Z2);
      reqX2Z2[1] = boostWorld.irecv(rankX2Z2, mpiTag, rParticleX2Z2);
    }
    if (rankY1Z1 >= 0) { // edge y1z1
      Rectangle containerY1Z1(v1.getX(), v1.getY() - segY, v1.getZ() - segZ,
			      v2.getX(), v1.getY(), v1.getZ());
      findParticleInRectangle(containerY1Z1, particleVec, particleY1Z1);
      reqY1Z1[0] = boostWorld.isend(rankY1Z1, mpiTag,  particleY1Z1);
      reqY1Z1[1] = boostWorld.irecv(rankY1Z1, mpiTag, rParticleY1Z1);
    }
    if (rankY1Z2 >= 0) { // edge y1z2
      Rectangle containerY1Z2(v1.getX(), v1.getY() - segY, v2.getZ(),
			      v2.getX(), v1.getY(), v2.getZ() + segZ);
      findParticleInRectangle(containerY1Z2, particleVec, particleY1Z2);
      reqY1Z2[0] = boostWorld.isend(rankY1Z2, mpiTag,  particleY1Z2);
      reqY1Z2[1] = boostWorld.irecv(rankY1Z2, mpiTag, rParticleY1Z2);
    }
    if (rankY2Z1 >= 0) { // edge y2z1
      Rectangle containerY2Z1(v1.getX(), v2.getY(), v1.getZ() - segZ,
			      v2.getX(), v2.getY() + segY, v1.getZ());
      findParticleInRectangle(containerY2Z1, particleVec, particleY2Z1);
      reqY2Z1[0] = boostWorld.isend(rankY2Z1, mpiTag,  particleY2Z1);
      reqY2Z1[1] = boostWorld.irecv(rankY2Z1, mpiTag, rParticleY2Z1);
    }
    if (rankY2Z2 >= 0) { // edge y2z2
      Rectangle containerY2Z2(v1.getX(), v2.getY(), v2.getZ(),
			      v2.getX(), v2.getY() + segY, v2.getZ() + segZ);
      findParticleInRectangle(containerY2Z2, particleVec, particleY2Z2);
      reqY2Z2[0] = boostWorld.isend(rankY2Z2, mpiTag,  particleY2Z2);
      reqY2Z2[1] = boostWorld.irecv(rankY2Z2, mpiTag, rParticleY2Z2);
    }
    // 8 vertices
    if (rankX1Y1Z1 >= 0) { // edge x1y1z1
      Rectangle containerX1Y1Z1(v1.getX() - segX, v1.getY() - segY, v1.getZ() - segZ,
				v1.getX(), v1.getY(), v1.getZ());
      findParticleInRectangle(containerX1Y1Z1, particleVec, particleX1Y1Z1);
      reqX1Y1Z1[0] = boostWorld.isend(rankX1Y1Z1, mpiTag,  particleX1Y1Z1);
      reqX1Y1Z1[1] = boostWorld.irecv(rankX1Y1Z1, mpiTag, rParticleX1Y1Z1);
    }
    if (rankX1Y1Z2 >= 0) { // edge x1y1z2
      Rectangle containerX1Y1Z2(v1.getX() - segX, v1.getY() - segY, v2.getZ(),
				v1.getX(), v1.getY(), v2.getZ() + segZ);
      findParticleInRectangle(containerX1Y1Z2, particleVec, particleX1Y1Z2);
      reqX1Y1Z2[0] = boostWorld.isend(rankX1Y1Z2, mpiTag,  particleX1Y1Z2);
      reqX1Y1Z2[1] = boostWorld.irecv(rankX1Y1Z2, mpiTag, rParticleX1Y1Z2);
    }
    if (rankX1Y2Z1 >= 0) { // edge x1y2z1
      Rectangle containerX1Y2Z1(v1.getX() - segX, v2.getY(), v1.getZ() - segZ,
				v1.getX(), v2.getY() + segY, v1.getZ());
      findParticleInRectangle(containerX1Y2Z1, particleVec, particleX1Y2Z1);
      reqX1Y2Z1[0] = boostWorld.isend(rankX1Y2Z1, mpiTag,  particleX1Y2Z1);
      reqX1Y2Z1[1] = boostWorld.irecv(rankX1Y2Z1, mpiTag, rParticleX1Y2Z1);
    }
    if (rankX1Y2Z2 >= 0) { // edge x1y2z2
      Rectangle containerX1Y2Z2(v1.getX() - segX, v2.getY(), v2.getZ(),
				v1.getX(), v2.getY() + segY, v2.getZ() + segZ);
      findParticleInRectangle(containerX1Y2Z2, particleVec, particleX1Y2Z2);
      reqX1Y2Z2[0] = boostWorld.isend(rankX1Y2Z2, mpiTag,  particleX1Y2Z2);
      reqX1Y2Z2[1] = boostWorld.irecv(rankX1Y2Z2, mpiTag, rParticleX1Y2Z2);
    }
    if (rankX2Y1Z1 >= 0) { // edge x2y1z1
      Rectangle containerX2Y1Z1(v2.getX(), v1.getY() - segY, v1.getZ() - segZ,
				v2.getX() + segX, v1.getY(), v1.getZ());
      findParticleInRectangle(containerX2Y1Z1, particleVec, particleX2Y1Z1);
      reqX2Y1Z1[0] = boostWorld.isend(rankX2Y1Z1, mpiTag,  particleX2Y1Z1);
      reqX2Y1Z1[1] = boostWorld.irecv(rankX2Y1Z1, mpiTag, rParticleX2Y1Z1);
    }
    if (rankX2Y1Z2 >= 0) { // edge x2y1z2
      Rectangle containerX2Y1Z2(v2.getX(), v1.getY() - segY, v2.getZ(),
				v2.getX() + segX, v1.getY(), v2.getZ() + segZ);
      findParticleInRectangle(containerX2Y1Z2, particleVec, particleX2Y1Z2);
      reqX2Y1Z2[0] = boostWorld.isend(rankX2Y1Z2, mpiTag,  particleX2Y1Z2);
      reqX2Y1Z2[1] = boostWorld.irecv(rankX2Y1Z2, mpiTag, rParticleX2Y1Z2);
    }
    if (rankX2Y2Z1 >= 0) { // edge x2y2z1
      Rectangle containerX2Y2Z1(v2.getX(), v2.getY(), v1.getZ() - segZ,
				v2.getX() + segX, v2.getY() + segY, v1.getZ());
      findParticleInRectangle(containerX2Y2Z1, particleVec, particleX2Y2Z1);
      reqX2Y2Z1[0] = boostWorld.isend(rankX2Y2Z1, mpiTag,  particleX2Y2Z1);
      reqX2Y2Z1[1] = boostWorld.irecv(rankX2Y2Z1, mpiTag, rParticleX2Y2Z1);
    }
    if (rankX2Y2Z2 >= 0) { // edge x2y2z2
      Rectangle containerX2Y2Z2(v2.getX(), v2.getY(), v2.getZ(),
				v2.getX() + segX, v2.getY() + segY, v2.getZ() + segZ);
      findParticleInRectangle(containerX2Y2Z2, particleVec, particleX2Y2Z2);
      reqX2Y2Z2[0] = boostWorld.isend(rankX2Y2Z2, mpiTag,  particleX2Y2Z2);
      reqX2Y2Z2[1] = boostWorld.irecv(rankX2Y2Z2, mpiTag, rParticleX2Y2Z2);
    }
    // 6 surfaces
    if (rankX1 >= 0) boost::mpi::wait_all(reqX1, reqX1 + 2);
    if (rankX2 >= 0) boost::mpi::wait_all(reqX2, reqX2 + 2);
    if (rankY1 >= 0) boost::mpi::wait_all(reqY1, reqY1 + 2);
    if (rankY2 >= 0) boost::mpi::wait_all(reqY2, reqY2 + 2);
    if (rankZ1 >= 0) boost::mpi::wait_all(reqZ1, reqZ1 + 2);
    if (rankZ2 >= 0) boost::mpi::wait_all(reqZ2, reqZ2 + 2);
    // 12 edges
    if (rankX1Y1 >= 0) boost::mpi::wait_all(reqX1Y1, reqX1Y1 + 2);
    if (rankX1Y2 >= 0) boost::mpi::wait_all(reqX1Y2, reqX1Y2 + 2);  
    if (rankX1Z1 >= 0) boost::mpi::wait_all(reqX1Z1, reqX1Z1 + 2);
    if (rankX1Z2 >= 0) boost::mpi::wait_all(reqX1Z2, reqX1Z2 + 2);
    if (rankX2Y1 >= 0) boost::mpi::wait_all(reqX2Y1, reqX2Y1 + 2);
    if (rankX2Y2 >= 0) boost::mpi::wait_all(reqX2Y2, reqX2Y2 + 2);  
    if (rankX2Z1 >= 0) boost::mpi::wait_all(reqX2Z1, reqX2Z1 + 2);
    if (rankX2Z2 >= 0) boost::mpi::wait_all(reqX2Z2, reqX2Z2 + 2); 
    if (rankY1Z1 >= 0) boost::mpi::wait_all(reqY1Z1, reqY1Z1 + 2);
    if (rankY1Z2 >= 0) boost::mpi::wait_all(reqY1Z2, reqY1Z2 + 2);
    if (rankY2Z1 >= 0) boost::mpi::wait_all(reqY2Z1, reqY2Z1 + 2);
    if (rankY2Z2 >= 0) boost::mpi::wait_all(reqY2Z2, reqY2Z2 + 2); 
    // 8 vertices
    if (rankX1Y1Z1 >= 0) boost::mpi::wait_all(reqX1Y1Z1, reqX1Y1Z1 + 2);
    if (rankX1Y1Z2 >= 0) boost::mpi::wait_all(reqX1Y1Z2, reqX1Y1Z2 + 2);
    if (rankX1Y2Z1 >= 0) boost::mpi::wait_all(reqX1Y2Z1, reqX1Y2Z1 + 2);
    if (rankX1Y2Z2 >= 0) boost::mpi::wait_all(reqX1Y2Z2, reqX1Y2Z2 + 2);
    if (rankX2Y1Z1 >= 0) boost::mpi::wait_all(reqX2Y1Z1, reqX2Y1Z1 + 2);
    if (rankX2Y1Z2 >= 0) boost::mpi::wait_all(reqX2Y1Z2, reqX2Y1Z2 + 2);
    if (rankX2Y2Z1 >= 0) boost::mpi::wait_all(reqX2Y2Z1, reqX2Y2Z1 + 2);
    if (rankX2Y2Z2 >= 0) boost::mpi::wait_all(reqX2Y2Z2, reqX2Y2Z2 + 2);  

    // delete outgoing particles
    removeParticleOutRectangle();

    // add incoming particles
    recvParticleVec.clear(); // new use of recvParticleVec
    // 6 surfaces
    if (rankX1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX1.begin(), rParticleX1.end());
    if (rankX2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX2.begin(), rParticleX2.end());
    if (rankY1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleY1.begin(), rParticleY1.end());
    if (rankY2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleY2.begin(), rParticleY2.end());
    if (rankZ1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleZ1.begin(), rParticleZ1.end());
    if (rankZ2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleZ2.begin(), rParticleZ2.end());
    // 12 edges
    if (rankX1Y1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX1Y1.begin(), rParticleX1Y1.end());
    if (rankX1Y2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX1Y2.begin(), rParticleX1Y2.end());
    if (rankX1Z1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX1Z1.begin(), rParticleX1Z1.end());
    if (rankX1Z2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX1Z2.begin(), rParticleX1Z2.end());
    if (rankX2Y1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX2Y1.begin(), rParticleX2Y1.end());
    if (rankX2Y2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX2Y2.begin(), rParticleX2Y2.end());
    if (rankX2Z1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX2Z1.begin(), rParticleX2Z1.end());
    if (rankX2Z2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX2Z2.begin(), rParticleX2Z2.end());
    if (rankY1Z1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleY1Z1.begin(), rParticleY1Z1.end());
    if (rankY1Z2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleY1Z2.begin(), rParticleY1Z2.end());
    if (rankY2Z1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleY2Z1.begin(), rParticleY2Z1.end());
    if (rankY2Z2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleY2Z2.begin(), rParticleY2Z2.end());
    // 8 vertices
    if (rankX1Y1Z1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX1Y1Z1.begin(), rParticleX1Y1Z1.end());
    if (rankX1Y1Z2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX1Y1Z2.begin(), rParticleX1Y1Z2.end());
    if (rankX1Y2Z1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX1Y2Z1.begin(), rParticleX1Y2Z1.end());
    if (rankX1Y2Z2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX1Y2Z2.begin(), rParticleX1Y2Z2.end());
    if (rankX2Y1Z1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX2Y1Z1.begin(), rParticleX2Y1Z1.end());
    if (rankX2Y1Z2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX2Y1Z2.begin(), rParticleX2Y1Z2.end());
    if (rankX2Y2Z1 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX2Y2Z1.begin(), rParticleX2Y2Z1.end());
    if (rankX2Y2Z2 >= 0) recvParticleVec.insert(recvParticleVec.end(), rParticleX2Y2Z2.begin(), rParticleX2Y2Z2.end());

    particleVec.insert(particleVec.end(), recvParticleVec.begin(), recvParticleVec.end());

    /*
      if (recvParticleVec.size() > 0) {    
      debugInf << "iter=" << std::setw(8) << iteration << " rank=" << std::setw(2) << mpiRank 
      << "   added=";
      for (std::vector<Particle*>::const_iterator it = recvParticleVec.begin(); it != recvParticleVec.end(); ++it)
      debugInf << std::setw(3) << (*it)->getId();
      debugInf << " now " << particleVec.size() << ": ";
      for (std::vector<Particle*>::const_iterator it = particleVec.begin(); it != particleVec.end(); ++it)
      debugInf << std::setw(3) << (*it)->getId();
      debugInf << std::endl;
      }
    */

    // do not release memory of received particles because they are part of and managed by particleVec
    // 6 surfaces
    rParticleX1.clear();
    rParticleX2.clear();
    rParticleY1.clear();
    rParticleY2.clear();
    rParticleZ1.clear();
    rParticleZ2.clear();
    // 12 edges
    rParticleX1Y1.clear();
    rParticleX1Y2.clear();
    rParticleX1Z1.clear();
    rParticleX1Z2.clear();
    rParticleX2Y1.clear();
    rParticleX2Y2.clear();
    rParticleX2Z1.clear();
    rParticleX2Z2.clear();
    rParticleY1Z1.clear();
    rParticleY1Z2.clear();
    rParticleY2Z1.clear();
    rParticleY2Z2.clear();
    // 8 vertices
    rParticleX1Y1Z1.clear();
    rParticleX1Y1Z2.clear();
    rParticleX1Y2Z1.clear();
    rParticleX1Y2Z2.clear();
    rParticleX2Y1Z1.clear();
    rParticleX2Y1Z2.clear();
    rParticleX2Y2Z1.clear();
    rParticleX2Y2Z2.clear();

    recvParticleVec.clear();
  }


  void Assembly::gatherParticle() {
    // update allParticleVec: process 0 collects all updated particles from each other process  
    if (mpiRank != 0) {// each process except 0
      boostWorld.send(0, mpiTag, particleVec);
    }
    else { // process 0
      // allParticleVec is cleared before filling with new data
      releaseGatheredParticle();

      // duplicate particleVec so that it is not destroyed by allParticleVec in next iteration, otherwise it causes memory error.
      std::vector<Particle*> dupParticleVec(particleVec.size());
      for (std::size_t i = 0; i < dupParticleVec.size(); ++i)
	dupParticleVec[i] = new Particle(*particleVec[i]);

      // fill allParticleVec with dupParticleVec and received particles
      allParticleVec.insert(allParticleVec.end(), dupParticleVec.begin(), dupParticleVec.end());

      std::vector<Particle*> tmpParticleVec;
      long gatherRam = 0;
      for (int iRank = 1; iRank < mpiSize; ++iRank) {

	tmpParticleVec.clear();// do not destroy particles!
	boostWorld.recv(iRank, mpiTag, tmpParticleVec);
	allParticleVec.insert(allParticleVec.end(), tmpParticleVec.begin(), tmpParticleVec.end());
	gatherRam += tmpParticleVec.size();

      }
      //debugInf << "gather: particleNum = " << gatherRam <<  " particleRam = " << gatherRam * sizeof(Particle) << std::endl;
    }
  }


  void Assembly::releaseGatheredParticle() {
    // clear allParticleVec, avoid long time memory footprint.
    // Releasing memory of a vector of pointers involves three steps:
    for (std::vector<Particle *>::iterator it = allParticleVec.begin(); it != allParticleVec.end(); ++it)
      delete (*it);
    allParticleVec.clear();
    std::vector<Particle *>().swap(allParticleVec);
  }


  void Assembly::gatherBdryContact() {
    if (isBdryProcess()) {
      // it is good to clear possibly contacting particles, otherwise it causes more MPI transmission.
      // however, the contactInfo of a boundary should not be cleared.
      for(std::vector<Boundary*>::iterator it = boundaryVec.begin(); it != boundaryVec.end(); ++it)
	(*it)->clearPossParticle();      

      if (mpiRank != 0)
	boostWorld.send(0, mpiTag, boundaryVec);
    }

    if (mpiRank == 0) {
      mergeBoundaryVec.clear(); // do not delete pointers as shared with boundaryVec
      std::vector<Boundary *>().swap(mergeBoundaryVec);

      mergeBoundaryVec = boundaryVec; 
      std::vector<Boundary*> tmpBoundaryVec;   
      for (std::size_t it = 0; it < bdryProcess.size(); ++it) {
	if (bdryProcess[it] != 0) {// not root process
	  tmpBoundaryVec.clear();  // do not destroy particles!
	  boostWorld.recv(bdryProcess[it], mpiTag, tmpBoundaryVec);
	  // merge tmpBoundaryVec into mergeBoundaryVec
	  assert(tmpBoundaryVec.size() == mergeBoundaryVec.size());
	  for (std::size_t jt = 0; jt < tmpBoundaryVec.size(); ++jt)
	    mergeBoundaryVec[jt]->getContactInfo().insert(   \
							  mergeBoundaryVec[jt]->getContactInfo().end(), \
							  tmpBoundaryVec[jt]->getContactInfo().begin(), \
							  tmpBoundaryVec[jt]->getContactInfo().end() );
	}    
      }

      // must update after collecting all boundary contact info
      for(std::vector<Boundary*>::iterator it = mergeBoundaryVec.begin(); it != mergeBoundaryVec.end(); ++it)
	(*it)->updateStatForce();
    }
  }
  

  void Assembly::printBdryContact(const char *str) const {
    std::ofstream ofs(str);
    if(!ofs) { debugInf << "stream error: printBdryContact" << std::endl; exit(-1); }
    ofs.setf(std::ios::scientific, std::ios::floatfield);
    ofs.precision(OPREC);
  
    ofs << std::setw(OWID) << mergeBoundaryVec.size() << std::endl << std::endl;
    for(std::vector<Boundary*>::const_iterator it = mergeBoundaryVec.begin(); it != mergeBoundaryVec.end(); ++it) {
      (*it)->printContactInfo(ofs);
    }
  
    ofs.close();
  }


#ifdef STRESS_STRAIN
  void Assembly::calcPrevGranularStress() {
    prevGranularStress.setZero();
    if (particleVec.size() >= 10) {
      updateGranularCell();
      calcGranularStress(prevGranularStress);
    }
  }


  void Assembly::gatherGranularStress(REAL timeStep, REAL timeIncr) {
    // no matter how many particles exist in the compute grid, these variables need to be cleared.
    cellVec.clear();
    granularStress.setZero();
    granularStressRate.setZero();
    OldroStressRate.setZero();  
    TruesStressRate.setZero(); 
    granularStrain.clear();

    if (particleVec.size() >= 10) {
      updateGranularCell();
      calcGranularStress(granularStress);
      calcGranularStrain(timeIncr);
      granularStressRate = (granularStress - prevGranularStress) / timeStep;
      // objective stress rate (must use l and d)
      Eigen::Matrix3d ml = granularStrain[3]; // this is a dangerous usage, must ensure existance.
      Eigen::Matrix3d md = granularStrain[4];
      OldroStressRate = granularStressRate - ml * granularStress - granularStress * ml.transpose();
      TruesStressRate = OldroStressRate + granularStress * md.trace();

      /*
      Eigen::IOFormat fmt(Eigen::FullPrecision, 0, ", ", ";\n", "", "", "[", "]");
      //Eigen::IOFormat fmt(6, 0, ", ", ";\n", "", "", "[", "]");
      std::cout << "iteration=" << iteration << " process=" << mpiRank << " process(i,j,k)=(" << mpiCoords[0] << " " << mpiCoords[1] << " " << mpiCoords[2] << ")"
		<< " //////////////////" << std::endl
		<< "prevStress= ..." << std::endl << prevGranularStress.format(fmt) << std::endl << std::endl
		<< "stress= ..." << std::endl << granularStress.format(fmt) << std::endl << std::endl
		<< "stressRate= ..." << std::endl << granularStressRate.format(fmt) << std::endl << std::endl
		<< "velocityGradient= ..." << std::endl << ml.format(fmt) << std::endl << std::endl
		<< "rateOfDeform= ..." << std::endl << md.format(fmt) << std::endl << std::endl
		<< "OldroStressRate= ..." << std::endl << OldroStressRate.format(fmt) << std::endl << std::endl
		<< "TruesStressRate= ..." << std::endl << TruesStressRate.format(fmt) << std::endl << std::endl
	;
      */
    }

    convertGranularStressForPrint();
    printStressVec.clear();
    gather(boostWorld, printStress, printStressVec, 0); // Boost MPI
  }


  void Assembly::calcGranularStrain(REAL timeIncr) {
    /*
    Eigen::Matrix3d  matrixF;   // deformation gradient
    Eigen::Matrix3d  matrixFdot;// rate of deformation gradient
    Eigen::Matrix3d  matrixR;   // rotation matrix in polar decomposition
    Eigen::Matrix3d  matrixU;   // stretch matrix in polar decomposition
    Eigen::Matrix3d  matrixE;   // Lagrangian-Green strain
    Eigen::Matrix3d  matrix_l;  // velocity gradient
    Eigen::Matrix3d  matrix_d;  // rate of deformation
    Eigen::Matrix3d  matrix_w;  // spin

    Eigen::Matrix3d  intgraF;   // snapshot-integrated F using Fdot = l F
    */

    Eigen::Matrix3d avg;
    REAL actualVolume;

    // averaging for matrixF, only accounting for initially "continuous" tetrahedra
    avg.setZero();
    actualVolume = 0;
    for (int i = 0; i < cellVec.size(); ++i) {
      if (cellVec[i].getMatrixF() != Eigen::Matrix3d::Zero(3,3)) {
	avg += cellVec[i].getMatrixF() * cellVec[i].getVolume();
	actualVolume += cellVec[i].getVolume();
      }
    }
    avg /= actualVolume;
    Eigen::Matrix3d matrixF = avg;

    // averaging for matrixFdot, only accounting for initially "continuous" tetrahedra
    avg.setZero();
    actualVolume = 0;
    for (int i = 0; i < cellVec.size(); ++i) {
      if (cellVec[i].getMatrixFdot() != Eigen::Matrix3d::Zero(3,3)) {
	avg += cellVec[i].getMatrixFdot() * cellVec[i].getVolume();
	actualVolume += cellVec[i].getVolume();
      }
    }
    avg /= actualVolume;
    Eigen::Matrix3d matrixFdot = avg;

    // averaging for matrix_l, not associated with initial state of any tetrahedra
    avg.setZero();
    for (int i = 0; i < cellVec.size(); ++i) {
      avg += cellVec[i].getMatrix_l() * cellVec[i].getVolume();
    }
    avg /= getGranularCellVolume();
    Eigen::Matrix3d matrix_l = avg;

    Eigen::Matrix3d matrixE = 0.5 * (matrixF.transpose() * matrixF - Eigen::Matrix3d::Identity(3,3));

    // polar decompostion of matrixF
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(matrixF.transpose() * matrixF);
    Eigen::Matrix3d matrixU = es.operatorSqrt();           // stretch tensor
    Eigen::Matrix3d matrixR = matrixF * matrixU.inverse(); // rotation tensor

    // symmetric and skew-symmetric decomposition of matrix_l
    Eigen::Matrix3d matrix_d = 0.5 * (matrix_l + matrix_l.transpose());
    Eigen::Matrix3d matrix_w = 0.5 * (matrix_l - matrix_l.transpose());

    // time integration for deformation gradient
    Eigen::Matrix3d intgraF = (timeIncr * matrix_l + Eigen::Matrix3d::Identity(3,3)) * prevSnapMatrixF;
    prevSnapMatrixF = intgraF; 

    ///*
    //Eigen::IOFormat fmt(Eigen::FullPrecision, 0, ", ", ";\n", "", "", "[", "]");
    Eigen::IOFormat fmt(6, 0, ", ", ";\n", "", "", "[", "]");
    std::cout << "iteration=" << iteration << " process=" << mpiRank << " process(i,j,k)=(" << mpiCoords[0] << " " << mpiCoords[1] << " " << mpiCoords[2] << ")"
	      << " //////////////////" << std::endl
	      << "intgraF= ..." << std::endl << intgraF.format(fmt) << std::endl << std::endl
	      << "matrixF= ..." << std::endl << matrixF.format(fmt) << std::endl << std::endl
	      << "matrixFdot= ..." << std::endl << matrixFdot.format(fmt) << std::endl << std::endl
	      << "matrixE= ..." << std::endl << matrixE.format(fmt) << std::endl << std::endl
	      << "matrixR= ..." << std::endl << matrixR.format(fmt) << std::endl << std::endl
	      << "matrixU= ..." << std::endl << matrixU.format(fmt) << std::endl << std::endl
	      << "matrix_l= ..." << std::endl << matrix_l.format(fmt) << std::endl << std::endl
	      << "matrix_d= ..." << std::endl << matrix_d.format(fmt) << std::endl << std::endl
	      << "matrix_w= ..." << std::endl << matrix_w.format(fmt) << std::endl << std::endl
      ;
    //*/
    
    // for printing purpose
    granularStrain.clear();
    granularStrain.push_back(matrixF);
    granularStrain.push_back(matrixR);
    granularStrain.push_back(matrixU);
    granularStrain.push_back(matrix_l);    
    granularStrain.push_back(matrix_d);
    granularStrain.push_back(matrix_w);
  }


  void Assembly::updateGranularCell() {
    /* testing rbox 10 D3
    std::stringstream ptclCoordStream("3 10 \
-0.0222149361131852 -0.366434993563625 0.3270621312102882 \
-0.06676722137887703 -0.1566931052661437 0.4589771055234383 \
0.02820502736438535 0.04189077954915421 0.05832764185809314 \
0.3126723396709863 0.08400649026409401 -0.1029227018383543 \
0.1781470954214661 0.1182274414396169 0.04860343742054274 \
-0.1220315663349177 0.01546165115708642 -0.1360330368727753 \
-0.3072535691850387 -0.01073880122111998 -0.4870359524963758 \
0.3867462923626847 0.04492879989084675 0.118335500935405 \
-0.1352406177997967 0.01093378431250691 -0.2358910583293913 \
0.3789805913148268 -0.4732086509216658 -0.2177962499836425");
    */

    std::stringstream ptclCoordStream;
    ptclCoordStream << 3 << " " << particleVec.size() << " ";
    for (int i = 0; i < particleVec.size(); ++i)
      ptclCoordStream << particleVec[i]->getCurrPos().getX() << " "
		      << particleVec[i]->getCurrPos().getY() << " "
		      << particleVec[i]->getCurrPos().getZ() << " ";
    //std::cout << "ptcl coordinate: " << std::endl << ptclCoordStream.str() << std::endl;    

    orgQhull::RboxPoints rbox;
    rbox.appendPoints(ptclCoordStream); 

    orgQhull::Qhull qhull;
    qhull.runQhull(rbox, "d Qt i"); // "d Qt Qz Qs i"

    std::stringstream cellConnectStream;
    qhull.setOutputStream(&cellConnectStream);
    qhull.outputQhull();
    //std::cout << "cell connectivity: " << std::endl << cellConnectStream.str() << std::endl;

    int totalNum;
    cellConnectStream >> totalNum;
    cellVec.clear();
    int m, n, i, j;
    for(int it = 0; it < totalNum; ++it){
      cellConnectStream >> m >> n >> i >> j;
      ++m;
      ++n;
      ++i;
      ++j;
      // Qhull IDs start from 0; FEM IDs start from 1; particleVec[it-1]: must -1 to obtain the corresponding coordinates. 
      Cell tmpCell(m, n, i, j, particleVec[m-1], particleVec[n-1], particleVec[i-1], particleVec[j-1]);
      if (fabs(tmpCell.getVolume()) > EPS)
	cellVec.push_back(tmpCell);
    }
    
    for (int i = 0; i < cellVec.size(); ++i)
      cellVec[i].setNodeOrderCalcMatrix();
  }


  // only snapshot particle positions for strain measurement
  void Assembly::snapParticlePos() {
    for(std::vector<Particle*>::iterator it = particleVec.begin(); it != particleVec.end(); ++it)
      (*it)->setSnapPos();
  }


  void Assembly::printGranularStressFEM(const char *str) const {
    std::ofstream ofs(str);
    if(!ofs) { debugInf << "stream error: printGranularStressFEM" << std::endl; exit(-1); }
    ofs.setf(std::ios::scientific, std::ios::floatfield);
    ofs.precision(OPREC);

    Vec v1 = grid.getMinCorner();
    Vec v2 = grid.getMaxCorner();
    Vec vspan = v2 - v1;

    ofs	<< "VARIABLES=" << std::endl
	<< " " << "x"
	<< " " << "y"
	<< " " << "z"

	<< " " << "sigma_xx"
	<< " " << "sigma_yy"
	<< " " << "sigma_zz"
	<< " " << "sigma_xy"
	<< " " << "sigma_xz"
	<< " " << "sigma_yz"

	<< " " << "sigmaDot_xx"
	<< " " << "sigmaDot_yy"
	<< " " << "sigmaDot_zz"
	<< " " << "sigmaDot_xy"
	<< " " << "sigmaDot_xz"
	<< " " << "sigmaDot_yz"

	<< " " << "Oldro_xx"
	<< " " << "Oldro_xy"
	<< " " << "Oldro_xz"
	<< " " << "Oldro_yx"
	<< " " << "Oldro_yy"
	<< " " << "Oldro_yz"
	<< " " << "Oldro_zx"
	<< " " << "Oldro_zy"
	<< " " << "Oldro_zz"

	<< " " << "Trues_xx"
	<< " " << "Trues_xy"
	<< " " << "Trues_xz"
	<< " " << "Trues_yx"
	<< " " << "Trues_yy"
	<< " " << "Trues_yz"
	<< " " << "Trues_zx"
	<< " " << "Trues_zy"
	<< " " << "Trues_zz"

	<< " " << "F_xX"
	<< " " << "F_xY"
	<< " " << "F_xZ"
	<< " " << "F_yX"
	<< " " << "F_yY"
	<< " " << "F_yZ"
	<< " " << "F_zX"
	<< " " << "F_zY"
	<< " " << "F_zZ"

	<< " " << "R_xX"
	<< " " << "R_xY"
	<< " " << "R_xZ"
	<< " " << "R_yX"
	<< " " << "R_yY"
	<< " " << "R_yZ"
	<< " " << "R_zX"
	<< " " << "R_zY"
	<< " " << "R_zZ"

	<< " " << "U_xX"
	<< " " << "U_yY"
	<< " " << "U_zZ"
	<< " " << "U_xY"
	<< " " << "U_xZ"
	<< " " << "U_yZ"

	<< " " << "l_xx"
	<< " " << "l_xy"
	<< " " << "l_xz"
	<< " " << "l_yx"
	<< " " << "l_yy"
	<< " " << "l_yz"
	<< " " << "l_zx"
	<< " " << "l_zy"
	<< " " << "l_zz"

	<< " " << "d_xx"
	<< " " << "d_yy"
	<< " " << "d_zz"
	<< " " << "d_xy"
	<< " " << "d_xz"
	<< " " << "d_yz"

	<< " " << "w_xy"
	<< " " << "w_xz"
	<< " " << "w_yz"

	<< " " << "norm_sigma"
	<< " " << "norm_sigmaDot"
	<< " " << "norm_Oldro"
	<< " " << "norm_Trues"
	<< " " << "norm_F"
	<< " " << "norm_U"
	<< " " << "norm_l"
	<< " " << "norm_d"
	<< " " << "norm_w"

	<< " " << "sigma_1"
	<< " " << "sigma_2"
	<< " " << "sigma_3"
	<< " " << "sigma_1_v1"
	<< " " << "sigma_1_v2"
	<< " " << "sigma_1_v3"
	<< " " << "sigma_2_v1"
	<< " " << "sigma_2_v2"
	<< " " << "sigma_2_v3"
	<< " " << "sigma_3_v1"
	<< " " << "sigma_3_v2"
	<< " " << "sigma_3_v3"

	<< " " << "sigmaDot_1"
	<< " " << "sigmaDot_2"
	<< " " << "sigmaDot_3"
	<< " " << "sigmaDot_1_v1"
	<< " " << "sigmaDot_1_v2"
	<< " " << "sigmaDot_1_v3"
	<< " " << "sigmaDot_2_v1"
	<< " " << "sigmaDot_2_v2"
	<< " " << "sigmaDot_2_v3"
	<< " " << "sigmaDot_3_v1"
	<< " " << "sigmaDot_3_v2"
	<< " " << "sigmaDot_3_v3"

	<< " " << "d_1"
	<< " " << "d_2"
	<< " " << "d_3"
	<< " " << "d_1_v1"
	<< " " << "d_1_v2"
	<< " " << "d_1_v3"
	<< " " << "d_2_v1"
	<< " " << "d_2_v2"
	<< " " << "d_2_v3"
	<< " " << "d_3_v1"
	<< " " << "d_3_v2"
	<< " " << "d_3_v3"

	<< std::endl;

    ofs	<< "ZONE T=\"stress\" N=" << (mpiProcX + 1) * (mpiProcY + 1) * (mpiProcZ + 1)
	<< ", E=" << mpiProcX * mpiProcY * mpiProcZ << ", DATAPACKING=BLOCK, \
VARLOCATION=([4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,\
31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,\
61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,\
91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,\
116,117,118,119,120]=CELLCENTERED), ZONETYPE=FEBRICK" << std::endl;

    long int totalCoord = (mpiProcX + 1) * (mpiProcY + 1) * (mpiProcZ + 1);
    std::vector<Vec> spaceCoords(totalCoord);
    std::size_t index = 0;
    for (std::size_t i = 0; i < mpiProcX + 1; ++i)
      for (std::size_t j = 0; j < mpiProcY + 1; ++j)
	for (std::size_t k = 0; k < mpiProcZ + 1; ++k)
	  spaceCoords[index++] = Vec(v1.getX() + vspan.getX() / mpiProcX * i,
				     v1.getY() + vspan.getY() / mpiProcY * j,
				     v1.getZ() + vspan.getZ() / mpiProcZ * k);

    // Tecplot: 
    // BLOCK format must be used for cell-centered data.
    // For nodal variables, provide the values for each variable in nodal order. 
    // Similarly, for cell-centered values,provide the variable values in cell order.
    for (std::size_t i = 0; i < spaceCoords.size(); ++i)
      ofs << std::setw(OWID) << spaceCoords[i].getX();
    ofs << std::endl;

    for (std::size_t i = 0; i < spaceCoords.size(); ++i)
      ofs << std::setw(OWID) << spaceCoords[i].getY();
    ofs << std::endl;

    for (std::size_t i = 0; i < spaceCoords.size(); ++i)
      ofs << std::setw(OWID) << spaceCoords[i].getZ();
    ofs << std::endl;

    // The order of MPI gather agrees with (int iRank = 0; iRank < mpiSize; ++iRank) below.
    // In setCommunicator(), int reorder = 0; // mpiRank not reordered
    int numCompo = 6;
    for (int j = 0; j < numCompo; ++j) {
      for (int i = 0; i < printStressVec.size(); ++i)
	ofs << std::setw(OWID) << printStressVec[i].stress[j];
      ofs << std::endl;
    }

    numCompo = 6;
    for (int j = 0; j < numCompo; ++j) {
      for (int i = 0; i < printStressVec.size(); ++i)
	ofs << std::setw(OWID) << printStressVec[i].stressRate[j];
      ofs << std::endl;
    }

    numCompo = 9;
    for (int j = 0; j < numCompo; ++j) {
      for (int i = 0; i < printStressVec.size(); ++i)
	ofs << std::setw(OWID) << printStressVec[i].OldroStressRate[j];
      ofs << std::endl;
    }

    numCompo = 9;
    for (int j = 0; j < numCompo; ++j) {
      for (int i = 0; i < printStressVec.size(); ++i)
	ofs << std::setw(OWID) << printStressVec[i].TruesStressRate[j];
      ofs << std::endl;
    }

    numCompo = 9;
    for (int j = 0; j < numCompo; ++j) {
      for (int i = 0; i < printStressVec.size(); ++i)
	ofs << std::setw(OWID) << printStressVec[i].deformGradient[j];
      ofs << std::endl;
    }

    numCompo = 9;
    for (int j = 0; j < numCompo; ++j) {
      for (int i = 0; i < printStressVec.size(); ++i)
	ofs << std::setw(OWID) << printStressVec[i].rotation[j];
      ofs << std::endl;
    }

    numCompo = 6;
    for (int j = 0; j < numCompo; ++j) {
      for (int i = 0; i < printStressVec.size(); ++i)
	ofs << std::setw(OWID) << printStressVec[i].stretch[j];
      ofs << std::endl;
    }

    numCompo = 9;
    for (int j = 0; j < numCompo; ++j) {
      for (int i = 0; i < printStressVec.size(); ++i)
	ofs << std::setw(OWID) << printStressVec[i].velocityGradient[j];
      ofs << std::endl;
    }

    numCompo = 6;
    for (int j = 0; j < numCompo; ++j) {
      for (int i = 0; i < printStressVec.size(); ++i)
	ofs << std::setw(OWID) << printStressVec[i].rateOfDeform[j];
      ofs << std::endl;
    }

    numCompo = 3;
    for (int j = 0; j < numCompo; ++j) {
      for (int i = 0; i < printStressVec.size(); ++i)
	ofs << std::setw(OWID) << printStressVec[i].spin[j];
      ofs << std::endl;
    }

    numCompo = 9; // for norm of 9 tensors
    for (int j = 0; j < numCompo; ++j) {
      for (int i = 0; i < printStressVec.size(); ++i)
	ofs << std::setw(OWID) << printStressVec[i].norm[j];
      ofs << std::endl;
    }

    // eigen of stress
    numCompo = 3;
    for (int j = 0; j < numCompo; ++j) {
      for (int i = 0; i < printStressVec.size(); ++i)
	ofs << std::setw(OWID) << printStressVec[i].stressEigenValue[j];
      ofs << std::endl;
    }
    numCompo = 9;
    for (int j = 0; j < numCompo; ++j) {
      for (int i = 0; i < printStressVec.size(); ++i)
	ofs << std::setw(OWID) << printStressVec[i].stressEigenVector[j];
      ofs << std::endl;
    }

    // eigen of stressRate
    numCompo = 3;
    for (int j = 0; j < numCompo; ++j) {
      for (int i = 0; i < printStressVec.size(); ++i)
	ofs << std::setw(OWID) << printStressVec[i].stressRateEigenValue[j];
      ofs << std::endl;
    }
    numCompo = 9;
    for (int j = 0; j < numCompo; ++j) {
      for (int i = 0; i < printStressVec.size(); ++i)
	ofs << std::setw(OWID) << printStressVec[i].stressRateEigenVector[j];
      ofs << std::endl;
    }

    // eigen of rateOfDeform
    numCompo = 3;
    for (int j = 0; j < numCompo; ++j) {
      for (int i = 0; i < printStressVec.size(); ++i)
	ofs << std::setw(OWID) << printStressVec[i].rateOfDeformEigenValue[j];
      ofs << std::endl;
    }
    numCompo = 9;
    for (int j = 0; j < numCompo; ++j) {
      for (int i = 0; i < printStressVec.size(); ++i)
	ofs << std::setw(OWID) << printStressVec[i].rateOfDeformEigenVector[j];
      ofs << std::endl;
    }

    // The order agrees with MPI gather order.
    // In setCommunicator(), int reorder = 0; // mpiRank not reordered
    for (int iRank = 0; iRank < mpiSize; ++iRank) {
      int coords[3];
      MPI_Cart_coords(cartComm, iRank, 3, coords);

      int id4 = 1 + coords[0]*(mpiProcZ+1)*(mpiProcY+1) + coords[1]*(mpiProcZ+1) + coords[2];
      int id1 = 1 + (coords[0]+1)*(mpiProcZ+1)*(mpiProcY+1) + coords[1]*(mpiProcZ+1) + coords[2];
      int id3 = 1 + coords[0]*(mpiProcZ+1)*(mpiProcY+1) + (coords[1]+1)*(mpiProcZ+1) + coords[2];
      int id2 = 1 + (coords[0]+1)*(mpiProcZ+1)*(mpiProcY+1) + (coords[1]+1)*(mpiProcZ+1) + coords[2];

      int id8 = 1 + coords[0]*(mpiProcZ+1)*(mpiProcY+1) + coords[1]*(mpiProcZ+1) + (coords[2]+1);
      int id5 = 1 + (coords[0]+1)*(mpiProcZ+1)*(mpiProcY+1) + coords[1]*(mpiProcZ+1) + (coords[2]+1);
      int id7 = 1 + coords[0]*(mpiProcZ+1)*(mpiProcY+1) + (coords[1]+1)*(mpiProcZ+1) + (coords[2]+1);
      int id6 = 1 + (coords[0]+1)*(mpiProcZ+1)*(mpiProcY+1) + (coords[1]+1)*(mpiProcZ+1) + (coords[2]+1);

      ofs << std::setw(8) << id1 << std::setw(8) << id2 << std::setw(8) << id3 << std::setw(8) << id4 
	  << std::setw(8) << id5 << std::setw(8) << id6 << std::setw(8) << id7 << std::setw(8) << id8 << std::endl;
    }

    ofs.close();
  }


  void Assembly::printGranularStressOrdered(const char *str) const {
    std::ofstream ofs(str);
    if(!ofs) { debugInf << "stream error: printGranularStressOrdered" << std::endl; exit(-1); }
    ofs.setf(std::ios::scientific, std::ios::floatfield);
    ofs.precision(OPREC);
    //ofs << std::setw(OWID) << printStressVec.size() << std::endl;

    ofs	<< std::setw(OWID) << "VARIABLES=" << std::endl
	<< std::setw(OWID) << "x"
	<< std::setw(OWID) << "y"
	<< std::setw(OWID) << "z"

	<< std::setw(OWID) << "sigma_xx"
	<< std::setw(OWID) << "sigma_yy"
	<< std::setw(OWID) << "sigma_zz"
	<< std::setw(OWID) << "sigma_xy"
	<< std::setw(OWID) << "sigma_xz"
	<< std::setw(OWID) << "sigma_yz"

	<< std::setw(OWID) << "sigmaDot_xx"
	<< std::setw(OWID) << "sigmaDot_yy"
	<< std::setw(OWID) << "sigmaDot_zz"
	<< std::setw(OWID) << "sigmaDot_xy"
	<< std::setw(OWID) << "sigmaDot_xz"
	<< std::setw(OWID) << "sigmaDot_yz"

	<< std::setw(OWID) << "Oldro_xx"
	<< std::setw(OWID) << "Oldro_xy"
	<< std::setw(OWID) << "Oldro_xz"
	<< std::setw(OWID) << "Oldro_yx"
	<< std::setw(OWID) << "Oldro_yy"
	<< std::setw(OWID) << "Oldro_yz"
	<< std::setw(OWID) << "Oldro_zx"
	<< std::setw(OWID) << "Oldro_zy"
	<< std::setw(OWID) << "Oldro_zz"

	<< std::setw(OWID) << "Trues_xx"
	<< std::setw(OWID) << "Trues_xy"
	<< std::setw(OWID) << "Trues_xz"
	<< std::setw(OWID) << "Trues_yx"
	<< std::setw(OWID) << "Trues_yy"
	<< std::setw(OWID) << "Trues_yz"
	<< std::setw(OWID) << "Trues_zx"
	<< std::setw(OWID) << "Trues_zy"
	<< std::setw(OWID) << "Trues_zz"

	<< std::setw(OWID) << "F_xX"
	<< std::setw(OWID) << "F_xY"
	<< std::setw(OWID) << "F_xZ"
	<< std::setw(OWID) << "F_yX"
	<< std::setw(OWID) << "F_yY"
	<< std::setw(OWID) << "F_yZ"
	<< std::setw(OWID) << "F_zX"
	<< std::setw(OWID) << "F_zY"
	<< std::setw(OWID) << "F_zZ"

	<< std::setw(OWID) << "R_xX"
	<< std::setw(OWID) << "R_xY"
	<< std::setw(OWID) << "R_xZ"
	<< std::setw(OWID) << "R_yX"
	<< std::setw(OWID) << "R_yY"
	<< std::setw(OWID) << "R_yZ"
	<< std::setw(OWID) << "R_zX"
	<< std::setw(OWID) << "R_zY"
	<< std::setw(OWID) << "R_zZ"

	<< std::setw(OWID) << "U_xX"
	<< std::setw(OWID) << "U_yY"
	<< std::setw(OWID) << "U_zZ"
	<< std::setw(OWID) << "U_xY"
	<< std::setw(OWID) << "U_xZ"
	<< std::setw(OWID) << "U_yZ"

	<< std::setw(OWID) << "l_xx"
	<< std::setw(OWID) << "l_xy"
	<< std::setw(OWID) << "l_xz"
	<< std::setw(OWID) << "l_yx"
	<< std::setw(OWID) << "l_yy"
	<< std::setw(OWID) << "l_yz"
	<< std::setw(OWID) << "l_zx"
	<< std::setw(OWID) << "l_zy"
	<< std::setw(OWID) << "l_zz"

	<< std::setw(OWID) << "d_xx"
	<< std::setw(OWID) << "d_yy"
	<< std::setw(OWID) << "d_zz"
	<< std::setw(OWID) << "d_xy"
	<< std::setw(OWID) << "d_xz"
	<< std::setw(OWID) << "d_yz"

	<< std::setw(OWID) << "w_xy"
	<< std::setw(OWID) << "w_xz"
	<< std::setw(OWID) << "w_yz"

	<< std::setw(OWID) << "norm_sigma"
	<< std::setw(OWID) << "norm_sigmaDot"
	<< std::setw(OWID) << "norm_Oldro"
	<< std::setw(OWID) << "norm_Trues"
	<< std::setw(OWID) << "norm_F"
	<< std::setw(OWID) << "norm_U"
	<< std::setw(OWID) << "norm_l"
	<< std::setw(OWID) << "norm_d"
	<< std::setw(OWID) << "norm_w"

	<< std::setw(OWID) << "sigma_1"
	<< std::setw(OWID) << "sigma_2"
	<< std::setw(OWID) << "sigma_3"
	<< std::setw(OWID) << "sigma_1_v1"
	<< std::setw(OWID) << "sigma_1_v2"
	<< std::setw(OWID) << "sigma_1_v3"
	<< std::setw(OWID) << "sigma_2_v1"
	<< std::setw(OWID) << "sigma_2_v2"
	<< std::setw(OWID) << "sigma_2_v3"
	<< std::setw(OWID) << "sigma_3_v1"
	<< std::setw(OWID) << "sigma_3_v2"
	<< std::setw(OWID) << "sigma_3_v3"

	<< std::setw(OWID) << "sigmaDot_1"
	<< std::setw(OWID) << "sigmaDot_2"
	<< std::setw(OWID) << "sigmaDot_3"
	<< std::setw(OWID) << "sigmaDot_1_v1"
	<< std::setw(OWID) << "sigmaDot_1_v2"
	<< std::setw(OWID) << "sigmaDot_1_v3"
	<< std::setw(OWID) << "sigmaDot_2_v1"
	<< std::setw(OWID) << "sigmaDot_2_v2"
	<< std::setw(OWID) << "sigmaDot_2_v3"
	<< std::setw(OWID) << "sigmaDot_3_v1"
	<< std::setw(OWID) << "sigmaDot_3_v2"
	<< std::setw(OWID) << "sigmaDot_3_v3"

	<< std::setw(OWID) << "d_1"
	<< std::setw(OWID) << "d_2"
	<< std::setw(OWID) << "d_3"
	<< std::setw(OWID) << "d_1_v1"
	<< std::setw(OWID) << "d_1_v2"
	<< std::setw(OWID) << "d_1_v3"
	<< std::setw(OWID) << "d_2_v1"
	<< std::setw(OWID) << "d_2_v2"
	<< std::setw(OWID) << "d_2_v3"
	<< std::setw(OWID) << "d_3_v1"
	<< std::setw(OWID) << "d_3_v2"
	<< std::setw(OWID) << "d_3_v3"

	<< std::endl;

    ofs << "ZONE I=" << mpiProcX
	<< ", J=" << mpiProcY
	<< ", K=" << mpiProcZ
	<< ", DATAPACKING=POINT"
	<< std::endl;

    for (int i = 0; i < printStressVec.size(); ++i) {
      printStressVec[i].print(ofs);
    }

    ofs.close();
  }


  void Assembly::convertGranularStressForPrint() {
    printStress.setZero();

    // container: last update in commuParticle(); next update in migrateParticle() 
    printStress.coord[0]  = container.getCenter().getX();
    printStress.coord[1]  = container.getCenter().getY();
    printStress.coord[2]  = container.getCenter().getZ();

    printStress.stress[0] = granularStress(0,0);
    printStress.stress[1] = granularStress(1,1);
    printStress.stress[2] = granularStress(2,2);
    printStress.stress[3] = (granularStress(0,1) + granularStress(1,0)) / 2;
    printStress.stress[4] = (granularStress(0,2) + granularStress(2,0)) / 2;
    printStress.stress[5] = (granularStress(1,2) + granularStress(2,1)) / 2;

    printStress.stressRate[0] = granularStressRate(0,0);
    printStress.stressRate[1] = granularStressRate(1,1);
    printStress.stressRate[2] = granularStressRate(2,2);
    printStress.stressRate[3] = (granularStressRate(0,1) + granularStressRate(1,0)) / 2;
    printStress.stressRate[4] = (granularStressRate(0,2) + granularStressRate(2,0)) / 2;
    printStress.stressRate[5] = (granularStressRate(1,2) + granularStressRate(2,1)) / 2;

    printStress.OldroStressRate[0] = OldroStressRate(0,0);
    printStress.OldroStressRate[1] = OldroStressRate(0,1);
    printStress.OldroStressRate[2] = OldroStressRate(0,2);
    printStress.OldroStressRate[3] = OldroStressRate(1,0);
    printStress.OldroStressRate[4] = OldroStressRate(1,1);
    printStress.OldroStressRate[5] = OldroStressRate(1,2);
    printStress.OldroStressRate[6] = OldroStressRate(2,0);
    printStress.OldroStressRate[7] = OldroStressRate(2,1);
    printStress.OldroStressRate[8] = OldroStressRate(2,2);

    printStress.TruesStressRate[0] = TruesStressRate(0,0);
    printStress.TruesStressRate[1] = TruesStressRate(0,1);
    printStress.TruesStressRate[2] = TruesStressRate(0,2);
    printStress.TruesStressRate[3] = TruesStressRate(1,0);
    printStress.TruesStressRate[4] = TruesStressRate(1,1);
    printStress.TruesStressRate[5] = TruesStressRate(1,2);
    printStress.TruesStressRate[6] = TruesStressRate(2,0);
    printStress.TruesStressRate[7] = TruesStressRate(2,1);
    printStress.TruesStressRate[8] = TruesStressRate(2,2);

    //std::cout << "granularStrain.size()=" << granularStrain.size() << std::endl;
    if (granularStrain.size() == 6) { // otherwise it could be empty

      int i = 0;
      printStress.deformGradient[0] = granularStrain[i](0,0);
      printStress.deformGradient[1] = granularStrain[i](0,1);
      printStress.deformGradient[2] = granularStrain[i](0,2);
      printStress.deformGradient[3] = granularStrain[i](1,0);
      printStress.deformGradient[4] = granularStrain[i](1,1);
      printStress.deformGradient[5] = granularStrain[i](1,2);
      printStress.deformGradient[6] = granularStrain[i](2,0);
      printStress.deformGradient[7] = granularStrain[i](2,1);
      printStress.deformGradient[8] = granularStrain[i](2,2);

      ++i;
      printStress.rotation[0] = granularStrain[i](0,0);
      printStress.rotation[1] = granularStrain[i](0,1);
      printStress.rotation[2] = granularStrain[i](0,2);
      printStress.rotation[3] = granularStrain[i](1,0);
      printStress.rotation[4] = granularStrain[i](1,1);
      printStress.rotation[5] = granularStrain[i](1,2);
      printStress.rotation[6] = granularStrain[i](2,0);
      printStress.rotation[7] = granularStrain[i](2,1);
      printStress.rotation[8] = granularStrain[i](2,2);

      ++i;
      printStress.stretch[0] = granularStrain[i](0,0);
      printStress.stretch[1] = granularStrain[i](1,1);
      printStress.stretch[2] = granularStrain[i](2,2);
      printStress.stretch[3] = granularStrain[i](0,1);
      printStress.stretch[4] = granularStrain[i](0,2);
      printStress.stretch[5] = granularStrain[i](1,2);

      ++i;
      printStress.velocityGradient[0] = granularStrain[i](0,0);
      printStress.velocityGradient[1] = granularStrain[i](0,1);
      printStress.velocityGradient[2] = granularStrain[i](0,2);
      printStress.velocityGradient[3] = granularStrain[i](1,0);
      printStress.velocityGradient[4] = granularStrain[i](1,1);
      printStress.velocityGradient[5] = granularStrain[i](1,2);
      printStress.velocityGradient[6] = granularStrain[i](2,0);
      printStress.velocityGradient[7] = granularStrain[i](2,1);
      printStress.velocityGradient[8] = granularStrain[i](2,2);

      ++i;
      printStress.rateOfDeform[0] = granularStrain[i](0,0);
      printStress.rateOfDeform[1] = granularStrain[i](1,1);
      printStress.rateOfDeform[2] = granularStrain[i](2,2);
      printStress.rateOfDeform[3] = granularStrain[i](0,1);
      printStress.rateOfDeform[4] = granularStrain[i](0,2);
      printStress.rateOfDeform[5] = granularStrain[i](1,2);

      ++i;
      printStress.spin[0] = granularStrain[i](0,1);
      printStress.spin[1] = granularStrain[i](0,2);
      printStress.spin[2] = granularStrain[i](1,2);
    }

    printStress.norm[0] = granularStress.norm();
    printStress.norm[1] = granularStressRate.norm();
    printStress.norm[2] = OldroStressRate.norm();
    printStress.norm[3] = TruesStressRate.norm();

    if (granularStrain.size() == 6) { // otherwise it could be empty
      printStress.norm[4] = granularStrain[0].norm();
      printStress.norm[5] = granularStrain[2].norm();
      printStress.norm[6] = granularStrain[3].norm();
      printStress.norm[7] = granularStrain[4].norm();
      printStress.norm[8] = granularStrain[5].norm();
    }

    Eigen::Vector3d value;
    Eigen::Matrix3d vectors;
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es;

    // Eigen: The eigenvalues are sorted in increasing order, and eigenvectors follow eigenvalues.
    es.compute(granularStress);
    value   = es.eigenvalues();
    vectors = es.eigenvectors();
    printStress.stressEigenValue[0] = value(0);
    printStress.stressEigenValue[1] = value(1);
    printStress.stressEigenValue[2] = value(2);
    printStress.stressEigenVector[0]= vectors.col(0)(0);
    printStress.stressEigenVector[1]= vectors.col(0)(1);
    printStress.stressEigenVector[2]= vectors.col(0)(2);
    printStress.stressEigenVector[3]= vectors.col(1)(0);
    printStress.stressEigenVector[4]= vectors.col(1)(1);
    printStress.stressEigenVector[5]= vectors.col(1)(2);
    printStress.stressEigenVector[6]= vectors.col(2)(0);
    printStress.stressEigenVector[7]= vectors.col(2)(1);
    printStress.stressEigenVector[8]= vectors.col(2)(2);

    es.compute(granularStressRate);
    value   = es.eigenvalues();
    vectors = es.eigenvectors();
    printStress.stressRateEigenValue[0] = value(0);
    printStress.stressRateEigenValue[1] = value(1);
    printStress.stressRateEigenValue[2] = value(2);
    printStress.stressRateEigenVector[0]= vectors.col(0)(0);
    printStress.stressRateEigenVector[1]= vectors.col(0)(1);
    printStress.stressRateEigenVector[2]= vectors.col(0)(2);
    printStress.stressRateEigenVector[3]= vectors.col(1)(0);
    printStress.stressRateEigenVector[4]= vectors.col(1)(1);
    printStress.stressRateEigenVector[5]= vectors.col(1)(2);
    printStress.stressRateEigenVector[6]= vectors.col(2)(0);
    printStress.stressRateEigenVector[7]= vectors.col(2)(1);
    printStress.stressRateEigenVector[8]= vectors.col(2)(2);

    if (granularStrain.size() == 6) { // otherwise it could be empty
      es.compute(granularStrain[4]);  // rateOfDeform
      value   = es.eigenvalues();
      vectors = es.eigenvectors();
      printStress.rateOfDeformEigenValue[0] = value(0);
      printStress.rateOfDeformEigenValue[1] = value(1);
      printStress.rateOfDeformEigenValue[2] = value(2);
      printStress.rateOfDeformEigenVector[0]= vectors.col(0)(0);
      printStress.rateOfDeformEigenVector[1]= vectors.col(0)(1);
      printStress.rateOfDeformEigenVector[2]= vectors.col(0)(2);
      printStress.rateOfDeformEigenVector[3]= vectors.col(1)(0);
      printStress.rateOfDeformEigenVector[4]= vectors.col(1)(1);
      printStress.rateOfDeformEigenVector[5]= vectors.col(1)(2);
      printStress.rateOfDeformEigenVector[6]= vectors.col(2)(0);
      printStress.rateOfDeformEigenVector[7]= vectors.col(2)(1);
      printStress.rateOfDeformEigenVector[8]= vectors.col(2)(2);
    }
  }


  REAL Assembly::getGranularCellVolume() {
    REAL volume = 0;
    for (int i = 0; i < cellVec.size(); ++i)
      volume += cellVec[i].getVolume();
    //std::cout << "volume = " << volume << std::endl;
    return volume;
  }


  void Assembly::calcGranularStress(Eigen::Matrix3d &stress) { // sigma(i,j) = 1/V sum(li Fj)
    stress.setZero();

    Eigen::Vector3d lc, p1Center, p2Center;	
    Eigen::RowVector3d Fc;
    lc.setZero();
    Fc.setZero();
    p1Center.setZero();
    p2Center.setZero();

    for (std::vector<Contact>::const_iterator it=contactVec.begin();it!=contactVec.end();++it) {
      p1Center(0) = it->getP1()->getCurrPos().getX();
      p1Center(1) = it->getP1()->getCurrPos().getY();
      p1Center(2) = it->getP1()->getCurrPos().getZ();

      p2Center(0) = it->getP2()->getCurrPos().getX();
      p2Center(1) = it->getP2()->getCurrPos().getY();
      p2Center(2) = it->getP2()->getCurrPos().getZ();

      lc = -(p1Center - p2Center); // contact force points to particle 1; "-" sign is for Elasticity convention.
      Fc(0) = (it->normalForceVec().getX())+(it->tgtForceVec().getX());
      Fc(1) = (it->normalForceVec().getY())+(it->tgtForceVec().getY());
      Fc(2) = (it->normalForceVec().getZ())+(it->tgtForceVec().getZ());

      //std::cout << "lc=" << lc.transpose() << " Fc=" << Fc << std::endl; 
      stress += lc * Fc;
    }

    //std::cout << mpiRank << std::endl << sress << std::endl << std::endl;
    stress /= getGranularCellVolume();
    //stress /= container.getVolume();
  }
#endif
// end of #ifdef STRESS_STRAIN

  void Assembly::gatherEnergy() {
    calcTransEnergy();
    calcRotatEnergy();
    calcKinetEnergy();
    calcGraviEnergy(allContainer.getMinCorner().getZ());
    calcMechaEnergy();
  }


  void Assembly::closeProg(std::ofstream &ofs) {
    ofs.close();
  }


  void  Assembly::openDepositProg(std::ofstream &ofs, const char *str) {
    ofs.open(str);
    if(!ofs) { debugInf << "stream error: openDepositProg" << std::endl; exit(-1); }
    ofs.setf(std::ios::scientific, std::ios::floatfield);
    ofs.precision(OPREC);

    ofs << std::setw(OWID) << "iteration"
	<< std::setw(OWID) << "normal_x1"
	<< std::setw(OWID) << "normal_x2"
	<< std::setw(OWID) << "normal_y1"
	<< std::setw(OWID) << "normal_y2"
	<< std::setw(OWID) << "normal_z1"
	<< std::setw(OWID) << "normal_z2"
    
	<< std::setw(OWID) << "contact_x1"
	<< std::setw(OWID) << "contact_x2"
	<< std::setw(OWID) << "contact_y1"
	<< std::setw(OWID) << "contact_y2"
	<< std::setw(OWID) << "contact_z1"
	<< std::setw(OWID) << "contact_z2"
	<< std::setw(OWID) << "contact_inside"
    
	<< std::setw(OWID) << "penetr_x1"
	<< std::setw(OWID) << "penetr_x2"
	<< std::setw(OWID) << "penetr_y1"
	<< std::setw(OWID) << "penetr_y2"
	<< std::setw(OWID) << "penetr_z1"
	<< std::setw(OWID) << "penetr_z2"

	<< std::setw(OWID) << "avgNormal"
	<< std::setw(OWID) << "avgShear"
	<< std::setw(OWID) << "avgPenetr"
    
	<< std::setw(OWID) << "transEnergy"
	<< std::setw(OWID) << "rotatEnergy"
	<< std::setw(OWID) << "kinetEnergy"
	<< std::setw(OWID) << "graviEnergy"
	<< std::setw(OWID) << "mechaEnergy"

	<< std::setw(OWID) << "vibra_est_dt"
	<< std::setw(OWID) << "impact_est_dt"
	<< std::setw(OWID) << "actual_dt"
	<< std::setw(OWID) << "accruedTime"
    
	<< std::endl;
  }


  void Assembly::printDepositProg(std::ofstream &ofs) {
    REAL var[6];
  
    // normalForce
    for (std::size_t i = 0; i < 6; ++i)
      var[i] = 0;
    for(std::vector<Boundary*>::const_iterator it = mergeBoundaryVec.begin(); it != mergeBoundaryVec.end(); ++it) {
      std::size_t id = (*it)->getId();
      Vec normal = (*it)->getNormalForce();
      switch (id) {
      case 1: 
	var[0] = fabs(normal.getX());
	break;
      case 2:
	var[1] = normal.getX();
	break;
      case 3:
	var[2] = fabs(normal.getY());
	break;
      case 4:
	var[3] = normal.getY();
	break;
      case 5:
	var[4] = fabs(normal.getZ());
	break;
      case 6:
	var[5] = normal.getZ();
	break;
      }
    }
    ofs << std::setw(OWID) << iteration;
    for (std::size_t i = 0; i < 6; ++i)
      ofs << std::setw(OWID) << var[i];
  
    // contactNum
    for (std::size_t i = 0; i < 6; ++i)
      var[i] = 0;
    for(std::vector<Boundary*>::const_iterator it = mergeBoundaryVec.begin(); it != mergeBoundaryVec.end(); ++it) {
      std::size_t id = (*it)->getId();
      var[id - 1] = (*it)->getContactNum();
    }
    for (std::size_t i = 0; i < 6; ++i)
      ofs << std::setw(OWID) << static_cast<std::size_t> (var[i]);
    ofs << std::setw(OWID) << allContactNum;
  
    // avgPenetr
    for (std::size_t i = 0; i < 6; ++i)
      var[i] = 0;
    for(std::vector<Boundary*>::const_iterator it = mergeBoundaryVec.begin(); it != mergeBoundaryVec.end(); ++it) {
      std::size_t id = (*it)->getId();
      var[id - 1] = (*it)->getAvgPenetr();
    }
    for (std::size_t i = 0; i < 6; ++i)
      ofs << std::setw(OWID) << var[i];
  
    // average data
    ofs << std::setw(OWID) << avgNormal
	<< std::setw(OWID) << avgShear
	<< std::setw(OWID) << avgPenetr;

    // energy
    ofs << std::setw(OWID) << transEnergy
	<< std::setw(OWID) << rotatEnergy
	<< std::setw(OWID) << kinetEnergy
	<< std::setw(OWID) << graviEnergy
	<< std::setw(OWID) << mechaEnergy;

    // time
    ofs << std::setw(OWID) << vibraTimeStep
	<< std::setw(OWID) << impactTimeStep
	<< std::setw(OWID) << timeStep
	<< std::setw(OWID) << timeAccrued;

    ofs << std::endl;
  }


  void Assembly::getStartDimension(REAL &distX, REAL &distY, REAL &distZ) {
    REAL x1, x2, y1, y2, z1, z2;
    // use boundaryVec
    for(std::vector<Boundary*>::const_iterator it = boundaryVec.begin(); it != boundaryVec.end(); ++it) {
      switch ((*it)->getId()) {
      case 1: 
	x1 = (*it)->getPoint().getX();
	break;
      case 2:
	x2 = (*it)->getPoint().getX();
	break;
      case 3:
	y1 = (*it)->getPoint().getY();
	break;
      case 4:
	y2 = (*it)->getPoint().getY();
	break;
      case 5:
	z1 = (*it)->getPoint().getZ();
	break;
      case 6:
	z2 = (*it)->getPoint().getZ();
	break;
      }
    }
    distX = x2 - x1;
    distY = y2 - y1;
    distZ = z2 - z1;
  }


  void  Assembly::openCompressProg(std::ofstream &ofs, const char *str) {
    ofs.open(str);
    if(!ofs) { debugInf << "stream error: openCompressProg" << std::endl; exit(-1); }
    ofs.setf(std::ios::scientific, std::ios::floatfield);
    ofs.precision(OPREC);

    ofs << std::setw(OWID) << "iteration"
	<< std::setw(OWID) << "traction_x1"
	<< std::setw(OWID) << "traction_x2"
	<< std::setw(OWID) << "traction_y1"
	<< std::setw(OWID) << "traction_y2"
	<< std::setw(OWID) << "traction_z1"
	<< std::setw(OWID) << "traction_z2"
	<< std::setw(OWID) << "mean_stress"

	<< std::setw(OWID) << "bulk_volume"
	<< std::setw(OWID) << "density"
	<< std::setw(OWID) << "epsilon_x"
	<< std::setw(OWID) << "epsilon_y"
	<< std::setw(OWID) << "epsilon_z"
	<< std::setw(OWID) << "epsilon_v"
	<< std::setw(OWID) << "void_ratio"
	<< std::setw(OWID) << "porosity"

	<< std::setw(OWID) << "velocity_x1"
	<< std::setw(OWID) << "velocity_x2"
	<< std::setw(OWID) << "velocity_y1"
	<< std::setw(OWID) << "velocity_y2"
	<< std::setw(OWID) << "velocity_z1"
	<< std::setw(OWID) << "velocity_z2"
    
	<< std::setw(OWID) << "contact_x1"
	<< std::setw(OWID) << "contact_x2"
	<< std::setw(OWID) << "contact_y1"
	<< std::setw(OWID) << "contact_y2"
	<< std::setw(OWID) << "contact_z1"
	<< std::setw(OWID) << "contact_z2"
	<< std::setw(OWID) << "contact_inside"
    
	<< std::setw(OWID) << "penetr_x1"
	<< std::setw(OWID) << "penetr_x2"
	<< std::setw(OWID) << "penetr_y1"
	<< std::setw(OWID) << "penetr_y2"
	<< std::setw(OWID) << "penetr_z1"
	<< std::setw(OWID) << "penetr_z2"
    
	<< std::setw(OWID) << "avgNormal"
	<< std::setw(OWID) << "avgShear"
	<< std::setw(OWID) << "avgPenetr"

	<< std::setw(OWID) << "transEnergy"
	<< std::setw(OWID) << "rotatEnergy"
	<< std::setw(OWID) << "kinetEnergy"
	<< std::setw(OWID) << "graviEnergy"
	<< std::setw(OWID) << "mechaEnergy"

	<< std::setw(OWID) << "vibra_est_dt"
	<< std::setw(OWID) << "impact_est_dt"
	<< std::setw(OWID) << "actual_dt"
    
	<< std::endl;
  }


  void Assembly::printCompressProg(std::ofstream &ofs, REAL distX, REAL distY, REAL distZ) {
    REAL x1, x2, y1, y2, z1, z2;
    for(std::vector<Boundary*>::const_iterator it = mergeBoundaryVec.begin(); it != mergeBoundaryVec.end(); ++it) {
      switch ((*it)->getId()) {
      case 1: 
	x1 = (*it)->getPoint().getX();
	break;
      case 2:
	x2 = (*it)->getPoint().getX();
	break;
      case 3:
	y1 = (*it)->getPoint().getY();
	break;
      case 4:
	y2 = (*it)->getPoint().getY();
	break;
      case 5:
	z1 = (*it)->getPoint().getZ();
	break;
      case 6:
	z2 = (*it)->getPoint().getZ();
	break;
      }
    }
    REAL areaX = (y2 - y1) * (z2 - z1);
    REAL areaY = (z2 - z1) * (x2 - x1);
    REAL areaZ = (x2 - x1) * (y2 - y1);
    REAL bulkVolume = (x2 - x1) * (y2 - y1) * (z2 - z1);
    REAL voidRatio = bulkVolume / getParticleVolume() - 1;

    REAL var[6], vel[6];
    // normalForce
    for (std::size_t i = 0; i < 6; ++i) {
      var[i] = 0;
      vel[i] = 0;
    }
    for(std::vector<Boundary*>::const_iterator it = mergeBoundaryVec.begin(); it != mergeBoundaryVec.end(); ++it) {
      std::size_t id = (*it)->getId();
      Vec normal = (*it)->getNormalForce();
      Vec veloc  = (*it)->getVeloc();
      switch (id) {
      case 1: 
	var[0] = fabs(normal.getX()) / areaX;
	vel[0] = veloc.getX();
	break;
      case 2:
	var[1] = normal.getX() / areaX;
	vel[1] = veloc.getX();
	break;
      case 3:
	var[2] = fabs(normal.getY()) / areaY;
	vel[2] = veloc.getY();
	break;
      case 4:
	var[3] = normal.getY() / areaY;
	vel[3] = veloc.getY();
	break;
      case 5:
	var[4] = fabs(normal.getZ()) / areaZ;
	vel[4] = veloc.getZ();
	break;
      case 6:
	var[5] = normal.getZ() / areaZ;
	vel[5] = veloc.getZ();
	break;
      }
    }
    ofs << std::setw(OWID) << iteration;
    REAL avg = 0;
    for (std::size_t i = 0; i < 6; ++i) {
      ofs << std::setw(OWID) << var[i];
      avg += var[i];
    }
    ofs << std::setw(OWID) << avg/6;  

    // volume
    ofs << std::setw(OWID) << bulkVolume
	<< std::setw(OWID) << getMass() / bulkVolume
	<< std::setw(OWID) << 1-(x2-x1)/distX
	<< std::setw(OWID) << 1-(y2-y1)/distY
	<< std::setw(OWID) << 1-(z2-z1)/distZ
	<< std::setw(OWID) << 3-(x2-x1)/distX - (y2-y1)/distY - (z2-z1)/distZ
	<< std::setw(OWID) << voidRatio
	<< std::setw(OWID) << voidRatio / (1 + voidRatio);

    // velocity
    for (std::size_t i = 0; i < 6; ++i)
      ofs << std::setw(OWID) << vel[i];  

    // contactNum
    for (std::size_t i = 0; i < 6; ++i)
      var[i] = 0;
    for(std::vector<Boundary*>::const_iterator it = mergeBoundaryVec.begin(); it != mergeBoundaryVec.end(); ++it) {
      std::size_t id = (*it)->getId();
      var[id - 1] = (*it)->getContactNum();
    }
    for (std::size_t i = 0; i < 6; ++i)
      ofs << std::setw(OWID) << static_cast<std::size_t> (var[i]);
    ofs << std::setw(OWID) << allContactNum;
  
    // avgPenetr
    for (std::size_t i = 0; i < 6; ++i)
      var[i] = 0;
    for(std::vector<Boundary*>::const_iterator it = mergeBoundaryVec.begin(); it != mergeBoundaryVec.end(); ++it) {
      std::size_t id = (*it)->getId();
      var[id - 1] = (*it)->getAvgPenetr();
    }
    for (std::size_t i = 0; i < 6; ++i)
      ofs << std::setw(OWID) << var[i];
  
    // average data
    ofs << std::setw(OWID) << avgNormal
	<< std::setw(OWID) << avgShear
	<< std::setw(OWID) << avgPenetr;

    // energy
    ofs << std::setw(OWID) << transEnergy
	<< std::setw(OWID) << rotatEnergy
	<< std::setw(OWID) << kinetEnergy
	<< std::setw(OWID) << graviEnergy
	<< std::setw(OWID) << mechaEnergy;

    // time
    ofs << std::setw(OWID) << vibraTimeStep
	<< std::setw(OWID) << impactTimeStep
	<< std::setw(OWID) << timeStep;

    ofs << std::endl;
  }


  void Assembly::readParticle(const char *inputParticle) {

    REAL young = dem::Parameter::getSingleton().parameter["young"];
    REAL poisson = dem::Parameter::getSingleton().parameter["poisson"];

    std::ifstream ifs(inputParticle);
    if(!ifs) { debugInf << "stream error: readParticle" << std::endl; exit(-1); }
    std::size_t particleNum;
    ifs >> particleNum;
    std::string str;
    ifs >> str >> str >> str >> str >> str >> str >> str >> str >> str >> str
	>> str >> str >> str >> str >> str >> str >> str >> str >> str >> str
	>> str >> str >> str >> str >> str >> str >> str >> str >> str;
  
    std::vector<Particle*>::iterator it;
    for(it = allParticleVec.begin(); it != allParticleVec.end(); ++it)
      delete (*it);
    allParticleVec.clear();

    std::size_t id, type;
    REAL a, b, c, px, py, pz, dax, day, daz, dbx, dby, dbz, dcx, dcy, dcz;
    REAL vx, vy, vz, omx, omy, omz, fx, fy, fz, mx, my, mz;
    for (std::size_t i = 0; i < particleNum; ++i){
      ifs >> id >> type >> a >> b >> c >> px >> py >> pz >> dax >> day >> daz >> dbx >> dby >> dbz >> dcx >> dcy >> dcz
	  >> vx >> vy >> vz >> omx >> omy >> omz >> fx >> fy >> fz >> mx >> my >> mz;
      Particle* pt= new Particle(id, type, Vec(a,b,c), Vec(px,py,pz), Vec(dax,day,daz), Vec(dbx,dby,dbz), Vec(dcx,dcy,dcz), young, poisson);
    
      // optional settings for a particle's initial status
      if ( (static_cast<std::size_t> (dem::Parameter::getSingleton().parameter["toInitParticle"])) == 1 ) {
	pt->setPrevVeloc(Vec(vx,vy,vz));
	pt->setCurrVeloc(Vec(vx,vy,vz));
	pt->setPrevOmga(Vec(omx,omy,omz));
	pt->setCurrOmga(Vec(omx,omy,omz));
	pt->setForce(Vec(fx,fy,fz));  // initial force
	pt->setMoment(Vec(mx,my,mz)); // initial moment
      }
    
      //pt->setConstForce(Vec(fx,fy,fz));  // constant force, not initial force
      //pt->setConstMoment(Vec(mx,my,mz)); // constant moment, not initial moment

      allParticleVec.push_back(pt);
    }
  
    std::size_t sieveNum;
    ifs >> sieveNum;
    std::vector<REAL> percent(sieveNum), size(sieveNum);
    for (std::size_t i = 0; i < sieveNum; ++i)
      ifs >> percent[i] >> size[i];
    REAL ratio_ba, ratio_ca;
    ifs >> ratio_ba >> ratio_ca;
    setGradation(Gradation(sieveNum, percent, size, ratio_ba, ratio_ca));
    ifs.close();
  }


  void Assembly::printParticle(const char *str) const {
    std::ofstream ofs(str);
    if(!ofs) { debugInf << "stream error: printParticle" << std::endl; exit(-1); }
    ofs.setf(std::ios::scientific, std::ios::floatfield);
    ofs.precision(OPREC);
    ofs << std::setw(OWID) << allParticleVec.size() << std::endl;
    ofs << std::setw(OWID) << "id"
	<< std::setw(OWID) << "type"
	<< std::setw(OWID) << "radius_a"
	<< std::setw(OWID) << "radius_b"
	<< std::setw(OWID) << "radius_c"
	<< std::setw(OWID) << "position_x"
	<< std::setw(OWID) << "position_y"
	<< std::setw(OWID) << "position_z"
	<< std::setw(OWID) << "axis_a_x"
	<< std::setw(OWID) << "axis_a_y"
	<< std::setw(OWID) << "axis_a_z"
	<< std::setw(OWID) << "axis_b_x"
	<< std::setw(OWID) << "axis_b_y"
	<< std::setw(OWID) << "axis_b_z"
	<< std::setw(OWID) << "axis_c_x"
	<< std::setw(OWID) << "axis_c_y"
	<< std::setw(OWID) << "axis_c_z"
	<< std::setw(OWID) << "velocity_x"
	<< std::setw(OWID) << "velocity_y"
	<< std::setw(OWID) << "velocity_z"
	<< std::setw(OWID) << "omga_x"
	<< std::setw(OWID) << "omga_y"
	<< std::setw(OWID) << "omga_z"
	<< std::setw(OWID) << "force_x"
	<< std::setw(OWID) << "force_y"
	<< std::setw(OWID) << "force_z"
	<< std::setw(OWID) << "moment_x"
	<< std::setw(OWID) << "moment_y"
	<< std::setw(OWID) << "moment_z"
	<< std::endl;
  
    Vec vObj;
    std::vector<Particle*>::const_iterator  it;
    for (it = allParticleVec.begin(); it != allParticleVec.end();++it)  {
      ofs << std::setw(OWID) << (*it)->getId()
	  << std::setw(OWID) << (*it)->getType()
	  << std::setw(OWID) << (*it)->getA()
	  << std::setw(OWID) << (*it)->getB()
	  << std::setw(OWID) << (*it)->getC();
    
      vObj=(*it)->getCurrPos();
      ofs << std::setw(OWID) << vObj.getX()
	  << std::setw(OWID) << vObj.getY()
	  << std::setw(OWID) << vObj.getZ();
    
      vObj=(*it)->getCurrDirecA();
      ofs << std::setw(OWID) << vObj.getX()
	  << std::setw(OWID) << vObj.getY()
	  << std::setw(OWID) << vObj.getZ();
    
      vObj=(*it)->getCurrDirecB();
      ofs << std::setw(OWID) << vObj.getX()
	  << std::setw(OWID) << vObj.getY()
	  << std::setw(OWID) << vObj.getZ();
    
      vObj=(*it)->getCurrDirecC();
      ofs << std::setw(OWID) << vObj.getX()
	  << std::setw(OWID) << vObj.getY()
	  << std::setw(OWID) << vObj.getZ();
    
      vObj=(*it)->getCurrVeloc();
      ofs << std::setw(OWID) << vObj.getX()
	  << std::setw(OWID) << vObj.getY()
	  << std::setw(OWID) << vObj.getZ();
    
      vObj=(*it)->getCurrOmga();
      ofs << std::setw(OWID) << vObj.getX()
	  << std::setw(OWID) << vObj.getY()
	  << std::setw(OWID) << vObj.getZ();
    
      vObj=(*it)->getForce();
      ofs << std::setw(OWID) << vObj.getX()
	  << std::setw(OWID) << vObj.getY()
	  << std::setw(OWID) << vObj.getZ();
    
      vObj=(*it)->getMoment();
      ofs << std::setw(OWID) << vObj.getX()
	  << std::setw(OWID) << vObj.getY()
	  << std::setw(OWID) << vObj.getZ() << std::endl;
    }
  
    std::size_t sieveNum = gradation.getSieveNum();
    std::vector<REAL> percent = gradation.getPercent();
    std::vector<REAL> size    = gradation.getSize();
    ofs << std::endl << std::setw(OWID) << sieveNum << std::endl;
    for (std::size_t i = 0; i < sieveNum; ++i)
      ofs << std::setw(OWID) << percent[i] << std::setw(OWID) << size[i] << std::endl;
    ofs << std::endl << std::setw(OWID) << gradation.getPtclRatioBA() << std::setw(OWID) << gradation.getPtclRatioCA() << std::endl;

    ofs.close();
  }


  void Assembly::printParticle(const char *str, std::vector<Particle*>  &particleVec) const {
    std::ofstream ofs(str);
    if(!ofs) { debugInf << "stream error: printParticle" << std::endl; exit(-1); }
    ofs.setf(std::ios::scientific, std::ios::floatfield);
    ofs.precision(OPREC);
    ofs << std::setw(OWID) << particleVec.size() << std::endl;
    ofs << std::setw(OWID) << "id"
	<< std::setw(OWID) << "type"
	<< std::setw(OWID) << "radius_a"
	<< std::setw(OWID) << "radius_b"
	<< std::setw(OWID) << "radius_c"
	<< std::setw(OWID) << "position_x"
	<< std::setw(OWID) << "position_y"
	<< std::setw(OWID) << "position_z"
	<< std::setw(OWID) << "axis_a_x"
	<< std::setw(OWID) << "axis_a_y"
	<< std::setw(OWID) << "axis_a_z"
	<< std::setw(OWID) << "axis_b_x"
	<< std::setw(OWID) << "axis_b_y"
	<< std::setw(OWID) << "axis_b_z"
	<< std::setw(OWID) << "axis_c_x"
	<< std::setw(OWID) << "axis_c_y"
	<< std::setw(OWID) << "axis_c_z"
	<< std::setw(OWID) << "velocity_x"
	<< std::setw(OWID) << "velocity_y"
	<< std::setw(OWID) << "velocity_z"
	<< std::setw(OWID) << "omga_x"
	<< std::setw(OWID) << "omga_y"
	<< std::setw(OWID) << "omga_z"
	<< std::setw(OWID) << "force_x"
	<< std::setw(OWID) << "force_y"
	<< std::setw(OWID) << "force_z"
	<< std::setw(OWID) << "moment_x"
	<< std::setw(OWID) << "moment_y"
	<< std::setw(OWID) << "moment_z"
	<< std::endl;
  
    Vec vObj;
    std::vector<Particle*>::const_iterator  it;
    for (it = particleVec.begin(); it != particleVec.end();++it)  {
      ofs << std::setw(OWID) << (*it)->getId()
	  << std::setw(OWID) << (*it)->getType()
	  << std::setw(OWID) << (*it)->getA()
	  << std::setw(OWID) << (*it)->getB()
	  << std::setw(OWID) << (*it)->getC();
    
      vObj=(*it)->getCurrPos();
      ofs << std::setw(OWID) << vObj.getX()
	  << std::setw(OWID) << vObj.getY()
	  << std::setw(OWID) << vObj.getZ();
    
      vObj=(*it)->getCurrDirecA();
      ofs << std::setw(OWID) << vObj.getX()
	  << std::setw(OWID) << vObj.getY()
	  << std::setw(OWID) << vObj.getZ();
    
      vObj=(*it)->getCurrDirecB();
      ofs << std::setw(OWID) << vObj.getX()
	  << std::setw(OWID) << vObj.getY()
	  << std::setw(OWID) << vObj.getZ();
    
      vObj=(*it)->getCurrDirecC();
      ofs << std::setw(OWID) << vObj.getX()
	  << std::setw(OWID) << vObj.getY()
	  << std::setw(OWID) << vObj.getZ();
    
      vObj=(*it)->getCurrVeloc();
      ofs << std::setw(OWID) << vObj.getX()
	  << std::setw(OWID) << vObj.getY()
	  << std::setw(OWID) << vObj.getZ();
    
      vObj=(*it)->getCurrOmga();
      ofs << std::setw(OWID) << vObj.getX()
	  << std::setw(OWID) << vObj.getY()
	  << std::setw(OWID) << vObj.getZ();
    
      vObj=(*it)->getForce();
      ofs << std::setw(OWID) << vObj.getX()
	  << std::setw(OWID) << vObj.getY()
	  << std::setw(OWID) << vObj.getZ();
    
      vObj=(*it)->getMoment();
      ofs << std::setw(OWID) << vObj.getX()
	  << std::setw(OWID) << vObj.getY()
	  << std::setw(OWID) << vObj.getZ() << std::endl;
    }
  
    ofs.close();
  }


  void Assembly::readBoundary(const char *str, const int gridUpdate) {
    std::ifstream ifs(str);
    if(!ifs) { debugInf << "stream error: readBoundary" << std::endl; exit(-1); }

    REAL x1, y1, z1, x2, y2, z2;
    ifs >> x1 >> y1 >> z1 >> x2 >> y2 >> z2;
    setContainer(Rectangle(x1, y1, z1, x2, y2, z2));

    // initial and ongoing compute grids covering particles may or may not be the same as container.
    if (gridUpdate == 1) { // explosion
      Vec v1 = allContainer.getMinCorner();
      Vec v2 = allContainer.getMaxCorner();
      Vec vspan = v2 - v1;
      setGrid(Rectangle(v1.getX() - 0.5*vspan.getX(),
			v1.getY() - 0.5*vspan.getY(),
			v1.getZ(),
			v2.getX() + 0.5*vspan.getX(),
			v2.getY() + 0.5*vspan.getY(),
			v2.getZ() ));
    } else if (gridUpdate == 0) { // side and bottom fixed on container; top varies.
      setGrid(Rectangle(x1,
			y1,
			z1, 
			x2,
			y2,
			getPtclMaxZ(allParticleVec) + gradation.getPtclMaxRadius()  ));
    } else if (gridUpdate == 2) { // bottom fixed on container; others vary.
      setGrid(Rectangle(getPtclMinX(allParticleVec) - gradation.getPtclMaxRadius(),
			getPtclMinY(allParticleVec) - gradation.getPtclMaxRadius(),
			z1,
			getPtclMaxX(allParticleVec) + gradation.getPtclMaxRadius(),
			getPtclMaxY(allParticleVec) + gradation.getPtclMaxRadius(),
			getPtclMaxZ(allParticleVec) + gradation.getPtclMaxRadius()  ));
    } else if (gridUpdate == 3) { // none fixed on container; all varies.
      setGrid(Rectangle(getPtclMinX(allParticleVec) - gradation.getPtclMaxRadius(),
			getPtclMinY(allParticleVec) - gradation.getPtclMaxRadius(),
			getPtclMinZ(allParticleVec) - gradation.getPtclMaxRadius(),
			getPtclMaxX(allParticleVec) + gradation.getPtclMaxRadius(),
			getPtclMaxY(allParticleVec) + gradation.getPtclMaxRadius(),
			getPtclMaxZ(allParticleVec) + gradation.getPtclMaxRadius()  ));
    }
    // setGrid(Rectangle(x1, y1, z1, x2, y2, z2)); 

    boundaryVec.clear();
    Boundary *bptr;
    std::size_t boundaryNum;
    std::size_t type;
    ifs >> boundaryNum;
    for(std::size_t i = 0; i < boundaryNum; ++i) {
      ifs >> type;
      if(type == 1) // plane boundary
	bptr = new planeBoundary(type, ifs);
      else if(type == 2) // cylindrical boundary
	bptr = new cylinderBoundary(type, ifs);

      boundaryVec.push_back(bptr);
    }

    ifs.close();
  }


  void Assembly::printBoundary(const char *str) const {
    std::ofstream ofs(str);
    if(!ofs) { debugInf << "stream error: printBoundary" << std::endl; exit(-1); }
    ofs.setf(std::ios::scientific, std::ios::floatfield);
  
    Vec v1 = allContainer.getMinCorner();
    Vec v2 = allContainer.getMaxCorner();
    REAL x1 = v1.getX();
    REAL y1 = v1.getY();
    REAL z1 = v1.getZ();
    REAL x2 = v2.getX();
    REAL y2 = v2.getY();
    REAL z2 = v2.getZ();
  
    ofs << std::setw(OWID) << x1 << std::setw(OWID) << y1 << std::setw(OWID) << z1
	<< std::setw(OWID) << x2 << std::setw(OWID) << y2 << std::setw(OWID) << z2 << std::endl << std::endl
	<< std::setw(OWID) << boundaryVec.size() << std::endl;

    for(std::vector<Boundary*>::const_iterator it = boundaryVec.begin(); it != boundaryVec.end(); ++it)
      (*it)->print(ofs);
  
    ofs.close();
  }


  void Assembly::plotBoundary(const char *str) const {
    std::ofstream ofs(str);
    if(!ofs) { debugInf << "stream error: plotBoundary" << std::endl; exit(-1); }
    ofs.setf(std::ios::scientific, std::ios::floatfield);
    ofs.precision(OPREC);

    REAL x1, y1, z1, x2, y2, z2;
    x1 = allContainer.getMinCorner().getX();
    y1 = allContainer.getMinCorner().getY();
    z1 = allContainer.getMinCorner().getZ();
    x2 = allContainer.getMaxCorner().getX();
    y2 = allContainer.getMaxCorner().getY();
    z2 = allContainer.getMaxCorner().getZ();

    ofs	<< "ZONE T=\"bdry\" N=8, E=1, DATAPACKING=POINT, ZONETYPE=FEBRICK" << std::endl;
    ofs << std::setw(OWID) << x2 << std::setw(OWID) << y1 << std::setw(OWID) << z1 << std::endl;
    ofs << std::setw(OWID) << x2 << std::setw(OWID) << y2 << std::setw(OWID) << z1 << std::endl;
    ofs << std::setw(OWID) << x1 << std::setw(OWID) << y2 << std::setw(OWID) << z1 << std::endl;
    ofs << std::setw(OWID) << x1 << std::setw(OWID) << y1 << std::setw(OWID) << z1 << std::endl;
    ofs << std::setw(OWID) << x2 << std::setw(OWID) << y1 << std::setw(OWID) << z2 << std::endl;
    ofs << std::setw(OWID) << x2 << std::setw(OWID) << y2 << std::setw(OWID) << z2 << std::endl;
    ofs << std::setw(OWID) << x1 << std::setw(OWID) << y2 << std::setw(OWID) << z2 << std::endl;
    ofs << std::setw(OWID) << x1 << std::setw(OWID) << y1 << std::setw(OWID) << z2 << std::endl;
    ofs << "1 2 3 4 5 6 7 8" << std::endl;

    ofs.close();
  }


  void Assembly::plotGrid(const char *str) const {
    std::ofstream ofs(str);
    if(!ofs) { debugInf << "stream error: plotGrid" << std::endl; exit(-1); }
    ofs.setf(std::ios::scientific, std::ios::floatfield);
    ofs.precision(OPREC);

    Vec v1 = grid.getMinCorner();
    Vec v2 = grid.getMaxCorner();
    Vec vspan = v2 - v1;

    ofs	<< "ZONE T=\"grid\" N=" << (mpiProcX + 1) * (mpiProcY + 1) * (mpiProcZ + 1)
	<< ", E=" << mpiProcX * mpiProcY * mpiProcZ << ", DATAPACKING=POINT, ZONETYPE=FEBRICK" << std::endl;

    std::vector<Vec> coords((mpiProcX + 1) * (mpiProcY + 1) * (mpiProcZ + 1));
    std::size_t index = 0;
    for (std::size_t i = 0; i < mpiProcX + 1; ++i)
      for (std::size_t j = 0; j < mpiProcY + 1; ++j)
	for (std::size_t k = 0; k < mpiProcZ + 1; ++k)
	  coords[index++] = Vec(v1.getX() + vspan.getX() / mpiProcX * i,
				v1.getY() + vspan.getY() / mpiProcY * j,
				v1.getZ() + vspan.getZ() / mpiProcZ * k);

    for (std::size_t i = 0; i < (mpiProcX + 1) * (mpiProcY + 1) * (mpiProcZ + 1); ++i)
      ofs << std::setw(OWID) << coords[i].getX() 
	  << std::setw(OWID) << coords[i].getY() 
	  << std::setw(OWID) << coords[i].getZ() << std::endl;

    for (int iRank = 0; iRank < mpiSize; ++iRank) {
      int coords[3];
      MPI_Cart_coords(cartComm, iRank, 3, coords);

      int id4 = 1 + coords[0]*(mpiProcZ+1)*(mpiProcY+1) + coords[1]*(mpiProcZ+1) + coords[2];
      int id1 = 1 + (coords[0]+1)*(mpiProcZ+1)*(mpiProcY+1) + coords[1]*(mpiProcZ+1) + coords[2];
      int id3 = 1 + coords[0]*(mpiProcZ+1)*(mpiProcY+1) + (coords[1]+1)*(mpiProcZ+1) + coords[2];
      int id2 = 1 + (coords[0]+1)*(mpiProcZ+1)*(mpiProcY+1) + (coords[1]+1)*(mpiProcZ+1) + coords[2];

      int id8 = 1 + coords[0]*(mpiProcZ+1)*(mpiProcY+1) + coords[1]*(mpiProcZ+1) + (coords[2]+1);
      int id5 = 1 + (coords[0]+1)*(mpiProcZ+1)*(mpiProcY+1) + coords[1]*(mpiProcZ+1) + (coords[2]+1);
      int id7 = 1 + coords[0]*(mpiProcZ+1)*(mpiProcY+1) + (coords[1]+1)*(mpiProcZ+1) + (coords[2]+1);
      int id6 = 1 + (coords[0]+1)*(mpiProcZ+1)*(mpiProcY+1) + (coords[1]+1)*(mpiProcZ+1) + (coords[2]+1);

      ofs << std::setw(8) << id1 << std::setw(8) << id2 << std::setw(8) << id3 << std::setw(8) << id4 
	  << std::setw(8) << id5 << std::setw(8) << id6 << std::setw(8) << id7 << std::setw(8) << id8 << std::endl;
    }

    ofs.close();
  }


#ifndef BIGON
  void Assembly::findContact() { // various implementations
    int ompThreads = dem::Parameter::getSingleton().parameter["ompThreads"];

    if (ompThreads == 1) { // non-openmp single-thread version, time complexity bigO(n x n), n is the number of particles.  
      contactVec.clear();
    
#ifdef DEM_PROFILE
      REAL time_r = 0; // time consumed in contact resolution, i.e., tmpContact.isOverlapped()
      gettimeofday(&time_p1, NULL); 
#endif
    
      std::size_t num1 = particleVec.size();      // particles inside container
      std::size_t num2 = mergeParticleVec.size(); // paticles inside container (at front) + particles from neighboring blocks (at end)
      for (std::size_t i = 0; i < num1; ++i) {    // NOT (num1 - 1), in parallel situation where one particle could contact received particles!
	Vec u = particleVec[i]->getCurrPos();
	for (std::size_t j = i + 1; j < num2; ++j){
	  Vec v = mergeParticleVec[j]->getCurrPos();
	  if ( ( vfabs(v - u) < particleVec[i]->getA() + mergeParticleVec[j]->getA())
	       && ( particleVec[i]->getType() !=  1 || mergeParticleVec[j]->getType() != 1  )      // not both are fixed particles
	       && ( particleVec[i]->getType() !=  5 || mergeParticleVec[j]->getType() != 5  )      // not both are free boundary particles
	       && ( particleVec[i]->getType() != 10 || mergeParticleVec[j]->getType() != 10 )  ) { // not both are ghost particles
	    Contact tmpContact(particleVec[i], mergeParticleVec[j]); // a local and temparory object
#ifdef DEM_PROFILE
	    gettimeofday(&time_r1, NULL); 
#endif
	    if(tmpContact.isOverlapped())
	      contactVec.push_back(tmpContact);    // containers use value semantics, so a "copy" is pushed back.
#ifdef DEM_PROFILE
	    gettimeofday(&time_r2, NULL); 
	    time_r += timediffsec(time_r1, time_r2);
#endif
	  }
	}
      }	
    
#ifdef DEM_PROFILE
      gettimeofday(&time_p2, NULL);
      debugInf<< std::setw(OWID) << "findContact=" << std::setw(OWID) << timediffsec(time_p1, time_p2) << std::setw(OWID) << "isOverlapped=" << std::setw(OWID) << time_r; 
#endif

    }

    else if (ompThreads > 1) { // openmp implementation: various loop scheduling - (static), (static,1), (dynamic), (dynamic,1)
      contactVec.clear();
    
#ifdef DEM_PROFILE
      gettimeofday(&time_p1, NULL); 
#endif
    
      std::size_t i, j;
      Vec u, v;
      std::size_t num1 = particleVec.size();
      std::size_t num2 = mergeParticleVec.size();  
      int ompThreads = dem::Parameter::getSingleton().parameter["ompThreads"];
    
#pragma omp parallel for num_threads(ompThreads) private(i, j, u, v) shared(num1, num2) schedule(dynamic)
      for (i = 0; i < num1; ++i) { 
	u = particleVec[i]->getCurrPos();
	for (j = i + 1; j < num2; ++j) {
	  v = mergeParticleVec[j]->getCurrPos();
	  if ( ( vfabs(v - u) < particleVec[i]->getA() + mergeParticleVec[j]->getA() )
	       && ( particleVec[i]->getType() !=  1 || mergeParticleVec[j]->getType() != 1  )      // not both are fixed particles
	       && ( particleVec[i]->getType() !=  5 || mergeParticleVec[j]->getType() != 5  )      // not both are free boundary particles
	       && ( particleVec[i]->getType() != 10 || mergeParticleVec[j]->getType() != 10 )  ) { // not both are ghost particles
	    Contact tmpContact(particleVec[i], mergeParticleVec[j]); // a local and temparory object
	    if(tmpContact.isOverlapped())
#pragma omp critical
	      contactVec.push_back(tmpContact);    // containers use value semantics, so a "copy" is pushed back.
	  }
	}
      }
    
#ifdef DEM_PROFILE
      gettimeofday(&time_p2, NULL);
      debugInf << std::setw(OWID) << "findContact=" << std::setw(OWID) << timediffsec(time_p1, time_p2); 
#endif

    } // end of openmp implementation

  }

// else of #ifndef BIGON
#else
  void Assembly::findContact() { 
  // 1. Binning methods, cell slightly larger than maximum particle.
  // 2. To work with parallelization, mergeParticleVec and an expanded/virtual container must be used to account for 
  //    particles received from other processes.
  // 3. updateParticle() only updates particleVec, not mergeParticleVec. 
    contactVec.clear();
  
#ifdef DEM_PROFILE
    REAL time_r = 0;
    gettimeofday(&time_p1,NULL); 
#endif
    REAL cellSize = gradation.getPtclMaxRadius() * 2.0;
    // beware that a virtualContainer gets unnecessary halo layers for boundary processes in parallel computing,
    // but it does not affect numerical results.
    Rectangle virtualContainer(container.getMinCorner() - Vec(cellSize), container.getMaxCorner() + Vec(cellSize));
    int  nx = floor (virtualContainer.getDimx() / cellSize);
    int  ny = floor (virtualContainer.getDimy() / cellSize);
    int  nz = floor (virtualContainer.getDimz() / cellSize);
    REAL dx = virtualContainer.getDimx() / nx;
    REAL dy = virtualContainer.getDimy() / ny;
    REAL dz = virtualContainer.getDimz() / nz;
    Vec  minCorner = virtualContainer.getMinCorner();
    REAL x0 = minCorner.getX();
    REAL y0 = minCorner.getY();
    REAL z0 = minCorner.getZ();
    // debugInf << "nx ny nz dx dy dz=" << " " << nx << " " << ny << " " << nz << " " << dx << " " << dy << " " << dz << std::endl;
    
    // 26 neighbors of each cell
    int neighbor[26][3];
    int count = 0;
    for (int i = -1; i < 2; ++i)
      for (int j = -1; j < 2; ++j)
	for (int k = -1; k < 2; ++k) {
	  if (! (i == 0 && j == 0 && k == 0 ) ) {
	    neighbor[count][0] = i;
	    neighbor[count][1] = j;
	    neighbor[count][2] = k;
	    ++count;
	  }
	}
 
    // 4-dimensional array of cellVec
    typedef std::pair<bool, std::vector<Particle*> > cellT;
    std::vector< std::vector< std::vector < cellT > > > cellVec;
    cellVec.resize(nx);
    for (int i = 0; i < cellVec.size(); ++i) {
      cellVec[i].resize(ny);
      for (int j = 0; j < cellVec[i].size(); ++j)
	cellVec[i][j].resize(nz);
    }
    // mark each cell as not searched
    for (int i = 0; i < nx; ++i)
      for (int j = 0; j < ny; ++j)
	for (int k = 0; k < nz; ++k)
	  cellVec[i][j][k].first = false; // has not ever been searched

#ifdef DEM_PROFILE
    static struct timeval time_p11;
    gettimeofday(&time_p11,NULL); 
#endif
    // assign particles to each cell, this is O(n) algorithm
    for (int pt = 0; pt < mergeParticleVec.size(); ++pt) {
      Vec center = mergeParticleVec[pt]->getCurrPos();
      int i = int((center.getX()-x0) / dx); // must use int, NOT floor, because floor at left border may result in -1, e.g. floor(-1.0e-8)
      int j = int((center.getY()-y0) / dy);
      int k = int((center.getZ()-z0) / dz);
      if (i > nx - 1) i = nx - 1; // in case a particle is out of right border
      if (j > ny - 1) j = ny - 1;
      if (k > nz - 1) k = nz - 1;
      // the above constrains work better than things like (i > -1 && i < nx) because it does not ignore any particles.
      cellVec[i][j][k].second.push_back( mergeParticleVec[pt] );
    }
    /* ensure no particles are missed
    int totalPtcl = 0;
    for (int i = 0; i < nx; ++i)
      for (int j = 0; j < ny; ++j)
	for (int k = 0; k < nz; ++k) {
	  totalPtcl += cellVec[i][j][k].second.size();
	}
    debugInf << "particle: input = " << mergeParticleVec.size() << " assigned = " << totalPtcl << std::endl;
    */
#ifdef DEM_PROFILE
    static struct timeval time_p22;
    gettimeofday(&time_p22,NULL); 
#endif

    /* this is actually a low efficiency O(n^2) algorithm, not recommended.
    std::vector<bool> mergeParticleVecFlag(mergeParticleVec.size(), false);
    REAL x1, x2, y1, y2, z1, z2;
    for (int i = 0; i < nx; ++i)
      for (int j = 0; j < ny; ++j)
	for (int k = 0; k < nz; ++k) {
	  x1 = x0 + dx * i;
	  x2 = x0 + dx * (i + 1);
	  y1 = y0 + dy * j;
	  y2 = y0 + dy * (j + 1);
	  z1 = z0 + dz * k;
	  z2 = z0 + dz * (k + 1);
	  for (int pt = 0; pt < mergeParticleVec.size(); ++pt) {
	    Vec center = mergeParticleVec[pt]->getCurrPos();
	    if (!mergeParticleVecFlag[pt] &&
		center.getX() - x1 >= -EPS && center.getX() - x2 < -EPS &&
		center.getY() - y1 >= -EPS && center.getY() - y2 < -EPS &&
		center.getZ() - z1 >= -EPS && center.getZ() - z2 < -EPS) {
	      cellVec[i][j][k].second.push_back( mergeParticleVec[pt] );
	      mergeParticleVecFlag[pt] = true;
	    }
	  }
	}
    */

    // for each cell
    Particle *it, *pt;
    Vec u, v;
    for (int i = 0; i < nx; ++i)    // use [0,nx-1] instead of [1,nx-2] because the latter could miss internal particles, e.g., left border of cell[1]
      for (int j = 0; j < ny; ++j)  // does not coincide with the original compute grid due to the numerical fact that dx (and dy, dz) may not be equal to cellSize precisely.
	for (int k = 0; k < nz; ++k) {

	  // for particles inside the cell	  
	  for (int m = 0; m < cellVec[i][j][k].second.size(); ++m) {
	    it = cellVec[i][j][k].second[m];
	    u  = it->getCurrPos();
	  
	    // for particles inside the cell itself   
	    for (int n = m + 1; n < cellVec[i][j][k].second.size(); ++n) {
	      //debugInf <<  i << " " << j << " " << k << " " << "m n size=" << m << " " << n << " " <<  cellVec[i][j][k].size() << std::endl;
	      pt = cellVec[i][j][k].second[n];
	      v  = pt->getCurrPos();
	      if ( ( vfabs(u-v) < it->getA() + pt->getA() )  &&
		   ( it->getType() !=  1 || pt->getType() != 1 ) &&   // not both are fixed particles
		   ( it->getType() !=  5 || pt->getType() != 5 ) &&   // not both are free boundary particles
		   ( it->getType() != 10 || pt->getType() != 10)  ) { // not both are ghost particles
		Contact tmpContact(it, pt); // a local and temparory object
#ifdef DEM_PROFILE
		gettimeofday(&time_r1,NULL); 
#endif
		if(tmpContact.isOverlapped())
		  contactVec.push_back(tmpContact);   // containers use value semantics, so a "copy" is pushed back.
#ifdef DEM_PROFILE
		gettimeofday(&time_r2,NULL); 
		time_r += timediffsec(time_r1, time_r2);
#endif
	      }
	    }
	  
	    // for 26 neighboring cells
	    for (int ncell = 0; ncell < 26; ++ncell ) {
	      int ci = i + neighbor[ncell][0];
	      int cj = j + neighbor[ncell][1];
	      int ck = k + neighbor[ncell][2];
	      if (ci > -1 && ci < nx && cj > -1 && cj < ny && ck > -1 && ck < nz && cellVec[ci][cj][ck].first == false ) {
		//debugInf << "i j k m ncell ci cj ck size contacts= " << i << " " << j << " " << k << " " << m  << " " << ncell << " " << ci << " " << cj << " " << ck << " " << cellVec[ci][cj][ck].second.size() << " "  << contactVec.size() << std::endl;
		std::vector<Particle*> vt = cellVec[ci][cj][ck].second;
		for (int n = 0; n < vt.size(); ++n) {
		  pt = vt[n];
		  v  = pt->getCurrPos();
		  if ( ( vfabs(u-v) < it->getA() + pt->getA() )  &&
		       ( it->getType() !=  1 || pt->getType() != 1 ) &&   // not both are fixed particles
		       ( it->getType() !=  5 || pt->getType() != 5 ) &&   // not both are free boundary particles
		       ( it->getType() != 10 || pt->getType() != 10)  ) { // not both are ghost particles
		    Contact tmpContact(it, pt); // a local and temparory object
#ifdef DEM_PROFILE
		    gettimeofday(&time_r1,NULL); 
#endif
		    if(tmpContact.isOverlapped())
		      contactVec.push_back(tmpContact);   // containers use value semantics, so a "copy" is pushed back.
#ifdef DEM_PROFILE
		    gettimeofday(&time_r2,NULL); 
		    time_r += timediffsec(time_r1, time_r2);
#endif
		  
		  }
		}
	      }
	    }
	  }

	  cellVec[i][j][k].first = true; // searched, will not be searched again. Note this marks the cell itself, not the neighbors.
	}
  
#ifdef DEM_PROFILE
    gettimeofday(&time_p2,NULL);
    debugInf << std::setw(OWID) << "findContact=" << std::setw(OWID) << timediffsec(time_p1, time_p2)
	     << std::setw(OWID) << "initialize=" << std::setw(OWID) << timediffsec(time_p1, time_p11) << std::setw(OWID) << "assigning=" << std::setw(OWID) <<  timediffsec(time_p11, time_p22)
	     << std::setw(OWID) << "searching=" << std::setw(OWID) << timediffsec(time_p22, time_p2) - time_r  << std::setw(OWID) << "isOverlapped=" << std::setw(OWID) << time_r;
#endif
  }
#endif
// else of #ifndef BIGON

  void Assembly::internalForce(){
    REAL pAvg[3], sum[3];
    for (std::size_t i = 0; i < 3; ++i) {
      pAvg[i] = 0;
      sum[i]  = 0;
    }

    if(contactVec.size() > 0) {
      for (std::vector<Contact>::iterator it = contactVec.begin(); it != contactVec.end(); ++it)
	it->checkinPrevTgt(contactTgtVec); // checkin previous tangential force and displacment    
    
#ifdef DEM_PROFILE
      gettimeofday(&time_p1,NULL); 
#endif 

      contactTgtVec.clear(); // contactTgtVec must be cleared before filling in new values.
      for (std::vector<Contact>::iterator it = contactVec.begin(); it != contactVec.end(); ++ it){
	it->contactForce();             // cannot be parallelized as it may change a particle's force simultaneously.
	it->checkoutTgt(contactTgtVec); // checkout current tangential force and displacment
	pAvg[0] += it->getNormalForce();
	pAvg[1] += it->getTgtForce();
	pAvg[2] += it->getPenetration();
      }
      for (std::size_t i = 0; i < 3; ++i)
	pAvg[i] /= contactVec.size();
    
#ifdef DEM_PROFILE
      gettimeofday(&time_p2,NULL);
      debugInf << std::setw(OWID) << "internalForce=" << std::setw(OWID) << timediffsec(time_p1, time_p2); 
#endif   
    }

    MPI_Reduce(pAvg, sum, 3, MPI_DOUBLE, MPI_SUM, 0, mpiWorld);
    avgNormal = sum[0]/mpiSize;
    avgShear  = sum[1]/mpiSize;
    avgPenetr = sum[2]/mpiSize;
  }


  void Assembly::dragForce(){
    for(std::vector<Particle*>::iterator it = particleVec.begin(); it != particleVec.end(); ++it)
      (*it)->dragForce();
  }


  void Assembly::updateParticle() {
#ifdef DEM_PROFILE
      gettimeofday(&time_p1,NULL); 
#endif 

    for(std::vector<Particle*>::iterator it = particleVec.begin(); it != particleVec.end(); ++it)
      (*it)->update();

#ifdef DEM_PROFILE
      gettimeofday(&time_p2,NULL);
      debugInf << std::setw(OWID) << "updatePtcl=" << std::setw(OWID) << timediffsec(time_p1, time_p2) << std::endl; 
#endif 
  }


  void Assembly::updateBoundary(REAL sigma, std::string type, REAL sigmaX, REAL sigmaY) {
    if (mpiRank == 0) {
      REAL x1, x2, y1, y2, z1, z2;
      for(std::vector<Boundary*>::const_iterator it = mergeBoundaryVec.begin(); it != mergeBoundaryVec.end(); ++it) {
	switch ((*it)->getId()) {
	case 1: 
	  x1 = (*it)->getPoint().getX();
	  break;
	case 2:
	  x2 = (*it)->getPoint().getX();
	  break;
	case 3:
	  y1 = (*it)->getPoint().getY();
	  break;
	case 4:
	  y2 = (*it)->getPoint().getY();
	  break;
	case 5:
	  z1 = (*it)->getPoint().getZ();
	  break;
	case 6:
	  z2 = (*it)->getPoint().getZ();
	  break;
	}
      }
      REAL areaX = (y2 - y1) * (z2 - z1);
      REAL areaY = (z2 - z1) * (x2 - x1);
      REAL areaZ = (x2 - x1) * (y2 - y1);

      if (type.compare("isotropic") == 0) {
	for(std::vector<Boundary*>::iterator it = mergeBoundaryVec.begin(); it != mergeBoundaryVec.end(); ++it)
	  (*it)->updateIsotropic(sigma, areaX, areaY, areaZ);
      } else if (type.compare("oedometer") == 0) {
	for(std::vector<Boundary*>::iterator it = mergeBoundaryVec.begin(); it != mergeBoundaryVec.end(); ++it)
	  (*it)->updateOedometer(sigma, areaX, areaY, areaZ);
      } else if (type.compare("triaxial") == 0) {
	for(std::vector<Boundary*>::iterator it = mergeBoundaryVec.begin(); it != mergeBoundaryVec.end(); ++it)
	  (*it)->updateTriaxial(sigma, areaX, areaY, areaZ);
      } else if (type.compare("plnstrn") == 0) {
	for(std::vector<Boundary*>::iterator it = mergeBoundaryVec.begin(); it != mergeBoundaryVec.end(); ++it)
	  (*it)->updatePlaneStrain(sigma, areaX, areaY, areaZ);
      } else if (type.compare("trueTriaxial") == 0) {
	for(std::vector<Boundary*>::iterator it = mergeBoundaryVec.begin(); it != mergeBoundaryVec.end(); ++it)
	  (*it)->updateTrueTriaxial(sigma, areaX, areaY, areaZ, sigmaX, sigmaY);
      }

      // update boundaryVec from mergeBoundaryVec and remove contactInfo to reduce MPI transmission
      boundaryVec = mergeBoundaryVec; 
      for(std::vector<Boundary*>::iterator it = boundaryVec.begin(); it != boundaryVec.end(); ++it)
	(*it)->clearContactInfo();

      // update allContainer
      for(std::vector<Boundary*>::const_iterator it = boundaryVec.begin(); it != boundaryVec.end(); ++it) {
	switch ((*it)->getId()) {
	case 1: 
	  x1 = (*it)->getPoint().getX();
	  break;
	case 2:
	  x2 = (*it)->getPoint().getX();
	  break;
	case 3:
	  y1 = (*it)->getPoint().getY();
	  break;
	case 4:
	  y2 = (*it)->getPoint().getY();
	  break;
	case 5:
	  z1 = (*it)->getPoint().getZ();
	  break;
	case 6:
	  z2 = (*it)->getPoint().getZ();
	  break;
	}
      }
      setContainer(Rectangle(x1, y1, z1, x2, y2, z2));
    }

    broadcast(boostWorld, boundaryVec, 0);

  }


  void Assembly::clearContactForce() {
    for(std::vector<Particle*>::iterator it = particleVec.begin(); it != particleVec.end(); ++it)
      (*it)->clearContactForce();
  }


  void Assembly::findBdryContact() {
    for(std::vector<Boundary*>::iterator it = boundaryVec.begin(); it != boundaryVec.end(); ++it)
      (*it)->findBdryContact(particleVec);
  }


  void Assembly::boundaryForce() {
    for(std::vector<Boundary*>::iterator it = boundaryVec.begin(); it != boundaryVec.end(); ++it)
      (*it)->boundaryForce(boundaryTgtMap);
  }


  void Assembly::printContact(char *str) const
  {
    // There are two implementions of printContact
    // implementation 1: parallel IO, each process prints to a shared data file using a shared pointer.
    //                   and use post-processing tool to remove redundant info.
    MPI_Status status;
    MPI_File contactFile;
    MPI_File_open(mpiWorld, str, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &contactFile);
    if(boostWorld.rank() == 0 && !contactFile) { debugInf << "stream error: printContact" << std::endl; exit(-1);}

    std::stringstream inf;
    inf.setf(std::ios::scientific, std::ios::floatfield);

    for (std::vector<Contact>::const_iterator it = contactVec.begin(); it != contactVec.end(); ++it)
      inf << std::setw(OWID) << it->getP1()->getId()
	  << std::setw(OWID) << it->getP2()->getId()
	  << std::setw(OWID) << it->getPoint1().getX()
	  << std::setw(OWID) << it->getPoint1().getY()
	  << std::setw(OWID) << it->getPoint1().getZ()
	  << std::setw(OWID) << it->getPoint2().getX()
	  << std::setw(OWID) << it->getPoint2().getY()
	  << std::setw(OWID) << it->getPoint2().getZ()
	  << std::setw(OWID) << it->getRadius1()
	  << std::setw(OWID) << it->getRadius2()
	  << std::setw(OWID) << it->getPenetration()
	  << std::setw(OWID) << it->getTgtDisp()
	  << std::setw(OWID) << it->getContactRadius()
	  << std::setw(OWID) << it->getR0()
	  << std::setw(OWID) << it->getE0()
	  << std::setw(OWID) << it->getNormalForce()
	  << std::setw(OWID) << it->getTgtForce()
	  << std::setw(OWID) << -it->normalForceVec().getZ() + it->normalDampForceVec().getZ()
	  << std::setw(OWID) << ( it->getPoint1().getX() + it->getPoint2().getX() )/2
	  << std::setw(OWID) << ( it->getPoint1().getY() + it->getPoint2().getY() )/2
	  << std::setw(OWID) << ( it->getPoint1().getZ() + it->getPoint2().getZ() )/2
	  << std::setw(OWID) << it->normalForceVec().getX()
	  << std::setw(OWID) << it->normalForceVec().getY()
	  << std::setw(OWID) << it->normalForceVec().getZ()
	  << std::setw(OWID) << it->tgtForceVec().getX()
	  << std::setw(OWID) << it->tgtForceVec().getY()
	  << std::setw(OWID) << it->tgtForceVec().getZ()
	  << std::setw(OWID) << it->getVibraTimeStep()
	  << std::setw(OWID) << it->getImpactTimeStep()
	  << std::endl;

    int length = (OWID*29 + 1) *contactVec.size();
    // write a file at a location specified by a shared file pointer (blocking, collective)
    // note MPI_File_write_shared is non-collective
    MPI_File_write_ordered(contactFile, const_cast<char*> (inf.str().c_str()), length, MPI_CHAR, &status);
    MPI_File_close(&contactFile);

    // implementation 2: each process prints to an individual file.
    //                   use post-processing tool to merge files and remove redundance.
    /*
    char csuf[10];
    combineString(csuf, ".p", mpiRank, 5);
    strcat(str, csuf);

    std::ofstream ofs(str);
    if(!ofs) { debugInf << "stream error: printContact" << std::endl; exit(-1); }
    ofs.setf(std::ios::scientific, std::ios::floatfield);
    ofs.precision(OPREC);
  
    ofs << std::setw(OWID) << contactVec.size() << std::endl;
    ofs << std::setw(OWID) << "ptcl_1"
	<< std::setw(OWID) << "ptcl_2"
	<< std::setw(OWID) << "point1_x"
	<< std::setw(OWID) << "point1_y"
	<< std::setw(OWID) << "point1_z"
	<< std::setw(OWID) << "point2_x"
	<< std::setw(OWID) << "point2_y"
	<< std::setw(OWID) << "point2_z"
	<< std::setw(OWID) << "radius_1"
	<< std::setw(OWID) << "radius_2"
	<< std::setw(OWID) << "penetration"
	<< std::setw(OWID) << "tangt_disp"
	<< std::setw(OWID) << "contact_radius"
	<< std::setw(OWID) << "R0"
	<< std::setw(OWID) << "E0"
	<< std::setw(OWID) << "normal_force"
	<< std::setw(OWID) << "tangt_force"
	<< std::setw(OWID) << "normal_all_z"
	<< std::setw(OWID) << "contact_x"
	<< std::setw(OWID) << "contact_y"
	<< std::setw(OWID) << "contact_z"
	<< std::setw(OWID) << "normal_x"
	<< std::setw(OWID) << "normal_y"
	<< std::setw(OWID) << "normal_z"
	<< std::setw(OWID) << "tangt_x"
	<< std::setw(OWID) << "tangt_y"
	<< std::setw(OWID) << "tangt_z"
	<< std::setw(OWID) << "vibra_t_step"
	<< std::setw(OWID) << "impact_t_step"
	<< std::endl;
  
    std::vector<Contact>::const_iterator it;
    for (it = contactVec.begin(); it != contactVec.end(); ++it)
      ofs << std::setw(OWID) << it->getP1()->getId()
	  << std::setw(OWID) << it->getP2()->getId()
	  << std::setw(OWID) << it->getPoint1().getX()
	  << std::setw(OWID) << it->getPoint1().getY()
	  << std::setw(OWID) << it->getPoint1().getZ()
	  << std::setw(OWID) << it->getPoint2().getX()
	  << std::setw(OWID) << it->getPoint2().getY()
	  << std::setw(OWID) << it->getPoint2().getZ()
	  << std::setw(OWID) << it->getRadius1()
	  << std::setw(OWID) << it->getRadius2()
	  << std::setw(OWID) << it->getPenetration()
	  << std::setw(OWID) << it->getTgtDisp()
	  << std::setw(OWID) << it->getContactRadius()
	  << std::setw(OWID) << it->getR0()
	  << std::setw(OWID) << it->getE0()
	  << std::setw(OWID) << it->getNormalForce()
	  << std::setw(OWID) << it->getTgtForce()
	  << std::setw(OWID) << -it->normalForceVec().getZ() + it->normalDampForceVec().getZ()
	  << std::setw(OWID) << ( it->getPoint1().getX() + it->getPoint2().getX() )/2
	  << std::setw(OWID) << ( it->getPoint1().getY() + it->getPoint2().getY() )/2
	  << std::setw(OWID) << ( it->getPoint1().getZ() + it->getPoint2().getZ() )/2
	  << std::setw(OWID) << it->normalForceVec().getX()
	  << std::setw(OWID) << it->normalForceVec().getY()
	  << std::setw(OWID) << it->normalForceVec().getZ()
	  << std::setw(OWID) << it->tgtForceVec().getX()
	  << std::setw(OWID) << it->tgtForceVec().getY()
	  << std::setw(OWID) << it->tgtForceVec().getZ()
	  << std::setw(OWID) << it->getVibraTimeStep()
	  << std::setw(OWID) << it->getImpactTimeStep()
	  << std::endl;
    ofs.close();
    */
  }


  void Assembly::calcTransEnergy() {
    REAL pEngy = 0;
    std::vector<Particle*>::const_iterator it;
    for (it = particleVec.begin(); it != particleVec.end(); ++it) {
      if ((*it)->getType() == 0)
	pEngy += (*it)->getTransEnergy();
    }
    MPI_Reduce(&pEngy, &transEnergy, 1, MPI_DOUBLE, MPI_SUM, 0,  mpiWorld);
  }


  void Assembly::calcRotatEnergy() {
    REAL pEngy = 0;
    std::vector<Particle*>::const_iterator it;
    for (it = particleVec.begin(); it != particleVec.end(); ++it) {
      if ((*it)->getType() == 0)
	pEngy += (*it)->getRotatEnergy();
    }
    MPI_Reduce(&pEngy, &rotatEnergy, 1, MPI_DOUBLE, MPI_SUM, 0, mpiWorld);
  }


  void Assembly::calcKinetEnergy() {
    REAL pEngy = 0;
    std::vector<Particle*>::const_iterator it;
    for (it = particleVec.begin();it != particleVec.end(); ++it) {
      if ((*it)->getType() == 0)
	pEngy += (*it)->getKinetEnergy();
    }
    MPI_Reduce(&pEngy, &kinetEnergy, 1, MPI_DOUBLE, MPI_SUM, 0, mpiWorld);
  }


  void Assembly::calcGraviEnergy(REAL ref) {
    REAL pEngy = 0;
    std::vector<Particle*>::const_iterator it;
    for (it = particleVec.begin();it != particleVec.end(); ++it) {
      if ((*it)->getType() == 0)
	pEngy += (*it)->getPotenEnergy(ref);
    }
    MPI_Reduce(&pEngy, &graviEnergy, 1, MPI_DOUBLE, MPI_SUM, 0, mpiWorld);
  }


  void Assembly::calcMechaEnergy() {
    mechaEnergy = kinetEnergy +  graviEnergy;
  }


  REAL Assembly::getMass() const {
    REAL var = 0;
    for (std::vector<Particle*>::const_iterator it = allParticleVec.begin(); it != allParticleVec.end(); ++it)
      var += (*it)->getMass();
    return var;
  }


  REAL Assembly::getParticleVolume() const {
    REAL var = 0;
    for (std::vector<Particle*>::const_iterator it = allParticleVec.begin(); it != allParticleVec.end(); ++it)
      if ((*it)->getType() == 0)
	var += (*it)->getVolume();
    return var;
  }


  void Assembly::calcTimeStep() {
    calcVibraTimeStep();
    calcImpactTimeStep();
    calcContactNum();

    std::valarray<REAL> dt(3);
    dt[0] = dem::Parameter::getSingleton().parameter["timeStep"];
    dt[1] = vibraTimeStep;
    dt[2] = impactTimeStep;

    timeStep = dt.min();
  }


  void Assembly::calcContactNum() {
    std::size_t pContactNum = contactVec.size();
    MPI_Reduce(&pContactNum, &allContactNum, 1, MPI_INT, MPI_SUM, 0, mpiWorld);
  }


  void Assembly::calcVibraTimeStep() {
    REAL pTimeStep = 1/EPS;
    if (contactVec.size() == 0)
      pTimeStep = 1/EPS;
    else {
      std::vector<Contact>::const_iterator it = contactVec.begin();
      pTimeStep = it->getVibraTimeStep();
      for (++it; it != contactVec.end(); ++it) {
	REAL val = it->getVibraTimeStep(); 
	pTimeStep = val < pTimeStep ? val : pTimeStep;
      }
    }

    MPI_Allreduce(&pTimeStep, &vibraTimeStep, 1, MPI_DOUBLE, MPI_MIN, mpiWorld);
  }


  void Assembly::calcImpactTimeStep() {
    REAL pTimeStep = 1/EPS;
    if (contactVec.size() == 0)
      pTimeStep = 1/EPS;
    else {
      std::vector<Contact>::const_iterator it = contactVec.begin();
      pTimeStep = it->getImpactTimeStep();
      for (++it; it != contactVec.end(); ++it) {
	REAL val = it->getImpactTimeStep(); 
	pTimeStep = val < pTimeStep ? val : pTimeStep;
      }
    }

    MPI_Allreduce(&pTimeStep, &impactTimeStep, 1, MPI_DOUBLE, MPI_MIN, mpiWorld);
  }


  REAL Assembly::getAvgTransVelocity() const {
    REAL avgv = 0;
    std::size_t count = 0;
    std::vector<Particle*>::const_iterator it;
    for (it = particleVec.begin(); it != particleVec.end(); ++it)
      if ((*it)->getType() == 0) {
	avgv += vfabs((*it)->getCurrVeloc());
	++count;
      }
    return avgv /= count;
  }


  REAL Assembly::getAvgRotatVelocity() const {
    REAL avgv = 0;
    std::size_t count = 0;
    std::vector<Particle*>::const_iterator it;
    for (it = particleVec.begin(); it != particleVec.end(); ++it)
      if ((*it)->getType() == 0) {
	avgv += vfabs((*it)->getCurrOmga());
	++count;
      }
    return avgv /= count;
  }


  REAL Assembly::getAvgForce() const {
    REAL avgv = 0;
    std::size_t count = 0;
    std::vector<Particle*>::const_iterator it;
    for (it = particleVec.begin(); it != particleVec.end(); ++it)
      if ((*it)->getType() == 0) {
	avgv += vfabs((*it)->getForce());
	++count;
      }
    return avgv/count;
  }


  REAL Assembly::getAvgMoment() const {
    REAL avgv = 0;
    std::size_t count = 0;
    std::vector<Particle*>::const_iterator it;
    for (it = particleVec.begin();it != particleVec.end(); ++it)
      if ((*it)->getType() == 0) {
	avgv += vfabs((*it)->getMoment());
	++count;
      }
    return avgv /= count;
  }


  void Assembly::buildBoundary(std::size_t boundaryNum,
			       const char *boundaryFile)
  {
    std::ofstream ofs(boundaryFile);
    if(!ofs) { debugInf << "stream error: buildBoundary" << std::endl; exit(-1);}
    ofs.setf(std::ios::scientific, std::ios::floatfield);

    Vec  v1 = allContainer.getMinCorner();
    Vec  v2 = allContainer.getMaxCorner();
    Vec  v0 = allContainer.getCenter();
    REAL x1 = v1.getX();
    REAL y1 = v1.getY();
    REAL z1 = v1.getZ();
    REAL x2 = v2.getX();
    REAL y2 = v2.getY();
    REAL z2 = v2.getZ();
    REAL x0 = v0.getX();
    REAL y0 = v0.getY();
    REAL z0 = v0.getZ();

    ofs << std::setw(OWID) << x1 << std::setw(OWID) << y1 << std::setw(OWID) << z1
	<< std::setw(OWID) << x2 << std::setw(OWID) << y2 << std::setw(OWID) << z2 << std::endl << std::endl
	<< std::setw(OWID) << boundaryNum << std::endl << std::endl;
  
    if (boundaryNum == 1) {   // only a bottom boundary, i.e., boundary 5
      ofs << std::setw(OWID) << 1
	  << std::setw(OWID) << 0 << std::endl
      
	  << std::setw(OWID) << 5
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << -1
	  << std::setw(OWID) << x0
	  << std::setw(OWID) << y0
	  << std::setw(OWID) << z1 << std::endl << std::endl;
    
    }
    else if (boundaryNum == 5) { // no top boundary, i.e., no boundary 6
      // boundary 1
      ofs << std::setw(OWID) << 1
	  << std::setw(OWID) << 1 << std::endl
      
	  << std::setw(OWID) << 1
	  << std::setw(OWID) << -1
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << 0 
	  << std::setw(OWID) << x1
	  << std::setw(OWID) << y0
	  << std::setw(OWID) << z0 << std::endl

	  << std::setw(OWID) << " "
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << 1 
	  << std::setw(OWID) << x0
	  << std::setw(OWID) << y0
	  << std::setw(OWID) << z2 << std::endl << std::endl
      
	// boundary 2
	  << std::setw(OWID) << 1
	  << std::setw(OWID) << 1 << std::endl

	  << std::setw(OWID) << 2
	  << std::setw(OWID) << 1
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << 0 
	  << std::setw(OWID) << x2     
	  << std::setw(OWID) << y0
	  << std::setw(OWID) << z0 << std::endl

	  << std::setw(OWID) << " "
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << 1 
	  << std::setw(OWID) << x0
	  << std::setw(OWID) << y0
	  << std::setw(OWID) << z2 << std::endl << std::endl
      
	// boundary 3
	  << std::setw(OWID) << 1
	  << std::setw(OWID) << 1 << std::endl

	  << std::setw(OWID) << 3
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << -1
	  << std::setw(OWID) << 0 
	  << std::setw(OWID) << x0
	  << std::setw(OWID) << y1
	  << std::setw(OWID) << z0 << std::endl

	  << std::setw(OWID) << " "
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << 1 
	  << std::setw(OWID) << x0
	  << std::setw(OWID) << y0
	  << std::setw(OWID) << z2 << std::endl << std::endl
      
	// boundary 4
	  << std::setw(OWID) << 1
	  << std::setw(OWID) << 1 << std::endl

	  << std::setw(OWID) << 4
	  << std::setw(OWID) << 0 
	  << std::setw(OWID) << 1
	  << std::setw(OWID) << 0 
	  << std::setw(OWID) << x0      
	  << std::setw(OWID) << y2
	  << std::setw(OWID) << z0 << std::endl

	  << std::setw(OWID) << " "
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << 1 
	  << std::setw(OWID) << x0
	  << std::setw(OWID) << y0
	  << std::setw(OWID) << z2 << std::endl << std::endl
      
	// boundary 5
	  << std::setw(OWID) << 1
	  << std::setw(OWID) << 0 << std::endl

	  << std::setw(OWID) << 5
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << -1
	  << std::setw(OWID) << x0
	  << std::setw(OWID) << y0
	  << std::setw(OWID) << z1 << std::endl << std::endl;
    }
    else if (boundaryNum == 6) { // all 6 boundaries
      // boundary 1
      ofs << std::setw(OWID) << 1
	  << std::setw(OWID) << 0 << std::endl

	  << std::setw(OWID) << 1      
	  << std::setw(OWID) << -1
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << 0 
	  << std::setw(OWID) << x1
	  << std::setw(OWID) << y0
	  << std::setw(OWID) << z0 << std::endl << std::endl

	// boundary 2
	  << std::setw(OWID) << 1
	  << std::setw(OWID) << 0 << std::endl

	  << std::setw(OWID) << 2
	  << std::setw(OWID) << 1
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << 0 
	  << std::setw(OWID) << x2     
	  << std::setw(OWID) << y0
	  << std::setw(OWID) << z0 << std::endl << std::endl
      
	// boundary 3
	  << std::setw(OWID) << 1
	  << std::setw(OWID) << 0 << std::endl

	  << std::setw(OWID) << 3
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << -1
	  << std::setw(OWID) << 0 
	  << std::setw(OWID) << x0
	  << std::setw(OWID) << y1
	  << std::setw(OWID) << z0 << std::endl << std::endl
      
	// boundary 4
	  << std::setw(OWID) << 1
	  << std::setw(OWID) << 0 << std::endl

	  << std::setw(OWID) << 4
	  << std::setw(OWID) << 0 
	  << std::setw(OWID) << 1
	  << std::setw(OWID) << 0 
	  << std::setw(OWID) << x0      
	  << std::setw(OWID) << y2
	  << std::setw(OWID) << z0 << std::endl << std::endl
      
	// boundary 5
	  << std::setw(OWID) << 1
	  << std::setw(OWID) << 0 << std::endl

	  << std::setw(OWID) << 5
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << -1
	  << std::setw(OWID) << x0
	  << std::setw(OWID) << y0
	  << std::setw(OWID) << z1 << std::endl << std::endl
      
	// boundary 6
	  << std::setw(OWID) << 1
	  << std::setw(OWID) << 0 << std::endl

	  << std::setw(OWID) << 6
	  << std::setw(OWID) << 0 
	  << std::setw(OWID) << 0
	  << std::setw(OWID) << 1    
	  << std::setw(OWID) << x0      
	  << std::setw(OWID) << y0
	  << std::setw(OWID) << z2 << std::endl << std::endl;
    }
  
    ofs.close();
  }

} // namespace dem ends

/*
// create a specimen from discreate particles through floating and then gravitation,
// boundaries are composed of fixed particles.
void Assembly::deposit_PtclBdry(gradation& grad,
int   particleLayers,
REAL rsize,
int   totalSteps,  
int   snapNum,
int   interval,
const char *iniptclfile,   
const char *ParticleFile, 
const char *contactfile,
const char *progressfile, 
const char *debugfile)
{
if (grad.rorc == 1) {
RORC = grad.rorc;
container.setCenter(Vec(0,0,0));
container.setDimx(grad.dimn);
container.setDimy(grad.dimn);
container.setDimz(grad.dimn);
	
generate_p(grad, iniptclfile, particleLayers, rsize, 4.0);
deposit_p(totalSteps,        // totalSteps
snapNum,          // number of snapNum
interval,           // print interval
grad.dimn,          // dimension of particle-composed-boundary
rsize,              // relative container size
iniptclfile,        // input file, initial particles
ParticleFile,       // output file, resulted particles, including snapNum 
contactfile,        // output file, resulted contacts, including snapNum 
progressfile,       // output file, statistical info
debugfile);         // output file, debug info
}
}
*/

 /*
 // particleLayers:
 // 0 - one free particle
 // 1 - a horizontal layer of free particles
 // 2 - multiple layers of free particles
 // ht- how many times of size would be the floating height
 void Assembly::generate_p(gradation&  grad,
 const char *ParticleFile,
 int particleLayers,
 REAL rsize,
 REAL ht)
 {
 REAL x,y,z;
 Particle* newptcl;
 particleNum = 0;
 REAL wall=2.2; // wall - wall height; ht - free particle height
 REAL est =1.02;
 int grid=static_cast<int> (nearbyint(rsize*10)-1);  

 // grid: dimension of free particle array.
 // 7 - small dimn container
 // 9 - medium dimn container 
 // 11- large dimn container 

 REAL dimn=grad.dimn;
 // particle boundary 1
 x=dimn/2*(grid+1)/10;
 for (y=-dimn/2*grid/10; y<dimn/2*grid/10*est; y+=dimn/2/5)
 for(z=-dimn/2; z<-dimn/2 + dimn*wall; z+=dimn/2/5) {
 newptcl = new Particle(particleNum+1, 1, Vec(x,y,z), grad.ptclsize[0]*0.99, young, poisson);
 particleVec.push_back(newptcl);
 ++particleNum;
 }

 // particle boundary 2
 y=dimn/2*(grid+1)/10;
 for (x=-dimn/2*grid/10; x<dimn/2*grid/10*est; x+=dimn/2/5)
 for(z=-dimn/2; z<-dimn/2 + dimn*wall; z+=dimn/2/5) {
 newptcl = new Particle(particleNum+1, 1, Vec(x,y,z), grad.ptclsize[0]*0.99, young, poisson);
 particleVec.push_back(newptcl);
 ++particleNum;
 }

 // particle boundary 3
 x=-dimn/2*(grid+1)/10;
 for (y=-dimn/2*grid/10; y<dimn/2*grid/10*est; y+=dimn/2/5)
 for(z=-dimn/2; z<-dimn/2 + dimn*wall; z+=dimn/2/5) {
 newptcl = new Particle(particleNum+1, 1, Vec(x,y,z), grad.ptclsize[0]*0.99, young, poisson);
 particleVec.push_back(newptcl);
 ++particleNum;
 }

 // particle boundary 4
 y=-dimn/2*(grid+1)/10;
 for (x=-dimn/2*grid/10; x<dimn/2*grid/10*est; x+=dimn/2/5)
 for(z=-dimn/2; z<-dimn/2 + dimn*wall; z+=dimn/2/5) {
 newptcl = new Particle(particleNum+1, 1, Vec(x,y,z), grad.ptclsize[0]*0.99, young, poisson);
 particleVec.push_back(newptcl);
 ++particleNum;
 }

 // particle boundary 6
 z=-dimn/2;
 for (y=-dimn/2*grid/10; y<dimn/2*grid/10*est; y+=dimn/2/5)
 for( x=-dimn/2*grid/10; x<dimn/2*grid/10*est; x+=dimn/2/5) {
 newptcl = new Particle(particleNum+1, 1, Vec(x,y,z), grad.ptclsize[0]*0.99, young, poisson);
 particleVec.push_back(newptcl);
 ++particleNum;
 }

 if (particleLayers == 0) {      // just one free particle
 newptcl = new Particle(particleNum+1, 0, Vec(dimn/2/40,dimn/2/20,dimn/2), grad, young, poisson);
 particleVec.push_back(newptcl);
 ++particleNum;
 }
 else if (particleLayers == 1) { // a horizontal layer of free particles
 z=dimn/2;
 for (x=-dimn/2*(grid-1)/10; x<dimn/2*(grid-1)/10*est; x+=dimn/2/5)
 for (y=-dimn/2*(grid-1)/10; y<dimn/2*(grid-1)/10*est; y+=dimn/2/5) {
 newptcl = new Particle(particleNum+1, 0, Vec(x,y,z), grad, young, poisson);
 particleVec.push_back(newptcl);
 ++particleNum;
 }
 }
 else if (particleLayers == 2) { // multiple layers of free particles
 for (z=dimn/2; z<dimn/2 + dimn*ht; z+=dimn/2/5)
 for (x=-dimn/2*(grid-1)/10; x<dimn/2*(grid-1)/10*est; x+=dimn/2/5)
 for (y=-dimn/2*(grid-1)/10; y<dimn/2*(grid-1)/10*est; y+=dimn/2/5) {
 newptcl = new Particle(particleNum+1, 0, Vec(x,y,z), grad, young, poisson);
 particleVec.push_back(newptcl);
 ++particleNum;
 }	
 }
    
 printParticle(ParticleFile);
    
 }
 */

 // rule out
 /*
   void Assembly::plotCavity(const char *str) const {
   std::ofstream ofs(str);
   if(!ofs) { debugInf << "stream error: plotCavity" << std::endl; exit(-1); }
   ofs.setf(std::ios::scientific, std::ios::floatfield);
   ofs.precision(OPREC);

   REAL x1,x2,y1,y2,z1,z2,x0,y0,z0;
   x1 = cavity.getMinCorner().getX();
   y1 = cavity.getMinCorner().getY();
   z1 = cavity.getMinCorner().getZ();
   x2 = cavity.getMaxCorner().getX();
   y2 = cavity.getMaxCorner().getY();
   z2 = cavity.getMaxCorner().getZ();

   ofs << "ZONE N=8, E=1, DATAPACKING=POINT, ZONETYPE=FEBRICK" << std::endl;
   ofs << std::setw(OWID) << x2 << std::setw(OWID) << y1 << std::setw(OWID) << z1 << std::endl;
   ofs << std::setw(OWID) << x2 << std::setw(OWID) << y2 << std::setw(OWID) << z1 << std::endl;
   ofs << std::setw(OWID) << x1 << std::setw(OWID) << y2 << std::setw(OWID) << z1 << std::endl;
   ofs << std::setw(OWID) << x1 << std::setw(OWID) << y1 << std::setw(OWID) << z1 << std::endl;
   ofs << std::setw(OWID) << x2 << std::setw(OWID) << y1 << std::setw(OWID) << z2 << std::endl;
   ofs << std::setw(OWID) << x2 << std::setw(OWID) << y2 << std::setw(OWID) << z2 << std::endl;
   ofs << std::setw(OWID) << x1 << std::setw(OWID) << y2 << std::setw(OWID) << z2 << std::endl;
   ofs << std::setw(OWID) << x1 << std::setw(OWID) << y1 << std::setw(OWID) << z2 << std::endl;
   ofs << "1 2 3 4 5 6 7 8" << std::endl;

   ofs.close();
   }

   void Assembly::plotSpring(const char *str) const {
   std::ofstream ofs(str);
   if(!ofs) { debugInf << "stream error: plotSpring" << std::endl; exit(-1); }
   ofs.setf(std::ios::scientific, std::ios::floatfield);
   ofs.precision(OPREC);

   std::size_t totalMemParticle = 0;
   for (std::size_t i = 0; i < memBoundary.size(); ++i) 
   for (std::size_t j = 0; j < memBoundary[i].size(); ++j) 
   for (std::size_t k = 0; k < memBoundary[i][j].size(); ++k) 
   ++totalMemParticle;
   std::size_t totalSpring = springVec.size();
   ofs << "ZONE N=" << totalMemParticle << ", E=" << totalSpring << ", DATAPACKING=POINT, ZONETYPE=FELINESEG" << std::endl;
   Particle *pt = NULL;
   Vec vt;
   for (std::size_t i = 0; i < memBoundary.size(); ++i) 
   for (std::size_t j = 0; j < memBoundary[i].size(); ++j) 
   for (std::size_t k = 0; k < memBoundary[i][j].size(); ++k) {
   pt = memBoundary[i][j][k]; 
   vt = pt->getCurrPos();
   ofs << std::setw(OWID) << vt.getX() << std::setw(OWID) << vt.getY() << std::setw(OWID) << vt.getZ() << std::endl;
   }
   for (std::size_t i = 0; i < springVec.size(); ++i) {
   ofs << std::setw(OWID) << springVec[i]->getParticleId1() - trimHistoryNum  << std::setw(OWID) << springVec[i]->getParticleId2() - trimHistoryNum << std::endl;
   }

   ofs.close();
   }  

   void Assembly::printMemParticle(const char *str) const  {
   std::ofstream ofs(str);
   if(!ofs) { debugInf << "stream error: printMemParticle" << std::endl; exit(-1); }
   ofs.setf(std::ios::scientific, std::ios::floatfield);
   ofs.precision(OPREC);
  
   std::size_t totalMemParticle = 0;
   for (std::size_t i = 0; i < memBoundary.size(); ++i) 
   for (std::size_t j = 0; j < memBoundary[i].size(); ++j) 
   for (std::size_t k = 0; k < memBoundary[i][j].size(); ++k) 
   ++totalMemParticle;
  
   ofs << std::setw(OWID) << totalMemParticle << std::setw(OWID) << 1 << std::endl;
   ofs << std::setw(OWID) << container.getCenter().getX()
   << std::setw(OWID) << container.getCenter().getY()
   << std::setw(OWID) << container.getCenter().getZ()
   << std::setw(OWID) << container.getDimx()
   << std::setw(OWID) << container.getDimy()
   << std::setw(OWID) << container.getDimz() << std::endl;
  
   ofs << std::setw(OWID) << "ID"
   << std::setw(OWID) << "type"
   << std::setw(OWID) << "radius_a"
   << std::setw(OWID) << "radius_b"
   << std::setw(OWID) << "radius_c"
   << std::setw(OWID) << "position_x"
   << std::setw(OWID) << "position_y"
   << std::setw(OWID) << "position_z"
   << std::setw(OWID) << "axis_a_x"
   << std::setw(OWID) << "axis_a_y"
   << std::setw(OWID) << "axis_a_z"
   << std::setw(OWID) << "axis_b_x"
   << std::setw(OWID) << "axis_b_y"
   << std::setw(OWID) << "axis_b_z"
   << std::setw(OWID) << "axis_c_x"
   << std::setw(OWID) << "axis_c_y"
   << std::setw(OWID) << "axis_c_z"
   << std::setw(OWID) << "velocity_x"
   << std::setw(OWID) << "velocity_y"
   << std::setw(OWID) << "velocity_z"
   << std::setw(OWID) << "omga_x"
   << std::setw(OWID) << "omga_y"
   << std::setw(OWID) << "omga_z"
   << std::setw(OWID) << "force_x"
   << std::setw(OWID) << "force_y"
   << std::setw(OWID) << "force_z"
   << std::setw(OWID) << "moment_x"
   << std::setw(OWID) << "moment_y"
   << std::setw(OWID) << "moment_z"
   << std::endl;
  
   Particle *it = NULL;
   Vec vObj;
   for (std::size_t i = 0; i < memBoundary.size(); ++i) 
   for (std::size_t j = 0; j < memBoundary[i].size(); ++j) 
   for (std::size_t k = 0; k < memBoundary[i][j].size(); ++k) {
   it = memBoundary[i][j][k];
   ofs << std::setw(OWID) << it->getId()
   << std::setw(OWID) << it->getType()
   << std::setw(OWID) << it->getA()
   << std::setw(OWID) << it->getB()
   << std::setw(OWID) << it->getC();
	
   vObj=it->getCurrPos();
   ofs << std::setw(OWID) << vObj.getX()
   << std::setw(OWID) << vObj.getY()
   << std::setw(OWID) << vObj.getZ();
	
   vObj=it->getCurrDirecA();
   ofs << std::setw(OWID) << vObj.getX()
   << std::setw(OWID) << vObj.getY()
   << std::setw(OWID) << vObj.getZ();
	
   vObj=it->getCurrDirecB();
   ofs << std::setw(OWID) << vObj.getX()
   << std::setw(OWID) << vObj.getY()
   << std::setw(OWID) << vObj.getZ();
	
   vObj=it->getCurrDirecC();
   ofs << std::setw(OWID) << vObj.getX()
   << std::setw(OWID) << vObj.getY()
   << std::setw(OWID) << vObj.getZ();
	
   vObj=it->getCurrVeloc();
   ofs << std::setw(OWID) << vObj.getX()
   << std::setw(OWID) << vObj.getY()
   << std::setw(OWID) << vObj.getZ();
	
   vObj=it->getCurrOmga();
   ofs << std::setw(OWID) << vObj.getX()
   << std::setw(OWID) << vObj.getY()
   << std::setw(OWID) << vObj.getZ();
	
   vObj=it->getForce();
   ofs << std::setw(OWID) << vObj.getX()
   << std::setw(OWID) << vObj.getY()
   << std::setw(OWID) << vObj.getZ();
	
   vObj=it->getMoment();
   ofs << std::setw(OWID) << vObj.getX()
   << std::setw(OWID) << vObj.getY()
   << std::setw(OWID) << vObj.getZ() << std::endl;
   }
   ofs.close();  
   }
 
   // vector elements are in the order of:
   // x1: inner, outer
   // x2: inner, outer
   // y1: inner, outer
   // y2: inner, outer
   // z1: inner, outer
   // z2: inner, outer
   void Assembly::checkMembrane(vector<REAL> &vx ) const {
   std::vector<Particle*> vec1d;  // 1-dimension
   std::vector< std::vector<Particle*>  > vec2d; // 2-dimension
   REAL in, out, tmp;
   REAL x1_in, x1_out, x2_in, x2_out;
   REAL y1_in, y1_out, y2_in, y2_out;
   REAL z1_in, z1_out, z2_in, z2_out;

   // surface x1
   vec2d = memBoundary[0];
   in = vec2d[0][0]->getCurrPos().getX();
   out= in;
   for (std::size_t i = 0; i < vec2d.size(); ++i)
   for (std::size_t j = 0; j < vec2d[i].size(); ++j) {
   tmp = vec2d[i][j]->getCurrPos().getX();
   if (tmp < out) out = tmp;
   if (tmp > in ) in  = tmp;
   }
   vx.push_back(in);
   vx.push_back(out);
   x1_in  = in;
   x1_out = out;

   // surface x2
   vec2d.clear();
   vec2d = memBoundary[1];
   in = vec2d[0][0]->getCurrPos().getX();
   out= in;
   for (std::size_t i = 0; i < vec2d.size(); ++i)
   for (std::size_t j = 0; j < vec2d[i].size(); ++j) {
   tmp = vec2d[i][j]->getCurrPos().getX();
   if (tmp > out) out = tmp;
   if (tmp < in ) in  = tmp;
   }
   vx.push_back(in);
   vx.push_back(out);
   x2_in  = in;
   x2_out = out;

   // surface y1
   vec2d.clear();
   vec2d = memBoundary[2];
   in = vec2d[0][0]->getCurrPos().getY();
   out= in;
   for (std::size_t i = 0; i < vec2d.size(); ++i)
   for (std::size_t j = 0; j < vec2d[i].size(); ++j) {
   tmp = vec2d[i][j]->getCurrPos().getY();
   if (tmp < out) out = tmp;
   if (tmp > in ) in  = tmp;
   }
   vx.push_back(in);
   vx.push_back(out);
   y1_in  = in;
   y1_out = out;

   // surface y2
   vec2d.clear();
   vec2d = memBoundary[3];
   in = vec2d[0][0]->getCurrPos().getY();
   out= in;
   for (std::size_t i = 0; i < vec2d.size(); ++i)
   for (std::size_t j = 0; j < vec2d[i].size(); ++j) {
   tmp = vec2d[i][j]->getCurrPos().getY();
   if (tmp > out) out = tmp;
   if (tmp < in ) in  = tmp;
   }
   vx.push_back(in);
   vx.push_back(out);
   y2_in  = in;
   y2_out = out;
  
   // surface z1
   vec2d.clear();
   vec2d = memBoundary[4];
   in = vec2d[0][0]->getCurrPos().getZ();
   out= in;
   for (std::size_t i = 0; i < vec2d.size(); ++i)
   for (std::size_t j = 0; j < vec2d[i].size(); ++j) {
   tmp = vec2d[i][j]->getCurrPos().getZ();
   if (tmp < out) out = tmp;
   if (tmp > in ) in  = tmp;
   }
   vx.push_back(in);
   vx.push_back(out);
   z1_in  = in;
   z1_out = out;

   // surface z2
   vec2d.clear();
   vec2d = memBoundary[5];
   in = vec2d[0][0]->getCurrPos().getZ();
   out= in;
   for (std::size_t i = 0; i < vec2d.size(); ++i)
   for (std::size_t j = 0; j < vec2d[i].size(); ++j) {
   tmp = vec2d[i][j]->getCurrPos().getZ();
   if (tmp > out) out = tmp;
   if (tmp < in ) in  = tmp;
   }
   vx.push_back(in);
   vx.push_back(out);
   z2_in  = in;
   z2_out = out;

   }
  
   //  1. it is important and helpful to mark a member function as const
   //     if it does NOT change member data.
   //  2. when a constant member function traverses member data, it can
   //     NOT change the data.
   //  3. then if it traverses a member data of a list, it should use a
   //     const_iterator, otherwise compiler will give errors.
   //  4. a const_iterator such as it also guarantees that (*it) will NOT
   //     change any data. if (*it) call a modification function, the 
   //     compiler will give errors.

   // OPENMP_IMPL: 
   // 0: implementation 0, ts partitions, based on linked list
   // 1: implementation 1, ts partitions, based on vector
   // 2: implementation 2, no partition, each thread leaps by ts until completed
   // 3: implementation 3, no partition, each thread leaps by ts until num/2 and handles two particles.
   // 4: implementation 4, no partition, parallel for, various loop scheduling: (static), (static,1), (dynamic), (dynamic,1)

   //start of def OPENMP 
   #ifdef OPENMP	

   #if OPENMP_IMPL == 0
   // OpenMP implementation 0: ts partitions, each thread handles a partition, max diff = n*n*(1-1/ts)/ts
   // implementation is based on linked list, also works for vector but not efficient.
   void Assembly::findContact() { 
   contactVec.clear();
   int possContact = 0;
  
   #ifdef DEM_PROFILE
   gettimeofday(&time_p1,NULL); 
   #endif

   int tid;   // thread id
   int ts;    // number of threads
   int num;   // number of particles
   int tnum;  // number of particles per thread
   int i, j;
   Vec u, v;
   num = particleVec.size();
   ot = particleVec.begin();
   std::vector<Particle*>::iterator ot, it, pt;
  
   #pragma omp parallel num_threads(nThreads) private(tid, ts, tnum, it, pt, i, j, u, v) shared(num) reduction(+: possContact)
   {
   tid = omp_get_thread_num();
   ts  = omp_get_num_threads();
   tnum = num / ts;  // divide itso ts partitions
   rnum = num % ts;  // remainder of the division
   it = ot;          // start particle of each thread
    
   // determine starting point and extend of each partition
   // this algorithm applies to both list and vector
   if (rnum == 0) {
   for (i = 0; i < tid * tnum; ++i)
   ++it;         // starting point of each partition
   }
   else {
   if (tid < rnum) {
   tnum += 1;    // tnum changed
   for (i = 0; i < tid * tnum ; ++i)
   ++it;
   }
   else {
   for (i = 0; i < rnum * (tnum + 1) + (tid - rnum) * tnum; ++ i)
   ++it;
   }
   }
    
   // explore each partition
   for (j = 0 ; j < tnum; ++j, ++it) { 
   u=(*it)->getCurrPos();
   for (pt = it, ++pt; pt != particleVec.end(); ++pt) {
   v=(*pt)->getCurrPos();
   if (   ( vfabs(v-u) < (*it)->getA() + (*pt)->getA())
   && ( (*it)->getType() !=  1 || (*pt)->getType() != 1  )      // not both are fixed particles
   && ( (*it)->getType() !=  5 || (*pt)->getType() != 5  )      // not both are free boundary particles
   && ( (*it)->getType() != 10 || (*pt)->getType() != 10 )  ) { // not both are ghost particles
   contact<Particle> tmpContact(*it, *pt); // a local and temparory object
   ++possContact;
   if(tmpContact.isOverlapped())
   #pragma omp critical
   contactVec.push_back(tmpContact);    // containers use value semantics, so a "copy" is pushed back.
   }
   }
   }
   }
  
   #ifdef DEM_PROFILE
   gettimeofday(&time_p2,NULL);
   debugInf << std::setw(OWID) << "findContact=" << std::setw(OWID) << timediffsec(time_p1, time_p2); 
   #endif
   possContactNum   = possContact;
   actualContactNum = contactVec.size();
   } // end of OpenMP implementation 0

   #elif OPENMP_IMPL == 1
   // OpenMP implementation 1: ts partitions, each thread handles a partition, max diff = n*n*(1-1/ts)/ts
   // implementation is based on vector index.
   void Assembly::findContact() { 
   contactVec.clear();
   int possContact = 0;
  
   #ifdef DEM_PROFILE
   gettimeofday(&time_p1,NULL); 
   #endif
   int tid;   // thread id
   int ts;    // number of threads
   int num;   // number of particles
   int start; // start particle index of each thread
   int end;   // last particle index of each thread
   int i, j;
   Vec u, v;
   num = particleVec.size();
  
   #pragma omp parallel num_threads(nThreads) private(tid, ts, start, end, i, j, u, v) shared(num) reduction(+: possContact)
   {
   tid = omp_get_thread_num();
   ts  = omp_get_num_threads();
   start = tid * num / ts;
   end   = (tid + 1) * num / ts - 1;
    
   // explore each partition
   for (i = start; i <= end; ++i) { 
   u = particleVec[i]->getCurrPos();
   for (j = i + 1; j < num; ++j) {
   v = particleVec[j]->getCurrPos();
   if (   ( vfabs(v-u) < particleVec[i]->getA() + particleVec[j]->getA() )
   && ( particleVec[i]->getType() !=  1 || particleVec[j]->getType() != 1  )      // not both are fixed particles
   && ( particleVec[i]->getType() !=  5 || particleVec[j]->getType() != 5  )      // not both are free boundary particles
   && ( particleVec[i]->getType() != 10 || particleVec[j]->getType() != 10 )  ) { // not both are ghost particles
   contact<Particle> tmpContact(particleVec[i], particleVec[j]); // a local and temparory object
   ++possContact;
   if(tmpContact.isOverlapped())
   #pragma omp critical
   contactVec.push_back(tmpContact);    // containers use value semantics, so a "copy" is pushed back.
   }
   }
   }
   }
  
   #ifdef DEM_PROFILE
   gettimeofday(&time_p2,NULL);
   debugInf <<  std::setw(OWID) << "findContact=" << std::setw(OWID) << timediffsec(time_p1, time_p2); 
   #endif
   possContactNum   = possContact;  
   actualContactNum = contactVec.size();
   } // end of OpenMP implementation 1

   #elif OPENMP_IMPL == 2
   // OpenMP implementation 2: no partitions, each thread leaps by ts until completed, max diff = n*(ts-1)/ts
   void Assembly::findContact() { 
   contactVec.clear();
   int possContact = 0;
  
   #ifdef DEM_PROFILE
   gettimeofday(&time_p1,NULL); 
   #endif
   int tid;   // thread id
   int ts;    // number of threads
   int num;   // number of particles
   int i, j;
   Vec u, v;
   num = particleVec.size();
  
   #pragma omp parallel num_threads(nThreads) private(tid, ts, i, j, u, v) shared(num) reduction(+: possContact)
   {
   tid = omp_get_thread_num();
   ts  = omp_get_num_threads();
    
   // explore each partition
   for (i = tid; i < num; i += ts) { 
   u = particleVec[i]->getCurrPos();
   for (j = i + 1; j < num; ++j) {
   v = particleVec[j]->getCurrPos();
   if (   ( vfabs(v-u) < particleVec[i]->getA() + particleVec[j]->getA() )
   && ( particleVec[i]->getType() !=  1 || particleVec[j]->getType() != 1  )      // not both are fixed particles
   && ( particleVec[i]->getType() !=  5 || particleVec[j]->getType() != 5  )      // not both are free boundary particles
   && ( particleVec[i]->getType() != 10 || particleVec[j]->getType() != 10 )  ) { // not both are ghost particles
   contact<Particle> tmpContact(particleVec[i], particleVec[j]); // a local and temparory object
   ++possContact;
   if(tmpContact.isOverlapped())
   #pragma omp critical
   contactVec.push_back(tmpContact);    // containers use value semantics, so a "copy" is pushed back.
   }
   }
   }
   }
  
   #ifdef DEM_PROFILE
   gettimeofday(&time_p2,NULL);
   debugInf << std::setw(OWID) << "findContact=" << std::setw(OWID) << timediffsec(time_p1, time_p2); 
   #endif
   possContactNum   = possContact;  
   actualContactNum = contactVec.size();
   } // end of OpenMP implementation 2

   #elif OPENMP_IMPL == 3
   // OpenMP implementation 3: no partitions, each thread leaps by ts until num/2 and handles two particles, max diff = 0
   void Assembly::findContact() { 
   contactVec.clear();
   int possContact = 0;
  
   #ifdef DEM_PROFILE
   gettimeofday(&time_p1,NULL); 
   #endif
   int tid;   // thread id
   int ts;    // number of threads
   int num;   // number of particles
   int i, j, k;
   Vec u, v;
   num = particleVec.size();
  
   #pragma omp parallel num_threads(nThreads) private(tid, ts, i, j, k, u, v) shared(num) reduction(+: possContact)
   {
   tid = omp_get_thread_num();
   ts  = omp_get_num_threads();
    
   // explore each partition, works whether num is odd or even
   for (i = tid; i <= num / 2; i += ts) {
   int inc = num - 1 - 2*i;
   if (inc == 0) inc = 1; // avoid infinite loop when num is odd
   for (k = i; k <= num - 1 - i; k += inc ) {
   u = particleVec[k]->getCurrPos();
   for (j = k + 1; j < num; ++j) {
   v = particleVec[j]->getCurrPos();
   if (   ( vfabs(v-u) < particleVec[k]->getA() + particleVec[j]->getA() )
   && ( particleVec[k]->getType() !=  1 || particleVec[j]->getType() != 1  )      // not both are fixed particles
   && ( particleVec[k]->getType() !=  5 || particleVec[j]->getType() != 5  )      // not both are free boundary particles
   && ( particleVec[k]->getType() != 10 || particleVec[j]->getType() != 10 )  ) { // not both are ghost particles
   contact<Particle> tmpContact(particleVec[k], particleVec[j]); // a local and temparory object
   ++possContact;
   if(tmpContact.isOverlapped())
   #pragma omp critical
   contactVec.push_back(tmpContact);    // containers use value semantics, so a "copy" is pushed back.
   }
   }
   }
   }
   }
  
   #ifdef DEM_PROFILE
   gettimeofday(&time_p2,NULL);
   debugInf << std::setw(OWID) << "findContact=" << std::setw(OWID) << timediffsec(time_p1, time_p2); 
   #endif
   possContactNum   = possContact;  
   actualContactNum = contactVec.size();
   } // end of OpenMP implementation 3

   #elif OPENMP_IMPL == 4
   // OpenMP implementation 4: no partitions, parallel for, various loop scheduling: (static), (static,1), (dynamic), (dynamic,1)
   void Assembly::findContact() { 
   contactVec.clear();
   int possContact = 0;
  
   #ifdef DEM_PROFILE
   gettimeofday(&time_p1,NULL); 
   #endif

   int num;   // number of particles
   int i, j;
   Vec u, v;
   num = particleVec.size();
  
   #pragma omp parallel for num_threads(nThreads) private(i, j, u, v) shared(num) reduction(+: possContact) schedule(dynamic)
   for (i = 0; i < num - 1; ++i) { 
   u = particleVec[i]->getCurrPos();
   for (j = i + 1; j < num; ++j) {
   v = particleVec[j]->getCurrPos();
   if (   ( vfabs(v-u) < particleVec[i]->getA() + particleVec[j]->getA() )
   && ( particleVec[i]->getType() !=  1 || particleVec[j]->getType() != 1  )      // not both are fixed particles
   && ( particleVec[i]->getType() !=  5 || particleVec[j]->getType() != 5  )      // not both are free boundary particles
   && ( particleVec[i]->getType() != 10 || particleVec[j]->getType() != 10 )  ) { // not both are ghost particles
   contact<Particle> tmpContact(particleVec[i], particleVec[j]); // a local and temparory object
   ++possContact;
   if(tmpContact.isOverlapped())
   #pragma omp critical
   contactVec.push_back(tmpContact);    // containers use value semantics, so a "copy" is pushed back.
   }
   }
   }
  
  
   #ifdef DEM_PROFILE
   gettimeofday(&time_p2,NULL);
   debugInf << std::setw(OWID) << "findContact=" << std::setw(OWID) << timediffsec(time_p1, time_p2); 
   #endif
   possContactNum   = possContact;  
   actualContactNum = contactVec.size();
   } // end of OpenMP implementation 4

   #endif

   #else //else of def OPENMP, i.e., serial versions start here:

   //start of ndef BINNING
   #ifndef BINNING
   void Assembly::findContact() { // serial version, O(n x n), n is the number of particles.
   contactVec.clear();
   possContactNum = 0;

   #ifdef DEM_PROFILE
   REAL time_r = 0; // time consumed in contact resolution, i.e., tmpContact.isOverlapped()
   gettimeofday(&time_p1,NULL); 
   #endif
    
   int num1 = particleVec.size();  // particles inside container
   int num2 = mergeParticleVec.size(); // particles inside container (at front) + particles from neighboring blocks (at end)
   for (int i = 0; i < num1 - 1; ++i) {
   Vec u = particleVec[i]->getCurrPos();
   for (int j = i + 1; j < num2; ++j) {
   Vec v = mergeParticleVec[j]->getCurrPos();
   if (   ( vfabs(v - u) < particleVec[i]->getA() + mergeParticleVec[j]->getA())
   && ( particleVec[i]->getType() !=  1 || mergeParticleVec[j]->getType() != 1  )      // not both are fixed particles
   && ( particleVec[i]->getType() !=  5 || mergeParticleVec[j]->getType() != 5  )      // not both are free boundary particles
   && ( particleVec[i]->getType() != 10 || mergeParticleVec[j]->getType() != 10 )  ) { // not both are ghost particles
   Contact tmpContact(particleVec[i], mergeParticleVec[j]); // a local and temparory object
   ++possContactNum;
   #ifdef DEM_PROFILE
   gettimeofday(&time_r1,NULL); 
   #endif
   if(tmpContact.isOverlapped())
   contactVec.push_back(tmpContact);    // containers use value semantics, so a "copy" is pushed back.
   #ifdef DEM_PROFILE
   gettimeofday(&time_r2,NULL); 
   time_r += timediffsec(time_r1, time_r2);
   #endif
   }
   }
   }	
    
   #ifdef DEM_PROFILE
   gettimeofday(&time_p2,NULL);
   debugInf << std::setw(OWID) << "findContact=" << std::setw(OWID) << timediffsec(time_p1, time_p2) << std::setw(OWID) << "isOverlapped=" << std::setw(OWID) << time_r; 
   #endif
    
   actualContactNum = contactVec.size();
   }

   //else of ndef BINNING
   #else
   void Assembly::findContact() { // serial version, binning methods, cell slightly larger than maximum particle
   contactVec.clear();
   possContactNum = 0;
  
   #ifdef DEM_PROFILE
   REAL time_r = 0;
   gettimeofday(&time_p1,NULL); 
   #endif
   REAL maxDiameter = gradation.getPtclMaxRadius() * 2;
   int  nx = floor (container.getDimx() / maxDiameter);
   int  ny = floor (container.getDimy() / maxDiameter);
   int  nz = floor (container.getDimz() *1.5 / maxDiameter);
   REAL dx = container.getDimx() / nx;
   REAL dy = container.getDimy() / ny;
   REAL dz = container.getDimz() *1.5 / nz;
   Vec  minCorner= container.getMinCorner();
   REAL x0 = minCorner.getX();
   REAL y0 = minCorner.getY();
   REAL z0 = minCorner.getZ();
  
   // 26 neighbors of each cell
   int neighbor[26][3];
   int count = 0;
   for (int i = -1; i < 2; ++i)
   for (int j = -1; j < 2; ++j)
   for (int k = -1; k < 2; ++k) {
   if (! (i == 0 && j == 0 && k==0 ) ) {
   neighbor[count][0] = i;
   neighbor[count][1] = j;
   neighbor[count][2] = k;
   ++count;
   }
   }
 
   // 4-dimensional array of cellVec
   typedef std::pair<bool, std::vector<Particle*> > cellT;
   std::vector< std::vector< std::vector < cellT > > > cellVec;
   cellVec.resize(nx);
   for (int i = 0; i < cellVec.size(); ++i) {
   cellVec[i].resize(ny);
   for (int j = 0; j < cellVec[i].size(); ++j)
   cellVec[i][j].resize(nz);
   }
   // mark each cell as not searched
   for (int i = 0; i < nx; ++i)
   for (int j = 0; j < ny; ++j)
   for (int k = 0; k < nz; ++k)
   cellVec[i][j][k].first = false; // has not ever been searched

   // find particles in each cell
   Vec center;
   REAL x1, x2, y1, y2, z1, z2;
   for (int i = 0; i < nx; ++i)
   for (int j = 0; j < ny; ++j)
   for (int k = 0; k < nz; ++k) {
   x1 = x0 + dx * i;
   x2 = x0 + dx * (i + 1);
   y1 = y0 + dy * j;
   y2 = y0 + dy * (j + 1);
   z1 = z0 + dz * k;
   z2 = z0 + dz * (k + 1);
   for (int pt = 0; pt < particleVec.size(); ++pt) {
   center = particleVec[pt]->getCurrPos();
   if (center.getX() >= x1 && center.getX() < x2 &&
   center.getY() >= y1 && center.getY() < y2 &&
   center.getZ() >= z1 && center.getZ() < z2)
   cellVec[i][j][k].second.push_back( particleVec[pt] );
   }
   }
  
   // for each cell:
   Particle *it, *pt;
   Vec u, v;
   for (int i = 0; i < nx; ++i)
   for (int j = 0; j < ny; ++j)
   for (int k = 0; k < nz; ++k) {
   // for particles inside the cell	  
   for (int m = 0; m < cellVec[i][j][k].second.size(); ++m) {
   it = cellVec[i][j][k].second[m];
   u  = it->getCurrPos();
	  
   // for particles inside the cell itself   
   for (int n = m + 1; n < cellVec[i][j][k].second.size(); ++n) {
   //debugInf <<  i << " " << j << " " << k << " " << "m n size=" << m << " " << n << " " <<  cellVec[i][j][k].size() << std::endl;
   pt = cellVec[i][j][k].second[n];
   v  = pt->getCurrPos();
   if ( ( vfabs(u-v) < it->getA() + pt->getA() )  &&
   ( it->getType() !=  1 || pt->getType() != 1 ) &&   // not both are fixed particles
   ( it->getType() !=  5 || pt->getType() != 5 ) &&   // not both are free boundary particles
   ( it->getType() != 10 || pt->getType() != 10)  ) { // not both are ghost particles
   contact<Particle> tmpContact(it, pt); // a local and temparory object
   ++possContactNum;
   #ifdef DEM_PROFILE
   gettimeofday(&time_r1,NULL); 
   #endif
   if(tmpContact.isOverlapped())
   contactVec.push_back(tmpContact);   // containers use value semantics, so a "copy" is pushed back.
   #ifdef DEM_PROFILE
   gettimeofday(&time_r2,NULL); 
   time_r += timediffsec(time_r1, time_r2);
   #endif
   }
   }
	  
   // for 26 neighboring cells
   for (int ncell = 0; ncell < 26; ++ncell ) {
   int ci = i + neighbor[ncell][0];
   int cj = j + neighbor[ncell][1];
   int ck = k + neighbor[ncell][2];
   if (ci > -1 && ci < nx && cj > -1 && cj < ny && ck > -1 && ck < nz && cellVec[ci][cj][ck].first == false ) {
   //debugInf << "i j k m ncell ci cj ck size contacts= " << i << " " << j << " " << k << " " << m  << " " << ncell << " " << ci << " " << cj << " " << ck << " " << cellVec[ci][cj][ck].second.size() << " "  << contactVec.size() << std::endl;
   std::vector<Particle*> vt = cellVec[ci][cj][ck].second;
   for (int n = 0; n < vt.size(); ++n) {
   pt = vt[n];
   v  = pt->getCurrPos();
   if ( ( vfabs(u-v) < it->getA() + pt->getA() )  &&
   ( it->getType() !=  1 || pt->getType() != 1 ) &&   // not both are fixed particles
   ( it->getType() !=  5 || pt->getType() != 5 ) &&   // not both are free boundary particles
   ( it->getType() != 10 || pt->getType() != 10)  ) { // not both are ghost particles
   contact<Particle> tmpContact(it, pt); // a local and temparory object
   ++possContactNum;
   #ifdef DEM_PROFILE
   gettimeofday(&time_r1,NULL); 
   #endif
   if(tmpContact.isOverlapped())
   contactVec.push_back(tmpContact);   // containers use value semantics, so a "copy" is pushed back.
   #ifdef DEM_PROFILE
   gettimeofday(&time_r2,NULL); 
   time_r += timediffsec(time_r1, time_r2);
   #endif
		  
   }
   }
   }
   }
   }
   cellVec[i][j][k].first = true; // searched, will not be searched again
	
   }
  
   #ifdef DEM_PROFILE
   gettimeofday(&time_p2,NULL);
   debugInf << std::setw(OWID) << "findContact=" << std::setw(OWID) << timediffsec(time_p1, time_p2) << std::setw(OWID) << "isOverlapped=" << std::setw(OWID) << time_r; 
   #endif
  
   actualContactNum = contactVec.size();
   }

   //end of ndef BINNING
   #endif

   //end of def OPENMP 
   #endif


   Vec Assembly::getTopFreeParticlePosition() const {
   std::vector<Particle*>::const_iterator it,jt,kt;
   it=particleVec.begin();
   while (it!=particleVec.end() && (*it)->getType()!=0)   // find the 1st free particle
   ++it;

   if (it==particleVec.end())    // no free particles
   return 0;

   jt=it; 
   kt=it;
    
   // two cases:
   // 1: 1st particle is not free
   // 2: 1st particle is free
   if (++kt!=particleVec.end()) { // case1: more than 2 particles; case 2: more than 1 particle
   for(++it;it!=particleVec.end();++it) {
   if ((*it)->getType()==0)
   if ((*it)->getCurrPos().getZ() > (*jt)->getCurrPos().getZ())
   jt=it;
   }
   return (*jt)->getCurrPos();
   }
   else {
   if ((*it)->getType()==0)  // case1: only 2 particles, the 2nd one is free; case2: only 1 particle
   return (*it)->getCurrPos();
   else
   return 0;
   }

   }



   REAL Assembly::ellipPileForce() {
   REAL val=0;
   for(std::vector<Particle*>::iterator it=particleVec.begin();it!=particleVec.end();++it)
   if ((*it)->getType()==3) {
   val = (*it)->getForce().getZ();
   break;
   }
   return val;
   }

   Vec Assembly::ellipPileDimn() {
   Vec val;
   for(std::vector<Particle*>::iterator it=particleVec.begin();it!=particleVec.end();++it)
   if ((*it)->getType()==3) {
   val = Vec((*it)->getA(), (*it)->getB(), (*it)->getC());
   break;
   }
   return val;
   }

   REAL Assembly::ellipPileTipZ() {
   REAL val=0;
   for(std::vector<Particle*>::iterator it=particleVec.begin();it!=particleVec.end();++it)
   if ((*it)->getType()==3) {
   val = (*it)->getCurrPos().getZ()-(*it)->getA();
   break;
   }
   return val;
   }

   REAL Assembly::ellipPilePeneVol() {
   REAL val=0;
   if (getTopFreeParticlePosition().getZ()-ellipPileTipZ() <= 0)
   val=0;
   else{
   // low: a signed number as lower limit for volumetric integration
   REAL low=ellipPileTipZ() + ellipPileDimn().getX() - getTopFreeParticlePosition().getZ(); 
   REAL lowint=low-pow(low,3)/3.0/pow(ellipPileDimn().getX(),2);
   val = Pi * ellipPileDimn().getY() * ellipPileDimn().getZ()
   *(2.0/3*ellipPileDimn().getX()-lowint);
   }
   return val;
   }

   void Assembly::ellipPileUpdate() {
   for(std::vector<Particle*>::iterator it=particleVec.begin();it!=particleVec.end();++it) {
   if ((*it)->getType()==3) {
   (*it)->setCurrVeloc(Vec(0, 0, -pileRate));
   (*it)->setCurrPos( (*it)->getPrevPos() + (*it)->getCurrVeloc() * timeStep);
   }
   }
   }





   void Assembly::springForce() {
   for (vector<Spring*>::iterator it = springVec.begin(); it != springVec.end(); ++it)
   (*it)->applyForce();
   }

   void Assembly::readCavityBoundary(const char *str) {
   std::ifstream ifs(str);
   if(!ifs) { debugInf << "stream error: readCavityBoundary" << std::endl; exit(-1); }  

   Boundary<Particle>* rbptr;
   int type;
   cavityBoundaryVec.clear();
   int boundaryNum;
   ifs >> boundaryNum;
   for(int i = 0; i < boundaryNum; i++) {
   ifs >> type;
   if(type == 1) // plane boundary
   rbptr = new planeBoundary<Particle>(ifs);
   cavityBoundaryVec.push_back(rbptr);
   }

   ifs.close();
   }


   void Assembly::printCavityBoundary(const char *str) const {
   std::ofstream ofs(str);
   if(!ofs) { debugInf << "stream error: printCavityBoundary" << std::endl; exit(-1); }
   ofs.setf(std::ios::scientific, std::ios::floatfield);
  
   ofs << std::setw(OWID) << cavityBoundaryVec.size() << std::endl;
   std::vector<Boundary*>::const_iterator rt;
   for(rt = cavityBoundaryVec.begin(); rt != cavityBoundaryVec.end(); ++rt)
   (*rt)->display(ofs);
   ofs << std::endl;
  
   ofs.close();
   }



   void Assembly::findCavityContact() {
   std::vector<Boundary*>::iterator rt;
   for(rt = cavityBoundaryVec.begin(); rt != cavityBoundaryVec.end(); ++rt)
   (*rt)->findBdryContact(allParticleVec);
   }


   void Assembly::cavityBoundaryForce() {
   std::vector<Boundary*>::iterator rt;
   for(rt = cavityBoundaryVec.begin(); rt != cavityBoundaryVec.end(); ++rt)
   (*rt)->boundaryForce(boundaryTgtMap);
   }

   Vec Assembly::getNormalForce(int bdry) const {
   std::vector<Boundary*>::const_iterator it;
   for(it=boundaryVec.begin();it!=boundaryVec.end();++it) {
   if((*it)->getBdryID()==bdry)
   return (*it)->getNormalForce();
   }
   return 0;
   }

   Vec Assembly::getShearForce(int bdry) const {
   std::vector<Boundary*>::const_iterator it;
   for(it=boundaryVec.begin();it!=boundaryVec.end();++it) {
   if((*it)->getBdryID()==bdry)
   return (*it)->getShearForce();
   }
   return 0;
   }

   REAL Assembly::getAvgNormal(int bdry) const {
   std::vector<Boundary*>::const_iterator it;
   for(it=boundaryVec.begin();it!=boundaryVec.end();++it) {
   if((*it)->getBdryID()==bdry)
   return (*it)->getAvgNormal();
   }
   return 0;
   }

   Vec Assembly::getApt(int bdry) const {
   std::vector<Boundary*>::const_iterator it;
   for(it=boundaryVec.begin();it!=boundaryVec.end();++it) {
   if((*it)->getBdryID()==bdry)
   return (*it)->getApt();
   }
   return 0;
   }


   Vec Assembly::getDirc(int bdry) const {
   std::vector<Boundary*>::const_iterator it;
   for(it=boundaryVec.begin();it!=boundaryVec.end();++it) {
   if((*it)->getBdryID()==bdry)
   return (*it)->getDirc();
   }
   return 0;
   }

   REAL Assembly::getArea(int n) const {
   std::vector<Boundary*>::const_iterator it;
   for(it=boundaryVec.begin();it!=boundaryVec.end();++it) {
   if((*it)->getBdryID()==n)
   return (*it)->area;
   }
   return 0;
   }

   void Assembly::setArea(int n, REAL a) {
   std::vector<Boundary*>::iterator it;
   for(it=boundaryVec.begin();it!=boundaryVec.end();++it) {
   if((*it)->getBdryID()==n)
   (*it)->area=a;
   }
   }

   REAL Assembly::getAvgPressure() const {
   std::vector<Boundary*>::const_iterator rt;
   REAL avgpres=0;
   for(rt=boundaryVec.begin();rt!=boundaryVec.end();++rt)
   avgpres+=vfabs((*rt)->getNormalForce())/(*rt)->getArea();
   return avgpres/=boundaryVec.size();
   }

   // only update CoefOfLimits[0] for specified boundaries
   void Assembly::updateBoundary(int bn[], UPDATECTL rbctl[], int num) {
   for(int i=0;i<num;i++) {
   for(std::vector<Boundary*>::iterator rt=boundaryVec.begin();rt!=boundaryVec.end();++rt) {
   if((*rt)->getBdryID()==bn[i]) {
   (*rt)->update(rbctl[i]);
   break;
   }
   }
   }
   }

   // update CoefOfLimits[1,2,3,4] for all 6 boundaries
   void Assembly::updateBoundary6() {
   for(std::vector<Boundary*>::iterator rt=boundaryVec.begin();rt!=boundaryVec.end();++rt) {
   if((*rt)->getBdryID()==1 || (*rt)->getBdryID()==3) {
   for(std::vector<Boundary*>::iterator lt=boundaryVec.begin();lt!=boundaryVec.end();++lt) {
   if((*lt)->getBdryID()==4)
   (*rt)->CoefOfLimits[1].apt=(*lt)->CoefOfLimits[0].apt;
   else if((*lt)->getBdryID()==2)
   (*rt)->CoefOfLimits[2].apt=(*lt)->CoefOfLimits[0].apt;
   else if((*lt)->getBdryID()==5)
   (*rt)->CoefOfLimits[3].apt=(*lt)->CoefOfLimits[0].apt;
   else if((*lt)->getBdryID()==6)
   (*rt)->CoefOfLimits[4].apt=(*lt)->CoefOfLimits[0].apt;
   }
   }
   else if((*rt)->getBdryID()==2 || (*rt)->getBdryID()==4) {
   for(std::vector<Boundary*>::iterator lt=boundaryVec.begin();lt!=boundaryVec.end();++lt) {
   if((*lt)->getBdryID()==1)
   (*rt)->CoefOfLimits[1].apt=(*lt)->CoefOfLimits[0].apt;
   else if((*lt)->getBdryID()==3)
   (*rt)->CoefOfLimits[2].apt=(*lt)->CoefOfLimits[0].apt;
   else if((*lt)->getBdryID()==5)
   (*rt)->CoefOfLimits[3].apt=(*lt)->CoefOfLimits[0].apt;
   else if((*lt)->getBdryID()==6)
   (*rt)->CoefOfLimits[4].apt=(*lt)->CoefOfLimits[0].apt;
   }

   }
   else if((*rt)->getBdryID()==5 || (*rt)->getBdryID()==6) {
   for(std::vector<Boundary*>::iterator lt=boundaryVec.begin();lt!=boundaryVec.end();++lt) {
   if((*lt)->getBdryID()==1)
   (*rt)->CoefOfLimits[1].apt=(*lt)->CoefOfLimits[0].apt;
   else if((*lt)->getBdryID()==3)
   (*rt)->CoefOfLimits[2].apt=(*lt)->CoefOfLimits[0].apt;
   else if((*lt)->getBdryID()==2)
   (*rt)->CoefOfLimits[3].apt=(*lt)->CoefOfLimits[0].apt;
   else if((*lt)->getBdryID()==4)
   (*rt)->CoefOfLimits[4].apt=(*lt)->CoefOfLimits[0].apt;
   }

   }
	
   }
   }


   void Assembly::deposit_repose(int   interval,
   const char *inibdryfile,
   const char *ParticleFile, 
   const char *contactfile,
   const char *progressfile, 
   const char *debugfile)
   {
   this->container = container;
  
   buildBoundary(5, inibdryfile); // container unchanged
  
   angleOfRepose(interval,           // print interval
   inibdryfile,        // input file, initial boundaries
   ParticleFile,       // output file, resulted particles, including snapNum 
   contactfile,        // output file, resulted contacts, including snapNum 
   progressfile,       // output file, statistical info
   debugfile);         // output file, debug info
   }

   void Assembly::angleOfRepose(int   interval,
   const char *inibdryfile,
   const char *ParticleFile, 
   const char *contactfile,
   const char *progressfile, 
   const char *debugfile)
   {
   // pre_1: open streams for output.
   progressInf.open(progressfile); 
   if(!progressInf) { debugInf << "stream error: angleOfRepose" << std::endl; exit(-1); }
   progressInf.setf(std::ios::scientific, std::ios::floatfield);
   progressInf.precision(OPREC);
   progressInf << std::setw(OWID) << "iteration"
   << std::setw(OWID) << "poss_contact"
   << std::setw(OWID) << "actual_contact"
   << std::setw(OWID) << "penetration"
   << std::setw(OWID) << "avg_normal"
   << std::setw(OWID) << "avg_tangt"
   << std::setw(OWID) << "avg_velocity"
   << std::setw(OWID) << "avg_omga"
   << std::setw(OWID) << "avg_force"
   << std::setw(OWID) << "avg_moment"
   << std::setw(OWID) << "trans_energy"
   << std::setw(OWID) << "rotat_energy"
   << std::setw(OWID) << "kinet_energy"
   << std::setw(OWID) << "poten_energy"
   << std::setw(OWID) << "total_energy"
   << std::setw(OWID) << "void_ratio"
   << std::setw(OWID) << "porosity"
   << std::setw(OWID) << "coord_number"
   << std::setw(OWID) << "density"
   << std::setw(OWID) << "sigma_y1"
   << std::setw(OWID) << "sigma_y2"
   << std::setw(OWID) << "sigma_x1"
   << std::setw(OWID) << "sigma_x2"
   << std::setw(OWID) << "sigma_z1"
   << std::setw(OWID) << "sigma_z2"
   << std::setw(OWID) << "mean_stress"
   << std::setw(OWID) << "dimx"
   << std::setw(OWID) << "dimy"
   << std::setw(OWID) << "dimz"
   << std::setw(OWID) << "volume"
   << std::setw(OWID) << "epsilon_x"
   << std::setw(OWID) << "epsilon_y"
   << std::setw(OWID) << "epsilon_z"
   << std::setw(OWID) << "epsilon_v"
   << std::setw(OWID) << "vibra_t_step"
   << std::setw(OWID) << "impact_t_step"
   << std::setw(OWID) << "wall_time" << std::endl;
  
   debugInf.open(debugfile);
   if(!debugInf) { debugInf << "stream error: angleOfRepose" << std::endl; exit(-1); }
   debugInf.setf(std::ios::scientific, std::ios::floatfield);
  
   // pre_2. create boundaries from existing files.
   readBoundary(inibdryfile);

   // pre_3: define variables used in iterations.
   REAL avgNormal=0;
   REAL avgTangt=0;
   int  stepsnum=0;
   char stepsstr[4];
   bool toSnapshot=false;
   char stepsfp[50];
   REAL void_ratio=0;
   REAL bdry_penetr[7] = {0,0,0,0,0,0,0};
   int  bdry_cntnum[7] = {0,0,0,0,0,0,0};

   REAL maxRadius = gradation.getPtclMaxRadius();
   REAL maxDiameter = maxRadius * 2.0;
   REAL z0 = container.getMinCorner().getZ();
   std::vector<Particle*> lastPtcls;
   Particle *newPtcl = NULL;
   int layers = 1; // how many layers of new particles to generate each time

   iteration = 0; 
   int particleNum = 0;
   REAL zCurr;
   gettimeofday(&time_w1,NULL);
   // iterations starting ...
   do
   {
   // 1. add particle
   if ( particleNum == 0 ) {
   zCurr = z0 + maxRadius;

   for ( int i = 0; i != layers; ++i) {
   newPtcl = new Particle(particleNum+1, 0, Vec(0, 0, zCurr + maxDiameter * i), gradation, young, poisson);
   particleVec.push_back(newPtcl);
   ++particleNum;
   lastPtcls.push_back(newPtcl);
	  
   newPtcl = new Particle(particleNum+1, 0, Vec(maxDiameter, 0, zCurr + maxDiameter * i), gradation, young, poisson);
   particleVec.push_back(newPtcl);
   ++particleNum;
   //lastPtcls.push_back(newPtcl);
	  
   newPtcl = new Particle(particleNum+1, 0, Vec(-maxDiameter, 0, zCurr + maxDiameter * i), gradation, young, poisson);
   particleVec.push_back(newPtcl);
   ++particleNum;
   //lastPtcls.push_back(newPtcl);
	  
   newPtcl = new Particle(particleNum+1, 0, Vec(0, maxDiameter, zCurr + maxDiameter * i), gradation, young, poisson);
   particleVec.push_back(newPtcl);
   ++particleNum;
   //lastPtcls.push_back(newPtcl);
	  
   newPtcl = new Particle(particleNum+1, 0, Vec(0, -maxDiameter, zCurr + maxDiameter * i), gradation, young, poisson);
   particleVec.push_back(newPtcl);
   ++particleNum;
   //lastPtcls.push_back(newPtcl);
   }
   toSnapshot = true;

   }
   else {
   vector<Particle*>::iterator it;
   bool allInContact = false;
   for ( it = lastPtcls.begin(); it != lastPtcls.end(); ++it) {
   if ( (*it)->isInContact() ) 
   allInContact = true;
   else {
   allInContact = false;
   break;
   }
   }

   if ( allInContact ) {

   lastPtcls.clear(); // do not delete those pointers to release memory; particleVec will do it.
   zCurr = getPtclMaxZ(allParticleVec) + maxDiameter;

   for ( int i = 0; i != layers; ++i) {
   newPtcl = new Particle(particleNum+1, 0, Vec(0, 0, zCurr + maxDiameter * i), gradation, young, poisson);
   particleVec.push_back(newPtcl);
   ++particleNum;
   lastPtcls.push_back(newPtcl);
	    
   newPtcl = new Particle(particleNum+1, 0, Vec(maxDiameter, 0, zCurr + maxDiameter * i), gradation, young, poisson);
   particleVec.push_back(newPtcl);
   ++particleNum;
   //lastPtcls.push_back(newPtcl);
	    
   newPtcl = new Particle(particleNum+1, 0, Vec(-maxDiameter, 0, zCurr + maxDiameter * i), gradation, young, poisson);
   particleVec.push_back(newPtcl);
   ++particleNum;
   //lastPtcls.push_back(newPtcl);
	    
   newPtcl = new Particle(particleNum+1, 0, Vec(0, maxDiameter, zCurr + maxDiameter * i), gradation, young, poisson);
   particleVec.push_back(newPtcl);
   ++particleNum;
   //lastPtcls.push_back(newPtcl);
	    
   newPtcl = new Particle(particleNum+1, 0, Vec(0, -maxDiameter, zCurr + maxDiameter * i), gradation, young, poisson);
   particleVec.push_back(newPtcl);
   ++particleNum;
   //lastPtcls.push_back(newPtcl);	
   }
   toSnapshot = true;
   }
   }
   // 2. create possible boundary particles and contacts between particles.
   findContact();
   findBdryContact();
      
   // 3. set particle forces/moments as zero before each re-calculation,
   clearContactForce();	
      
   // 4. calculate contact forces/moments and apply them to particles.
   internalForce(avgNormal, avgTangt);
      
   // 5. calculate boundary forces/moments and apply them to particles.
   boundaryForce(bdry_penetr, bdry_cntnum);
      
   // 6. update particles' velocity/omga/position/orientation based on force/moment.
   updateParticle();
      
   // 7. (1) output particles and contacts information as snapNum.
   if (toSnapshot) {
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp, ParticleFile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printParticle(stepsfp);
	
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp, contactfile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printContact(stepsfp);
   time(&timeStamp);
   g_timeinf << std::setw(4) << stepsnum << " " << ctime(&timeStamp) << std::flush;
   ++stepsnum;
   toSnapshot = false;
   }
      
   // 8. (2) output stress and strain info.
   if (iteration % interval == 0) {
   gettimeofday(&time_w2,NULL);
   REAL t1=getTransEnergy();
   REAL t2=getRotatEnergy();
   REAL t3=getPotenEnergy(-0.025);
   progressInf << std::setw(OWID) << iteration
   << std::setw(OWID) << getPossContactNum()
   << std::setw(OWID) << getActualContactNum()
   << std::setw(OWID) << getAvgPenetration()
   << std::setw(OWID) << avgNormal
   << std::setw(OWID) << avgTangt
   << std::setw(OWID) << getAvgVelocity() 
   << std::setw(OWID) << getAvgOmga()
   << std::setw(OWID) << getAvgForce()   
   << std::setw(OWID) << getAvgMoment()
   << std::setw(OWID) << t1
   << std::setw(OWID) << t2
   << std::setw(OWID) << (t1+t2)
   << std::setw(OWID) << t3
   << std::setw(OWID) << (t1+t2+t3)
   << std::setw(OWID) << void_ratio
   << std::setw(OWID) << void_ratio/(1+void_ratio)
   << std::setw(OWID) << 2.0*(getActualContactNum()
   +bdry_cntnum[1]+bdry_cntnum[2]+bdry_cntnum[3]
   +bdry_cntnum[4]+bdry_cntnum[6])/allParticleVec.size()
   << std::setw(OWID) << "0"
   << std::setw(OWID) << "0"
   << std::setw(OWID) << "0"
   << std::setw(OWID) << "0"
   << std::setw(OWID) << "0"
   << std::setw(OWID) << "0"
   << std::setw(OWID) << "0"
   << std::setw(OWID) << "0"
   << std::setw(OWID) << "0"
   << std::setw(OWID) << "0"
   << std::setw(OWID) << "0"
   << std::setw(OWID) << "0"
   << std::setw(OWID) << "0"
   << std::setw(OWID) << "0"
   << std::setw(OWID) << "0"
   << std::setw(OWID) << "0"
   << std::setw(OWID) << getVibraTimeStep()
   << std::setw(OWID) << getImpactTimeStep()
   << std::setw(OWID) << timediffsec(time_w1,time_w2)
   << std::endl;
   }
      
   // 7. loop break conditions.
   ++iteration;
      
   } while (particleNum < 2000); //( zCurr < container.getMaxCorner().getZ() );  //(++iteration < totalSteps);
    
   // post_1. store the final snapshot of particles & contacts.
   strcpy(stepsfp, ParticleFile); strcat(stepsfp, "_end");
   printParticle(stepsfp);

   strcpy(stepsfp, contactfile); strcat(stepsfp, "_end");
   printContact(stepsfp);
   g_timeinf << std::setw(4) << "end" << " " << ctime(&timeStamp) << std::flush;

   // post_2. close streams
   progressInf.close();
   debugInf.close();
   }


   void Assembly::scale_PtclBdry(int   totalSteps,  
   int   snapNum,
   int   interval,
   REAL dimn,
   REAL rsize,
   const char *iniptclfile,   
   const char *ParticleFile, 
   const char *contactfile,
   const char *progressfile, 
   const char *debugfile)
   {
   deposit_p(totalSteps,        // totalSteps
   snapNum,          // number of snapNum
   interval,           // print interval
   dimn,               // dimension of particle-composed-boundary
   rsize,              // relative container size
   iniptclfile,        // input file, initial particles
   ParticleFile,       // output file, resulted particles, including snapNum 
   contactfile,        // output file, resulted contacts, including snapNum 
   progressfile,       // output file, statistical info
   debugfile);         // output file, debug info
   }


   // collapse a deposited specimen through gravitation
   void Assembly::collapse(int   totalSteps,  
   int   snapNum,
   int   interval,
   const char *iniptclfile,
   const char *initboundary,
   const char *ParticleFile,
   const char *contactfile,
   const char *progressfile,
   const char *debugfile)
   {
   buildBoundary(1,              // 1-only bottom boundary; 5-no top boundary;6-boxed 6 boundaries
   initboundary);  // output file, containing boundaries info
  
   deposit(totalSteps,        // number of iterations
   snapNum,          // number of snapNum
   interval,           // print interval
   iniptclfile,        // input file, initial particles
   initboundary,       // input file, boundaries
   ParticleFile,       // output file, resulted particles, including snapNum 
   contactfile,        // output file, resulted contacts, including snapNum 
   progressfile,       // output file, statistical info
   debugfile);         // output file, debug info
   }

  


   // make a cavity inside the sample and remove particles in the cavity
   void Assembly::trimCavity(bool toRebuild,
   const char *ParticleFile,
   const char *cavParticleFile)
   {
   if (toRebuild) readParticle(ParticleFile);
   trimHistoryNum = allParticleVec.size();

   REAL x1,x2,y1,y2,z1,z2,x0,y0,z0;
   x1 = cavity.getMinCorner().getX();
   y1 = cavity.getMinCorner().getY();
   z1 = cavity.getMinCorner().getZ();
   x2 = cavity.getMaxCorner().getX();
   y2 = cavity.getMaxCorner().getY();
   z2 = cavity.getMaxCorner().getZ();
   x0 = cavity.getCenter().getX();
   y0 = cavity.getCenter().getY();
   z0 = cavity.getCenter().getZ();
 
   std::vector<Particle*>::iterator itr;
   Vec center;
   REAL delta = gradation.getPtclMaxRadius();

   for (itr = particleVec.begin(); itr != particleVec.end(); ) {
   center=(*itr)->getCurrPos();
   if(center.getX() + delta  >= x1 && center.getX() - delta <= x2 &&
   center.getY() + delta  >= y1 && center.getY() - delta <= y2 &&
   center.getZ() + delta  >= z1 && center.getZ() - delta <= z2 )
   {
   delete (*itr); // release memory
   itr = particleVec.erase(itr); 
   }
   else
   ++itr;
   }
  
   printParticle(cavParticleFile);
   }


   // expand partcile size by some percentage for particles inside cavity
   void Assembly::expandCavityParticle(bool toRebuild,
   REAL percent,
   const char *cavityptclfile,
   const char *ParticleFile,
   const char *newptclfile)
   {
   if (toRebuild) readParticle(ParticleFile);
   trimHistoryNum = allParticleVec.size();

   REAL x1,x2,y1,y2,z1,z2;
   x1 = cavity.getMinCorner().getX();
   y1 = cavity.getMinCorner().getY();
   z1 = cavity.getMinCorner().getZ();
   x2 = cavity.getMaxCorner().getX();
   y2 = cavity.getMaxCorner().getY();
   z2 = cavity.getMaxCorner().getZ();
 
   std::vector<Particle*>::iterator itr;
   Vec center;

   int cavityPtclNum = 0;
   for (itr = particleVec.begin(); itr != particleVec.end(); ++itr ) {
   center=(*itr)->getCurrPos();
   if(center.getX() > x1 && center.getX() < x2 &&
   center.getY() > y1 && center.getY() < y2 &&
   center.getZ() > z1 && center.getZ() < z2 )
   ++cavityPtclNum;
   }

   printCavityParticle(cavityPtclNum, cavityptclfile);

   for (itr = particleVec.begin(); itr != particleVec.end(); ++itr ) {
   center=(*itr)->getCurrPos();
   if(center.getX() > x1 && center.getX() < x2 &&
   center.getY() > y1 && center.getY() < y2 &&
   center.getZ() > z1 && center.getZ() < z2 )
   (*itr)->expand(percent);
   }

   printParticle(newptclfile);
   }


   void Assembly::printCavityParticle(int total, const char *str) const {
   std::ofstream ofs(str);
   if(!ofs) { debugInf << "stream error: printCavityParticle" << std::endl; exit(-1); }
   ofs.setf(std::ios::scientific, std::ios::floatfield);
   ofs.precision(OPREC);
   ofs << std::setw(OWID) << total << std::setw(OWID) << 1 << std::endl;
   ofs << std::setw(OWID) << cavity.getCenter().getX()
   << std::setw(OWID) << cavity.getCenter().getY()
   << std::setw(OWID) << cavity.getCenter().getZ()
   << std::setw(OWID) << cavity.getDimx()
   << std::setw(OWID) << cavity.getDimy()
   << std::setw(OWID) << cavity.getDimz() << std::endl;
  
   ofs << std::setw(OWID) << "ID"
   << std::setw(OWID) << "type"
   << std::setw(OWID) << "radius_a"
   << std::setw(OWID) << "radius_b"
   << std::setw(OWID) << "radius_c"
   << std::setw(OWID) << "position_x"
   << std::setw(OWID) << "position_y"
   << std::setw(OWID) << "position_z"
   << std::setw(OWID) << "axis_a_x"
   << std::setw(OWID) << "axis_a_y"
   << std::setw(OWID) << "axis_a_z"
   << std::setw(OWID) << "axis_b_x"
   << std::setw(OWID) << "axis_b_y"
   << std::setw(OWID) << "axis_b_z"
   << std::setw(OWID) << "axis_c_x"
   << std::setw(OWID) << "axis_c_y"
   << std::setw(OWID) << "axis_c_z"
   << std::setw(OWID) << "velocity_x"
   << std::setw(OWID) << "velocity_y"
   << std::setw(OWID) << "velocity_z"
   << std::setw(OWID) << "omga_x"
   << std::setw(OWID) << "omga_y"
   << std::setw(OWID) << "omga_z"
   << std::setw(OWID) << "force_x"
   << std::setw(OWID) << "force_y"
   << std::setw(OWID) << "force_z"
   << std::setw(OWID) << "moment_x"
   << std::setw(OWID) << "moment_y"
   << std::setw(OWID) << "moment_z"
   << std::endl;

   REAL x1,x2,y1,y2,z1,z2;
   x1 = cavity.getMinCorner().getX();
   y1 = cavity.getMinCorner().getY();
   z1 = cavity.getMinCorner().getZ();
   x2 = cavity.getMaxCorner().getX();
   y2 = cavity.getMaxCorner().getY();
   z2 = cavity.getMaxCorner().getZ();
  
   Vec tmp;
   std::vector<Particle*>::const_iterator  it;
   for (it=particleVec.begin();it!=particleVec.end();++it)  {
   Vec center=(*it)->getCurrPos();
   if(center.getX() > x1 && center.getX() < x2 &&
   center.getY() > y1 && center.getY() < y2 &&
   center.getZ() > z1 && center.getZ() < z2 ) {

   ofs << std::setw(OWID) << (*it)->getId()
   << std::setw(OWID) << (*it)->getType()
   << std::setw(OWID) << (*it)->getA()
   << std::setw(OWID) << (*it)->getB()
   << std::setw(OWID) << (*it)->getC();
    
   tmp=(*it)->getCurrPos();
   ofs << std::setw(OWID) << tmp.getX()
   << std::setw(OWID) << tmp.getY()
   << std::setw(OWID) << tmp.getZ();
    
   tmp=(*it)->getCurrDirecA();
   ofs << std::setw(OWID) << tmp.getX()
   << std::setw(OWID) << tmp.getY()
   << std::setw(OWID) << tmp.getZ();
    
   tmp=(*it)->getCurrDirecB();
   ofs << std::setw(OWID) << tmp.getX()
   << std::setw(OWID) << tmp.getY()
   << std::setw(OWID) << tmp.getZ();
    
   tmp=(*it)->getCurrDirecC();
   ofs << std::setw(OWID) << tmp.getX()
   << std::setw(OWID) << tmp.getY()
   << std::setw(OWID) << tmp.getZ();
    
   tmp=(*it)->getCurrVeloc();
   ofs << std::setw(OWID) << tmp.getX()
   << std::setw(OWID) << tmp.getY()
   << std::setw(OWID) << tmp.getZ();
    
   tmp=(*it)->getCurrOmga();
   ofs << std::setw(OWID) << tmp.getX()
   << std::setw(OWID) << tmp.getY()
   << std::setw(OWID) << tmp.getZ();
    
   tmp=(*it)->getForce();
   ofs << std::setw(OWID) << tmp.getX()
   << std::setw(OWID) << tmp.getY()
   << std::setw(OWID) << tmp.getZ();
    
   tmp=(*it)->getMoment();
   ofs << std::setw(OWID) << tmp.getX()
   << std::setw(OWID) << tmp.getY()
   << std::setw(OWID) << tmp.getZ() << std::endl;
   }
   }
  
   ofs.close();
   }


   // bdrymum = 6 by default
   // the variable existMaxID is important because cavity and container
   // use the same boundaryTgtMap.
   void Assembly::buildCavityBoundary(int existMaxId, const char *boundaryFile)
   {
   std::ofstream ofs(boundaryFile);
   if(!ofs) { debugInf << "stream error: buildCavityBoundary" << std::endl; exit(-1); }

   REAL x1,x2,y1,y2,z1,z2,x0,y0,z0;
   x1 = cavity.getMinCorner().getX();
   y1 = cavity.getMinCorner().getY();
   z1 = cavity.getMinCorner().getZ();
   x2 = cavity.getMaxCorner().getX();
   y2 = cavity.getMaxCorner().getY();
   z2 = cavity.getMaxCorner().getZ();
   x0 = cavity.getCenter().getX();
   y0 = cavity.getCenter().getY();
   z0 = cavity.getCenter().getZ();

   int boundaryNum = 6;

   ofs.setf(std::ios::scientific, std::ios::floatfield);
   ofs << std::setw(OWID) << 0
   << std::setw(OWID) << boundaryNum << std::endl << std::endl;

   // boundary 1
   ofs << std::setw(OWID) << 1 << std::endl
   << std::setw(OWID) << existMaxId + 1
   << std::setw(OWID) << 5
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << -1
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0     
   << std::setw(OWID) << x2
   << std::setw(OWID) << y0
   << std::setw(OWID) << z0     
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << -1
   << std::setw(OWID) << 0
   << std::setw(OWID) << x0    
   << std::setw(OWID) << y1
   << std::setw(OWID) << z0     
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0 
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << x0      
   << std::setw(OWID) << y2
   << std::setw(OWID) << z0     
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0
   << std::setw(OWID) << 1
   << std::setw(OWID) << x0     
   << std::setw(OWID) << y0    
   << std::setw(OWID) << z2 
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 
   << std::setw(OWID) << -1
   << std::setw(OWID) << x0     
   << std::setw(OWID) << y0     
   << std::setw(OWID) << z1
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl << std::endl
    
   // boundary 2
   << std::setw(OWID) << 1 << std::endl
   << std::setw(OWID) << existMaxId + 2
   << std::setw(OWID) << 5
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << -1
   << std::setw(OWID) << 0     
   << std::setw(OWID) << x0    
   << std::setw(OWID) << y2
   << std::setw(OWID) << z0     
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0 
   << std::setw(OWID) << 0
   << std::setw(OWID) << x2
   << std::setw(OWID) << y0
   << std::setw(OWID) << z0     
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << -1
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0
   << std::setw(OWID) << x1 
   << std::setw(OWID) << y0
   << std::setw(OWID) << z0     
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0
   << std::setw(OWID) << 1
   << std::setw(OWID) << x0     
   << std::setw(OWID) << y0    
   << std::setw(OWID) << z2 
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 
   << std::setw(OWID) << -1
   << std::setw(OWID) << x0     
   << std::setw(OWID) << y0      
   << std::setw(OWID) << z1
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl << std::endl
    
   // boundary 3
   << std::setw(OWID) << 1 << std::endl
   << std::setw(OWID) << existMaxId + 3
   << std::setw(OWID) << 5
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0     
   << std::setw(OWID) << x1
   << std::setw(OWID) << y0
   << std::setw(OWID) << z0      
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << -1
   << std::setw(OWID) << 0
   << std::setw(OWID) << x0     
   << std::setw(OWID) << y1
   << std::setw(OWID) << z0      
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0  
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << x0       
   << std::setw(OWID) << y2
   << std::setw(OWID) << z0      
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0
   << std::setw(OWID) << 1
   << std::setw(OWID) << x0      
   << std::setw(OWID) << y0     
   << std::setw(OWID) << z2 
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 
   << std::setw(OWID) << -1
   << std::setw(OWID) << x0      
   << std::setw(OWID) << y0      
   << std::setw(OWID) << z1
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl << std::endl
    
   // boundary 4
   << std::setw(OWID) << 1 << std::endl
   << std::setw(OWID) << existMaxId + 4
   << std::setw(OWID) << 5
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0 
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0     
   << std::setw(OWID) << x0      
   << std::setw(OWID) << y1
   << std::setw(OWID) << z0      
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0 
   << std::setw(OWID) << 0
   << std::setw(OWID) << x2
   << std::setw(OWID) << y0
   << std::setw(OWID) << z0      
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << -1 
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0
   << std::setw(OWID) << x1 
   << std::setw(OWID) << y0
   << std::setw(OWID) << z0      
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0
   << std::setw(OWID) << 1
   << std::setw(OWID) << x0      
   << std::setw(OWID) << y0     
   << std::setw(OWID) << z2 
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 
   << std::setw(OWID) << -1
   << std::setw(OWID) << x0      
   << std::setw(OWID) << y0      
   << std::setw(OWID) << z1
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl << std::endl
    
   // boundary 5
   << std::setw(OWID) << 1 << std::endl
   << std::setw(OWID) << existMaxId + 5
   << std::setw(OWID) << 5
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0 
   << std::setw(OWID) << 0
   << std::setw(OWID) << -1     
   << std::setw(OWID) << x0      
   << std::setw(OWID) << y0
   << std::setw(OWID) << z2 
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0 
   << std::setw(OWID) << 0
   << std::setw(OWID) << x2
   << std::setw(OWID) << y0
   << std::setw(OWID) << z0      
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << -1 
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0
   << std::setw(OWID) << x1 
   << std::setw(OWID) << y0
   << std::setw(OWID) << z0      
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << x0      
   << std::setw(OWID) << y2
   << std::setw(OWID) << z0      
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << -1
   << std::setw(OWID) << 0
   << std::setw(OWID) << x0      
   << std::setw(OWID) << y1
   << std::setw(OWID) << z0
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl << std::endl
    
   // boundary 6
   << std::setw(OWID) << 1 << std::endl
   << std::setw(OWID) << existMaxId + 6
   << std::setw(OWID) << 5
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0 
   << std::setw(OWID) << 0
   << std::setw(OWID) << 1    
   << std::setw(OWID) << x0      
   << std::setw(OWID) << y0
   << std::setw(OWID) << z1
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0 
   << std::setw(OWID) << 0
   << std::setw(OWID) << x2
   << std::setw(OWID) << y0
   << std::setw(OWID) << z0      
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << -1 
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0
   << std::setw(OWID) << x1 
   << std::setw(OWID) << y0
   << std::setw(OWID) << z0      
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << x0      
   << std::setw(OWID) << y2
   << std::setw(OWID) << z0      
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl
    
   << std::setw(OWID) << 1
   << std::setw(OWID) << 0
   << std::setw(OWID) << -1
   << std::setw(OWID) << 0 
   << std::setw(OWID) << x0      
   << std::setw(OWID) << y1
   << std::setw(OWID) << z0
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::endl << std::endl; 

   ofs.close();
   }


   // create boundary particles and springs connecting those boundary particles
   void Assembly::createMemParticle(REAL rRadius,
   bool toRebuild,
   const char *ParticleFile,
   const char *allParticle)
   {
   if (toRebuild) readParticle(ParticleFile);

   REAL radius = gradation.getMinPtclRadius();
   if (gradation.getSize().size() == 1 &&
   gradation.getPtclRatioBA() == 1.0 && 
   gradation.getPtclRatioCA() == 1.0)
   radius *= rRadius; // determine how tiny the boundary particles are
   REAL diameter = radius*2;
   Vec v1 = allContainer.getMinCorner();
   Vec v2 = allContainer.getMaxCorner();
   Vec v0 = allContainer.getCenter();
   REAL x1 = v1.getX();
   REAL y1 = v1.getY();
   REAL z1 = v1.getZ();
   REAL x2 = v2.getX();
   REAL y2 = v2.getY();
   REAL z2 = v2.getZ();
   REAL x0 = v0.getX();
   REAL y0 = v0.getY();
   REAL z0 = v0.getZ();

   Particle* newptcl = NULL;
   REAL x, y, z;
  
   std::vector<Particle*> vec1d;  // 1-dimension
   std::vector< std::vector<Particle*>  > vec2d; // 2-dimension
   Spring* newSpring = NULL;
   int memPtclIndex = trimHistoryNum;
   // process in the order of surfaces: x1 x2 y1 y2 z1 z2
   // surface x1
   x = x1 - radius;
   for (z = z1 + radius; z <= z2 - radius + EPS; z += diameter) {
   vec1d.clear();
   for (y = y1 + radius; y <= y2 - radius + EPS; y += diameter) {
   newptcl = new Particle(++memPtclIndex, 5, Vec(x,y,z), radius, young, poisson);
   vec1d.push_back(newptcl);
   particleVec.push_back(newptcl);
   }
   vec2d.push_back(vec1d);
   }
   memBoundary.push_back(vec2d);
   for (int i = 0; i != vec2d.size() ; ++i)
   for (int j = 0; j != vec2d[i].size() ; ++ j) {
   if (j + 1 < vec2d[i].size() ) {
   newSpring = new Spring(*vec2d[i][j], *vec2d[i][j+1], memYoung);
   springVec.push_back(newSpring);
   }
   if (i + 1 < vec2d.size() ) {
   newSpring = new Spring(*vec2d[i][j], *vec2d[i+1][j], memYoung);
   springVec.push_back(newSpring);
   }
   }

   // surface x2     
   vec2d.clear();
   x = x2 + radius;
   for (z = z1 + radius; z <= z2 - radius + EPS; z += diameter) {
   vec1d.clear();
   for (y = y1 + radius; y <= y2 - radius + EPS; y += diameter) {
   newptcl = new Particle(++memPtclIndex, 5, Vec(x,y,z), radius, young, poisson);
   vec1d.push_back(newptcl);
   particleVec.push_back(newptcl);
   }
   vec2d.push_back(vec1d);
   }
   memBoundary.push_back(vec2d);
   for (int i = 0; i != vec2d.size() ; ++i)
   for (int j = 0; j != vec2d[i].size() ; ++ j) {
   if (j + 1 < vec2d[i].size() ) {
   newSpring = new Spring(*vec2d[i][j], *vec2d[i][j+1], memYoung);
   springVec.push_back(newSpring);
   }
   if (i + 1 < vec2d.size() ) {
   newSpring = new Spring(*vec2d[i][j], *vec2d[i+1][j], memYoung);
   springVec.push_back(newSpring);
   }
   }
 
   // surface y1
   vec2d.clear();
   y = y1 - radius;
   for (z = z1 + radius; z <= z2 - radius + EPS; z += diameter) {
   vec1d.clear();
   for (x = x1 + radius; x <= x2 - radius + EPS; x += diameter) {
   newptcl = new Particle(++memPtclIndex, 5, Vec(x,y,z), radius, young, poisson);
   vec1d.push_back(newptcl);
   particleVec.push_back(newptcl);
   }
   vec2d.push_back(vec1d);
   }
   memBoundary.push_back(vec2d);
   for (int i = 0; i != vec2d.size() ; ++i)
   for (int j = 0; j != vec2d[i].size() ; ++ j) {
   if (j + 1 < vec2d[i].size() ) {
   newSpring = new Spring(*vec2d[i][j], *vec2d[i][j+1], memYoung);
   springVec.push_back(newSpring);
   }
   if (i + 1 < vec2d.size() ) {
   newSpring = new Spring(*vec2d[i][j], *vec2d[i+1][j], memYoung);
   springVec.push_back(newSpring);
   }
   }

   // surface y2
   vec2d.clear();
   y = y2 + radius;
   for (z = z1 + radius; z <= z2 - radius + EPS; z += diameter) {
   vec1d.clear();
   for (x = x1 + radius; x <= x2 - radius + EPS; x += diameter) {
   newptcl = new Particle(++memPtclIndex, 5, Vec(x,y,z), radius, young, poisson);
   vec1d.push_back(newptcl);
   particleVec.push_back(newptcl);
   }
   vec2d.push_back(vec1d);
   }
   memBoundary.push_back(vec2d);
   for (int i = 0; i != vec2d.size() ; ++i)
   for (int j = 0; j != vec2d[i].size() ; ++ j) {
   if (j + 1 < vec2d[i].size() ) {
   newSpring = new Spring(*vec2d[i][j], *vec2d[i][j+1], memYoung);
   springVec.push_back(newSpring);
   }
   if (i + 1 < vec2d.size() ) {
   newSpring = new Spring(*vec2d[i][j], *vec2d[i+1][j], memYoung);
   springVec.push_back(newSpring);
   }
   }

   // surface z1
   vec2d.clear();
   z = z1 - radius;
   for (y = y1 + radius; y <= y2 - radius + EPS; y += diameter) {
   vec1d.clear();
   for (x = x1 + radius; x <= x2 - radius + EPS; x += diameter) {
   newptcl = new Particle(++memPtclIndex, 5, Vec(x,y,z), radius, young, poisson);
   vec1d.push_back(newptcl);
   particleVec.push_back(newptcl);
   }
   vec2d.push_back(vec1d);
   }
   memBoundary.push_back(vec2d);
   for (int i = 0; i != vec2d.size() ; ++i)
   for (int j = 0; j != vec2d[i].size() ; ++ j) {
   if (j + 1 < vec2d[i].size() ) {
   newSpring = new Spring(*vec2d[i][j], *vec2d[i][j+1], memYoung);
   springVec.push_back(newSpring);
   }
   if (i + 1 < vec2d.size() ) {
   newSpring = new Spring(*vec2d[i][j], *vec2d[i+1][j], memYoung);
   springVec.push_back(newSpring);
   }
   }

   // surface z2
   vec2d.clear();
   z = z2 + radius;
   for (y = y1 + radius; y <= y2 - radius + EPS; y += diameter) {
   vec1d.clear();
   for (x = x1 + radius; x <= x2 - radius + EPS; x += diameter) {
   newptcl = new Particle(++memPtclIndex, 5, Vec(x,y,z), radius, young, poisson);
   vec1d.push_back(newptcl);
   particleVec.push_back(newptcl);
   }
   vec2d.push_back(vec1d);
   }
   memBoundary.push_back(vec2d);
   for (int i = 0; i != vec2d.size() ; ++i)
   for (int j = 0; j != vec2d[i].size() ; ++ j) {
   if (j + 1 < vec2d[i].size() ) {
   newSpring = new Spring(*vec2d[i][j], *vec2d[i][j+1], memYoung);
   springVec.push_back(newSpring);
   }
   if (i + 1 < vec2d.size() ) {
   newSpring = new Spring(*vec2d[i][j], *vec2d[i+1][j], memYoung);
   springVec.push_back(newSpring);
   }
   }

   // membrane particles at the edges of each surface, for example,
   // x1y1 means particles on surface x1 connecting to particles on surface y1
   std::vector<Particle*> x1y1;
   std::vector<Particle*> x1y2;
   std::vector<Particle*> x1z1;
   std::vector<Particle*> x1z2;

   std::vector<Particle*> x2y1;
   std::vector<Particle*> x2y2;
   std::vector<Particle*> x2z1;
   std::vector<Particle*> x2z2;

   std::vector<Particle*> y1x1;
   std::vector<Particle*> y1x2;
   std::vector<Particle*> y1z1;
   std::vector<Particle*> y1z2;

   std::vector<Particle*> y2x1;
   std::vector<Particle*> y2x2;
   std::vector<Particle*> y2z1;
   std::vector<Particle*> y2z2;

   std::vector<Particle*> z1x1;
   std::vector<Particle*> z1x2;
   std::vector<Particle*> z1y1;
   std::vector<Particle*> z1y2;

   std::vector<Particle*> z2x1;
   std::vector<Particle*> z2x2;
   std::vector<Particle*> z2y1;
   std::vector<Particle*> z2y2;

   // find edge particles for each surface
   // memBoundary[0, 1, 2, 3, 4, 5] correspond to 
   // surface     x1 x2 y1 y2 z1 z2 respectively
   // surface x1
   vec2d.clear();
   vec2d = memBoundary[0];
   x1z1  = vec2d[0];
   x1z2  = vec2d[vec2d.size() - 1];
   for (int i = 0; i < vec2d.size(); ++i) {
   x1y1.push_back(vec2d[i][0]);
   x1y2.push_back(vec2d[i][ vec2d[i].size() - 1 ]);  
   }
   // surface x2
   vec2d.clear();
   vec2d = memBoundary[1];
   x2z1  = vec2d[0];
   x2z2  = vec2d[vec2d.size() - 1];
   for (int i = 0; i < vec2d.size(); ++i) {
   x2y1.push_back(vec2d[i][0]);
   x2y2.push_back(vec2d[i][ vec2d[i].size() - 1 ]);  
   }
   // surface y1
   vec2d.clear();
   vec2d = memBoundary[2];
   y1z1  = vec2d[0];
   y1z2  = vec2d[vec2d.size() - 1];
   for (int i = 0; i < vec2d.size(); ++i) {
   y1x1.push_back(vec2d[i][0]);
   y1x2.push_back(vec2d[i][ vec2d[i].size() - 1 ]);  
   }
   // surface y2
   vec2d.clear();
   vec2d = memBoundary[3];
   y2z1  = vec2d[0];
   y2z2  = vec2d[vec2d.size() - 1];
   for (int i = 0; i < vec2d.size(); ++i) {
   y2x1.push_back(vec2d[i][0]);
   y2x2.push_back(vec2d[i][ vec2d[i].size() - 1 ]);  
   }
   // surface z1
   vec2d.clear();
   vec2d = memBoundary[4];
   z1y1  = vec2d[0];
   z1y2  = vec2d[vec2d.size() - 1];
   for (int i = 0; i < vec2d.size(); ++i) {
   z1x1.push_back(vec2d[i][0]);
   z1x2.push_back(vec2d[i][ vec2d[i].size() - 1 ]);  
   }
   // surface z2
   vec2d.clear();
   vec2d = memBoundary[5];
   z2y1  = vec2d[0];
   z2y2  = vec2d[vec2d.size() - 1];
   for (int i = 0; i < vec2d.size(); ++i) {
   z2x1.push_back(vec2d[i][0]);
   z2x2.push_back(vec2d[i][ vec2d[i].size() - 1 ]);  
   }

   // create springs connecting 12 edges of a cube
   // 4 edges on surface x1
   assert(x1y1.size() == y1x1.size());
   for (int i = 0; i < x1y1.size(); ++i) {
   newSpring = new Spring(*x1y1[i], *y1x1[i], memYoung);
   springVec.push_back(newSpring);
   }
   assert(x1y2.size() == y2x1.size());
   for (int i = 0; i < x1y2.size(); ++i) {
   newSpring = new Spring(*x1y2[i], *y2x1[i], memYoung);
   springVec.push_back(newSpring);
   }
   assert(x1z1.size() == z1x1.size());
   for (int i = 0; i < x1z1.size(); ++i) {
   newSpring = new Spring(*x1z1[i], *z1x1[i], memYoung);
   springVec.push_back(newSpring);
   }
   assert(x1z2.size() == z2x1.size());
   for (int i = 0; i < x1z2.size(); ++i) {
   newSpring = new Spring(*x1z2[i], *z2x1[i], memYoung);
   springVec.push_back(newSpring);
   }
   // 4 edges on surface x2  
   assert(x2y1.size() == y1x2.size());
   for (int i = 0; i < x2y1.size(); ++i) {
   newSpring = new Spring(*x2y1[i], *y1x2[i], memYoung);
   springVec.push_back(newSpring);
   }
   assert(x2y2.size() == y2x2.size());
   for (int i = 0; i < x2y2.size(); ++i) {
   newSpring = new Spring(*x2y2[i], *y2x2[i], memYoung);
   springVec.push_back(newSpring);
   }
   assert(x2z1.size() == z1x2.size());
   for (int i = 0; i < x2z1.size(); ++i) {
   newSpring = new Spring(*x2z1[i], *z1x2[i], memYoung);
   springVec.push_back(newSpring);
   }
   assert(x2z2.size() == z2x2.size());
   for (int i = 0; i < x2z2.size(); ++i) {
   newSpring = new Spring(*x2z2[i], *z2x2[i], memYoung);
   springVec.push_back(newSpring);
   }
   // 2 edges on surface y1 
   assert(y1z1.size() == z1y1.size());
   for (int i = 0; i < y1z1.size(); ++i) {
   newSpring = new Spring(*y1z1[i], *z1y1[i], memYoung);
   springVec.push_back(newSpring);
   }
   assert(y1z2.size() == z2y1.size());
   for (int i = 0; i < y1z2.size(); ++i) {
   newSpring = new Spring(*y1z2[i], *z2y1[i], memYoung);
   springVec.push_back(newSpring);
   }
   // 2 edges on surface y2
   assert(y2z1.size() == z1y2.size());
   for (int i = 0; i < y2z1.size(); ++i) {
   newSpring = new Spring(*y2z1[i], *z1y2[i], memYoung);
   springVec.push_back(newSpring);
   }
   assert(y2z2.size() == z2y2.size());
   for (int i = 0; i < y2z2.size(); ++i) {
   newSpring = new Spring(*y2z2[i], *z2y2[i], memYoung);
   springVec.push_back(newSpring);
   }

   printParticle(allParticle);

   }


   void Assembly::TrimPtclBdryByHeight(REAL height,
   const char *iniptclfile,
   const char *ParticleFile)
   {
   readParticle(iniptclfile);
  
   std::vector<Particle*>::iterator itr;
   for (itr = particleVec.begin(); itr != particleVec.end(); ) {
   if ( (*itr)->getType() == 1 ) { // 1-fixed
   Vec center=(*itr)->getCurrPos();
   if(center.getZ() > height)
   {
   delete (*itr); // release memory
   itr = particleVec.erase(itr); 
   }
   else {
   (*itr)->setType(10); // 10-ghost
   ++itr;
   }
   }
   }
  
   printParticle(ParticleFile);
   }


   void Assembly::deGravitation(int   totalSteps,  
   int   snapNum,
   int   interval,
   bool  toRebuild,
   const char *iniptclfile,   
   const char *ParticleFile, 
   const char *contactfile,
   const char *progressfile, 
   const char *debugfile)
   {
   // pre_1: open streams for output.
   progressInf.open(progressfile); 
   if(!progressInf) { debugInf << "stream error: deGravitation" << std::endl; exit(-1); }
   progressInf.setf(std::ios::scientific, std::ios::floatfield);
   progressInf.precision(OPREC);
   progressInf << std::setw(OWID) << "iteration"
   << std::setw(OWID) << "poss_contact"
   << std::setw(OWID) << "actual_contact"
   << std::setw(OWID) << "penetration"
   << std::setw(OWID) << "avg_normal"
   << std::setw(OWID) << "avg_tangt"
   << std::setw(OWID) << "avg_velocity"
   << std::setw(OWID) << "avg_omga"
   << std::setw(OWID) << "avg_force"
   << std::setw(OWID) << "avg_moment"
   << std::setw(OWID) << "trans_energy"
   << std::setw(OWID) << "rotat_energy"
   << std::setw(OWID) << "kinet_energy"
   << std::endl;
  
   debugInf.open(debugfile);
   if(!debugInf) { debugInf << "stream error: deGravitation" << std::endl; exit(-1); }
   debugInf.setf(std::ios::scientific, std::ios::floatfield);
  
   // pre_2. create particles from existing files.
   if (toRebuild) readParticle(iniptclfile); // create container and particles, velocity and omga are set zero. 
  
   // pre_3. define variables used in iterations
   REAL avgNormal=0;
   REAL avgTangt=0;
   int  stepsnum=0;
   char stepsstr[4];
   char stepsfp[50];

   // iterations starting ...
   iteration=0; 
   do
   {
   // 1. find contacts between particles.
   findContact();
      
   // 2. set particles' forces/moments as zero before each re-calculation,
   clearContactForce();	
      
   // 3. calculate contact forces/moments and apply them to particles.
   internalForce(avgNormal, avgTangt);
      
   // 4. update particles' velocity/omga/position/orientation based on force/moment.
   updateParticle();
      
   // 5. (1) output particles and contacts information as snapNum.
   if (iteration % (totalSteps/snapNum) == 0) {
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp, ParticleFile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printParticle(stepsfp);
	
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp, contactfile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printContact(stepsfp);
   time(&timeStamp);
   g_timeinf << std::setw(4) << "end" << " " << ctime(&timeStamp) << std::flush;
   ++stepsnum;
   }
      
   // 5. (2) output stress and strain info.
   if (iteration % interval == 0) {
   progressInf << std::setw(OWID) << iteration
   << std::setw(OWID) << getPossContactNum()
   << std::setw(OWID) << getActualContactNum()
   << std::setw(OWID) << getAvgPenetration()
   << std::setw(OWID) << avgNormal
   << std::setw(OWID) << avgTangt
   << std::setw(OWID) << getAvgVelocity() 
   << std::setw(OWID) << getAvgOmga()
   << std::setw(OWID) << getAvgForce()   
   << std::setw(OWID) << getAvgMoment()
   << std::setw(OWID) << getTransEnergy()
   << std::setw(OWID) << getRotatEnergy()
   << std::setw(OWID) << getKinetEnergy()
   << std::endl;
   }
      
   // 7. loop break conditions.
   if (contactVec.size() == 0) break;
      
   } while (++iteration < totalSteps);
  
   // post_1. store the final snapshot of particles & contacts.
   strcpy(stepsfp, ParticleFile); strcat(stepsfp, "_end");
   printParticle(stepsfp);
  
   strcpy(stepsfp, contactfile); strcat(stepsfp, "_end");
   printContact(stepsfp);
   g_timeinf << std::setw(4) << "end" << " " << ctime(&timeStamp) << std::flush;
  
   // post_2. close streams
   progressInf.close();
   debugInf.close();
   }


   // actual deposit function for the case of fixed particle boundaries
   void Assembly::deposit_p(int   totalSteps,  
   int   snapNum,
   int   interval,
   REAL dimn,
   REAL rsize,
   const char *iniptclfile,   
   const char *ParticleFile, 
   const char *contactfile,
   const char *progressfile, 
   const char *debugfile)
   {
   // pre_1: open streams for output.
   // ParticleFile and contactfile are used for snapNum at the end.
   progressInf.open(progressfile); 
   if(!progressInf) { debugInf << "stream error: deposit_p" << std::endl; exit(-1); }
   progressInf.setf(std::ios::scientific, std::ios::floatfield);
   progressInf << "deposit..." << std::endl
   << "     iteration possible  actual      average	    average         average         average"
   << "         average         average         average       translational    rotational       "
   << "kinetic        potential         total           void            sample       coordination"
   << "       sample           sample          sample          sample          sample          sample"
   << "          sample          sample          sample         sample           sample         "
   << " sample          sample          sample          sample          sample" << std::endl
   << "       number  contacts contacts   penetration   contact_normal  contact_tangt     velocity"
   << "         omga            force           moment         energy           energy          "
   << "energy         energy            energy          ratio          porosity         number       "
   << "   density         sigma1_1        sigma1_2        sigma2_1        sigma2_2        "
   << "sigma3_1        sigma3_2           p             width          length           "
   << "height          volume         epsilon_w       epsilon_l       epsilon_h       "
   << "epsilon_v" << std::endl;

   debugInf.open(debugfile);
   if(!debugInf) { debugInf << "stream error: deposit_p" << std::endl; exit(-1); }
   debugInf.setf(std::ios::scientific, std::ios::floatfield);

   // pre_2. create particles and boundaries from existing files.
   readParticle(iniptclfile); // create container and particles, velocity and omga are set zero. 

   // pre_3: define variables used in iterations.
   REAL l13, l24, l56;
   REAL avgNormal=0;
   REAL avgTangt=0;
   int         stepsnum=0;
   char        stepsstr[4];
   char        stepsfp[50];
   REAL void_ratio=0;

   // iterations starting ...
   iteration=0;
   do
   {
   // 1. create possible boundary particles and contacts between particles.
   findContact();

   // 2. set particles' forces/moments as zero before each re-calculation,
   clearContactForce();	

   // 3. calculate contact forces/moments and apply them to particles.
   internalForce(avgNormal, avgTangt);

   // 4. update particles' velocity/omga/position/orientation based on force/moment.
   updateParticle();

   // 5. calculate specimen void ratio.
   l56=getTopFreeParticlePosition().getZ() - (-dimn/2);
   l24=dimn*rsize;
   l13=dimn*rsize;
   bulkVolume=l13*l24*l56;
   void_ratio=bulkVolume/getParticleVolume()-1;

   // 6. (1) output particles and contacts information as snapNum.
   if (iteration % (totalSteps/snapNum) == 0) {
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp, ParticleFile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printParticle(stepsfp);
	    
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp, contactfile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printContact(stepsfp);
   ++stepsnum;
   }

   // 6. (2) output statistics info.
   if (iteration % interval == 0) {
   REAL t1=getTransEnergy();
   REAL t2=getRotatEnergy();
   REAL t3=getPotenEnergy(-0.025);
   progressInf << std::setw(OWID) << iteration
   << std::setw(OWID) << getPossContactNum()
   << std::setw(OWID) << getActualContactNum()
   << std::setw(OWID) << getAvgPenetration()
   << std::setw(OWID) << avgNormal
   << std::setw(OWID) << avgTangt
   << std::setw(OWID) << getAvgVelocity() 
   << std::setw(OWID) << getAvgOmga()
   << std::setw(OWID) << getAvgForce()   
   << std::setw(OWID) << getAvgMoment()
   << std::setw(OWID) << t1
   << std::setw(OWID) << t2
   << std::setw(OWID) << (t1+t2)
   << std::setw(OWID) << t3
   << std::setw(OWID) << (t1+t2+t3)
   << std::setw(OWID) << void_ratio
   << std::setw(OWID) << void_ratio/(1+void_ratio)
   << std::setw(OWID) << 2.0*getActualContactNum()/allParticleVec.size()
   << std::endl;
   }

   // 7. loop break conditions.


   } while (++iteration < totalSteps);
    
   // post_1. store the final snapshot of particles & contacts.
   strcpy(stepsfp, ParticleFile); strcat(stepsfp, "_end");
   printParticle(stepsfp);

   strcpy(stepsfp, contactfile); strcat(stepsfp, "_end");
   printContact(stepsfp);

   // post_2. close streams
   progressInf.close();
   debugInf.close();
   }


   // squeeze paticles inside a container by moving the boundaries
   void Assembly::squeeze(int   totalSteps,  
   int   init_steps,
   int   snapNum,
   int   interval,
   int   flag,
   const char *iniptclfile,   
   const char *inibdryfile,
   const char *ParticleFile, 
   const char *boundaryfile,
   const char *contactfile,
   const char *progressfile, 
   const char *debugfile)
   {
   // pre_1: open streams for output.
   // ParticleFile and contactfile are used for snapNum at the end.
   progressInf.open(progressfile); 
   if(!progressInf) { debugInf << "stream error: squeeze" << std::endl; exit(-1); }
   progressInf.setf(std::ios::scientific, std::ios::floatfield);
   progressInf << "deposit..." << std::endl
   << "     iteration possible  actual      average	    average         average         average"
   << "         average         average         average       translational    rotational       "
   << "kinetic        potential         total           void            sample       coordination"
   << "       sample           sample          sample          sample          sample          sample"
   << "          sample          sample          sample         sample           sample         "
   << " sample          sample          sample          sample          sample" << std::endl
   << "       number  contacts contacts   penetration   contact_normal  contact_tangt     velocity"
   << "         omga            force           moment         energy           energy          "
   << "energy         energy            energy          ratio          porosity         number       "
   << "   density         sigma1_1        sigma1_2        sigma2_1        sigma2_2        "
   << "sigma3_1        sigma3_2           p             width          length           "
   << "height          volume         epsilon_w       epsilon_l       epsilon_h       "
   << "epsilon_v" << std::endl;

   debugInf.open(debugfile);
   if(!debugInf) { debugInf << "stream error: squeeze" << std::endl; exit(-1); }
   debugInf.setf(std::ios::scientific, std::ios::floatfield);

   // pre_2. create particles and boundaries from existing files.
   readParticle(iniptclfile); // create container and particles, velocity and omga are set zero. 
   readBoundary(inibdryfile);   // create boundaries.

   // pre_3: define variables used in iterations.
   REAL l13, l24, l56;
   REAL avgNormal=0;
   REAL avgTangt=0;
   int         stepsnum=0;
   char        stepsstr[4];
   char        stepsfp[50];

   int         mid[2]={1,3};    // boundary 1 and 3
   UPDATECTL   midctl[2];
   REAL void_ratio=0;
   REAL bdry_penetr[7];
   int         bdry_cntnum[7];
   for (int i=0;i<7;++i) {
   bdry_penetr[i]=0; bdry_cntnum[i]=0;
   }

   // iterations starting ...
   iteration=0;
   do
   {
   // 1. create possible boundary particles and contacts between particles.
   findContact();
   findBdryContact();

   // 2. set particles' forces/moments as zero before each re-calculation,
   clearContactForce();	

   // 3. calculate contact forces/moments and apply them to particles.
   internalForce(avgNormal, avgTangt);

   // 4. calculate boundary forces/moments and apply them to particles.
   boundaryForce(bdry_penetr, bdry_cntnum);
	
   // 5. update particles' velocity/omga/position/orientation based on force/moment.
   updateParticle();

   // 6. calculate sample void ratio.
   l56=getTopFreeParticlePosition().getZ() -getApt(6).getZ();
   l24=getApt(2).getY()-getApt(4).getY();
   l13=getApt(1).getX()-getApt(3).getX(); bulkVolume=l13*l24*l56;
   void_ratio=bulkVolume/getParticleVolume()-1;

   // displacement control
   if (iteration > init_steps) {
   if (flag==1) // loosen, totally remove the wall
   midctl[0].tran=Vec(timeStep*1.0e+0*flag,0,0);
   else         // squeeze
   midctl[0].tran=Vec(timeStep*5.0e-3*flag,0,0);
   updateBoundary(mid,midctl,2);
   }

   // 7. (1) output particles and contacts information as snapNum.
   if (iteration % (totalSteps/snapNum) == 0) {
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp, ParticleFile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printParticle(stepsfp);
	    
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp, contactfile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printContact(stepsfp);
   ++stepsnum;
   }

   // 7. (2) output stress and strain info.
   if (iteration % interval == 0) {
   REAL t1=getTransEnergy();
   REAL t2=getRotatEnergy();
   REAL t3=getPotenEnergy(-0.025);
   progressInf << std::setw(OWID) << iteration
   << std::setw(OWID) << getPossContactNum()
   << std::setw(OWID) << getActualContactNum()
   << std::setw(OWID) << getAvgPenetration()
   << std::setw(OWID) << avgNormal
   << std::setw(OWID) << avgTangt
   << std::setw(OWID) << getAvgVelocity() 
   << std::setw(OWID) << getAvgOmga()
   << std::setw(OWID) << getAvgForce()   
   << std::setw(OWID) << getAvgMoment()
   << std::setw(OWID) << t1
   << std::setw(OWID) << t2
   << std::setw(OWID) << (t1+t2)
   << std::setw(OWID) << t3
   << std::setw(OWID) << (t1+t2+t3)
   << std::setw(OWID) << void_ratio
   << std::setw(OWID) << void_ratio/(1+void_ratio)
   << std::setw(OWID) << 2.0*(getActualContactNum()
   +bdry_cntnum[1]+bdry_cntnum[2]+bdry_cntnum[3]
   +bdry_cntnum[4]+bdry_cntnum[6])/allParticleVec.size()
   << std::endl;
   debugInf << std::setw(OWID) << iteration
   << std::setw(OWID) << bdry_penetr[1]
   << std::setw(OWID) << bdry_penetr[2]
   << std::setw(OWID) << bdry_penetr[3]
   << std::setw(OWID) << bdry_penetr[4]
   << std::setw(OWID) << bdry_penetr[6]
   << std::setw(OWID) << bdry_cntnum[1]
   << std::setw(OWID) << bdry_cntnum[2]
   << std::setw(OWID) << bdry_cntnum[3]
   << std::setw(OWID) << bdry_cntnum[4]
   << std::setw(OWID) << bdry_cntnum[6]
   << std::endl;

   }

   // 8. loop break conditions.

   } while (++iteration < totalSteps);
    
   // post_1. store the final snapshot of particles & contacts.
   strcpy(stepsfp, ParticleFile); strcat(stepsfp, "_end");
   printParticle(stepsfp);

   strcpy(stepsfp, contactfile); strcat(stepsfp, "_end");
   printContact(stepsfp);

   strcpy(stepsfp, boundaryfile); strcat(stepsfp, "_end");
   printBoundary(stepsfp);

   // post_2. close streams
   progressInf.close();
   debugInf.close();
   }


   void Assembly::iso_MemBdry(int   totalSteps,  
   int   snapNum, 
   int   interval,
   REAL  sigma3,
   REAL  rRadius,
   bool  toRebuild,
   const char *iniptclfile, 
   const char *ParticleFile,
   const char *contactfile, 
   const char *progressfile,
   const char *debugfile) 
   {
   // pre_1: open streams for output
   // ParticleFile and contactfile are used for snapNum at the end.
   progressInf.open(progressfile);
   if(!progressInf) { debugInf << "stream error: isoMemBdry" << std::endl; exit(-1);}
   progressInf.setf(std::ios::scientific, std::ios::floatfield);
   progressInf.precision(OPREC);
   progressInf << std::setw(OWID) << "iteration"
   << std::setw(OWID) << "poss_contact"
   << std::setw(OWID) << "actual_contact"
   << std::setw(OWID) << "penetration"
   << std::setw(OWID) << "avg_normal"
   << std::setw(OWID) << "avg_tangt"
   << std::setw(OWID) << "avg_velocity"
   << std::setw(OWID) << "avg_omga"
   << std::setw(OWID) << "avg_force"
   << std::setw(OWID) << "avg_moment"
   << std::setw(OWID) << "trans_energy"
   << std::setw(OWID) << "rotat_energy"
   << std::setw(OWID) << "kinet_energy"
   << std::endl;
  
   debugInf.open(debugfile);
   if(!debugInf) { debugInf << "stream error: isoMemBdry" << std::endl; exit(-1);}
   debugInf.setf(std::ios::scientific, std::ios::floatfield);
  
   // pre_2. create particles from file and calculate forces caused by hydraulic pressure
   if (toRebuild) readParticle(iniptclfile);
   REAL radius = gradation.getMinPtclRadius();
   if (gradation.getSize().size() == 1 &&
   gradation.getPtclRatioBA() == 1.0 && 
   gradation.getPtclRatioCA() == 1.0)
   radius *= rRadius; // determine how tiny the boundary particles are
   REAL mag  = radius*radius*4*sigma3;
   Vec v1 = allContainer.getMinCorner();
   Vec v2 = allContainer.getMaxCorner();
   REAL x1 = v1.getX();
   REAL y1 = v1.getY();
   REAL z1 = v1.getZ();
   REAL x2 = v2.getX();
   REAL y2 = v2.getY();
   REAL z2 = v2.getZ();
   std::vector<Particle*>::const_iterator  it;
   Vec pos;
   for (it=particleVec.begin();it!=particleVec.end();++it)
   {
   pos = (*it)->getCurrPos();
   if (pos.getX() < x1)
   (*it)->setConstForce( Vec(mag, 0, 0) );
   else if (pos.getX() > x2)
   (*it)->setConstForce( Vec(-mag, 0, 0) );
   else if (pos.getY() < y1)
   (*it)->setConstForce( Vec(0, mag, 0) );
   else if (pos.getY() > y2)
   (*it)->setConstForce( Vec(0, -mag, 0) );
   else if (pos.getZ() < z1)
   (*it)->setConstForce( Vec(0, 0, mag) );
   else if (pos.getZ() > z2)
   (*it)->setConstForce( Vec(0, 0, -mag) );
   }

   // pre_3. define variables used in iterations
   REAL avgNormal=0;
   REAL avgTangt=0;
   int  stepsnum=0;
   char stepsstr[4];
   char stepsfp[50];
  
   // iterations start here...
   iteration=0;
   do 
   {
   // 1. find contacts between particles
   findContact();

   // 2. set particles forces/moments to zero before each re-calculation
   clearContactForce(); // const_force/moment NOT cleared by this call	
      
   // 3. calculate contact forces/moments and apply them to particles
   internalForce(avgNormal, avgTangt);

   // 4. calculate and apply spring forces to boundary particles
   springForce();
      
   // 5. update particles velocity/omga/position/orientation based on force/moment
   updateParticle();
      
   // 6. (1) output particles and contacts information
   if (iteration % (totalSteps/snapNum) == 0) {
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp,ParticleFile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printParticle(stepsfp);

   strcpy(stepsfp,"iso_membrane_"); strcat(stepsfp, stepsstr);
   printMemParticle(stepsfp);
   strcpy(stepsfp,"iso_spring_"); strcat(stepsfp, stepsstr); strcat(stepsfp, ".dat");
   plotSpring(stepsfp);
	
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp, contactfile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr); 
   printContact(stepsfp);
   time(&timeStamp);
   g_timeinf << std::setw(4) << stepsnum << " " << ctime(&timeStamp) << std::flush;
   ++stepsnum;
   }
      
   // 6. (2) output stress and strain info
   if (iteration % interval == 0 ) {
   progressInf << std::setw(OWID) << iteration
   << std::setw(OWID) << getPossContactNum()
   << std::setw(OWID) << getActualContactNum()
   << std::setw(OWID) << getAvgPenetration()
   << std::setw(OWID) << avgNormal
   << std::setw(OWID) << avgTangt
   << std::setw(OWID) << getAvgVelocity() 
   << std::setw(OWID) << getAvgOmga()
   << std::setw(OWID) << getAvgForce()   
   << std::setw(OWID) << getAvgMoment()
   << std::setw(OWID) << getTransEnergy()
   << std::setw(OWID) << getRotatEnergy()
   << std::setw(OWID) << getKinetEnergy()
   << std::endl;
   }
	  
   } while (++iteration < totalSteps);
  
   // post_1. store the final snapshot of particles, contacts and boundaries.
   strcpy(stepsfp, ParticleFile); strcat(stepsfp, "_end");
   printParticle(stepsfp);

   strcpy(stepsfp, "iso_membrane_end");
   printMemParticle(stepsfp);
   strcpy(stepsfp, "iso_spring_end.dat");
   plotSpring(stepsfp);  

   strcpy(stepsfp, contactfile); strcat(stepsfp, "_end");
   printContact(stepsfp);
   g_timeinf << std::setw(4) << "end" << " " << ctime(&timeStamp) << std::flush;
  
   // post_2. close streams
   progressInf.close();
   debugInf.close();
   }


   // This function initializes triaxial sample to a certain confining pressure.
   void Assembly::triaxialPtclBdryIni(int   totalSteps,  
   int   snapNum, 
   int   interval,
   REAL  sigma,
   const char *iniptclfile, 
   const char *inibdryfile,
   const char *ParticleFile,
   const char *boundaryfile,
   const char *contactfile, 
   const char *progressfile,
   const char *debugfile) 
   {
   // pre_1: open streams for output
   // ParticleFile and contactfile are used for snapNum at the end.
   progressInf.open(progressfile);
   if(!progressInf) { debugInf << "stream error: triaxialPtclBdryIni" << std::endl; exit(-1);}
   progressInf.setf(std::ios::scientific, std::ios::floatfield);
   progressInf << "triaxial..." << std::endl
   << "     iteration possible  actual      average	    average         average         average"
   << "         average         average         average        sample            sample     "
   << "     sample          sample          sample          sample          sample          "
   << "sample          sample         sample           sample          sample          sample     "
   << "     sample          sample          sample          void            sample        coordinate"
   << std::endl
   << "       number  contacts contacts   penetration   contact_normal  contact_tangt     velocity"
   << "          omga            force           moment        density          "
   << "sigma1_1        sigma1_2        sigma2_1        sigma2_2        "
   << "sigma3_1        sigma3_2           p             width          length           "
   << "height          volume         epsilon_w       epsilon_l       epsilon_h       epsilon_v"
   << "        ratio          porosity         number"
   << std::endl;

   debugInf.open(debugfile);
   if(!debugInf) { debugInf << "stream error: triaxialPtclBdryIni" << std::endl; exit(-1);}
   debugInf.setf(std::ios::scientific, std::ios::floatfield);

   // pre_2. create particles and boundaries from files
   readParticle(iniptclfile); // create container and particles, velocity and omga are set zero. 
   readBoundary(inibdryfile);   // create boundaries

   // pre_3. define variables used in iterations
   REAL H0 = getApt(5).getZ()-getApt(6).getZ();
   REAL l56= 0;
   REAL sigma3_1, sigma3_2;
   REAL epsilon_h;
   REAL avgNormal=0;
   REAL avgTangt=0;
   int         stepsnum=0;
   char        stepsstr[4];
   char        stepsfp[50];
    
   int         min[2]={5,6};    // boundary 5 and 6
   UPDATECTL   minctl[2];

   // iterations start here...
   iteration=0;
   do
   {
   // 1. create possible boundary particles and contacts between particles
   findContact();
   findBdryContact();

   // 2. set particles' forces/moments as zero before each re-calculation
   clearContactForce();	

   // 3. calculate contact forces/moments and apply them to particles
   internalForce(avgNormal, avgTangt);
	
   // 4. calculate boundary forces/moments and apply them to particles
   boundaryForce();

   // 5. update particles' velocity/omga/position/orientation based on force/moment
   updateParticle();
	
   // 6. update boundaries' position and orientation
   sigma3_1=vfabs(getNormalForce(5))/2.5e-3; sigma3_2=vfabs(getNormalForce(6))/2.5e-3;

   // force control
   if (sigma3_1 < sigma)
   minctl[0].tran=Vec(0,0,-timeStep*boundaryRate);
   else
   minctl[0].tran=Vec(0,0, timeStep*boundaryRate);

   if (sigma3_2 < sigma)
   minctl[1].tran=Vec(0,0, timeStep*boundaryRate);
   else
   minctl[1].tran=Vec(0,0,-timeStep*boundaryRate);

   updateBoundary(min,minctl,2);
	
   // 7. (1) output particles and contacts information
   if (iteration % (totalSteps/snapNum) == 0) {
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp,ParticleFile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printParticle(stepsfp);
	    
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp, contactfile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printContact(stepsfp);
   ++stepsnum;
   }

   // 7. (2) output stress and strain info
   l56=getApt(5).getZ()-getApt(6).getZ();
   epsilon_h = (H0-l56)/H0;
   if (iteration % interval == 0 ) {
   progressInf << std::setw(OWID) << iteration
   << std::setw(OWID) << getPossContactNum()
   << std::setw(OWID) << getActualContactNum()
   << std::setw(OWID) << getAvgPenetration()
   << std::setw(OWID) << avgNormal
   << std::setw(OWID) << avgTangt
   << std::setw(OWID) << getAvgVelocity() 
   << std::setw(OWID) << getAvgOmga()
   << std::setw(OWID) << getAvgForce()   
   << std::setw(OWID) << getAvgMoment()
   << std::setw(OWID) << getDensity()
   << std::setw(OWID) << 0 << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::setw(OWID) << 0
   << std::setw(OWID) << sigma3_1 << std::setw(OWID) << sigma3_2
   << std::setw(OWID) << getAvgPressure()
   << std::setw(OWID) << 0 << std::setw(OWID) << 0 << std::setw(OWID) << l56
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0
   << std::setw(OWID) << epsilon_h
   << std::setw(OWID) << epsilon_h
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0
   << std::setw(OWID) << 2.0*getActualContactNum()/allParticleVec.size()
   << std::endl;

   }

   // 9. loop break condition: through displacement control mechanism
   if (   fabs(sigma3_1-sigma)/sigma < boundaryStressTol && fabs(sigma3_2-sigma)/sigma < boundaryStressTol )
   break;
	
   } while (++iteration < totalSteps);

   // post_1. store the final snapshot of particles, contacts and boundaries.
   strcpy(stepsfp, ParticleFile); strcat(stepsfp, "_end");
   printParticle(stepsfp);

   strcpy(stepsfp, contactfile); strcat(stepsfp, "_end");
   printContact(stepsfp);

   strcpy(stepsfp, boundaryfile); strcat(stepsfp, "_end");
   printBoundary(stepsfp);
    
   // post_2. close streams
   progressInf.close();
   debugInf.close();
   }


   // This function performs triaxial compression test.
   // Displacement boundaries are used in axial direction.
   void Assembly::triaxialPtclBdry(int   totalSteps,  
   int   snapNum, 
   int   interval,
   const char *iniptclfile, 
   const char *inibdryfile,
   const char *ParticleFile,
   const char *boundaryfile,
   const char *contactfile, 
   const char *progressfile,
   const char *balancedfile,
   const char *debugfile) 
   {
   // pre_1: open streams for output
   // ParticleFile and contactfile are used for snapNum at the end.
   progressInf.open(progressfile);
   if(!progressInf) { debugInf << "stream error: triaxialPtclBdry" << std::endl; exit(-1);}
   progressInf.setf(std::ios::scientific, std::ios::floatfield);
   progressInf << "triaxial..." << std::endl
   << "     iteration possible  actual      average	    average         average         average"
   << "         average         average         average        sample            sample     "
   << "     sample          sample          sample          sample          sample          "
   << "sample          sample         sample           sample          sample          sample     "
   << "     sample          sample          sample          void            sample        coordinate"
   << std::endl
   << "       number  contacts contacts   penetration   contact_normal  contact_tangt     velocity"
   << "          omga            force           moment        density          "
   << "sigma1_1        sigma1_2        sigma2_1        sigma2_2        "
   << "sigma3_1        sigma3_2           p             width          length           "
   << "height          volume         epsilon_w       epsilon_l       epsilon_h       epsilon_v"
   << "        ratio          porosity         number"
   << std::endl;

   std::ofstream balancedinf(balancedfile);
   if(!balancedinf) { debugInf << "stream error: triaxialPtclBdry" << std::endl; exit(-1);}
   balancedinf.setf(std::ios::scientific, std::ios::floatfield);
   balancedinf << "triaxial..." << std::endl
   << "     iteration possible  actual      average	    average         average         average"
   << "         average         average         average        sample            sample     "
   << "     sample          sample          sample          sample          sample          "
   << "sample          sample         sample           sample          sample          sample     "
   << "     sample          sample          sample          void            sample        coordinate"
   << std::endl
   << "       number  contacts contacts   penetration   contact_normal  contact_tangt     velocity"
   << "          omga            force           moment        density          "
   << "sigma1_1        sigma1_2        sigma2_1        sigma2_2        "
   << "sigma3_1        sigma3_2           p             width          length           "
   << "height          volume         epsilon_w       epsilon_l       epsilon_h       epsilon_v"
   << "        ratio          porosity         number"
   << std::endl;

   debugInf.open(debugfile);
   if(!debugInf) { debugInf << "stream error: triaxialPtclBdry" << std::endl; exit(-1);}
   debugInf.setf(std::ios::scientific, std::ios::floatfield);

   // pre_2. create particles and boundaries from files
   readParticle(iniptclfile); // create container and particles, velocity and omga are set zero. 
   readBoundary(inibdryfile);   // create boundaries

   // pre_3. define variables used in iterations
   REAL H0 = getApt(5).getZ()-getApt(6).getZ();
   REAL l56= 0;
   REAL sigma3_1, sigma3_2;
   REAL epsilon_h;
   REAL avgNormal=0;
   REAL avgTangt=0;
   int         stepsnum=0;
   char        stepsstr[4];
   char        stepsfp[50];
    
   int         min[2]={5,6};    // boundary 5 and 6
   UPDATECTL   minctl[2];

   // iterations start here...
   iteration=0;
   do
   {
   // 1. create possible boundary particles and contacts between particles
   findContact();
   findBdryContact();

   // 2. set particles' forces/moments as zero before each re-calculation
   clearContactForce();	

   // 3. calculate contact forces/moments and apply them to particles
   internalForce(avgNormal, avgTangt);
	
   // 4. calculate boundary forces/moments and apply them to particles
   boundaryForce();

   // 5. update particles' velocity/omga/position/orientation based on force/moment
   updateParticle();
	
   // 6. update boundaries' position and orientation
   sigma3_1=vfabs(getNormalForce(5))/2.5e-3; sigma3_2=vfabs(getNormalForce(6))/2.5e-3;

   // displacement control
   if(iteration < 100001) {
   minctl[0].tran=Vec(0,0,-timeStep*boundaryRate);
   minctl[1].tran=Vec(0,0, timeStep*boundaryRate);

   updateBoundary(min,minctl,2);
   }
   // 7. (1) output particles and contacts information
   if (iteration % (totalSteps/snapNum) == 0) {
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp,ParticleFile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printParticle(stepsfp);
	    
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp, contactfile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printContact(stepsfp);
   ++stepsnum;
   }

   // 7. (2) output stress and strain info
   l56=getApt(5).getZ()-getApt(6).getZ();
   epsilon_h = (H0-l56)/H0;
   if (iteration % interval == 0 ) {
   progressInf << std::setw(OWID) << iteration
   << std::setw(OWID) << getPossContactNum()
   << std::setw(OWID) << getActualContactNum()
   << std::setw(OWID) << getAvgPenetration()
   << std::setw(OWID) << avgNormal
   << std::setw(OWID) << avgTangt
   << std::setw(OWID) << getAvgVelocity() 
   << std::setw(OWID) << getAvgOmga()
   << std::setw(OWID) << getAvgForce()   
   << std::setw(OWID) << getAvgMoment()
   << std::setw(OWID) << getDensity()
   << std::setw(OWID) << 0 << std::setw(OWID) << 0
   << std::setw(OWID) << 0 << std::setw(OWID) << 0
   << std::setw(OWID) << sigma3_1 << std::setw(OWID) << sigma3_2
   << std::setw(OWID) << getAvgPressure()
   << std::setw(OWID) << 0 << std::setw(OWID) << 0 << std::setw(OWID) << l56
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0
   << std::setw(OWID) << epsilon_h
   << std::setw(OWID) << epsilon_h
   << std::setw(OWID) << 0
   << std::setw(OWID) << 0
   << std::setw(OWID) << 2.0*getActualContactNum()/allParticleVec.size()
   << std::endl;

   }

   // 9. loop break condition: through displacement control mechanism
	
   } while (++iteration < totalSteps);

   // post_1. store the final snapshot of particles, contacts and boundaries.
   strcpy(stepsfp, ParticleFile); strcat(stepsfp, "_end");
   printParticle(stepsfp);

   strcpy(stepsfp, contactfile); strcat(stepsfp, "_end");
   printContact(stepsfp);

   strcpy(stepsfp, boundaryfile); strcat(stepsfp, "_end");
   printBoundary(stepsfp);
    
   // post_2. close streams
   progressInf.close();
   balancedinf.close();
   debugInf.close();
   }


   // The specimen has been deposited with gravitation within boundaries composed of particles.
   // A rectangular pile is then drived into the particles using displacement control.
   void Assembly::rectPile_Disp(int   totalSteps,  
   int   snapNum, 
   int   interval,
   const char *iniptclfile,  
   const char *inibdryfile,
   const char *ParticleFile, 
   const char *boundaryfile,
   const char *contactfile,  
   const char *progressfile,
   const char *debugfile) 
   {
   // pre_1: open streams for output
   // ParticleFile and contactfile are used for snapNum at the end.
   progressInf.open(progressfile); 
   if(!progressInf) { debugInf << "stream error:recPile_Disp" << std::endl; exit(-1); }
   progressInf.setf(std::ios::scientific, std::ios::floatfield);
   progressInf << "pile penetrate..." << std::endl
   << "     iteration possible  actual      average	    average         average         average"
   << "         average         average         average       translational    rotational       "
   << "kinetic        potential        total           sample           sample     "
   << "     sample          sample          sample          sample          sample          "
   << "sample          sample         sample           sample          sample          sample"
   << "          sample          sample          sample" << std::endl
   << "       number  contacts contacts   penetration   contact_normal  contact_tangt     velocity"
   << "         omga            force           moment         energy           energy          "
   << "energy         energy          energy          density         "
   << "sigma1_1        sigma1_2        sigma2_1        sigma2_2        "
   << "sigma3_1        sigma3_2           p             width          length           "
   << "height          volume         epsilon_w       epsilon_l       epsilon_h       "
   << "epsilon_v" << std::endl;

   debugInf.open(debugfile);
   if(!debugInf) { debugInf << "stream error: recPile_Disp" << std::endl; exit(-1);}
   debugInf.setf(std::ios::scientific, std::ios::floatfield);
   debugInf << " iteration    end_bearing     side_friction   total_force" << std::endl;

   // pre_2. create particles and boundaries from files
   readParticle(iniptclfile); // create container and particles, velocity and omga are set zero. 
   readBoundary(inibdryfile);   // create boundaries

   // pre_3. define variables used in iterations
   int    stepsnum=0;
   char   stepsstr[4];
   char   stepsfp[50];
   REAL avgNormal=0;
   REAL avgTangt=0;
    
   int pile[2]={11,12}; // top and down boundaries
   UPDATECTL pilectl[2];

   // iterations start here...
   iteration=0;
   do
   {
   // 1. create possible boundary particles and contacts between particles
   findContact();
   findBdryContact();
	
   // 2. set particles' forces/moments as zero before each re-calculation
   clearContactForce();	

   // 3. calculate contact forces/moments and apply them to particles
   internalForce(avgNormal, avgTangt);
	
   // 4. calculate boundary forces/moments and apply them to particles
   boundaryForce();

   // 5. update particles' velocity/omga/position/orientation based on force/moment
   updateParticle();
	
   // 6. update boundaries' position and orientation

   // displacement control of the pile
   pilectl[0].tran=Vec(0,0,-timeStep*pileRate);
   pilectl[1].tran=Vec(0,0,-timeStep*pileRate);

   updateBoundary(pile, pilectl, 2); 
   updateRectPile();
   if (iteration % interval == 0) {
   REAL  f7=getShearForce( 7).getZ();
   REAL  f8=getShearForce( 8).getZ();
   REAL  f9=getShearForce( 9).getZ();
   REAL f10=getShearForce(10).getZ();
   REAL  fn=getNormalForce(12).getZ();
   debugInf << std::setw(OWID) << iteration
   << std::setw(OWID) << fn
   << std::setw(OWID) << (f7+f8+f9+f10)
   << std::setw(OWID) << (fn+f7+f8+f9+f10)
   << std::endl;
   }

   // 7. (1) output particles and contacts information
   if (iteration % (totalSteps/snapNum) == 0) {
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp,ParticleFile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printParticle(stepsfp);
   printRectPile(stepsfp);
	    
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp, contactfile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printContact(stepsfp);
   ++stepsnum;
   }

   // 7. (2) output statistics info.
   if (iteration % interval == 0) {
   REAL t1=getTransEnergy();
   REAL t2=getRotatEnergy();
   REAL t3=getPotenEnergy(-0.025);
   progressInf << std::setw(OWID) << iteration
   << std::setw(OWID) << getPossContactNum()
   << std::setw(OWID) << getActualContactNum()
   << std::setw(OWID) << getAvgPenetration()
   << std::setw(OWID) << avgNormal
   << std::setw(OWID) << avgTangt
   << std::setw(OWID) << getAvgVelocity() 
   << std::setw(OWID) << getAvgOmga()
   << std::setw(OWID) << getAvgForce()   
   << std::setw(OWID) << getAvgMoment()
   << std::setw(OWID) << t1
   << std::setw(OWID) << t2
   << std::setw(OWID) << (t1+t2)
   << std::setw(OWID) << t3
   << std::setw(OWID) << (t1+t2+t3) << std::endl;
   }

   // 8. loop break condition
	
   } while (++iteration < totalSteps);

   // post_1. store the final snapshot of particles, contacts and boundaries.
   strcpy(stepsfp, ParticleFile); strcat(stepsfp, "_end");
   printParticle(stepsfp);
   printRectPile(stepsfp);

   strcpy(stepsfp, contactfile); strcat(stepsfp, "_end");
   printContact(stepsfp);

   strcpy(stepsfp, boundaryfile); strcat(stepsfp, "_end");
   printBoundary(stepsfp);
    
   // post_2. close streams
   progressInf.close();
   debugInf.close();
   }


   // The specimen has been deposited with gravitation within boundaries composed of particles.
   // An ellipsoidal pile is then drived into the particles using displacement control.
   void Assembly::ellipPile_Disp(int   totalSteps,  
   int   snapNum, 
   int   interval,
   REAL dimn,
   REAL rsize,
   const char *iniptclfile,
   const char *ParticleFile, 
   const char *contactfile,  
   const char *progressfile,
   const char *debugfile) 
   {
   // pre_1: open streams for output
   // ParticleFile and contactfile are used for snapNum at the end.
   progressInf.open(progressfile); 
   if(!progressInf) { debugInf << "stream error: ellipPile_Disp" << std::endl; exit(-1); }
   progressInf.setf(std::ios::scientific, std::ios::floatfield);
   progressInf << "pile penetrate..." << std::endl
   << "     iteration possible  actual      average	    average         average         average"
   << "         average         average         average       translational    rotational       "
   << "kinetic        potential         total           void            sample       coordination"
   << "       sample           sample          sample          sample          sample          sample"
   << "          sample          sample          sample         sample           sample         "
   << " sample          sample          sample          sample          sample" << std::endl
   << "       number  contacts contacts   penetration   contact_normal  contact_tangt     velocity"
   << "         omga            force           moment         energy           energy          "
   << "energy         energy            energy          ratio          porosity         number       "
   << "   density         sigma1_1        sigma1_2        sigma2_1        sigma2_2        "
   << "sigma3_1        sigma3_2           p             width          length           "
   << "height          volume         epsilon_w       epsilon_l       epsilon_h       "
   << "epsilon_v" << std::endl;

   debugInf.open(debugfile);
   if(!debugInf) { debugInf << "stream error: ellipPile_Disp" << std::endl; exit(-1);}
   debugInf.setf(std::ios::scientific, std::ios::floatfield);

   // pre_2. create particles and boundaries from files
   readParticle(iniptclfile); // create container and particles, velocity and omga are set zero. 

   // pre_3. define variables used in iterations
   REAL l13, l24, l56;
   REAL avgNormal=0;
   REAL avgTangt=0;
   int         stepsnum=0;
   char        stepsstr[4];
   char        stepsfp[50];
   REAL void_ratio=0;
    
   // iterations start here...
   iteration=0;
   do
   {
   // 1. create possible boundary particles and contacts between particles
   findContact();

   // 2. set particles' forces/moments as zero before each re-calculation
   clearContactForce();	

   // 3. calculate contact forces/moments and apply them to particles
   internalForce(avgNormal, avgTangt);
	
   // 4. update particles' velocity/omga/position/orientation based on force/moment
   updateParticle();
	
   // 5. calculate specimen void ratio.
   l56=getTopFreeParticlePosition().getZ() - (-dimn/2);
   l24=dimn*rsize;
   l13=dimn*rsize;
   bulkVolume=l13*l24*l56-ellipPilePeneVol();
   void_ratio=bulkVolume/getParticleVolume()-1;

   // 6. (1) output particles and contacts information
   if (iteration % (totalSteps/snapNum) == 0) {
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp,ParticleFile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printParticle(stepsfp);

   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp, contactfile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printContact(stepsfp);
   ++stepsnum;
   }

   // 6. (2) output statistics info.
   if (iteration % interval == 0) {
   REAL t1=getTransEnergy();
   REAL t2=getRotatEnergy();
   REAL t3=getPotenEnergy(-0.025);
   progressInf << std::setw(OWID) << iteration
   << std::setw(OWID) << getPossContactNum()
   << std::setw(OWID) << getActualContactNum()
   << std::setw(OWID) << getAvgPenetration()
   << std::setw(OWID) << avgNormal
   << std::setw(OWID) << avgTangt
   << std::setw(OWID) << getAvgVelocity() 
   << std::setw(OWID) << getAvgOmga()
   << std::setw(OWID) << getAvgForce()   
   << std::setw(OWID) << getAvgMoment()
   << std::setw(OWID) << t1
   << std::setw(OWID) << t2
   << std::setw(OWID) << (t1+t2)
   << std::setw(OWID) << t3
   << std::setw(OWID) << (t1+t2+t3)
   << std::setw(OWID) << void_ratio
   << std::setw(OWID) << void_ratio/(1+void_ratio)
   << std::setw(OWID) << 2.0*getActualContactNum()/allParticleVec.size()
   << std::endl;
   debugInf << std::setw(OWID) << iteration
   << std::setw(OWID) << getTopFreeParticlePosition().getZ()
   << std::setw(OWID) << ellipPileTipZ()
   << std::setw(OWID) << getTopFreeParticlePosition().getZ()-ellipPileTipZ()
   << std::setw(OWID) << l13*l24*l56
   << std::setw(OWID) << ellipPilePeneVol()
   << std::setw(OWID) << bulkVolume
   << std::endl;
   }

   // 7. loop break condition
	
   } while (++iteration < totalSteps);

   // post_1. store the final snapshot of particles, contacts and boundaries.
   strcpy(stepsfp, ParticleFile); strcat(stepsfp, "_end");
   printParticle(stepsfp);

   strcpy(stepsfp, contactfile); strcat(stepsfp, "_end");
   printContact(stepsfp);
    
   // post_2. close streams
   progressInf.close();
   debugInf.close();
   }


   // The specimen has been deposited with gravitation within rigid boundaries.
   // An ellipsoidal penetrator is then impacted into the particles with initial velocity.
   void Assembly::ellipPile_Impact(int   totalSteps,  
   int   snapNum, 
   int   interval,
   REAL dimn,
   const char *iniptclfile,
   const char *inibdryfile,
   const char *ParticleFile, 
   const char *contactfile,  
   const char *progressfile,
   const char *debugfile) 
   {
   // pre_1: open streams for output
   // ParticleFile and contactfile are used for snapNum at the end.
   progressInf.open(progressfile); 
   if(!progressInf) { debugInf << "stream error: ellipPile_Impact" << std::endl; exit(-1); }
   progressInf.setf(std::ios::scientific, std::ios::floatfield);
   progressInf << "penetrator impact..." << std::endl
   << "     iteration possible  actual      average	    average         average         average"
   << "         average         average         average       translational    rotational       "
   << "kinetic        potential         total           void            sample       coordination"
   << "       sample           sample          sample          sample          sample          sample"
   << "          sample          sample          sample         sample           sample         "
   << " sample          sample          sample          sample          sample" << std::endl
   << "       number  contacts contacts   penetration   contact_normal  contact_tangt     velocity"
   << "         omga            force           moment         energy           energy          "
   << "energy         energy            energy          ratio          porosity         number       "
   << "   density         sigma1_1        sigma1_2        sigma2_1        sigma2_2        "
   << "sigma3_1        sigma3_2           p             width          length           "
   << "height          volume         epsilon_w       epsilon_l       epsilon_h       "
   << "epsilon_v" << std::endl;

   debugInf.open(debugfile);
   if(!debugInf) { debugInf << "stream error: ellipPile_Impact" << std::endl; exit(-1);}
   debugInf.setf(std::ios::scientific, std::ios::floatfield);

   // pre_2. create particles and boundaries from files
   readParticle(iniptclfile); // create container and particles
   readBoundary(inibdryfile);   // create boundaries.

   // pre_3. define variables used in iterations
   REAL l13, l24, l56;
   REAL avgNormal=0;
   REAL avgTangt=0;
   int         stepsnum=0;
   char        stepsstr[4];
   char        stepsfp[50];
   REAL void_ratio=0;
   REAL bdry_penetr[7];
   int         bdry_cntnum[7];
   for (int i=0;i<7;++i) {
   bdry_penetr[i]=0; bdry_cntnum[i]=0;
   }
    
   // iterations start here...
   iteration=0;
   do
   {
   // 1. create possible boundary particles and contacts between particles
   findContact();

   // 2. set particles' forces/moments as zero before each re-calculation
   clearContactForce();	

   // 3. calculate contact forces/moments and apply them to particles
   internalForce(avgNormal, avgTangt);

   // 4. calculate boundary forces/moments and apply them to particles.
   boundaryForce(bdry_penetr, bdry_cntnum);
	
   // 5. update particles' velocity/omga/position/orientation based on force/moment
   updateParticle();
	
   // 6. calculate specimen void ratio.
   l56=getTopFreeParticlePosition().getZ() - (-dimn/2);
   l24=dimn;
   l13=dimn;
   bulkVolume=l13*l24*l56-ellipPilePeneVol();
   void_ratio=bulkVolume/getParticleVolume()-1;

   // 7. (1) output particles and contacts information
   if (iteration % (totalSteps/snapNum) == 0) {
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp,ParticleFile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printParticle(stepsfp);

   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp, contactfile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printContact(stepsfp);
   ++stepsnum;
   }

   // 7. (2) output statistics info.
   if (iteration % interval == 0) {
   REAL t1=getTransEnergy();
   REAL t2=getRotatEnergy();
   REAL t3=getPotenEnergy(-0.025);
   progressInf << std::setw(OWID) << iteration
   << std::setw(OWID) << getPossContactNum()
   << std::setw(OWID) << getActualContactNum()
   << std::setw(OWID) << getAvgPenetration()
   << std::setw(OWID) << avgNormal
   << std::setw(OWID) << avgTangt
   << std::setw(OWID) << getAvgVelocity() 
   << std::setw(OWID) << getAvgOmga()
   << std::setw(OWID) << getAvgForce()   
   << std::setw(OWID) << getAvgMoment()
   << std::setw(OWID) << t1
   << std::setw(OWID) << t2
   << std::setw(OWID) << (t1+t2)
   << std::setw(OWID) << t3
   << std::setw(OWID) << (t1+t2+t3)
   << std::setw(OWID) << void_ratio
   << std::setw(OWID) << void_ratio/(1+void_ratio)
   << std::setw(OWID) << 2.0*(getActualContactNum()
   +bdry_cntnum[1]+bdry_cntnum[2]+bdry_cntnum[3]
   +bdry_cntnum[4]+bdry_cntnum[6])/allParticleVec.size()
   << std::endl;
   debugInf << std::setw(OWID) << iteration
   << std::setw(OWID) << bdry_penetr[1]
   << std::setw(OWID) << bdry_penetr[2]
   << std::setw(OWID) << bdry_penetr[3]
   << std::setw(OWID) << bdry_penetr[4]
   << std::setw(OWID) << bdry_penetr[6]
   << std::setw(OWID) << bdry_cntnum[1]
   << std::setw(OWID) << bdry_cntnum[2]
   << std::setw(OWID) << bdry_cntnum[3]
   << std::setw(OWID) << bdry_cntnum[4]
   << std::setw(OWID) << bdry_cntnum[6]
   << std::endl;

   }

   // 8. loop break condition
	
   } while (++iteration < totalSteps);

   // post_1. store the final snapshot of particles, contacts and boundaries.
   strcpy(stepsfp, ParticleFile); strcat(stepsfp, "_end");
   printParticle(stepsfp);

   strcpy(stepsfp, contactfile); strcat(stepsfp, "_end");
   printContact(stepsfp);
    
   // post_2. close streams
   progressInf.close();
   debugInf.close();
   }


   // The specimen has been deposited with gravitation within particle boundaries.
   // An ellipsoidal penetrator is then impacted into the particles with initial velocity.
   void Assembly::ellipPile_Impact_p(int   totalSteps,  
   int   snapNum, 
   int   interval,
   REAL dimn,
   const char *iniptclfile,
   const char *ParticleFile, 
   const char *contactfile,  
   const char *progressfile,
   const char *debugfile) 
   {
   // pre_1: open streams for output
   // ParticleFile and contactfile are used for snapNum at the end.
   progressInf.open(progressfile); 
   if(!progressInf) { debugInf << "stream error: ellipPile_Impact_p" << std::endl; exit(-1); }
   progressInf.setf(std::ios::scientific, std::ios::floatfield);
   progressInf << "penetrator impact..." << std::endl
   << "     iteration possible  actual      average	    average         average         average"
   << "         average         average         average       translational    rotational       "
   << "kinetic        potential         total           void            sample       coordination"
   << "       sample           sample          sample          sample          sample          sample"
   << "          sample          sample          sample         sample           sample         "
   << " sample          sample          sample          sample          sample" << std::endl
   << "       number  contacts contacts   penetration   contact_normal  contact_tangt     velocity"
   << "         omga            force           moment         energy           energy          "
   << "energy         energy            energy          ratio          porosity         number       "
   << "   density         sigma1_1        sigma1_2        sigma2_1        sigma2_2        "
   << "sigma3_1        sigma3_2           p             width          length           "
   << "height          volume         epsilon_w       epsilon_l       epsilon_h       "
   << "epsilon_v" << std::endl;

   debugInf.open(debugfile);
   if(!debugInf) { debugInf << "stream error: ellipPile_Impact_p" << std::endl; exit(-1);}
   debugInf.setf(std::ios::scientific, std::ios::floatfield);

   // pre_2. create particles and boundaries from files
   readParticle(iniptclfile); // create container and particles

   // pre_3. define variables used in iterations
   REAL l13, l24, l56;
   REAL avgNormal=0;
   REAL avgTangt=0;
   int         stepsnum=0;
   char        stepsstr[4];
   char        stepsfp[50];
   REAL void_ratio=0;
    
   // iterations start here...
   iteration=0;
   do
   {
   // 1. create possible boundary particles and contacts between particles
   findContact();

   // 2. set particles' forces/moments as zero before each re-calculation
   clearContactForce();	

   // 3. calculate contact forces/moments and apply them to particles
   internalForce(avgNormal, avgTangt);
	
   // 4. update particles' velocity/omga/position/orientation based on force/moment
   updateParticle();
	
   // 5. calculate specimen void ratio.
   l56=getTopFreeParticlePosition().getZ() - (-dimn/2);
   l24=dimn;
   l13=dimn;
   bulkVolume=l13*l24*l56-ellipPilePeneVol();
   void_ratio=bulkVolume/getParticleVolume()-1;

   // 6. (1) output particles and contacts information
   if (iteration % (totalSteps/snapNum) == 0) {
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp,ParticleFile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printParticle(stepsfp);

   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp, contactfile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printContact(stepsfp);
   ++stepsnum;
   }

   // 6. (2) output statistics info.
   if (iteration % interval == 0) {
   REAL t1=getTransEnergy();
   REAL t2=getRotatEnergy();
   REAL t3=getPotenEnergy(-0.025);
   progressInf << std::setw(OWID) << iteration
   << std::setw(OWID) << getPossContactNum()
   << std::setw(OWID) << getActualContactNum()
   << std::setw(OWID) << getAvgPenetration()
   << std::setw(OWID) << avgNormal
   << std::setw(OWID) << avgTangt
   << std::setw(OWID) << getAvgVelocity() 
   << std::setw(OWID) << getAvgOmga()
   << std::setw(OWID) << getAvgForce()   
   << std::setw(OWID) << getAvgMoment()
   << std::setw(OWID) << t1
   << std::setw(OWID) << t2
   << std::setw(OWID) << (t1+t2)
   << std::setw(OWID) << t3
   << std::setw(OWID) << (t1+t2+t3)
   << std::setw(OWID) << void_ratio
   << std::setw(OWID) << void_ratio/(1+void_ratio)
   << std::setw(OWID) << 2.0*getActualContactNum()/allParticleVec.size()
   << std::endl;
   debugInf << std::setw(OWID) << iteration
   << std::setw(OWID) << getTopFreeParticlePosition().getZ()
   << std::setw(OWID) << ellipPileTipZ()
   << std::setw(OWID) << getTopFreeParticlePosition().getZ()-ellipPileTipZ()
   << std::setw(OWID) << l13*l24*l56
   << std::setw(OWID) << ellipPilePeneVol()
   << std::setw(OWID) << bulkVolume
   << std::endl;
   }

   // 7. loop break condition
	
   } while (++iteration < totalSteps);

   // post_1. store the final snapshot of particles, contacts and boundaries.
   strcpy(stepsfp, ParticleFile); strcat(stepsfp, "_end");
   printParticle(stepsfp);

   strcpy(stepsfp, contactfile); strcat(stepsfp, "_end");
   printContact(stepsfp);
    
   // post_2. close streams
   progressInf.close();
   debugInf.close();
   }



   // The specimen has been deposited with gravitation within boundaries composed of particles.
   // An ellipsoidal pile is then drived into the particles using force control.
   // Not recommended.
   void Assembly::ellipPile_Force(int   totalSteps,  
   int   snapNum,
   int   interval,
   REAL dimn,
   REAL force,
   int   division,
   const char *iniptclfile,
   const char *ParticleFile, 
   const char *contactfile,  
   const char *progressfile,
   const char *balancedfile,
   const char *debugfile) 
   {
   // pre_1: open streams for output
   // ParticleFile and contactfile are used for snapNum at the end.
   progressInf.open(progressfile); 
   if(!progressInf) { debugInf << "stream error: ellipPile_Force" << std::endl; exit(-1); }
   progressInf.setf(std::ios::scientific, std::ios::floatfield);
   progressInf << "pile penetrate..." << std::endl
   << "     iteration possible  actual      average	    average         average         average"
   << "         average         average         average       translational    rotational       "
   << "kinetic        potential         total           void            sample       coordination"
   << "       sample           sample          sample          sample          sample          sample"
   << "          sample          sample          sample         sample           sample         "
   << " sample          sample          sample          sample          sample" << std::endl
   << "       number  contacts contacts   penetration   contact_normal  contact_tangt     velocity"
   << "         omga            force           moment         energy           energy          "
   << "energy         energy            energy          ratio          porosity         number       "
   << "   density         sigma1_1        sigma1_2        sigma2_1        sigma2_2        "
   << "sigma3_1        sigma3_2           p             width          length           "
   << "height          volume         epsilon_w       epsilon_l       epsilon_h       "
   << "epsilon_v" << std::endl;

   std::ofstream balancedinf(balancedfile);
   if(!balancedinf) { debugInf << "stream error: ellipPile_Force" << std::endl; exit(-1);}
   balancedinf.setf(std::ios::scientific, std::ios::floatfield);
   balancedinf << "pile penetrate..." << std::endl
   << "   iteration   apply_force    pile_tip_pos     pile_force" << std::endl;

   debugInf.open(debugfile);
   if(!debugInf) { debugInf << "stream error: ellipPile_Force" << std::endl; exit(-1);}
   debugInf.setf(std::ios::scientific, std::ios::floatfield);

   // pre_2. create particles and boundaries from files
   readParticle(iniptclfile); // create container and particles, velocity and omga are set zero. 

   // pre_3. define variables used in iterations
   REAL l13, l24, l56;
   REAL avgNormal=0;
   REAL avgTangt=0;
   int         stepsnum=0;
   char        stepsstr[4];
   char        stepsfp[50];
   REAL void_ratio=0;

   REAL zforce_inc=force/division;
   REAL zforce=zforce_inc;

   // iterations start here...
   iteration=0;
   do
   {
   // 1. create possible boundary particles and contacts between particles
   findContact();

   // 2. set particles' forces/moments as zero before each re-calculation
   clearContactForce();	

   // 3. calculate contact forces/moments and apply them to particles
   internalForce(avgNormal, avgTangt);
	
   // 4. update particles' velocity/omga/position/orientation based on force/moment
   updateParticle();

   // 5. calculate specimen void ratio.
   l56=getTopFreeParticlePosition().getZ() - (-dimn/2);
   l24=dimn;
   l13=dimn;
   bulkVolume=l13*l24*l56-ellipPilePeneVol();
   void_ratio=bulkVolume/getParticleVolume()-1;
	
   // 6. update pile external force and position
   if(zforce>ellipPileForce())
   ellipPileUpdate();

   if(fabs(ellipPileForce()-zforce)/zforce < boundaryStressTol ) {
   balancedinf << std::setw(OWID) << iteration
   << std::setw(OWID) << zforce
   << std::setw(OWID) << getTopFreeParticlePosition().getZ()-ellipPileTipZ()
   << std::setw(OWID) << ellipPileForce()
   << std::endl;
   zforce += zforce_inc;
   }

   if( iteration % interval == 0) {
   debugInf << std::setw(OWID) << iteration
   << std::setw(OWID) << zforce
   << std::setw(OWID) << getTopFreeParticlePosition().getZ()-ellipPileTipZ()
   << std::setw(OWID) << ellipPileForce()
   << std::endl;
   }

   // 7. (1) output particles and contacts information
   if (iteration % (totalSteps/snapNum) == 0) {
   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp,ParticleFile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printParticle(stepsfp);

   sprintf(stepsstr, "%03d", stepsnum); 
   strcpy(stepsfp, contactfile); strcat(stepsfp, "_"); strcat(stepsfp, stepsstr);
   printContact(stepsfp);
   ++stepsnum;
   }

   // 7. (2) output statistics info.
   if (iteration % interval == 0) {
   REAL t1=getTransEnergy();
   REAL t2=getRotatEnergy();
   REAL t3=getPotenEnergy(-0.025);
   progressInf << std::setw(OWID) << iteration
   << std::setw(OWID) << getPossContactNum()
   << std::setw(OWID) << getActualContactNum()
   << std::setw(OWID) << getAvgPenetration()
   << std::setw(OWID) << avgNormal
   << std::setw(OWID) << avgTangt
   << std::setw(OWID) << getAvgVelocity() 
   << std::setw(OWID) << getAvgOmga()
   << std::setw(OWID) << getAvgForce()   
   << std::setw(OWID) << getAvgMoment()
   << std::setw(OWID) << t1
   << std::setw(OWID) << t2
   << std::setw(OWID) << (t1+t2)
   << std::setw(OWID) << t3
   << std::setw(OWID) << (t1+t2+t3)
   << std::setw(OWID) << void_ratio
   << std::setw(OWID) << void_ratio/(1+void_ratio)
   << std::setw(OWID) << 2.0*getActualContactNum()/allParticleVec.size()
   << std::endl;
   }

   // 8. loop break condition
   if (fabs((zforce-force)/force)<0.001)
   break;
	
   } while (++iteration < totalSteps);

   // post_1. store the final snapshot of particles, contacts and boundaries.
   strcpy(stepsfp, ParticleFile); strcat(stepsfp, "_end");
   printParticle(stepsfp);

   strcpy(stepsfp, contactfile); strcat(stepsfp, "_end");
   printContact(stepsfp);
    
   // post_2. close streams
   progressInf.close();
   balancedinf.close();
   debugInf.close();
   }

 */ 
 // rule out

