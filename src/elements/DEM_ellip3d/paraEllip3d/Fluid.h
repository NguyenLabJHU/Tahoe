#ifndef FLUID_H
#define FLUID_H

#include "Parameter.h"
#include "realtypes.h"
#include "Vec.h"
#include "Gradation.h"
#include "Rectangle.h"
#include "Boundary.h"
#include "Particle.h"
#include <cstddef>
#include <valarray>

namespace dem {
  
  class Fluid {
    typedef std::valarray< std::valarray< std::valarray <REAL> > > Array3D;
    typedef std::valarray< std::valarray< std::valarray <std::valarray<REAL> > > > Array4D;
    typedef std::valarray< std::valarray< std::valarray <std::valarray< std::valarray<REAL>  > > > > Array5D;

  private:
    static const REAL Rs = 287.06; // specific gas constant

    std::size_t nx; // nx = total cell centers = parts + two boundary points in x direction
    std::size_t ny; // ny = total cell centers = parts + two boundary points in y direction
    std::size_t nz; // nz = total cell centers = parts + two boundary points in z direction
    REAL dx;
    REAL dy;
    REAL dz;

    REAL RK;           // Runge-Kutta scheme
    REAL CFL;          // Courant-Friedrichs-Lewy condition
    REAL gamma;        // ratio of specific heat capacity of air
    REAL arrayBC[6];   // boundary condition

    int  leftType;     // type of "left" part
    REAL z1L;          // lower bound z1L of "left" part
    REAL z2L;          // upper bound z2L of "left" part
    REAL x1L;          // lower bound x1L of "left" part
    REAL x2L;          // upper bound x2L of "left" part
    REAL y1L;          // lower bound y1L of "left" part
    REAL y2L;          // upper bound y2L of "left" part
    REAL x0L;          // center of "left" part, x-coordinate
    REAL y0L;          // center of "left" part, y-coordinate
    REAL z0L;          // center of "left" part, z-coordinate
    REAL r0L;          // radius of "left" part

    REAL rhoL, uL, pL; // unknown
    REAL rhoR, uR, pR; // known
    REAL Mach;         // shock Mach number, known
    REAL shockSpeed;   // unknown
    REAL Cd;           // drag coefficient
    REAL porosity;     // particle porosity regarded as porous media
    int  volFrac;      // use grid volume fraction or not

    std::size_t nDim, nVar, nInteg, varDen, varEng, varPrs, varMsk;
    std::size_t varMom[3], varVel[3];

    Array4D arrayU;
    Array4D arrayUtmp;
    // 4-dimensional, defined at cell centers
    // nx, ny, nz, nVar
    // (a) fixed:
    // arrayU[i][j][k][0]: varDen
    // arrayU[i][j][k][1]: varMom[0]
    // arrayU[i][j][k][2]: varMom[1]
    // arrayU[i][j][k][3]: varMom[2]
    // arrayU[i][j][k][4]: varEng    // total energy per unit volume, E = rho * (1/2*V^2 + e), NOT total specific energy
    // arrayU[i][j][k][5]: varVel[0]
    // arrayU[i][j][k][6]: varVel[1]
    // arrayU[i][j][k][7]: varVel[2]
    // arrayU[i][j][k][8]: varPrs
    // (b) extended:
    // arrayU[i][j][k][9]: varMsk

    Array4D arrayGridCoord; 
    // fluid grid coordinates, 4-dimensional
    // nx, ny, nz, nDim
    // arrayGridCoord[i][j][k][0]: coordX
    // arrayGridCoord[i][j][k][1]: coordY
    // arrayGridCoord[i][j][k][2]: coordZ

    Array4D arrayPenalForce;
    // fluid grid forces, 4-dimensional
    // nx, ny, nz, nDim
    // arrayPenalForce[i][j][k][0]: forceX
    // arrayPenalForce[i][j][k][1]: forceY
    // arrayPenalForce[i][j][k][2]: forceZ

    Array4D arrayPressureForce;
    // fluid grid forces, 4-dimensional
    // nx, ny, nz, nDim
    // arrayPressureForce[i][j][k][0]: forceX
    // arrayPressureForce[i][j][k][1]: forceY
    // arrayPressureForce[i][j][k][2]: forceZ

    Array4D arrayFlux;
    // 4-dimensional, defined at cell centers
    // nx, ny, nz, nInteg
    // arrayFlux[i][j][k][0]: varDen
    // arrayFlux[i][j][k][1]: varMom[0]
    // arrayFlux[i][j][k][2]: varMom[1]
    // arrayFlux[i][j][k][3]: varMom[2]
    // arrayFlux[i][j][k][4]: varEng

    Array5D arrayRoeFlux; 
    Array5D arrayRoeFluxStep2; 
    Array5D arrayRoeFluxStep3; 
    // 5-dimensional, defined at cell faces
    // nx-1, ny-1, nz-1, nInteg, nDim
    // arrayRoeFlux[i][j][k][0]: varDen
    // arrayRoeFlux[i][j][k][1]: varMom[0]
    // arrayRoeFlux[i][j][k][2]: varMom[1]
    // arrayRoeFlux[i][j][k][3]: varMom[2]
    // arrayRoeFlux[i][j][k][4]: varEng

    Array4D arrayRoeFluxTmp;
    // 4-dimensional
    // nx-1, ny-1, nz-1, nInteg

    Array3D arrayH; 
    // Enthalpy, 3-dimensional, total specific enthalpy, NOT static specific enthalpy
    // nx, ny, nz

    Array3D arraySoundSpeed; 
    // speed of sound, 3-dimensional
    // nx, ny, nz

  public:
    Fluid() {}
    
    void initParameter(Rectangle &container, Gradation &gradation);
    void initialize();
    void initialCondition();
    REAL calcTimeStep();
    void RankineHugoniot();
    void addGhostPoints();
    void soundSpeed();
    void enthalpy();
    void flux(std::size_t, std::vector<Particle *> &ptcls);
    void RoeFlux(REAL uL[], REAL uR[], REAL FL[], REAL FR[], REAL HL, REAL HR, std::size_t idim,  std::size_t i, std::size_t j, std::size_t k);
    void UtoW(); // U - integrated; W - primitive
    void WtoU();
    void rotateIJK(std::vector<Particle *> &ptcls);
    void inteStep1(std::vector<Particle *> &ptcls);
    void inteStep2(std::vector<Particle *> &ptcls);
    void inteStep3(std::vector<Particle *> &ptcls);

    void getParticleInfo(std::vector<Particle *> &ptcls);
    void runOneStep(std::vector<Particle *> &ptcls);
    void calcParticleForce(std::vector<Particle *> &ptcls, std::ofstream &ofs);
    void penalize(std::vector<Particle *> &ptcls);
    void plot(const char *) const;
    
  };
  
} // name space dem
#endif
