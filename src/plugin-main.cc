#include "obs-module.h"
#include "plugin-support.h"
#include "faust-obs.h"
#include "basicfilter.h"
#include "echo.h"
#include "loudspeaker.h"
#include "radio.h"
#include "zitarev1.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

using namespace nm;

namespace {

const obs_source_info basic_filter = make_faust_audio_filter<BasicFilter>();
const obs_source_info echo = make_faust_audio_filter<Echo>();
const obs_source_info loud_speaker = make_faust_audio_filter<LoudSpeaker>();
const obs_source_info radio = make_faust_audio_filter<Radio>();
const obs_source_info zita_rev1 = make_faust_audio_filter<ZitaRev1>();

}

bool obs_module_load(void)
{
	obs_register_source(&basic_filter);
	obs_register_source(&echo);
	obs_register_source(&loud_speaker);
	obs_register_source(&radio);
	obs_register_source(&zita_rev1);
	obs_log(LOG_INFO, "plugin loaded successfully (version %s)",
		PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "plugin unloaded");
}
