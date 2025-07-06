declare name "Echo";
declare obs_id "audio_filter_faust_echo";

import("stdfaust.lib");

dur = hslider("[0]Duration [unit:s]", 0.20, 0, 2, 0.01);
feed = hslider("[1]Feedback", 0.15, 0, 1, 0.01);
k = ba.db2linear(hslider("[2]Output Gain [unit:dB]", 0, -30, 30, 0.1));

process = par(i,2,ef.echo(2,dur,feed)) <: par(i,2,*(k));
