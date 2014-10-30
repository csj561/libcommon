#include "avtool.h"

bool av_init::is_register=false;
av_input::av_input(const std::string &_filename):in_ctx(NULL),filename(_filename),is_opened(false)
{
	int ret;
	av_init_packet(&pkt);
	if((ret=avformat_open_input(&in_ctx,filename.c_str(),NULL,NULL))<0)
	{
		AV_ERR(ret);
		return ;
	}
	audio_stream_index=av_find_best_stream(in_ctx, AVMEDIA_TYPE_AUDIO, -1,-1,NULL,0);
	if(audio_stream_index<0)
		ERR("Can't find audio stream in this file");
	video_stream_index=av_find_best_stream(in_ctx, AVMEDIA_TYPE_VIDEO, -1,-1,NULL,0);
	if(video_stream_index<0)
		ERR("Can't find video stream in this file");
	avformat_find_stream_info(in_ctx,NULL);
	is_opened=true;
}
int av_input::get_frame(AVPacket *outbuf)
{
	if(!outbuf)
	{
		ERR("Outbuf Can't be NULL!");
		return -1;
	}
	av_init_packet(outbuf);
	return av_read_frame(in_ctx, outbuf);
}
int av_input::get_video_frame(AVPacket *outbuf)
{
	if(video_stream_index<0)
		return -1;
	do{
	int ret=get_frame(outbuf);
	if(ret<0)
		return ret;
	if(outbuf->stream_index==video_stream_index)
		break;
	av_free_packet(outbuf);
	}while(1);
	return 0;
}
int av_input::get_audio_frame(AVPacket *outbuf)
{
	if(audio_stream_index<0)
		return -1;
	do{
	int ret=get_frame(outbuf);
	if(ret<0)
		return ret;
	if(outbuf->stream_index==audio_stream_index)
		break;
	av_free_packet(outbuf);
	}while(1);
	return 0;
}

av_output::av_output(const std::string &_filename):out_ctx(NULL),is_opend(false)
{
	int ret=0;
	ret=avformat_alloc_output_context2(&out_ctx, NULL, NULL, _filename.c_str());
	if(ret<0)
	{AV_ERR(ret);return;}
	ret=avio_open(&out_ctx->pb, out_ctx->filename, AVIO_FLAG_WRITE);
	if(ret<0)
	{AV_ERR(ret);return;}
	is_opend=true;
}

av_output::av_output(const std::string &_filename,const std::string &_short_name):out_ctx(NULL),is_opend(false)
{
	int ret=0;
	ret=avformat_alloc_output_context2(&out_ctx, NULL,_short_name.c_str(), _filename.c_str());
	//sprintf(out_ctx->filename,"%s",_filename.c_str());
	if(ret<0)
	{AV_ERR(ret);return;}
	ret=avio_open(&out_ctx->pb, out_ctx->filename, AVIO_FLAG_WRITE);
	if(ret<0)
	{AV_ERR(ret);return;}
	is_opend=true;
}
AVStream *av_output::add_stream(int codec_id)
{
	AVCodec *codec=avcodec_find_encoder((enum AVCodecID)codec_id);
	if(!codec)
		return NULL;
	return avformat_new_stream(out_ctx,codec);
}
