function synth_waveforms

M=4; % number of channels
T=1000; % number of timepoints

W=zeros(M,T,0);

t=linspace(-1,1,T); 
fr=3;
rp=4;

k=0;
firing_rates=[];
refractory_periods=[];

k=k+1;
W(1,:,k)=make_waveform(t,1,10,60,10);
W(2,:,k)=make_waveform(t,0.3,10,60,10);
W(3,:,k)=make_waveform(t,0.1,10,60,10);
W(4,:,k)=make_waveform(t,0.1,10,60,10);
firing_rates(k)=fr;
refractory_periods(k)=rp;

k=k+1;
W(1,:,k)=make_waveform(t,0.5,10,60,20);
W(2,:,k)=make_waveform(t,1,10,60,20);
W(3,:,k)=make_waveform(t,0.1,10,60,20);
W(4,:,k)=make_waveform(t,0.1,10,60,20);
firing_rates(k)=fr;
refractory_periods(k)=rp;

k=k+1;
W(1,:,k)=make_waveform(t,0.2,10,60,30);
W(2,:,k)=make_waveform(t,0.8,10,60,30);
W(3,:,k)=make_waveform(t,1,10,60,30);
W(4,:,k)=make_waveform(t,0.2,10,60,30);
firing_rates(k)=fr;
refractory_periods(k)=rp;

k=k+1;
W(1,:,k)=make_waveform(t,0.2,10,50,30);
W(2,:,k)=make_waveform(t,0.8,8,140,30);
W(3,:,k)=make_waveform(t,1,15,80,30);
W(4,:,k)=make_waveform(t,0.1,10,50,30);
firing_rates(k)=fr;
refractory_periods(k)=rp;

k=k+1;
W(1,:,k)=make_waveform(t,0.1,10,60,40);
W(2,:,k)=make_waveform(t,0.2,10,60,40);
W(3,:,k)=make_waveform(t,0.2,10,60,40);
W(4,:,k)=make_waveform(t,0.4,10,60,40);
firing_rates(k)=fr/2;
refractory_periods(k)=rp;

k=k+1;
W(1,:,k)=make_waveform(t,0.1,12,60,20);
W(2,:,k)=make_waveform(t,0.2,12,60,20);
W(3,:,k)=make_waveform(t,0.2,12,60,20);
W(4,:,k)=make_waveform(t,0.4,12,60,20);
firing_rates(k)=fr/2;
refractory_periods(k)=rp;

k=k+1;
W(1,:,k)=make_waveform(t,0.1,12,60,20);
W(2,:,k)=make_waveform(t,0.1,12,60,20);
W(3,:,k)=make_waveform(t,0.5,8,60,20);
W(4,:,k)=make_waveform(t,0.5,8,60,20);
firing_rates(k)=fr/2;
refractory_periods(k)=rp;

k=k+1;
W(1,:,k)=make_waveform(t,0.8,14,80,30);
W(2,:,k)=make_waveform(t,0.8,14,80,30);
W(3,:,k)=make_waveform(t,0.8,14,80,30);
W(4,:,k)=make_waveform(t,0.72,14,80,30);
firing_rates(k)=fr/2;
refractory_periods(k)=rp;

figure; 
ms_view_templates(W);

K=k;
writemda(W,'waveforms.mda');
info=zeros(0,K);
info(1,:)=firing_rates'; %spikes per second
info(2,:)=refractory_periods'; %milliseconds
info(3,:)=0.7;
info(4,:)=1; %vscale
info(5,:)=0.7;
info(6,:)=1; %hscale
writemda(info,'info.mda');
info


%figure; plot(W1);
%figure; plot(W2);

%figure; plot(x,W1,'b.',x,W2,'r.',x,W3,'g.',x,W4,'m.');

end

function y=make_waveform(x,a,b,c,d)

%a=1; %amplitude of main peak
%b=10; %sharpnessof main peak
%c=60; %amplitude of second peak
%d=30; %sharpness of second peak

y = a*exp(-(x*b).^2);
y = y - a*c*(x>0).*(x*d/10).^2.*exp(-((x*d)));

end