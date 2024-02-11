function [out] = cfd_fluidplot_mpoint(fileToRead1, xCoord, yCoord, zCoord)

%newData1 = importdata(fileToRead1); % it is very slow
%numeric = newData1.data;
fid = fopen(fileToRead1);
text  = fscanf(fid, '%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s', 28);
numeric = fscanf(fid, '%e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e', [21 inf]);
fclose(fid);
numeric = numeric.';

point=zeros(size(zCoord,1), 21);
for i = 1:size(zCoord,1)
  point(i, :) = numeric( ( numeric(:, 1) == xCoord & numeric(:, 2) == yCoord & numeric(:, 3) == zCoord(i) ), :);
end

colheaders = {'x' 'y' 'z' 'mach' 'density' 'momentumX' 'momentumY' 'momentumZ' ...
              'energy' 'velocityX' 'velocityY' 'velocityZ' 'pressure' 'temperature' ...
              'mask' 'penalFx' 'penalFy' 'penalFz' 'pressureFx' 'pressureFy' 'pressureFz'};
for i = 1:size(point, 2)
    assignin('caller', genvarname(colheaders{i}), point(:,i));
end

z = evalin('caller', 'z');
energy = evalin('caller', 'energy');
mach = evalin('caller', 'mach');
density = evalin('caller', 'density');
velocityZ = evalin('caller', 'velocityZ');
pressure = evalin('caller', 'pressure');
temperature = evalin('caller', 'temperature');
penalFz = evalin('caller', 'penalFz');
pressureFz = evalin('caller', 'pressureFz');

out(:, 1) = pressure;
out(:, 2) = density;
out(:, 3) = velocityZ;
out(:, 4) = mach;
out(:, 5) = energy;
out(:, 6) = temperature;
