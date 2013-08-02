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

    dx = (maxX - minX) / nx;
    dy = (maxY - minY) / ny;
    dz = (maxZ - minZ) / nz;
    dt = dem::Parameter::getSingleton().parameter["timeStep"];

    nx += 2;
    ny += 2;
    nz += 2;
    
    gamma = dem::Parameter::getSingleton().parameter["airGamma"];
    reflecting = dem::Parameter::getSingleton().parameter["reflecting"];
    rhoL = dem::Parameter::getSingleton().parameter["leftDensity"];
    uL   = dem::Parameter::getSingleton().parameter["leftVelocity"];
    pL   = dem::Parameter::getSingleton().parameter["leftPressure"];
    uR   = dem::Parameter::getSingleton().parameter["rightVelocity"];
    z0   = minZ + 0.25*(maxZ - minZ);

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

    /*
    std::cout << "rhoL=" << rhoL << std::endl;
    std::cout << "reflecting=" << reflecting << std::endl;
    std::cout << "uL=" << uL << std::endl;
    std::cout << "pL=" << pL << std::endl;
    std::cout << "uR=" << uR << std::endl;
    std::cout << "z0=" << z0 << std::endl;
    std::cout << "dt=" << dt << std::endl;
    std::cout << "n_var = " << n_var << std::endl;
    std::cout << "n_integ = " << n_integ << std::endl;
    std::cout << "var_den = " << var_den  << std::endl;    
    std::cout << "var_mom[0] = " << var_mom[0] << std::endl;    
    std::cout << "var_mom[1] = " << var_mom[1] << std::endl;
    std::cout << "var_mom[2] = " << var_mom[2] << std::endl;    
    std::cout << "var_eng = " << var_eng  << std::endl;    
    std::cout << "var_vel[0] = " << var_vel[0] << std::endl;    
    std::cout << "var_vel[1] = " << var_vel[1] << std::endl;    
    std::cout << "var_vel[2] = " << var_vel[2] << std::endl;    
    std::cout << "var_prs = " << var_prs  << std::endl;    
    std::cout << "var_msk = " << var_msk  << std::endl;    
    */

    // nx, ny, nz, n_dim
    arrayGrid.resize(nx);
    for (std::size_t i = 0; i < arrayGrid.size(); ++i) {
      arrayGrid[i].resize(ny);
      for (std::size_t j = 0; j < arrayGrid[i].size(); ++j) {
	arrayGrid[i][j].resize(nz);
	for (std::size_t k = 0; k < arrayGrid[i][j].size(); ++k) 
	  arrayGrid[i][j][k].resize(n_dim);
      }
    }

    // coordinates
    for (std::size_t i = 0; i < arrayGrid.size(); ++i)
      for (std::size_t j = 0; j < arrayGrid[i].size(); ++j)
	for (std::size_t k = 0; k < arrayGrid[i][j].size(); ++k) {
	  arrayGrid[i][j][k][0] = (minX - dx/2) + i * dx;
	  arrayGrid[i][j][k][1] = (minY - dy/2) + j * dy;
	  arrayGrid[i][j][k][2] = (minZ - dz/2) + k * dz;
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
    // soundSpeed();
    dt = std::min(dem::Parameter::getSingleton().parameter["timeStep"], timeStep());
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
	    REAL uL[5], uR[5], FL[5], FR[5], HL, HR; // local variable only
	    HL = arrayH[IL[0]] [IL[1]] [IL[2]];
	    HR = arrayH[IR[0]] [IR[1]] [IR[2]];
	    //std::cout << "HL="<<HL<< " HR="<<HR << " ";
	    for (std::size_t m = 0; m < n_integ; ++m) {
	      uL[m] = arrayUtmp[IL[0]] [IL[1]] [IL[2]] [m];
	      uR[m] = arrayUtmp[IR[0]] [IR[1]] [IR[2]] [m];
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
	for (std::size_t j = 0; j < ny - 1 ; ++j)
	  for (std::size_t k = 0; k < nz -1; ++k)
	    for (std::size_t m = 0; m < n_dim; ++m)
	      arrayRoeFlux[i][j][k][var_mom[m]][idim] = arrayRoeFluxTmp[i][j][k][ var_mom[id[idim][m]] ];

    } // end of for x, y, z directions

    // update conservative variables at the next time step
    for (std::size_t i = 1; i < nx - 1 ; ++i)
      for (std::size_t j = 1; j < ny - 1; ++j)
	for (std::size_t k = 1; k < nz - 1; ++k)
	  for (std::size_t m = 0; m < n_integ; ++m)
	    arrayU[i][j][k][m] -= (   dt / dx * (arrayRoeFlux[i][j][k][m][0] - arrayRoeFlux[i-1][j][k][m][0])
				    + dt / dy * (arrayRoeFlux[i][j][k][m][1] - arrayRoeFlux[i][j-1][k][m][1])
				    + dt / dz * (arrayRoeFlux[i][j][k][m][2] - arrayRoeFlux[i][j][k-1][m][2]) );

    // calculate primitive after finding conservative variables
    UtoW(); 
    /*
    for (std::size_t k = 1; k < nz; ++k) {
      for (std::size_t j = 1; j < 4; ++j) {
	for (std::size_t i = 1; i < 4; ++i) {
	  std::cout << arrayU[i][j][k][var_den] << " ";
	}
	std::cout << std::endl;
      }
      std::cout << "----------------------" << std::endl;
    }
    */
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
    if (reflecting) {
      for (std::size_t j = 1; j < ny - 1; ++j)
	for (std::size_t k = 1; k < nz - 1; ++k)
	  for (std::size_t m = 0; m < n_dim; ++m) {
	    arrayU[0][j][k][var_mom[m]]    = -arrayU[1][j][k][var_mom[m]]; 
	    arrayU[nx-1][j][k][var_mom[m]] = -arrayU[nx-2][j][k][var_mom[m]]; 
	    arrayU[0][j][k][var_vel[m]]    = -arrayU[1][j][k][var_vel[m]]; 
	    arrayU[nx-1][j][k][var_vel[m]] = -arrayU[nx-2][j][k][var_vel[m]]; 
	  }

      for (std::size_t i = 1; i < nx - 1; ++i)
	for (std::size_t k = 1; k < nz - 1; ++k)
	  for (std::size_t m = 0; m < n_dim; ++m) {
	    arrayU[i][0][k][var_mom[m]]    = -arrayU[i][1][k][var_mom[m]]; 
	    arrayU[i][ny-1][k][var_mom[m]] = -arrayU[i][ny-2][k][var_mom[m]];
	    arrayU[i][0][k][var_vel[m]]    = -arrayU[i][1][k][var_vel[m]]; 
	    arrayU[i][ny-1][k][var_vel[m]] = -arrayU[i][ny-2][k][var_vel[m]];  
	  }

      for (std::size_t i = 1; i < nx - 1; ++i)
	for (std::size_t j = 1; j < ny - 1; ++j)
	  for (std::size_t m = 0; m < n_dim; ++m) {
	    arrayU[i][j][0][var_mom[m]]    = -arrayU[i][j][1][var_mom[m]]; 
	    arrayU[i][j][nz-1][var_mom[m]] = -arrayU[i][j][nz-2][var_mom[m]]; 
	    arrayU[i][j][0][var_vel[m]]    = -arrayU[i][j][1][var_vel[m]]; 
	    arrayU[i][j][nz-1][var_vel[m]]  = -arrayU[i][j][nz-2][var_vel[m]]; 
	  }
    }
  }

  REAL Fluid::timeStep() {
    return 1;
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
	  if ( arrayGrid[i][j][k][2] <= z0) {
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
    REAL alpha = rhoL * pow(uL - uR ,2);
    REAL beta  = sqrt(16*pL*gamma/(alpha*(1+gamma)*(1+gamma)) + 1);
    rhoR = rhoL*(4*pL*gamma + alpha*(1+gamma)*(1-beta)) / (2*(alpha*(gamma-1) + 2*pL*gamma));
    pR = pL - alpha * (1+gamma) * (beta-1) / 4;
    shockSpeed = uR + (uL - uR) * (3-gamma + (1+gamma)*beta) / 4;

    /*
    std::cout << "alpla=" << alpha << std::endl;
    std::cout << "beta=" << beta << std::endl;
    std::cout << "rhoR=" << rhoR << std::endl;
    std::cout << "uR=" << uR << std::endl;
    std::cout << "pR=" << pR << std::endl;
    std::cout << "shock=" << shockSpeed << std::endl;
    */
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
    
    //std::cout << it << " " << jt << " " << kt << " " << "HL=" << HL << " HR=" << HR << std::endl;
    //<< "avgH=" << avgH  <<" u="<<avgU << " v="<<avgV << " w=" << avgW <<  " sound=" << avgSoundSpeed << std::endl;

    REAL eigen[5];
    eigen[var_den]    = avgU - avgSoundSpeed;
    eigen[var_mom[0]] = eigen[var_mom[1]] = eigen[var_mom[2]] = avgU;
    eigen[var_eng]    = avgU + avgSoundSpeed;

    REAL avgWaveStr[5], du[5];
    for (std::size_t i = 0; i < n_integ; ++i)
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
    for (std::size_t i = 1; i < arrayGrid.size() - 1 ; ++i)
      for (std::size_t j = 1; j <  arrayGrid[i].size() - 1; ++j)
	for (std::size_t k = 1; k <  arrayGrid[i][j].size() -1; ++k) {
	  arrayU[i][j][k][var_msk] = 0;
	  REAL x = arrayGrid[i][j][k][0];
	  REAL y = arrayGrid[i][j][k][1];
	  REAL z = arrayGrid[i][j][k][2];
	  for (std::vector<Particle*>::const_iterator it = ptcls.begin(); it != ptcls.end(); ++it) {
	    if ( (*it)->surfaceError(Vec(x,y,z)) < 0 ) {// inside particle surface
	      arrayU[i][j][k][var_msk] = 1;
	      // (*it)->getCurrPos();
	      // (*it)->getCurrDirecA();
	      // (*it)->getCurrVeloc();
	      // (*it)->getCurrOmea();
	    }
	  }
	}
  }

  void Fluid::calcParticleForce(std::vector<Particle *> &ptcls) {
    for (std::vector<Particle*>::const_iterator it = ptcls.begin(); it != ptcls.end(); ++it) {
      // check each grid and evaluate forces and moments.
      ;
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
	<< std::endl;

    ofs << "ZONE I=" << nx -2
	<< ", J=" << ny -2
	<< ", K=" << nz -2
	<< ", DATAPACKING=POINT"
	<< std::endl;

    for (std::size_t k = 1; k < nz - 1; ++k)
      for (std::size_t j = 1; j < ny - 1; ++j)
	for (std::size_t i = 1; i < nx -1; ++i) {
	  ofs << std::setw(OWID) << arrayGrid[i][j][k][0]
	      << std::setw(OWID) << arrayGrid[i][j][k][1]
	      << std::setw(OWID) << arrayGrid[i][j][k][2]
	      << std::setw(OWID) << arrayU[i][j][k][0]
	      << std::setw(OWID) << arrayU[i][j][k][1]
	      << std::setw(OWID) << arrayU[i][j][k][2]
	      << std::setw(OWID) << arrayU[i][j][k][3]
	      << std::setw(OWID) << arrayU[i][j][k][4]
	      << std::setw(OWID) << arrayU[i][j][k][5]
	      << std::setw(OWID) << arrayU[i][j][k][6]
	      << std::setw(OWID) << arrayU[i][j][k][7]
	      << std::setw(OWID) << arrayU[i][j][k][8] << std::endl;
	}

    ofs.close();
  }
  
} // name space dem
