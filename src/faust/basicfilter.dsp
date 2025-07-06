declare name "Basic Filter";
declare obs_id "audio_filter_faust_basicfilter";

import("stdfaust.lib");

fc = nentry("[0]Cutoff [unit:Hz]", 100, 20, 10000, 1);

process = par(i,2,fi.highpass(4, fc)) <: _,_;
