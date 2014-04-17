%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Program to read and interpolate 2D efficiency functions %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
clear;


%%%%%%%%%%%%%%%%%%%%
% Global variables %
%%%%%%%%%%%%%%%%%%%%
q2Indx   = 0; % q^2 bin index
fileName = '../../efficiency/H2Deff_MisTag_q2Bin';
NstepsX  = 100; % [100]
NstepsY  = 100; % [100]
NbinsX   = 5;
NbinsY   = 5;
addContraints    = false; % [false]
useMethodInterp2 = false; % If true 'interp2' else 'scatteredInterpolant' [false]
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ==> Interpolation methods <==                          %
% - for 'interp2': linear, nearest, cubic, spline        %
% - for 'scatteredInterpolant': linear, nearest, natural %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
interpMethod    = 'natural'; % [natural]
effAtBoundaries = 1e-4;      % [1e-4]
effMinValue     = 1e-5;      % [1e-5]


if addContraints == true
    delta = 2;
else
    delta = 0;
end

Xvec = zeros(1,NbinsX+delta);
Yvec = zeros(1,NbinsY+delta);
Values = zeros(NbinsX+delta,NbinsY+delta);


%%%%%%%%%%%%%%%%%%%%%%%%%
% Read values from file %
%%%%%%%%%%%%%%%%%%%%%%%%%
fileIDin = fopen(strcat(fileName,sprintf('_%d.txt',q2Indx)),'r');
formatSpec = '%f   %f   %f   %f   %f   %f';
i = 1;
j = 1;
fprintf('@@@ Reading 2D binned efficiency from txt file @@@\n');
while i <= NbinsX
    row = textscan(fileIDin,formatSpec,1);

    Xvec(i+delta/2) = row{1,1} + row{1,2}/2;
    Yvec(j+delta/2) = row{1,3} + row{1,4}/2;
    Values(j+delta/2,i+delta/2) = row{1,5};
    j = j + 1;
    
    if (j > NbinsY)
        j = 1;
        i = i + 1;
    end
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Add contraints at the boundaries %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if addContraints == true    
    Xvec(1) = -1;
    Xvec(NbinsX+delta) = 1;
    Yvec(1) = -1;
    Yvec(NbinsY+delta) = 1;
    
    for i = 1:NbinsX+delta
        Values(1,i) = effAtBoundaries;
        Values(NbinsY+delta,i) = effAtBoundaries;
    end

    for j = 1:NbinsY+delta
        Values(j,1) = effAtBoundaries;
        Values(j,NbinsX+delta) = effAtBoundaries;
    end
end


%%%%%%%%%%%%%%%%
% Plot options %
%%%%%%%%%%%%%%%%
figure;
surf(Xvec, Yvec, Values);
colorbar;
title('Efficiency');
xlabel('cos(\theta_K)');
ylabel('cos(\theta_l)');


%%%%%%%%%%%%%%%%%%%%%%
% Create interpolant %
%%%%%%%%%%%%%%%%%%%%%%
[newXvec_,newYvec_] = meshgrid(Xvec, Yvec);
if useMethodInterp2 == true
    [newXvec,newYvec] = meshgrid(linspace(-1,1,NstepsX), linspace(-1,1,NstepsY));
    newValues = interp2(Xvec, Yvec, Values, newXvec, newYvec, interpMethod, effMinValue);
else
    newValues_ = scatteredInterpolant(newXvec_(:), newYvec_(:), Values(:), interpMethod, 'nearest'); % 'linear' 'nearest' 'none'
    [newXvec,newYvec] = meshgrid(linspace(-1,1,NstepsX), linspace(-1,1,NstepsY));
    newValues = newValues_(newXvec, newYvec);
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Check if new efficiency goes negative %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
isPOS = true;
for i = 1:length(newXvec)
    for j = 1:length(newYvec)
        if newValues(j,i) < 0
            isPOS = false;
            newValues(j,i) = effMinValue;
        end
    end
end
if isPOS == false
   fprintf('@@@ The efficiency is negative --> corrected to %e @@@\n',effMinValue);
else
    fprintf('@@@ The efficiency is OK @@@\n');
end


%%%%%%%%%%%%%%%%
% Plot options %
%%%%%%%%%%%%%%%%
figure;
surf(newXvec, newYvec, newValues);
colorbar;
title('Interpolated Efficiency');
xlabel('cos(\theta_K)');
ylabel('cos(\theta_l)');


%%%%%%%%%%%%%%%%%%%%%%%
% Save values to file %
%%%%%%%%%%%%%%%%%%%%%%%
fileIDout = fopen(strcat(fileName,sprintf('_interp_%d.txt',q2Indx)),'w');
i = 1;
j = 1;
fprintf('@@@ Saving 2D binned efficiency to txt file @@@\n');
while i <= NstepsX-1
    newRow = sprintf(' ');
    newRow = strcat(newRow,sprintf('%f   %f',newXvec(j,i),newXvec(j,i+1) - newXvec(j,i)));
    newRow = strcat(newRow,sprintf('   %f   %f',newYvec(j,i),newYvec(j+1,i) - newYvec(j,i)));
    newRow = strcat(newRow,sprintf('   %f   %f',newValues(j,i),0));

    fprintf(fileIDout,'%s\n',newRow);
    j = j + 1;
    
    if (j > NstepsY-1)
        j = 1;
        i = i + 1;
    end
    
    clear newRow;
end


fprintf('@@@ I''ve generated interpolated efficiency @@@\n');
fclose(fileIDout);
fclose(fileIDin);