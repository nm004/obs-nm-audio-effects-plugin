declare name "Reverb - Stereo (Zita-rev1)";
declare obs_id "audio_filter_faust_zita_rev1_stereo";

import("stdfaust.lib");

rdel = hslider("[0]Delay [unit:ms]", 40, 20, 100, 1);
f1 = nentry("[1]F1 [unit:Hz]", 100, 20, 10000, 1);
f2 = nentry("[2]F2 [unit:Hz]", 3000, 20, 10000, 1);
t60dc = hslider("[3]T60DC [unit:s]", 3.0, 1, 8, 0.1);
t60m = hslider("[4]T60M [unit:s]", 4.0, 1, 8, 0.1);
k = ba.db2linear(hslider("[5]Output Gain [unit:dB]", 0, -30, 30, 0.1));

process = re.zita_rev1_stereo(rdel, f1, f2, t60dc, t60m, ma.SR) : par(i,2,*(k));
