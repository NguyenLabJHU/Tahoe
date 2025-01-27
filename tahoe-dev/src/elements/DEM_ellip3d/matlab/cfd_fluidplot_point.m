function [prs, den, uZ, ma, eng, tmp] = cfd_fluidplot_point(fileToRead1, xCoord, yCoord, zCoord)

%newData1 = importdata(fileToRead1); % it is very slow
%numeric = newData1.data;
fid = fopen(fileToRead1);
text  = fscanf(fid, '%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s', 28);
numeric = fscanf(fid, '%e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e', [21 inf]);
fclose(fid);
numeric = numeric.';
point = numeric(( numeric(:, 1) == xCoord & numeric(:, 2) == yCoord & numeric(:, 3) == zCoord ), :);

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

prs = pressure;
den = density;
uZ  = velocityZ;
ma  = mach;
eng = energy;
tmp = temperature;
