#pragma once

#include <av/Frame.hpp>
#include <av/common.hpp>

namespace av
{

class Resample : NoCopyable
{
	explicit Resample(SwrContext* swr) noexcept
	    : swr_(swr)
	{}

public:
	static Expected<Ptr<Resample>> create(int inChannels, AVSampleFormat inSampleFmt, int inSampleRate,
	                                      int outChannels, AVSampleFormat outSampleFmt, int outSampleRate) noexcept
	{
		/*
          * Create a resampler context for the conversion.
          * Set the conversion parameters.
          * Default channel layouts based on the number of channels
          * are assumed for simplicity (they are sometimes not detected
          * properly by the demuxer and/or decoder).
          */

		LOG_AV_DEBUG("Creating swr context: input - channel_layout: {} sample_rate: {} format: {} output - channel_layout: {} sample_rate: {} format: {}",
		             av_get_default_channel_layout(inChannels), inSampleRate, av_get_sample_fmt_name(inSampleFmt),
		             av_get_default_channel_layout(outChannels), outSampleRate, av_get_sample_fmt_name(outSampleFmt));

		auto swr = swr_alloc_set_opts(nullptr,
		                              av_get_default_channel_layout(outChannels),
		                              outSampleFmt,
		                              outSampleRate,
		                              av_get_default_channel_layout(inChannels),
		                              inSampleFmt,
		                              inSampleRate,
		                              0, nullptr);

		if (!swr)
			RETURN_AV_ERROR("Failed to create swr context");

		/* Open the resampler with the specified parameters. */
		int err = 0;
		if ((err = swr_init(swr)) < 0)
		{
			swr_free(&swr);
			RETURN_AV_ERROR("Could not open resample context: {}", avErrorStr(err));
		}

		return Ptr<Resample>{new Resample{swr}};
	}

	~Resample()
	{
		if (swr_)
			swr_free(&swr_);
	}

SwrContext* native() noexcept
	{
		return swr_;
	}
	const SwrContext* native() const noexcept
	{
		return swr_;
	}

  Expected<void> convertData(uint8_t **out, int out_count, const uint8_t **in, int in_count) noexcept
	{
			auto err = swr_convert(swr_, out, out_count, in, in_count);
			if (err < 0) {
				RETURN_AV_ERROR("Could not convert input samples: {}", avErrorStr(err));
			}
			return {};
	}

	Expected<void> convert(const Frame& input, Frame& output) noexcept
	{
		//LOG_AV_DEBUG("input - channel_layout: {} sample_rate: {} format: {}", input.native()->channel_layout, input.native()->sample_rate, av_get_sample_fmt_name((AVSampleFormat)input.native()->format));
		//LOG_AV_DEBUG("output - channel_layout: {} sample_rate: {} format: {}", output.native()->channel_layout, output.native()->sample_rate, av_get_sample_fmt_name((AVSampleFormat)output.native()->format));
		/* Convert the samples using the resampler. */
		/* auto oFrame = output.native();
	  auto format = oFrame->format;
		auto channels = oFrame->channels;
		auto channel_layout = oFrame->channel_layout;
		auto sample_rate = oFrame->sample_rate;


		// memset(oFrame, 0, sizeof(AVFrame));

		oFrame->format = format;
    oFrame->channels = channels;
    oFrame->channel_layout = channel_layout;
    oFrame->sample_rate = sample_rate;

/*
		oFrame->nb_samples = int(swr_get_delay(swr_, sample_rate))
                        + input.native()->nb_samples
                        * sample_rate
                        / input.native()->sample_rate
                        + 3;
*/

		// oFrame->nb_samples = av_rescale_rnd( input.native()->nb_samples + swr_get_delay(swr_, input.native()->sample_rate), sample_rate, input.native()->sample_rate, AV_ROUND_UP);

		//printf("isamples: %d SAMPLES: %d delay %d\n", input.native()->nb_samples, output.native()->nb_samples, swr_get_delay(swr_, output.native()->sample_rate));

	/*static AVFrame *reframe = av_frame_alloc();
				reframe->channel_layout =  3;
				reframe->channels = 2;
				reframe->sample_rate = 48000;
				reframe->format = AV_SAMPLE_FMT_FLTP; */
		// output.native()->linesize[0] = 0;
		// output.native()->nb_samples = 120;
	//	auto err = swr_convert_frame(swr_, nullptr, *input);
 		output.native()->linesize[0] = 0;
		auto err = swr_convert_frame(swr_, *output, *input);
		// auto err = swr_convert_frame(swr_, reframe, *input);
		if (err < 0)
			RETURN_AV_ERROR("Could not convert input samples: {}", avErrorStr(err));

		//output.native()->linesize[0] = 2*4*120;
		//output.native()->nb_samples = 120;
		//err = swr_convert_frame(swr_, *output, nullptr);
// 		auto err = swr_convert_frame(swr_, *output, *input);
		// auto err = swr_convert_frame(swr_, reframe, *input);
		//if (err < 0)
		//	RETURN_AV_ERROR("Could not convert input samples: {}", avErrorStr(err));



		// av_frame_copy(*output, reframe);

		return {};
	}

private:
	SwrContext* swr_{nullptr};
};

}// namespace av
