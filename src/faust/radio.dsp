declare name "Radio";
declare obs_id "audio_filter_faust_radio";

import("stdfaust.lib");

k0 = ba.db2linear(hslider("[0]Input Gain [unit:dB]", 0, 0, 60, 0.1));
off = hslider("[1]Input Offset", 0, 0, 0.300, 0.001) : si.bsmooth;
cry = hslider("[2]Cry Baby", 0.5, 0, 1, 0.01);
w = ba.db2linear(hslider("[3]Noise Level [unit:dB]", -18, -60, 0, 0.1));
fc = nentry("[4]Cutoff [unit:Hz]", 3400, 20, 10000, 1);
k = ba.db2linear(hslider("[5]Output Gain [unit:dB]", 0, -30, 30, 0.1));

y(k0, off, cry, w, fc)
    = +(off) : *(k0) : (ef.softclipQuadratic) : fi.dcblocker
    : ve.crybaby(cry) <: (_, *(w*(no.noise))) :> fi.lowpass(3, fc);
process = par(i,2,y(k0,off,cry,w,fc)) <: par(i,2,*(k));