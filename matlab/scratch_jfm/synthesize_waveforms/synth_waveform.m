function Y=synth_waveform(N,durations,amps)

if (nargin<1) test_synth_waveform; return; end

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

close all;

X=readmda('waveforms.mda');
%figure; ms_view_templates(X);
Y=reshape(permute(X,[2,1,3]),size(X,2),size(X,1)*size(X,3));
[FF,subspace]=ms_event_features(reshape(Y,1,size(Y,1),size(Y,2)),10);
%figure; ms_view_templates(subspace);
figure; plot(subspace(1,:,1));

durations=[200,10,30,200];
amps=[0.5,10,-1,0];

Y=synth_waveform(1000,durations,amps);

Yhat=fftshift(fft(fftshift(Y)));
figure; plot(1:length(Yhat),real(Yhat),'b',1:length(Yhat),imag(Yhat),'r');

figure; plot(1:length(Y),Y);

M=5;
T=800;
K=20;
waveforms=zeros(M,T,K);
for k=1:K
    loc=(k-1)/(K-1)*(M+1);
    for m=1:M
        dist=abs(loc-m);
        durations0=durations+floor([randn*10,randn*4,randn*6,randn*20]);
        amps0=amps+[randn*0.2,randn*3,randn*0.5,0];
        waveform0=synth_waveform(T,durations0,amps0);
        disp(dist);
        waveforms(m,:,k)=waveform0/(0.2+dist);
    end;
end;

figure; ms_view_templates(waveforms);


% figure;
% for j=1:10
%     durations0=durations+floor([randn*10,randn*4,randn*6,randn*20]);
%     amps0=amps+[randn*0.2,randn*3,randn*0.5,0];
%     Y0=synth_waveform(1000,durations0,amps0);
%     plot(1:length(Y0),Y0);
%     hold on;
% end;


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