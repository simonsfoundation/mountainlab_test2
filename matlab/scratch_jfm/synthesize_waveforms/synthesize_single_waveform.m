function Y=synthesize_single_waveform(N,durations,amps)

if nargin<1 test_synth_waveform; return; end;

% Recommended N=800
if nargin<2 durations=[200,10,30,200]; end;
if nargin<3 amps=[0.5,10,-1,0]; end;

durations=floor(durations);
if (sum(durations)<N) durations(end)=N-sum(durations); end;

timepoints=[1,cumsum(durations)];

t=1:sum(durations);
Y=zeros(size(t));
Y(timepoints(1):timepoints(2))=exp_growth(0,amps(1),durations(1),durations(1)/4);
Y(timepoints(2):timepoints(3))=exp_growth(amps(1),amps(2),durations(2)+1,durations(2));
Y(timepoints(3):timepoints(4))=exp_decay(amps(2),amps(3),durations(3)+1,durations(3)/4);
Y(timepoints(4):timepoints(5))=exp_decay(amps(3),amps(4),durations(4)+1,durations(4)/5);
Y=smooth_it(Y,3);
Y=Y-linspace(Y(1),Y(end),length(Y));

Y=[Y,zeros(1,N-length(Y))];

Nmid=floor(N/2);
[~,peakind]=max(abs(Y));
Y=circshift(Y,[0,Nmid-peakind]);

end

function test_synth_waveform
Y=synthesize_single_waveform(800);
figure; plot(Y);
end

function Y=exp_growth(amp1,amp2,dur1,dur2)
t=1:dur1;
Y=exp(t/dur2);
% Want Y(1)=amp1
% Want Y(end)=amp2
Y=Y/(Y(end)-Y(1))*(amp2-amp1);
Y=Y-Y(1)+amp1;
end

function Y=exp_decay(amp1,amp2,dur1,dur2)
Y=exp_growth(amp2,amp1,dur1,dur2);
Y=Y(end:-1:1);
end

function Z=smooth_it(Y,t)
Z=Y;
Z(1+t:end-t)=0;
for j=-t:t
    Z(1+t:end-t)=Z(1+t:end-t)+Y(1+t+j:end-t+j)/(2*t+1);
end;
end