%% General mex-ing options
FloatOrDouble = {'FLOAT','DOUBLE'};

for k = 1:length(FloatOrDouble),   
    Options = ['-compatibleArrayDims COPTIMFLAGS=''-Ofast -DNDEBUG''  -DMEX -D' FloatOrDouble{k} ' -I../../../Code/Eigen -I../ -I/usr/local/include -L/usr/local/lib/'];
    
    os = system_dependent('getos');
    if ~isempty(findstr('Darwin',os)),
        Options = [Options ' -DNAMED_SEMAPHORES -D_ACCELERATE_ON_']; % -Dchar16_t=UINT16_T'];
        Options = [Options ' -D__MACOSX_CORE__ LDFLAGS=''\$LDFLAGS -framework Accelerate'''];
    end
    
    Files = '../ThreadsWithCounter.C ../IDLList.C ../IDLListNode.C ../Vector.C ../Cover.C ../Point.C ../CoverNode.C ../EnlargeData.C ../Timer.C ../Distances.C ../TimeUtils.cpp';
    
    %% Compile covertree
    fprintf('\n Compiling covertree (%s)...',FloatOrDouble{k});
    if strcmpi(FloatOrDouble{k},'DOUBLE')
        outputfilenameaddition = 'D';
    else
        outputfilenameaddition = '';
    end
    eval(['mex ' Options ' covertree.C ' Files ' -lpthread -output covertree' outputfilenameaddition]);
    
    fprintf('done.');
    
    %% Compile findwithin
    fprintf('\n Compiling findwithin (%s)...',FloatOrDouble{k});
    eval(['mex ' Options ' findwithinMEX.C ' Files ' ../FindWithinData.C ../findWithin.C -lpthread -output findwithinMEX' outputfilenameaddition]);
    fprintf('done.');
    
    %% Compile findnearest
    fprintf('\n Compiling findnearest (%s)...',FloatOrDouble{k});
    eval(['mex ' Options ' findnearestMEX.C ' Files ' ../FindWithinData.C ../findNearest.C -lpthread -output findnearestMEX' outputfilenameaddition']);
    fprintf('done.');
    
    
    fprintf('\n');
end

