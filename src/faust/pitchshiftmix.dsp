declare name "Pitch Shift Mix";
declare obs_id "audio_filter_pitch_shift_mix";

import("stdfaust.lib");

g1 = ba.db2linear(hslider("v:[0]Shifter1/[0]Gain [unit:dB]", 0, -60, 0, 0.1));
st1 = hslider("v:[0]Shifter1/[1]Shift [unit:st]", -1, -12, 12, 0.01);

g2 = ba.db2linear(hslider("v:[1]Shifter2/[0]Gain [unit:dB]", 0, -60, 0, 0.1));
st2 = hslider("v:[1]Shifter2/[1]Shift [unit:st]", 1, -12, 12, 0.01);

k = ba.db2linear(hslider("[2]Output Gain [unit:dB]", 0, -30, 30, 0.1));

y(g1, st1, g2, st2) = (g1*ef.transpose(256, 256, st1), g2*ef.transpose(256, 256, st2)) :> _;
process = par(i,2,y(g1,st1,g2,st2)) <: par(i,2,*(k));
