declare name "Loud Speaker";
declare obs_id "audio_filter_faust_loudspeaker";

import("stdfaust.lib");

dur = hslider("[0]Duration [unit:s]", 0.30, 0, 2, 0.01);
feed = hslider("[1]Feedback", 0.20, 0, 1, 0.01);
cry = hslider("[2]Cry Baby", 0.7, 0, 1, 0.01);
k = ba.db2linear(hslider("[3]Output Gain [unit:dB]", 0, -30, 30, 0.1));

y(duration, feedback, cry) = ef.echo(2,dur,feed) : ve.crybaby(cry);
process = par(i,2,y(dur,feed,cry)) <: par(i,2,*(k));
