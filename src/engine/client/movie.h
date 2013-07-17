/*
    unsigned char*
*/
#define ENGINE_CLIENT_MOVIE_H
#ifndef ENGINE_CLIENT_MOVIE_H
#define ENGINE_CLIENT_MOVIE_H

#include <engine/movie.h>


class CRecordMovie : public IRecordMovie
{
public:
	//IEngineGraphics *m_pGraphics;
	//IStorageTW *m_pStorage;

	virtual void Init();

	CRecordMovie();

    void DemoToMovie(const char *demo);

private:
    bool RecordScreen(const char *filename, int codec_id);


    /**************************************************************/
    /* audio output */

    float t, tincr, tincr2;
    int16_t *samples;
    uint8_t *audio_outbuf;
    int audio_outbuf_size;
    int audio_input_frame_size;

    AVStream *add_audio_stream(AVFormatContext *oc, int codec_id);
    void open_audio(AVFormatContext *oc, AVStream *st);
    void get_audio_frame(int16_t *samples, int frame_size, int nb_channels);
    void write_audio_frame(AVFormatContext *oc, AVStream *st);
    void close_audio(AVFormatContext *oc, AVStream *st);

    /**************************************************************/
    /* video output */

    AVFrame *picture, *tmp_picture;
    uint8_t *video_outbuf;
    int frame_count, video_outbuf_size;

    AVStream *add_video_stream(AVFormatContext *oc, int codec_id);
    AVFrame *alloc_picture(int pix_fmt, int width, int height);
    void open_video(AVFormatContext *oc, AVStream *st);
    void fill_yuv_image(AVFrame *pict, int frame_index, int width, int height);
    void write_video_frame(AVFormatContext *oc, AVStream *st);
    void close_video(AVFormatContext *oc, AVStream *st);

};

#endif
