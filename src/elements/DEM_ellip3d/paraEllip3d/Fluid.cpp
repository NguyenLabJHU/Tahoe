#include "Fluid.h"
#include "const.h"
#include <cmath>

namespace dem {

  void Fluid::initParameter(Rectangle &container, Gradation &gradation) {

    minX = container.getMinCorner().getX();
    minY = container.getMinCorner().getY();
    minZ = container.getMinCorner().getZ();
    maxX = container.getMaxCorner().getX();
    maxY = container.getMaxCorner().getY();
    maxZ = container.getMaxCorner().getZ();
    REAL minR = gradation.getPtclMinRadius();

    dx = (minR * 2) / 4; // minimum diameter divided by 4
    dy = dx;
    dz = dx;
    nx = static_cast<std::size_t> (ceil((maxX - minX) / dx));
    ny = static_cast<std::size_t> (ceil((maxY - minY) / dy));
    nz = static_cast<std::size_t> (ceil((maxZ - minZ) / dz));
    /*
    nx = 2;
    ny = 2;
    nz = 50;
    */
    dx = (maxX - minX) / nx;
    dy = (maxY - minY) / ny;
    dz = (maxZ - minZ) / nz;

    nx += 2;
    ny += 2;
    nz += 2;
    
    CFL   = dem::Parameter::getSingleton().parameter["CFL"];
    gamma = dem::Parameter::getSingleton().parameter["airGamma"];
    rhoR = dem::Parameter::getSingleton().parameter["rightDensity"];
    uR   = dem::Parameter::getSingleton().parameter["rightVelocity"];
    pR   = dem::Parameter::getSingleton().parameter["rightPressure"];
    mach = dem::Parameter::getSingleton().parameter["MachNumber"];
    Cd   = dem::Parameter::getSingleton().parameter["Cd"];
    z0   = minZ + (maxZ - minZ) * dem::Parameter::getSingleton().parameter["z0Ratio"];
    arrayBC[0] = dem::Parameter::getSingleton().parameter["x1Reflecting"];
    arrayBC[1] = dem::Parameter::getSingleton().parameter["x2Reflecting"];
    arrayBC[2] = dem::Parameter::getSingleton().parameter["y1Reflecting"];
    arrayBC[3] = dem::Parameter::getSingleton().parameter["y2Reflecting"];
    arrayBC[4] = dem::Parameter::getSingleton().parameter["z1Reflecting"];
    arrayBC[5] = dem::Parameter::getSingleton().parameter["z2Reflecting"];

    // fixed
    n_dim = 3;
    n_var = 0; 
    n_integ = 0;

    var_den = n_var++; n_integ++;
    var_mom[0] = n_var++; n_integ++;
    var_mom[1] = n_var++; n_integ++;
    var_mom[2] = n_var++; n_integ++;
    var_eng = n_var++; n_integ++;

    var_vel[0] = n_var++;
    var_vel[1] = n_var++;
    var_vel[2] = n_var++;
    var_prs = n_var++; 

    // extended
    var_msk = n_var++;

    ///*
    debugInf << "dx=" << dx << std::endl;
    debugInf << "CFL=" << CFL << std::endl;
    debugInf << "gamma=" << gamma << std::endl;
    debugInf << "x1Rflecting=" << arrayBC[0] << std::endl;
    debugInf << "x2Rflecting=" << arrayBC[1] << std::endl;
    debugInf << "y1Rflecting=" << arrayBC[2] << std::endl;
    debugInf << "y2Rflecting=" << arrayBC[3] << std::endl;
    debugInf << "z1Rflecting=" << arrayBC[4] << std::endl;
    debugInf << "z2Rflecting=" << arrayBC[5] << std::endl;
    debugInf << "rhoR=" << rhoR << std::endl;
    debugInf << "uR=" << uR << std::endl;
    debugInf << "pR=" << pR << std::endl;
    debugInf << "Mach=" << mach << std::endl;
    debugInf << "Cd=" << Cd << std::endl;
    debugInf << "z0=" << z0 << std::endl;

    debugInf << "n_var = " << n_var << std::endl;
    debugInf << "n_integ = " << n_integ << std::endl;
    debugInf << "var_den = " << var_den  << std::endl;    
    debugInf << "var_mom[0] = " << var_mom[0] << std::endl;    
    debugInf << "var_mom[1] = " << var_mom[1] << std::endl;
    debugInf << "var_mom[2] = " << var_mom[2] << std::endl;    
    debugInf << "var_eng = " << var_eng  << std::endl;    
    debugInf << "var_vel[0] = " << var_vel[0] << std::endl;    
    debugInf << "var_vel[1] = " << var_vel[1] << std::endl;    
    debugInf << "var_vel[2] = " << var_vel[2] << std::endl;    
    debugInf << "var_prs = " << var_prs  << std::endl;    
    debugInf << "var_msk = " << var_msk  << std::endl;    
    //*/

    // nx, ny, nz, n_dim
    arrayGridCoord.resize(nx);
    for (std::size_t i = 0; i < arrayGridCoord.size(); ++i) {
      arrayGridCoord[i].resize(ny);
      for (std::size_t j = 0; j < arrayGridCoord[i].size(); ++j) {
	arrayGridCoord[i][j].resize(nz);
	for (std::size_t k = 0; k < arrayGridCoord[i][j].size(); ++k) 
	  arrayGridCoord[i][j][k].resize(n_dim);
      }
    }

    // coordinates
    for (std::size_t i = 0; i < arrayGridCoord.size(); ++i)
      for (std::size_t j = 0; j < arrayGridCoord[i].size(); ++j)
	for (std::size_t k = 0; k < arrayGridCoord[i][j].size(); ++k) {
	  arrayGridCoord[i][j][k][0] = (minX - dx/2) + i * dx;
	  arrayGridCoord[i][j][k][1] = (minY - dy/2) + j * dy;
	  arrayGridCoord[i][j][k][2] = (minZ - dz/2) + k * dz;
	}

    // nx, ny, nz, n_dim
    arrayPenalForce.resize(nx);
    for (std::size_t i = 0; i < arrayPenalForce.size(); ++i) {
      arrayPenalForce[i].resize(ny);
      for (std::size_t j = 0; j < arrayPenalForce[i].size(); ++j) {
	arrayPenalForce[i][j].resize(nz);
	for (std::size_t k = 0; k < arrayPenalForce[i][j].size(); ++k) 
	  arrayPenalForce[i][j][k].resize(n_dim);
      }
    }

    // nx, ny, nz, n_dim
    arrayPressureForce.resize(nx);
    for (std::size_t i = 0; i < arrayPressureForce.size(); ++i) {
      arrayPressureForce[i].resize(ny);
      for (std::size_t j = 0; j < arrayPressureForce[i].size(); ++j) {
	arrayPressureForce[i][j].resize(nz);
	for (std::size_t k = 0; k < arrayPressureForce[i][j].size(); ++k) 
	  arrayPressureForce[i][j][k].resize(n_dim);
      }
    }

    // nx, ny, nz, n_var
    arrayU.resize(nx);
    for (std::size_t i = 0; i < arrayU.size(); ++i) {
      arrayU[i].resize(ny);
      for (std::size_t j = 0; j < arrayU[i].size(); ++j) {
	arrayU[i][j].resize(nz);
	for (std::size_t k = 0; k < arrayU[i][j].size(); ++k) 
	  arrayU[i][j][k].resize(n_var);
      }
    }

    // nx, ny, nz, n_var
    arrayUtmp.resize(nx);
    for (std::size_t i = 0; i < arrayUtmp.size(); ++i) {
      arrayUtmp[i].resize(ny);
      for (std::size_t j = 0; j < arrayUtmp[i].size(); ++j) {
	arrayUtmp[i][j].resize(nz);
	for (std::size_t k = 0; k < arrayUtmp[i][j].size(); ++k) 
	  arrayUtmp[i][j][k].resize(n_var);
      }
    }

    // nx, ny, nz, n_integ
    arrayFlux.resize(nx);
    for (std::size_t i = 0; i < arrayFlux.size(); ++i) {
      arrayFlux[i].resize(ny);
      for (std::size_t j = 0; j < arrayFlux[i].size(); ++j) {
	arrayFlux[i][j].resize(nz);
	for (std::size_t k = 0; k < arrayFlux[i][j].size(); ++k) 
	  arrayFlux[i][j][k].resize(n_integ);
      }
    }

    // nx-1, ny-1, nz-1, n_integ, n_dim
    arrayRoeFlux.resize(nx-1);
    for (std::size_t i = 0; i < arrayRoeFlux.size(); ++i) {
      arrayRoeFlux[i].resize(ny-1);
      for (std::size_t j = 0; j < arrayRoeFlux[i].size(); ++j) {
	arrayRoeFlux[i][j].resize(nz-1);
	for (std::size_t k = 0; k < arrayRoeFlux[i][j].size(); ++k) {
	  arrayRoeFlux[i][j][k].resize(n_integ);
	  for (std::size_t m = 0; m < arrayRoeFlux[i][j][k].size(); ++m)
	    arrayRoeFlux[i][j][k][m].resize(n_dim);
	}
      }
    }

    // nx-1, ny-1, nz-1, n_integ
    arrayRoeFluxTmp.resize(nx-1);
    for (std::size_t i = 0; i < arrayRoeFluxTmp.size(); ++i) {
      arrayRoeFluxTmp[i].resize(ny-1);
      for (std::size_t j = 0; j < arrayRoeFluxTmp[i].size(); ++j) {
	arrayRoeFluxTmp[i][j].resize(nz-1);
	for (std::size_t k = 0; k < arrayRoeFluxTmp[i][j].size(); ++k) {
	  arrayRoeFluxTmp[i][j][k].resize(n_integ);
	}
      }
    }
    
    // nx, ny, nz
    arrayH.resize(nx);
    for (std::size_t i = 0; i < arrayH.size(); ++i) {
      arrayH[i].resize(ny);
      for (std::size_t j = 0; j < arrayH[i].size(); ++j)
	arrayH[i][j].resize(nz);
    }

    // nx, ny, nz
    arraySoundSpeed.resize(nx);
    for (std::size_t i = 0; i < arraySoundSpeed.size(); ++i) {
      arraySoundSpeed[i].resize(ny);
      for (std::size_t j = 0; j < arraySoundSpeed[i].size(); ++j)
	arraySoundSpeed[i][j].resize(nz);
    }

  }

  void Fluid::initialize() {
    RankineHugoniot();
    initialCondition(); 
  }

  void Fluid::runOneStep() { 
    addGhostPoints();
    soundSpeed();
    timeStep = std::min(timeStep, calcTimeStep());
    debugInf << "iter=" << std::setw(8) << iteration << " dt=" << std::setw(OWID) << timeStep << std::endl;
    enthalpy();

    std::size_t id[3][3] = {{0,1,2},{1,0,2},{2,1,0}};

    // for x, y, z directions
    for (std::size_t idim = 0; idim < n_dim; ++idim) {
      arrayUtmp = arrayU; // must have same rank and extent

      // switch components
      for (std::size_t i = 0; i < nx; ++i)
	for (std::size_t j = 0; j < ny; ++j)
	  for (std::size_t k = 0; k < nz; ++k) 
	    for (std::size_t jdim = 0; jdim < n_dim; ++jdim) {
	      arrayUtmp[i][j][k][  var_mom[jdim]  ] = arrayU[i][j][k][  var_mom[id[idim][jdim]]  ];
	      arrayUtmp[i][j][k][  var_vel[jdim]  ] = arrayU[i][j][k][  var_vel[id[idim][jdim]]  ];
	    }

      flux();

      // for local Riemann problem
      for (std::size_t i = 0; i < nx- 1; ++i) {
	for (std::size_t j = 0; j < ny - 1; ++j) {
	  for (std::size_t k = 0; k < nz -1; ++k) {
	    std::size_t IL[3] = {i, j, k};
	    std::size_t IR[3] = {i, j, k};
	    IR[idim] += 1;
	    REAL uL[9], uR[9], FL[5], FR[5], HL, HR; // local variable only
	    HL = arrayH[IL[0]] [IL[1]] [IL[2]];
	    HR = arrayH[IR[0]] [IR[1]] [IR[2]];
	    for (std::size_t m = 0; m < n_var; ++m) {
	      uL[m] = arrayUtmp[IL[0]] [IL[1]] [IL[2]] [m];
	      uR[m] = arrayUtmp[IR[0]] [IR[1]] [IR[2]] [m];
	    }	
	    for (std::size_t m = 0; m < n_integ; ++m) {
	      FL[m] = arrayFlux [IL[0]] [IL[1]] [IL[2]] [m];
	      FR[m] = arrayFlux [IR[0]] [IR[1]] [IR[2]] [m];
	    }    
	    RoeFlux(uL, uR, FL, FR, HL, HR, idim, i, j, k);
	  }
	}
      }

      for (std::size_t i = 0; i < nx -1; ++i)
	for (std::size_t j = 0; j < ny -1; ++j)
	  for (std::size_t k = 0; k < nz -1; ++k)
	    for (std::size_t m = 0; m < n_integ; ++m)
	      arrayRoeFluxTmp[i][j][k][m] = arrayRoeFlux[i][j][k][m][idim];

      // switch components back for consistency with u
      for (std::size_t i = 0; i < nx - 1; ++i)
	for (std::size_t j = 0; j < ny - 1; ++j)
	  for (std::size_t k = 0; k < nz -1; ++k)
	    for (std::size_t m = 0; m < n_dim; ++m)
	      arrayRoeFlux[i][j][k][var_mom[m]][idim] = arrayRoeFluxTmp[i][j][k][ var_mom[id[idim][m]] ];

    } // end of for x, y, z directions


    // update conservative variables at the next time step
    for (std::size_t i = 1; i < nx - 1 ; ++i)
      for (std::size_t j = 1; j < ny - 1; ++j)
	for (std::size_t k = 1; k < nz - 1; ++k) {
	  for (std::size_t m = 0; m < n_integ; ++m)
	    arrayU[i][j][k][m] -= (   timeStep / dx * (arrayRoeFlux[i][j][k][m][0] - arrayRoeFlux[i-1][j][k][m][0])
				    + timeStep / dy * (arrayRoeFlux[i][j][k][m][1] - arrayRoeFlux[i][j-1][k][m][1])
				    + timeStep / dz * (arrayRoeFlux[i][j][k][m][2] - arrayRoeFlux[i][j][k-1][m][2]) );
	}

    // calculate primitive after finding conservative variables
    UtoW(); 
  }
  
  void Fluid::penalize() {
    // Brinkman penalization
    for (std::size_t i = 0; i < nx; ++i)
      for (std::size_t j = 0; j < ny; ++j)
	for (std::size_t k = 0; k < nz; ++k)
	  for (std::size_t m = 0; m < n_dim; ++m)
	    arrayU[i][j][k][var_mom[m]] -= arrayU[i][j][k][var_msk] * arrayPenalForce[i][j][k][m] * timeStep;
  }
  
  void Fluid::addGhostPoints() {
    // non-reflecting BCs
    for (std::size_t j = 1; j < ny - 1; ++j)
      for (std::size_t k = 1; k < nz - 1; ++k)
	for (std::size_t m = 0; m < n_var; ++m) {
	  arrayU[0][j][k][m]    = arrayU[1][j][k][m]; 
	  arrayU[nx-1][j][k][m] = arrayU[nx-2][j][k][m]; 
	}

    for (std::size_t i = 1; i < nx - 1; ++i)
      for (std::size_t k = 1; k < nz -1; ++k)
	for (std::size_t m = 0; m < n_var; ++m) {
	  arrayU[i][0][k][m]    = arrayU[i][1][k][m]; 
	  arrayU[i][ny-1][k][m] = arrayU[i][ny-2][k][m]; 
	}

    for (std::size_t i = 1; i < nx - 1; ++i)
      for (std::size_t j = 1; j < ny - 1; ++j)
	for (std::size_t m = 0; m < n_var; ++m) {
	  arrayU[i][j][0][m]    = arrayU[i][j][1][m]; 
	  arrayU[i][j][nz-1][m] = arrayU[i][j][nz-2][m]; 
	}

    // reflecting BCs
    bool reflecting = false;
    for (std::size_t it = 0; it < 6; ++it) {
      if (arrayBC[it] > 0) {
	reflecting = true;
	break;
      }
    }

    if (reflecting) {
      for (std::size_t j = 1; j < ny - 1; ++j)
	for (std::size_t k = 1; k < nz - 1; ++k)
	  for (std::size_t m = 0; m < 1; ++m) { // x-direction
	    arrayU[0][j][k][var_mom[m]]    *= (1-2*arrayBC[0]); 
	    arrayU[nx-1][j][k][var_mom[m]] *= (1-2*arrayBC[1]); 
	    arrayU[0][j][k][var_vel[m]]    *= (1-2*arrayBC[0]); 
	    arrayU[nx-1][j][k][var_vel[m]] *= (1-2*arrayBC[1]); 
	  }

      for (std::size_t i = 1; i < nx - 1; ++i)
	for (std::size_t k = 1; k < nz - 1; ++k)
	  for (std::size_t m = 1; m < 2; ++m) { // y-direction
	    arrayU[i][0][k][var_mom[m]]    *= (1-2*arrayBC[2]); 
	    arrayU[i][ny-1][k][var_mom[m]] *= (1-2*arrayBC[3]);
	    arrayU[i][0][k][var_vel[m]]    *= (1-2*arrayBC[2]); 
	    arrayU[i][ny-1][k][var_vel[m]] *= (1-2*arrayBC[3]);  
	  }

      for (std::size_t i = 1; i < nx - 1; ++i)
	for (std::size_t j = 1; j < ny - 1; ++j)
	  for (std::size_t m = 2; m < 3; ++m) { // z-direction
	    arrayU[i][j][0][var_mom[m]]    *= (1-2*arrayBC[4]); 
	    arrayU[i][j][nz-1][var_mom[m]] *= (1-2*arrayBC[5]); 
	    arrayU[i][j][0][var_vel[m]]    *= (1-2*arrayBC[4]); 
	    arrayU[i][j][nz-1][var_vel[m]] *= (1-2*arrayBC[5]); 
	  }
    }
    
  }
  
  REAL Fluid::calcTimeStep() {
    std::valarray<REAL> allGrid(nx * ny * nz);
    for (std::size_t i = 0; i < nx ; ++i)
      for (std::size_t j = 0; j < ny; ++j)
	for (std::size_t k = 0; k < nz; ++k)
	  allGrid[i + j * nx + k * nx * ny] = fabs(arrayU[i][j][k][var_vel[0]]) + arraySoundSpeed[i][j][k];
    REAL sx = allGrid.max();

    for (std::size_t i = 0; i < nx ; ++i)
      for (std::size_t j = 0; j < ny; ++j)
	for (std::size_t k = 0; k < nz; ++k)
	  allGrid[i + j * nx + k * nx * ny] = fabs(arrayU[i][j][k][var_vel[1]]) + arraySoundSpeed[i][j][k];
    REAL sy = allGrid.max();

    for (std::size_t i = 0; i < nx ; ++i)
      for (std::size_t j = 0; j < ny; ++j)
	for (std::size_t k = 0; k < nz; ++k)
	  allGrid[i + j * nx + k * nx * ny] = fabs(arrayU[i][j][k][var_vel[2]]) + arraySoundSpeed[i][j][k];
    REAL sz = allGrid.max();

    std::valarray<REAL> dtMin(3);
    dtMin[0] = dx/sx;
    dtMin[1] = dy/sy;
    dtMin[2] = dz/sz;
    
    return CFL * dtMin.min();
  }

  void Fluid::soundSpeed() {
    for (std::size_t i = 0; i < nx ; ++i)
      for (std::size_t j = 0; j < ny; ++j)
	for (std::size_t k = 0; k < nz; ++k)
	  arraySoundSpeed[i][j][k] = sqrt(gamma * arrayU[i][j][k][var_prs] /  arrayU[i][j][k][var_den]);
  }

  void Fluid::enthalpy() {
    for (std::size_t i = 0; i < nx ; ++i)
      for (std::size_t j = 0; j < ny; ++j)
	for (std::size_t k = 0; k < nz; ++k)
	  arrayH[i][j][k] = (arrayU[i][j][k][var_eng] + arrayU[i][j][k][var_prs]) / arrayU[i][j][k][var_den];
  }

  void Fluid::initialCondition() {
    // normal shock
    for (std::size_t i = 0; i < nx; ++i)
      for (std::size_t j = 0; j < ny; ++j)
	for (std::size_t k = 0; k < nz; ++k) {
	  if ( arrayGridCoord[i][j][k][2] <= z0) {
	    arrayU[i][j][k][var_den] = rhoL;
	    arrayU[i][j][k][var_vel[2]] = uL;
	    arrayU[i][j][k][var_prs] = pL;
	  } else {
	    arrayU[i][j][k][var_den] = rhoR;
	    arrayU[i][j][k][var_vel[2]] = uR;
	    arrayU[i][j][k][var_prs] = pR;
	  }
	}

    WtoU();
  }

  void Fluid::RankineHugoniot() {
    shockSpeed = mach*sqrt(gamma*pR/rhoR);
    pL = (pR*(1-gamma)+2*rhoR*pow(shockSpeed-uR,2)) / (1+gamma);
    rhoL = ( pow(rhoR*(shockSpeed-uR),2)*(1+gamma) ) / ( rhoR*pow(shockSpeed-uR,2)*(gamma-1) + 2*pR*gamma);
    uL = ( rhoR*(shockSpeed-uR)*(2*shockSpeed + uR*(gamma-1)) - 2*pR*gamma ) / (rhoR * (shockSpeed-uR) * (1+gamma));
    ///*
    debugInf << "rhoL=" << rhoL << std::endl;
    debugInf << "uL=" << uL << std::endl;
    debugInf << "pL=" << pL << std::endl;
    debugInf << "shockSpeed=" << shockSpeed << std::endl;
    //*/
  }

  void Fluid::flux() {
    for (std::size_t i = 0; i < nx; ++i)
      for (std::size_t j = 0; j < ny; ++j)
	for (std::size_t k = 0; k < nz; ++k) {
	  arrayFlux[i][j][k][var_den]    = arrayUtmp[i][j][k][var_den] * arrayUtmp[i][j][k][var_vel[0]]; // rho*u
	  arrayFlux[i][j][k][var_mom[0]] = arrayUtmp[i][j][k][var_den] * pow(arrayUtmp[i][j][k][var_vel[0]],2) + arrayUtmp[i][j][k][var_prs]; // rho*u^2 + p
	  arrayFlux[i][j][k][var_mom[1]] = arrayUtmp[i][j][k][var_den] * arrayUtmp[i][j][k][var_vel[0]] * arrayUtmp[i][j][k][var_vel[1]]; // rho*u*v
	  arrayFlux[i][j][k][var_mom[2]] = arrayUtmp[i][j][k][var_den] * arrayUtmp[i][j][k][var_vel[0]] * arrayUtmp[i][j][k][var_vel[2]]; // rho*u*w
	  arrayFlux[i][j][k][var_eng]    = arrayUtmp[i][j][k][var_vel[0]] * (arrayUtmp[i][j][k][var_eng] + arrayUtmp[i][j][k][var_prs]);  // u*(E + p)
	}  
  }

  void Fluid::RoeFlux(REAL uL[], REAL uR[], REAL FL[], REAL FR[], REAL HL, REAL HR, std::size_t idim, std::size_t it, std::size_t jt, std::size_t kt) {
    REAL avgRho = sqrt(uL[var_den]*uR[var_den]);
    REAL avgH   = (sqrt(uL[var_den])*HL + sqrt(uR[var_den])*HR)/(sqrt(uL[var_den]) + sqrt(uR[var_den]));
    REAL avgU   = (sqrt(uL[var_den])*uL[var_vel[0]] + sqrt(uR[var_den])*uR[var_vel[0]])/(sqrt(uL[var_den]) + sqrt(uR[var_den]));
    REAL avgV   = (sqrt(uL[var_den])*uL[var_vel[1]] + sqrt(uR[var_den])*uR[var_vel[1]])/(sqrt(uL[var_den]) + sqrt(uR[var_den]));
    REAL avgW   = (sqrt(uL[var_den])*uL[var_vel[2]] + sqrt(uR[var_den])*uR[var_vel[2]])/(sqrt(uL[var_den]) + sqrt(uR[var_den]));
    REAL avgSoundSpeed = sqrt( (gamma-1)*(avgH - 0.5*(avgU*avgU + avgV*avgV + avgW*avgW)) );

    REAL eigen[5];
    eigen[var_den]    = avgU - avgSoundSpeed;
    eigen[var_mom[0]] = eigen[var_mom[1]] = eigen[var_mom[2]] = avgU;
    eigen[var_eng]    = avgU + avgSoundSpeed;

    REAL avgWaveStr[5], du[9];
    for (std::size_t i = 0; i < n_var; ++i)
      du[i] = uR[i] - uL[i];

    avgWaveStr[var_den]    = (du[var_prs] - avgRho*avgSoundSpeed*du[var_vel[0]]) / (2*avgSoundSpeed*avgSoundSpeed);
    avgWaveStr[var_mom[0]] = du[var_den] - du[var_prs]/(avgSoundSpeed*avgSoundSpeed);
    avgWaveStr[var_mom[1]] = avgRho * du[var_vel[1]];
    avgWaveStr[var_mom[2]] = avgRho * du[var_vel[2]];
    avgWaveStr[var_eng]    = (du[var_prs] + avgRho*avgSoundSpeed*du[var_vel[0]]) / (2*avgSoundSpeed*avgSoundSpeed);

    REAL avgK[5][5]; // right eigenvectors
    avgK[var_den][var_den]    = 1;
    avgK[var_mom[0]][var_den] = avgU - avgSoundSpeed;
    avgK[var_mom[1]][var_den] = avgV;
    avgK[var_mom[2]][var_den] = avgW;
    avgK[var_eng][var_den]    = avgH - avgU * avgSoundSpeed;

    avgK[var_den][var_mom[0]]    = 1;
    avgK[var_mom[0]][var_mom[0]] = avgU;
    avgK[var_mom[1]][var_mom[0]] = avgV;
    avgK[var_mom[2]][var_mom[0]] = avgW;
    avgK[var_eng][var_mom[0]]    = 0.5 * (avgU*avgU + avgV*avgV + avgW*avgW);

    avgK[var_den][var_mom[1]]    = 0;
    avgK[var_mom[0]][var_mom[1]] = 0;
    avgK[var_mom[1]][var_mom[1]] = 1;
    avgK[var_mom[2]][var_mom[1]] = 0;
    avgK[var_eng][var_mom[1]]    = avgV;

    avgK[var_den][var_mom[2]]    = 0;
    avgK[var_mom[0]][var_mom[2]] = 0;
    avgK[var_mom[1]][var_mom[2]] = 0;
    avgK[var_mom[2]][var_mom[2]] = 1;
    avgK[var_eng][var_mom[2]]    = avgW;

    avgK[var_den][var_eng]    = 1;
    avgK[var_mom[0]][var_eng] = avgU + avgSoundSpeed;
    avgK[var_mom[1]][var_eng] = avgV;
    avgK[var_mom[2]][var_eng] = avgW;
    avgK[var_eng][var_eng]    = avgH + avgU * avgSoundSpeed;

    REAL RF[5];
    for (std::size_t i = 0; i < n_integ; ++i)
      RF[i] = 0.5*(FL[i] + FR[i]);

    for (std::size_t ie = 0; ie < n_integ; ++ie) {
      for (std::size_t je = 0; je < n_integ; ++je)
	RF[ie] -= 0.5 * avgWaveStr[je] * fabs(eigen[je]) * avgK[ie][je];
      arrayRoeFlux[it][jt][kt][ie][idim] = RF[ie];
    }
  }

  void Fluid::UtoW() {// converting conservative variables into primitive
    for (std::size_t i = 0; i < nx; ++i)
      for (std::size_t j = 0; j < ny; ++j)
	for (std::size_t k = 0; k < nz; ++k) {

	  for (std::size_t m = 0; m < n_dim; ++m)
	    arrayU[i][j][k][var_vel[m]] = arrayU[i][j][k][var_mom[m]] / arrayU[i][j][k][var_den];

	  arrayU[i][j][k][var_prs] = 0;
	  for (std::size_t m = 0; m < n_dim; ++m)
	    arrayU[i][j][k][var_prs] += arrayU[i][j][k][var_den] * pow(arrayU[i][j][k][var_vel[m]],2)/2 ;
	  arrayU[i][j][k][var_prs] = (arrayU[i][j][k][var_eng] - arrayU[i][j][k][var_prs]) * (gamma-1);

	}
  }

  void Fluid::WtoU() { // converting primitive variables into conservative
    for (std::size_t i = 0; i < nx; ++i)
      for (std::size_t j = 0; j < ny; ++j)
	for (std::size_t k = 0; k < nz; ++k) {
	  for (std::size_t m = 0; m < n_dim; ++m)
	    arrayU[i][j][k][var_mom[m]] = arrayU[i][j][k][var_den] * arrayU[i][j][k][var_vel[m]];

	  arrayU[i][j][k][var_eng] = 0;
	  for (std::size_t m = 0; m < n_dim; ++m)
	    arrayU[i][j][k][var_eng] += arrayU[i][j][k][var_den] * pow(arrayU[i][j][k][var_vel[m]],2)/2 ;
	  arrayU[i][j][k][var_eng] += arrayU[i][j][k][var_prs] / (gamma-1);
	}
  }

  void Fluid::getParticleInfo(std::vector<Particle *> &ptcls) {
    for (std::vector<Particle*>::const_iterator it = ptcls.begin(); it != ptcls.end(); ++it)
      (*it)->clearFluidGrid();

    // 0 ~ (n-1), including boundaries
    for (std::size_t i = 0; i < arrayGridCoord.size() ; ++i)
      for (std::size_t j = 0; j <  arrayGridCoord[i].size(); ++j)
	for (std::size_t k = 0; k <  arrayGridCoord[i][j].size(); ++k) {
	  arrayU[i][j][k][var_msk] = 0;
	  REAL x = arrayGridCoord[i][j][k][0];
	  REAL y = arrayGridCoord[i][j][k][1];
	  REAL z = arrayGridCoord[i][j][k][2];
	  for (std::vector<Particle*>::const_iterator it = ptcls.begin(); it != ptcls.end(); ++it) {
	    if ( (*it)->surfaceError(Vec(x,y,z)) < 0 ) {// inside particle surface
	      arrayU[i][j][k][var_msk] = 1; 
	      (*it)->recordFluidGrid(i, j, k);
	    }
	  }
	}
  }

  void Fluid::calcParticleForce(std::vector<Particle *> &ptcls, std::ofstream &ofs) {
    for (std::vector<Particle *>::const_iterator it = ptcls.begin(); it != ptcls.end(); ++it) {
      REAL etaB = 8.0/3.0 * (*it)->getC() / Cd;
      Vec penalForce = 0, presForce = 0;
      Vec penalMoment = 0, presMoment = 0;
      std::vector<std::vector<std::size_t> > fluidGrid = (*it)->getFluidGrid();
      for (std::size_t iter = 0; iter < fluidGrid.size(); ++iter) {
	std::size_t i = fluidGrid[iter][0];
	std::size_t j = fluidGrid[iter][1];
	std::size_t k = fluidGrid[iter][2];

	REAL uxFluid = arrayU[i][j][k][var_vel[0]];
	REAL uyFluid = arrayU[i][j][k][var_vel[1]];
	REAL uzFluid = arrayU[i][j][k][var_vel[2]];
	
	Vec dist = Vec(arrayGridCoord[i][j][k][0], arrayGridCoord[i][j][k][1], arrayGridCoord[i][j][k][2]) - (*it)->getCurrPos();
	Vec omgar = (*it)->getCurrOmga() * dist; // w X r = Omga*distVector, where * is overloaded as cross product

	REAL ux = (*it)->getCurrVeloc().getX() + omgar.getX(); 
	REAL uy = (*it)->getCurrVeloc().getY() + omgar.getY(); 
	REAL uz = (*it)->getCurrVeloc().getZ() + omgar.getZ();

	arrayPenalForce[i][j][k][0] = arrayU[i][j][k][var_den]*fabs(uxFluid - ux)*(uxFluid - ux) / etaB;
	arrayPenalForce[i][j][k][1] = arrayU[i][j][k][var_den]*fabs(uyFluid - uy)*(uyFluid - uy) / etaB;
	arrayPenalForce[i][j][k][2] = arrayU[i][j][k][var_den]*fabs(uzFluid - uz)*(uzFluid - uz) / etaB;

	arrayPressureForce[i][j][k][0] = -(arrayU[i+1][j][k][var_prs] - arrayU[i-1][j][k][var_prs])/(2*dx);
	arrayPressureForce[i][j][k][1] = -(arrayU[i][j+1][k][var_prs] - arrayU[i][j-1][k][var_prs])/(2*dy);
	arrayPressureForce[i][j][k][2] = -(arrayU[i][j][k+1][var_prs] - arrayU[i][j][k-1][var_prs])/(2*dz);

	penalForce += Vec(arrayPenalForce[i][j][k][0], arrayPenalForce[i][j][k][1], arrayPenalForce[i][j][k][2]);
	presForce  += Vec(arrayPressureForce[i][j][k][0], arrayPressureForce[i][j][k][1], arrayPressureForce[i][j][k][2]);

	// r X F
	penalMoment += dist * Vec(arrayPenalForce[i][j][k][0], arrayPenalForce[i][j][k][1], arrayPenalForce[i][j][k][2]);
	presMoment  += dist * Vec(arrayPressureForce[i][j][k][0], arrayPressureForce[i][j][k][1], arrayPressureForce[i][j][k][2]);
      }

      penalForce *= dx*dy*dz;
      presForce  *= dx*dy*dz;
      (*it)->addForce(penalForce);
      (*it)->addForce(presForce);

      penalMoment *= dx*dy*dz;
      presMoment  *= dx*dy*dz;
      //(*it)->addMoment(penalMoment);
      //(*it)->addMoment(presMoment);

      if ((*it)->getId() == 1) {
	ofs << std::setw(OWID) << iteration
	    << std::setw(OWID) << penalForce.getX()
	    << std::setw(OWID) << penalForce.getY()
	    << std::setw(OWID) << penalForce.getZ()
	    << std::setw(OWID) << presForce.getX()
	    << std::setw(OWID) << presForce.getY()
	    << std::setw(OWID) << presForce.getZ()
	    << std::setw(OWID) << (*it)->getCurrVeloc().getX()
	    << std::setw(OWID) << (*it)->getCurrVeloc().getY()
	    << std::setw(OWID) << (*it)->getCurrVeloc().getZ()
	    << std::setw(OWID) << vfabs(penalForce)
	    << std::setw(OWID) << vfabs(presForce)
	    << std::setw(OWID) << vfabs((*it)->getCurrVeloc())
	    << std::endl;    
      } 
    }    
  }

  void Fluid::plot(const char *str) const {
    std::ofstream ofs(str);
    if(!ofs) { std::cout << "stream error: Fluid::plot" << std::endl; exit(-1); }
    ofs.setf(std::ios::scientific, std::ios::floatfield);
    ofs.precision(OPREC);
    
    ofs	<< std::setw(OWID) << "VARIABLES = \"x\""
	<< std::setw(OWID) << "\"y\""
	<< std::setw(OWID) << "\"z\""
	<< std::setw(OWID) << "\"density\""
	<< std::setw(OWID) << "\"momentum_x\""
	<< std::setw(OWID) << "\"momentum_y\""
	<< std::setw(OWID) << "\"momentum_z\""
	<< std::setw(OWID) << "\"energy\""
	<< std::setw(OWID) << "\"velocity_x\""
	<< std::setw(OWID) << "\"velocity_y\""
	<< std::setw(OWID) << "\"velocity_z\""
	<< std::setw(OWID) << "\"pressure\""
	<< std::setw(OWID) << "\"temperature\""
	<< std::setw(OWID) << "\"mask\""
	<< std::setw(OWID) << "\"penalFx\""
	<< std::setw(OWID) << "\"penalFy\""
	<< std::setw(OWID) << "\"penalFz\""
	<< std::setw(OWID) << "\"pressureFx\""
	<< std::setw(OWID) << "\"pressureFy\""
	<< std::setw(OWID) << "\"pressureFz\""
	<< std::endl;

    ofs << "ZONE I=" << nx -2
	<< ", J=" << ny -2
	<< ", K=" << nz -2
	<< ", DATAPACKING=POINT"
	<< std::endl;

    for (std::size_t k = 1; k < nz - 1; ++k)
      for (std::size_t j = 1; j < ny - 1; ++j)
	for (std::size_t i = 1; i < nx -1; ++i) {
	  ofs << std::setw(OWID) << arrayGridCoord[i][j][k][0]
	      << std::setw(OWID) << arrayGridCoord[i][j][k][1]
	      << std::setw(OWID) << arrayGridCoord[i][j][k][2]
	      << std::setw(OWID) << arrayU[i][j][k][var_den]
	      << std::setw(OWID) << arrayU[i][j][k][var_mom[0]]
	      << std::setw(OWID) << arrayU[i][j][k][var_mom[1]]
	      << std::setw(OWID) << arrayU[i][j][k][var_mom[2]]
	      << std::setw(OWID) << arrayU[i][j][k][var_eng]
	      << std::setw(OWID) << arrayU[i][j][k][var_vel[0]]
	      << std::setw(OWID) << arrayU[i][j][k][var_vel[1]]
	      << std::setw(OWID) << arrayU[i][j][k][var_vel[2]]
	      << std::setw(OWID) << arrayU[i][j][k][var_prs]
	      << std::setw(OWID) << arrayU[i][j][k][var_prs]/(8.31*arrayU[i][j][k][var_den])
	      << std::setw(OWID) << arrayU[i][j][k][var_msk]  
	      << std::setw(OWID) << arrayPenalForce[i][j][k][0] 
	      << std::setw(OWID) << arrayPenalForce[i][j][k][1] 
	      << std::setw(OWID) << arrayPenalForce[i][j][k][2]
	      << std::setw(OWID) << arrayPressureForce[i][j][k][0] 
	      << std::setw(OWID) << arrayPressureForce[i][j][k][1] 
	      << std::setw(OWID) << arrayPressureForce[i][j][k][2] 
	      << std::endl;
	}

    ofs.close();
  }
  
} // name space dem


    /*
    for (std::size_t k = 1; k < nz; ++k) {
      for (std::size_t j = 1; j < ny; ++j) {
	for (std::size_t i = 1; i < nx; ++i) {
	  std::cout << arrayU[i][j][k][var_den] << " ";
	}
	debugInf << std::endl;
      }
      debugInf << "----------------------" << std::endl;
    }
    */
