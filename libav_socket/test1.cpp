#include <iostream>
#include <vector>
#include <cstring>
#include "avtool.h"
#include <sockets.h>
enum RTSP_ERR
{
	
	RTSP_SOCKET_DISCONNECT=0,
	RTSP_SOCKET_ERR,
	RTSP_NO_REQUEST,
	RTSP_UNKNOW_OPTION,
	RTSP_UNKNOW_CSEQ
};
enum RTSP_STAT
{
	RTSP_OPTION=1,
	RTSP_OPTION_DESCRIBE,
	RTSP_OPTION_SETUP,
	RTSP_OPTION_PLAY,
	RTSP_OPTION_TEARDOWN
};
class RtspSession
{
private:
	tcpSender sock_hander;
	std::vector<std::string> reqest;
public:
	RtspSession(){}
	RtspSession(SOCKET _fd):sock_hander(_fd){}
	RtspSession(const tcpSender &tcpsender):sock_hander(tcpsender){}
	int read_reqest();
	int response_request();
	int get_mothod();
	int get_cseq();
	void dump_requst();
};
int RtspSession::read_request()
{
#ifndef K
#define K 1024
#endif
	char buf[K*2]={0};
	const char *end_mark="\r\n";
	char *end;
	int ret;
	char *p;
	char *cur;
	reqest.clear();
	ret=sock_hander.tcpRecv(buf, K*2);
	if(ret<0)
		ret=-RTSP_SOCKET_ERR;
	else if(ret==0)
		ret=-RTSP_SOCKET_DISCONNECT;
	end=buf+strlen(buf);
	cur=buf;
	do
	{
	p=strstr(cur,end_mark);
	if(!p||p+strlen(end_mark)==end)//search end
		break;
	*p=0;
	std::string s(cur);
	reqest.push_back(s);
	p+=strlen(end_mark);
	cur=p;
	}while(1);
	return ret;
}
int RtspSession::get_mothod()
{
	const char *cmd_option="OPTION";
	const char *cmd_desribe="DESCRIBE";
	const char *cmd_setup="SETUP";
	const char *cmd_play="PLAY";
	const char *cmd_teardown="TEARDOWN";
	if(reqest.empty())
		return -RTSP_NO_REQUEST;
	std::vector<std::string>::iterator iter=reqest.begin();
	if(!iter->compare(0,strlen(cmd_option),cmd_option))
		return RTSP_OPTION;
	else if(!iter->compare(0,strlen(cmd_desribe),cmd_desribe))
		return RTSP_OPTION_DESCRIBE;
	else if(!iter->compare(0,strlen(cmd_setup),cmd_setup))
		return RTSP_OPTION_SETUP;
	else if(!iter->compare(0,strlen(cmd_play),cmd_play))
		return RTSP_OPTION_PLAY;
	else if(!iter->compare(0,strlen(cmd_teardown),cmd_teardown))
		return RTSP_OPTION_TEARDOWN;
	else
		return RTSP_UNKNOW_OPTION;
}

int RtspSession::get_cseq()
{
	const char *cseq_mark="CSeq:";
	if(reqest.empty())
		return -RTSP_NO_REQUEST;
	for(std::vector<std::string>::iterator iter=reqest.begin();iter!=reqest.end();iter++)
	{
		 if(!iter->compare(0,strlen(cseq_mark),cseq_mark))
		 	return atoi(iter->c_str()+strlen(cseq_mark));
	}
	return -RTSP_UNKNOW_CSEQ;
	
}

int RtspSession::response_request()
{
}
void RtspSession::dump_requst()
{
	for(std::vector<std::string>::iterator iter=reqest.begin();iter!=reqest.end();iter++)
		std::cerr<<*iter<<std::endl;
}
int main()
{
	tcpListener rtsp_ctx(9999,20);
	RtspSession rtspsession(rtsp_ctx.tcpAccept());
	rtspsession.read_request();
	rtspsession.dump_requst();
	return 0;
}

#if 0
int main(int argc ,char **argv)
{
	int i,ret,count_a=0,count_v=0;
	AVPacket pkt;
	avformat_network_init();
	if(argc!=2)
	{
		printf("Two parameters !!!\n");
		return -1;
	}
	//const char *outfile="rtp://172.21.2.242:56789";
	const char *outfile="out.avi";
	av_input test(argv[1]);
	//av_output out(outfile,"rtp");
	av_output out(outfile);
	
	//video
	AVStream *sv=out.add_stream(test.get_video_codecID());
	AVCodecContext *c;
	if(sv)
	{
		c=sv->codec;
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;
		c->time_base=test.get_video_stream()->time_base;
		c->width=test.get_width();
		c->height=test.get_height();
		av_opt_set(c->priv_data, "preset", "ultrafast", 0);
		av_opt_set(c->priv_data, "tune","stillimage,fastdecode,zerolatency",0);
		av_opt_set(c->priv_data, "x264opts","crf=26:vbv-maxrate=728:vbv-bufsize=364:keyint=15",0);
	}
	
	//audio
	AVStream *sa=NULL;//=out.add_stream(test.get_audio_codecID());
	AVCodecContext *ca;
	if(sa)
	{
		ca=sa->codec;
		ca->flags |= CODEC_FLAG_GLOBAL_HEADER;
		printf("surce num %d den %d\n",test.get_audio_timebase().num,test.get_audio_timebase().den);
		printf("dest num %d den %d\n",sa->time_base.num,sa->time_base.den);
		//check_sample_fmt(ca->codec, test.get_audio_stream()->codec->sample_fmt);
		ca->sample_rate=test.get_audio_stream()->codec->sample_rate;
		ca->sample_fmt=test.get_audio_stream()->codec->sample_fmt;
		ca->channels=test.get_audio_stream()->codec->channels;
		ca->bit_rate=test.get_audio_stream()->codec->bit_rate;
	}

	out.write_header();
	char sdp[2000];
	//av_sdp_create(out.get_ctx_pri(),1,sdp,2000);
	//printf("RTP:SDP\n%s",sdp);
	while(!test.get_frame(&pkt))
	{
		if(pkt.stream_index==test.get_video_index())
		{
			//printf("video : pts %10lld dts %10lld\n",pkt.pts,pkt.dts);
			pkt.pts=av_rescale_q(pkt.pts,test.get_video_timebase(),sv->time_base);
			pkt.stream_index=sv->index;
		}
		else if(0&&pkt.stream_index==test.get_audio_index())
		{
			//printf("audio : pts %10lld dts %10lld\n",pkt.pts,pkt.dts);
			pkt.pts=av_rescale_q(pkt.pts,ca->time_base,sa->time_base);
			pkt.stream_index=sa->index;
		}
		else
			{av_free_packet(&pkt);continue; }
		pkt.dts=pkt.pts;
		out.write_frame(&pkt);
		//usleep(1000000/test.get_video_fps());
		//printf("write\n");
		av_free_packet(&pkt);
	}
	out.write_tailer();
	return 0;
}
#endif
