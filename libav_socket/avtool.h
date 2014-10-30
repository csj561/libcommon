#include <iostream>
extern "C"{
#ifdef __cplusplus
 #define __STDC_CONSTANT_MACROS
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
#endif
}
//#define __STDC_FORMAT_MACROS
extern "C"
{
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
}
#include <string>
#define AV_ERR(x) do{if(1){\
	fprintf(stderr,"ERROR: %s %d: %s\n",__FILE__,__LINE__,av_err2str(ret));\
	}}while(0)
#define ERR(x,...) do{if(1){\
	fprintf(stderr,"ERROR: %s %d: "x"\n",__FILE__,__LINE__,##__VA_ARGS__);\
	}}while(0)
class av_init
{
	static bool is_register;
public:
	av_init(){if(!is_register) {av_register_all();is_register=true;}}
};


class av_input: public av_init
{
	AVFormatContext *in_ctx;
	AVPacket pkt;
	std::string filename;
	bool is_opened;
	int audio_stream_index,video_stream_index;
public:
	av_input(const std::string &);
	~av_input(){if(is_opened){avformat_close_input(&in_ctx);avformat_free_context(in_ctx);}}
	int get_frame(AVPacket *);
	int get_audio_frame(AVPacket *);
	int get_video_frame(AVPacket *);
	int get_video_index(){return video_stream_index;}
	int get_audio_index(){return audio_stream_index;}
	AVStream *get_video_stream(){if(video_stream_index<0) return NULL;return in_ctx->streams[video_stream_index];}
	AVStream *get_audio_stream(){if(audio_stream_index<0) return NULL;return in_ctx->streams[audio_stream_index];}
	int get_width(){if(video_stream_index<0) return -1;return in_ctx->streams[video_stream_index]->codec->width;}
	int get_height(){if(video_stream_index<0) return -1;return in_ctx->streams[video_stream_index]->codec->height;}
	int get_video_codecID(){if(video_stream_index<0) return -1;return in_ctx->streams[video_stream_index]->codec->codec_id;}
	int get_audio_codecID(){if(audio_stream_index<0) return -1;return in_ctx->streams[audio_stream_index]->codec->codec_id;}
	int get_video_fps(){if(video_stream_index<0) return -1;return in_ctx->streams[video_stream_index]->r_frame_rate.num;}
	int get_audio_fps(){if(audio_stream_index<0) return -1;return
		in_ctx->streams[audio_stream_index]->codec->time_base.den/in_ctx->streams[audio_stream_index]->codec->time_base.num;}
	AVRational get_video_timebase(){AVRational ret={0,0};if(video_stream_index<0) return ret;return in_ctx->streams[video_stream_index]->time_base;}
	AVRational get_audio_timebase(){AVRational ret={0,0};if(audio_stream_index<0) return ret;return in_ctx->streams[audio_stream_index]->time_base;}
	void dump(){av_dump_format(in_ctx,0,filename.c_str(),0);}
	AVFormatContext *get_ctx(){return in_ctx;}
};


class av_output:public av_init
{
	AVFormatContext *out_ctx;
	bool is_opend;
public:
	av_output(const std::string & );
	av_output(const std::string &,const std::string &);
	int write_header(){return avformat_write_header(out_ctx,NULL);}
	int write_frame(AVPacket *pkt){return av_interleaved_write_frame(out_ctx,pkt);}
	int write_tailer(){ return av_write_trailer(out_ctx);}
	AVStream *add_stream(int codec_id);
	void dump(){av_dump_format(out_ctx,0,out_ctx->filename,0);}
	AVFormatContext *get_ctx(){return out_ctx;}
	~av_output(){if(is_opend){avio_close(out_ctx->pb);avformat_free_context(out_ctx);}}
};