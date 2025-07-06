#include "obs-module.h"
#include "faust/dsp/dsp.h"
#include "faust/gui/meta.h"
#include "faust/gui/UI.h"
#include "faust/gui/PathBuilder.h"
#include "faust/gui/MetaDataUI.h"
#include <string_view>
#include <unordered_map>
#include <vector>

namespace nm::obs::detail {

enum class OBSPropType {
	Number,
	Slider,
};

struct obs_prop_info {
	const char *name;
	const char *desc;
	const char *suffix;
	FAUSTFLOAT *zone;
	FAUSTFLOAT init;
	FAUSTFLOAT min;
	FAUSTFLOAT max;
	FAUSTFLOAT step;
	OBSPropType prop_type;
};

class OBSUI final : public UI, PathBuilder, MetaDataUI {
public:
        void openTabBox(const char* label) override
        {
		pushLabel(label);
        }

        void openHorizontalBox(const char* label) override
        {
		pushLabel(label);
        }

        void openVerticalBox(const char* label) override
        {
		pushLabel(label);
        }

        void closeBox() override
        {
		if (!popLabel()) {
			return;
		}

		for (int i = 0; i < prop_info.size(); i++) {
			auto &p{prop_info[i]};
			p.name = fFullPaths[i].data();
			p.suffix = fUnit[p.zone].data();
		}
        }
        
        void addButton(const char* label, FAUSTFLOAT* zone) override
        {
        }

        void addCheckButton(const char* label, FAUSTFLOAT* zone) override
        {
        }

        void addVerticalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT fmin, FAUSTFLOAT fmax, FAUSTFLOAT step) override
        {
		addHorizontalSlider(label, zone, init, fmin, fmax, step);
        }

        void addHorizontalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT fmin, FAUSTFLOAT fmax, FAUSTFLOAT step) override
        {
		addFullPath(label);
		// Both name and suffix are filled later.
		prop_info.push_back({nullptr, label, nullptr, zone, init, fmin, fmax, step, OBSPropType::Slider});
        }

        void addNumEntry(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT fmin, FAUSTFLOAT fmax, FAUSTFLOAT step) override
        {
		addFullPath(label);
		prop_info.push_back({nullptr, label, nullptr, zone, init, fmin, fmax, step, OBSPropType::Number});
        }

        void addHorizontalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT fmin, FAUSTFLOAT fmax) override
        {
        }

        void addVerticalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT fmin, FAUSTFLOAT fmax) override
        {
        }
    
        void addSoundfile(const char* label, const char* filename, Soundfile** sf_zone) override {}

        void declare(FAUSTFLOAT* zone, const char* key, const char* val) override
	{
		MetaDataUI::declare(zone, key, val);
	}

	constexpr const std::vector<obs_prop_info> &get_prop_info()
	{
		return prop_info;
	}

private:
	std::vector<obs_prop_info> prop_info;
};


template <class DSP>
struct Data {
	DSP dsp;
	OBSUI ui;
};

struct TypeData {
	const char *name;
	OBSUI ui;
};

inline const char *get_name(void *type_data)
{
	return (static_cast<TypeData *>(type_data))->name;
}

template <class DSP>
void update(void *data, obs_data_t *settings);

template <class DSP>
void *create(obs_data_t *settings, obs_source_t *source)
{
	auto data{new (bmalloc(sizeof (Data<DSP>))) Data<DSP>};
	auto audio{obs_get_audio()};
	data->dsp.buildUserInterface(&data->ui);
	data->dsp.init(audio_output_get_sample_rate(audio));

	update<DSP>(data, settings);
	return data;
}

template <class DSP>
void destroy(void *data_)
{
	auto data{static_cast<Data<DSP> *>(data_)};
	data->dsp.~DSP();
	bfree(data_);
}

template <class DSP>
void update(void *data_, obs_data_t *settings)
{
	auto data{static_cast<Data<DSP> *>(data_)};
	for (const auto &p: data->ui.get_prop_info()) {
		*p.zone = static_cast<FAUSTFLOAT>(obs_data_get_double(settings, p.name));
	}
}

template <class DSP>
obs_audio_data *filter_audio(void *data_, obs_audio_data *audio)
{
	auto data{static_cast<Data<DSP> *>(data_)};
	auto adata{reinterpret_cast<float **>(audio->data)};
	data->dsp.compute(audio->frames, adata, adata);
	return audio;
}

inline void free_type_data(void *type_data_)
{
	auto type_data{reinterpret_cast<TypeData *>(type_data_)};
	type_data->~TypeData();
	bfree(type_data_);
}

void get_defaults2(void *type_data_, obs_data_t *settings)
{
	auto type_data{reinterpret_cast<TypeData *>(type_data_)};
	for (const auto &p: type_data->ui.get_prop_info()) {
		obs_data_set_default_double(settings, p.name, p.init);
	}
}

template <class DSP>
obs_properties_t *get_properties2(void * /* data */, void *type_data_)
{
	auto type_data{reinterpret_cast<TypeData *>(type_data_)};
 	auto ppts{obs_properties_create()};
	for (const auto &p: type_data->ui.get_prop_info()) {
		obs_property_t *prop;
		switch (p.prop_type)
		{
		case OBSPropType::Number:
			prop = obs_properties_add_float(ppts, p.name, p.desc, p.min, p.max, p.step);
			break;
		case OBSPropType::Slider:
			prop = obs_properties_add_float_slider(ppts, p.name, p.desc, p.min, p.max, p.step);
			break;
		default:
			continue;
		}
		obs_property_float_set_suffix(prop, p.suffix);
	}
	return ppts;
}

} // namespace nm::obs::detail

namespace nm {

template <class DSP>
obs_source_info make_faust_audio_filter()
{
	using namespace nm::obs::detail;
	using namespace std;

	struct final : Meta {
		std::unordered_map<std::string_view, std::string_view> m;
		void declare(const char *key, const char *val) override { m.emplace(key, val); }
	} m;

	auto type_data{new (bmalloc(sizeof(TypeData))) TypeData};

	auto dsp{new (bmalloc(sizeof(DSP))) DSP};
	dsp->metadata(&m);
	dsp->buildUserInterface(&type_data->ui);
	dsp->~DSP();
	bfree(dsp);

	type_data->name = m.m.at("name").data();
	return {
		.id = m.m.at("obs_id").data(),
		.type = OBS_SOURCE_TYPE_FILTER,
		.output_flags = OBS_SOURCE_AUDIO,
		.get_name = get_name,
		.create = create<DSP>,
		.destroy = destroy<DSP>,
		.update = update<DSP>,
		.filter_audio = filter_audio<DSP>,
		.type_data = type_data,
		.free_type_data = free_type_data,
		.get_defaults2 = get_defaults2,
		.get_properties2 = get_properties2<DSP>,
	};
}

} // namespace nm
