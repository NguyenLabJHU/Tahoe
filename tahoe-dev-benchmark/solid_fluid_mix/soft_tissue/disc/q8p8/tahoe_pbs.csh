#!/bin/csh
#
# select = # of nodes
# ncpus is ALWAYS set to 8!
# mpiprocs is the amount of cpus on each node to use
# this is a 4 cpu run because (select)x(mpiprocs)=4
# this is a 8 cpu run because (select)x(mpiprocs)=8
# this is a 16 cpu run because (select)x(mpiprocs)=16
# this is a 32 cpu run because (select)x(mpiprocs)=32
#PBS -l select=2:ncpus=8:mpiprocs=1
#PBS -l place=scatter:excl
#PBS -N Tahoe_example
#PBS -q debug  
#PBS -l walltime=01:00:00
#PBS -V
#PBS -k eo
#PBS -A ONRDC34502393
#PBS -r n

#############################################################
# change the below input variable to your own filename
#############################################################
set input_file = disc-traction-drained-parallel-NatBC-QtrDisc-nucleus.xml
set output_file = disc-traction-drained-parallel-NatBC-QtrDisc-nucleus.out.txt
#set input_file = disc-traction-consol-parallel-NatBC-QtrDisc-nucleus.xml
#set output_file = disc-traction-consol-parallel-NatBC-QtrDisc-nucleus.out.txt
#set input_file = disc-traction-impulse-parallel-NatBC-QtrDisc-nucleus.xml
#set output_file = disc-traction-impulse-parallel-NatBC-QtrDisc-nucleus.out.txt

# set directory with inputs in it (usually the current dir, i.e. `pwd`)
set source_dir = /usr/people/regueiro/Code/Tahoe/development_benchmark_xml/solid_fluid_mix/soft_tissue/disc/q8p8

# number of processors nr_processors = (select * mpiprocs)
set nr_processors = 2

# set location of the executable file
set parallel_exe = /usr/people/regueiro/Code/Tahoe/parallel/tahoe-install/tahoe/tahoe_p

##################################################
# you may need to modify below this

#go to directory with data
setenv JOBIDVAR `echo ${PBS_JOBID} | cut -f1 -d.`
cd $source_dir

# create work dir, copy pertinent info
mkdir -p /usr/var/tmp/${PBS_O_LOGNAME}/${PBS_JOBID}
cp disc-traction-drained-parallel-NatBC-QtrDisc-nucleus.xml /usr/var/tmp/${PBS_O_LOGNAME}/${PBS_JOBID}
#cp disc-traction-consol-parallel-NatBC-QtrDisc-nucleus.xml /usr/var/tmp/${PBS_O_LOGNAME}/${PBS_JOBID}
#cp disc-traction-impulse-parallel-NatBC-QtrDisc-nucleus.xml /usr/var/tmp/${PBS_O_LOGNAME}/${PBS_JOBID}
cp quarter_disc_nucleus_hex8.g /usr/var/tmp/${PBS_O_LOGNAME}/${PBS_JOBID}

# go to work dir
cd /usr/var/tmp/${PBS_O_LOGNAME}/${PBS_JOBID}

#  The below section of commands verifies access to all the compute nodes to be used.
#===================================================================================
setenv MY_HOSTS `cat ${PBS_NODEFILE} | tr ' ' '\n' `
setenv xTMP_HOSTS `echo ${MY_HOSTS} |  sed 's/ /\n/g' | sort -u `
foreach host ($xTMP_HOSTS)
  echo "Working on $host ...."
  /usr/bin/ssh -o StrictHostKeyChecking=no $host pwd
end

#====================================================================================
# load the modules...
source /usr/cta/modules/3.1.6/init/csh
module purge
module load compiler/gcc4.4.1 mpi/openmpi-1.4.1 pbs

# now run the code
echo ">>> Run the simulation."
openmpirun.pbs $parallel_exe -f $input_file >> $output_file
echo ">>> Ran the simulation."
