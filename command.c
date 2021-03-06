#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>

#include "config.h"
#include "input/input.h"
#include "stream/stream.h"
#include "libmpdemux/demuxer.h"
#include "libmpdemux/stheader.h"
#include "mplayer.h"
#include "libvo/sub.h"
#include "m_option.h"
#include "m_property.h"
#include "help_mp.h"
#include "metadata.h"
#include "libmpcodecs/mp_image.h"
#include "libmpcodecs/vf.h"
#include "libmpcodecs/vd.h"
#include "libvo/video_out.h"
#include "playtree.h"
#include "libao2/audio_out.h"
#include "mpcommon.h"
#include "mixer.h"
#include "libmpdemux/matroska.h"
#include "libmpcodecs/dec_video.h"
#include "vobsub.h"
#include "spudec.h"
#ifdef USE_TV
#include "stream/tv.h"
#endif
#ifdef USE_RADIO
#include "stream/stream_radio.h"
#endif
#ifdef HAS_DVBIN_SUPPORT
#include "stream/dvbin.h"
#endif
#ifdef USE_DVDREAD
#include "stream/stream_dvd.h"
#endif
#ifdef USE_DVDNAV
#include "stream/stream_dvdnav.h"
#endif
#ifdef USE_ASS
#include "libass/ass.h"
#include "libass/ass_mp.h"
#endif
#ifdef HAVE_NEW_GUI
#include "Gui/interface.h"
#endif

#include "mp_core.h"

#define ROUND(x) ((int)((x)<0 ? (x)-0.5 : (x)+0.5))

static void rescale_input_coordinates(int ix, int iy, double *dx, double *dy)
{
    //remove the borders, if any, and rescale to the range [0,1],[0,1]
    if (vo_fs) {		//we are in full-screen mode
	if (vo_screenwidth > vo_dwidth)	//there are borders along the x axis
	    ix -= (vo_screenwidth - vo_dwidth) / 2;
	if (vo_screenheight > vo_dheight)	//there are borders along the y axis (usual way)
	    iy -= (vo_screenheight - vo_dheight) / 2;

	if (ix < 0 || ix > vo_dwidth) {
	    *dx = *dy = -1.0;
	    return;
	}			//we are on one of the borders
	if (iy < 0 || iy > vo_dheight) {
	    *dx = *dy = -1.0;
	    return;
	}			//we are on one of the borders
    }

    *dx = (double) ix / (double) vo_dwidth;
    *dy = (double) iy / (double) vo_dheight;

    mp_msg(MSGT_CPLAYER, MSGL_V,
	   "\r\nrescaled coordinates: %.3lf, %.3lf, screen (%d x %d), vodisplay: (%d, %d), fullscreen: %d\r\n",
	   *dx, *dy, vo_screenwidth, vo_screenheight, vo_dwidth,
	   vo_dheight, vo_fs);
}

static int sub_source(MPContext * mpctx)
{
    int source = -1;
    int top = -1;
    int i;
    for (i = 0; i < SUB_SOURCES; i++) {
	int j = mpctx->global_sub_indices[i];
	if ((j >= 0) && (j > top) && (mpctx->global_sub_pos >= j)) {
	    source = i;
	    top = j;
	}
    }
    return source;
}

/**
 * \brief Log the currently displayed subtitle to a file
 *
 * Logs the current or last displayed subtitle together with filename
 * and time information to ~/.mplayer/subtitle_log
 *
 * Intended purpose is to allow convenient marking of bogus subtitles
 * which need to be fixed while watching the movie.
 */

static void log_sub(void)
{
    char *fname;
    FILE *f;
    int i;

    if (subdata == NULL || vo_sub_last == NULL)
	return;
    fname = get_path("subtitle_log");
    f = fopen(fname, "a");
    if (!f)
	return;
    fprintf(f, "----------------------------------------------------------\n");
    if (subdata->sub_uses_time) {
	fprintf(f,
		"N: %s S: %02ld:%02ld:%02ld.%02ld E: %02ld:%02ld:%02ld.%02ld\n",
		filename, vo_sub_last->start / 360000,
		(vo_sub_last->start / 6000) % 60,
		(vo_sub_last->start / 100) % 60, vo_sub_last->start % 100,
		vo_sub_last->end / 360000, (vo_sub_last->end / 6000) % 60,
		(vo_sub_last->end / 100) % 60, vo_sub_last->end % 100);
    } else {
	fprintf(f, "N: %s S: %ld E: %ld\n", filename, vo_sub_last->start,
		vo_sub_last->end);
    }
    for (i = 0; i < vo_sub_last->lines; i++) {
	fprintf(f, "%s\n", vo_sub_last->text[i]);
    }
    fclose(f);
}


/// \defgroup Properties
///@{

/// \defgroup GeneralProperties General properties
/// \ingroup Properties
///@{

/// OSD level (RW)
static int mp_property_osdlevel(m_option_t * prop, int action, void *arg,
				MPContext * mpctx)
{
    return m_property_choice(prop, action, arg, &osd_level);
}

/// Playback speed (RW)
static int mp_property_playback_speed(m_option_t * prop, int action,
				      void *arg, MPContext * mpctx)
{
    switch (action) {
    case M_PROPERTY_SET:
	if (!arg)
	    return M_PROPERTY_ERROR;
	M_PROPERTY_CLAMP(prop, *(float *) arg);
	playback_speed = *(float *) arg;
	build_afilter_chain(mpctx->sh_audio, &ao_data);
	return M_PROPERTY_OK;
    case M_PROPERTY_STEP_UP:
    case M_PROPERTY_STEP_DOWN:
	playback_speed += (arg ? *(float *) arg : 0.1) *
	    (action == M_PROPERTY_STEP_DOWN ? -1 : 1);
	M_PROPERTY_CLAMP(prop, playback_speed);
	build_afilter_chain(mpctx->sh_audio, &ao_data);
	return M_PROPERTY_OK;
    }
    return m_property_float_range(prop, action, arg, &playback_speed);
}

/// filename with path (RO)
static int mp_property_path(m_option_t * prop, int action, void *arg,
			    MPContext * mpctx)
{
    return m_property_string_ro(prop, action, arg, filename);
}

/// filename without path (RO)
static int mp_property_filename(m_option_t * prop, int action, void *arg,
				MPContext * mpctx)
{
    char *f;
    if (!filename)
	return M_PROPERTY_UNAVAILABLE;
    if (((f = strrchr(filename, '/')) || (f = strrchr(filename, '\\'))) && f[1])
	f++;
    else
	f = filename;
    return m_property_string_ro(prop, action, arg, f);
}

/// Demuxer name (RO)
static int mp_property_demuxer(m_option_t * prop, int action, void *arg,
			       MPContext * mpctx)
{
    if (!mpctx->demuxer)
	return M_PROPERTY_UNAVAILABLE;
    return m_property_string_ro(prop, action, arg,
				(char *) mpctx->demuxer->desc->name);
}

/// Position in the stream (RW)
static int mp_property_stream_pos(m_option_t * prop, int action, void *arg,
				  MPContext * mpctx)
{
    if (!mpctx->demuxer || !mpctx->demuxer->stream)
	return M_PROPERTY_UNAVAILABLE;
    if (!arg)
	return M_PROPERTY_ERROR;
    switch (action) {
    case M_PROPERTY_GET:
	*(off_t *) arg = stream_tell(mpctx->demuxer->stream);
	return M_PROPERTY_OK;
    case M_PROPERTY_SET:
	M_PROPERTY_CLAMP(prop, *(off_t *) arg);
	stream_seek(mpctx->demuxer->stream, *(off_t *) arg);
	return M_PROPERTY_OK;
    }
    return M_PROPERTY_NOT_IMPLEMENTED;
}

/// Stream start offset (RO)
static int mp_property_stream_start(m_option_t * prop, int action,
				    void *arg, MPContext * mpctx)
{
    if (!mpctx->demuxer || !mpctx->demuxer->stream)
	return M_PROPERTY_UNAVAILABLE;
    switch (action) {
    case M_PROPERTY_GET:
	*(off_t *) arg = mpctx->demuxer->stream->start_pos;
	return M_PROPERTY_OK;
    }
    return M_PROPERTY_NOT_IMPLEMENTED;
}

/// Stream end offset (RO)
static int mp_property_stream_end(m_option_t * prop, int action, void *arg,
				  MPContext * mpctx)
{
    if (!mpctx->demuxer || !mpctx->demuxer->stream)
	return M_PROPERTY_UNAVAILABLE;
    switch (action) {
    case M_PROPERTY_GET:
	*(off_t *) arg = mpctx->demuxer->stream->end_pos;
	return M_PROPERTY_OK;
    }
    return M_PROPERTY_NOT_IMPLEMENTED;
}

/// Stream length (RO)
static int mp_property_stream_length(m_option_t * prop, int action,
				     void *arg, MPContext * mpctx)
{
    if (!mpctx->demuxer || !mpctx->demuxer->stream)
	return M_PROPERTY_UNAVAILABLE;
    switch (action) {
    case M_PROPERTY_GET:
	*(off_t *) arg =
	    mpctx->demuxer->stream->end_pos - mpctx->demuxer->stream->start_pos;
	return M_PROPERTY_OK;
    }
    return M_PROPERTY_NOT_IMPLEMENTED;
}

/// Media length in seconds (RO)
static int mp_property_length(m_option_t * prop, int action, void *arg,
			      MPContext * mpctx)
{
    double len;

    if (!mpctx->demuxer ||
	!(int) (len = demuxer_get_time_length(mpctx->demuxer)))
	return M_PROPERTY_UNAVAILABLE;

    switch (action) {
    case M_PROPERTY_PRINT:
	if (!arg)
	    return M_PROPERTY_ERROR;
	else {
	    int h, m, s = len;
	    h = s / 3600;
	    s -= h * 3600;
	    m = s / 60;
	    s -= m * 60;
	    *(char **) arg = malloc(20);
	    if (h > 0)
		sprintf(*(char **) arg, "%d:%02d:%02d", h, m, s);
	    else if (m > 0)
		sprintf(*(char **) arg, "%d:%02d", m, s);
	    else
		sprintf(*(char **) arg, "%d", s);
	    return M_PROPERTY_OK;
	}
	break;
    }
    return m_property_double_ro(prop, action, arg, len);
}

///@}

/// \defgroup AudioProperties Audio properties
/// \ingroup Properties
///@{

/// Volume (RW)
static int mp_property_volume(m_option_t * prop, int action, void *arg,
			      MPContext * mpctx)
{

    if (!mpctx->sh_audio)
	return M_PROPERTY_UNAVAILABLE;

    switch (action) {
    case M_PROPERTY_GET:
	if (!arg)
	    return M_PROPERTY_ERROR;
	mixer_getbothvolume(&mpctx->mixer, arg);
	return M_PROPERTY_OK;
    case M_PROPERTY_PRINT:{
	    float vol;
	    if (!arg)
		return M_PROPERTY_ERROR;
	    mixer_getbothvolume(&mpctx->mixer, &vol);
	    return m_property_float_range(prop, action, arg, &vol);
	}
    case M_PROPERTY_STEP_UP:
    case M_PROPERTY_STEP_DOWN:
    case M_PROPERTY_SET:
	break;
    default:
	return M_PROPERTY_NOT_IMPLEMENTED;
    }

    if (mpctx->edl_muted)
	return M_PROPERTY_DISABLED;
    mpctx->user_muted = 0;

    switch (action) {
    case M_PROPERTY_SET:
	if (!arg)
	    return M_PROPERTY_ERROR;
	M_PROPERTY_CLAMP(prop, *(float *) arg);
	mixer_setvolume(&mpctx->mixer, *(float *) arg, *(float *) arg);
	return M_PROPERTY_OK;
    case M_PROPERTY_STEP_UP:
	if (arg && *(float *) arg <= 0)
	    mixer_decvolume(&mpctx->mixer);
	else
	    mixer_incvolume(&mpctx->mixer);
	return M_PROPERTY_OK;
    case M_PROPERTY_STEP_DOWN:
	if (arg && *(float *) arg <= 0)
	    mixer_incvolume(&mpctx->mixer);
	else
	    mixer_decvolume(&mpctx->mixer);
	return M_PROPERTY_OK;
    }
    return M_PROPERTY_NOT_IMPLEMENTED;
}

/// Mute (RW)
static int mp_property_mute(m_option_t * prop, int action, void *arg,
			    MPContext * mpctx)
{

    if (!mpctx->sh_audio)
	return M_PROPERTY_UNAVAILABLE;

    switch (action) {
    case M_PROPERTY_SET:
	if (mpctx->edl_muted)
	    return M_PROPERTY_DISABLED;
	if (!arg)
	    return M_PROPERTY_ERROR;
	if ((!!*(int *) arg) != mpctx->mixer.muted)
	    mixer_mute(&mpctx->mixer);
	mpctx->user_muted = mpctx->mixer.muted;
	return M_PROPERTY_OK;
    case M_PROPERTY_STEP_UP:
    case M_PROPERTY_STEP_DOWN:
	if (mpctx->edl_muted)
	    return M_PROPERTY_DISABLED;
	mixer_mute(&mpctx->mixer);
	mpctx->user_muted = mpctx->mixer.muted;
	return M_PROPERTY_OK;
    case M_PROPERTY_PRINT:
	if (!arg)
	    return M_PROPERTY_ERROR;
	if (mpctx->edl_muted) {
	    *(char **) arg = strdup(MSGTR_EnabledEdl);
	    return M_PROPERTY_OK;
	}
    default:
	return m_property_flag(prop, action, arg, &mpctx->mixer.muted);

    }
}

/// Audio delay (RW)
static int mp_property_audio_delay(m_option_t * prop, int action,
				   void *arg, MPContext * mpctx)
{
    if (!(mpctx->sh_audio && mpctx->sh_video))
	return M_PROPERTY_UNAVAILABLE;
    switch (action) {
    case M_PROPERTY_SET:
    case M_PROPERTY_STEP_UP:
    case M_PROPERTY_STEP_DOWN:
	if (!arg)
	    return M_PROPERTY_ERROR;
	else {
	    float delay = audio_delay;
	    m_property_delay(prop, action, arg, &audio_delay);
	    if (mpctx->sh_audio)
		mpctx->delay -= audio_delay - delay;
	}
	return M_PROPERTY_OK;
    default:
	return m_property_delay(prop, action, arg, &audio_delay);
    }
}

/// Audio codec tag (RO)
static int mp_property_audio_format(m_option_t * prop, int action,
				    void *arg, MPContext * mpctx)
{
    if (!mpctx->sh_audio)
	return M_PROPERTY_UNAVAILABLE;
    return m_property_int_ro(prop, action, arg, mpctx->sh_audio->format);
}

/// Audio bitrate (RO)
static int mp_property_audio_bitrate(m_option_t * prop, int action,
				     void *arg, MPContext * mpctx)
{
    if (!mpctx->sh_audio)
	return M_PROPERTY_UNAVAILABLE;
    return m_property_int_ro(prop, action, arg, mpctx->sh_audio->i_bps);
}

/// Samplerate (RO)
static int mp_property_samplerate(m_option_t * prop, int action, void *arg,
				  MPContext * mpctx)
{
    if (!mpctx->sh_audio)
	return M_PROPERTY_UNAVAILABLE;
    return m_property_int_ro(prop, action, arg, mpctx->sh_audio->samplerate);
}

/// Number of channels (RO)
static int mp_property_channels(m_option_t * prop, int action, void *arg,
				MPContext * mpctx)
{
    if (!mpctx->sh_audio)
	return M_PROPERTY_UNAVAILABLE;
    switch (action) {
    case M_PROPERTY_PRINT:
	if (!arg)
	    return M_PROPERTY_ERROR;
	switch (mpctx->sh_audio->channels) {
	case 1:
	    *(char **) arg = strdup("mono");
	    break;
	case 2:
	    *(char **) arg = strdup("stereo");
	    break;
	default:
	    *(char **) arg = malloc(32);
	    sprintf(*(char **) arg, "%d channels", mpctx->sh_audio->channels);
	}
	return M_PROPERTY_OK;
    }
    return m_property_int_ro(prop, action, arg, mpctx->sh_audio->channels);
}

/// Selected audio id (RW)
static int mp_property_audio(m_option_t * prop, int action, void *arg,
			     MPContext * mpctx)
{
    int current_id = -1, tmp;

    switch (action) {
    case M_PROPERTY_GET:
	if (!mpctx->sh_audio)
	    return M_PROPERTY_UNAVAILABLE;
	if (!arg)
	    return M_PROPERTY_ERROR;
	*(int *) arg = audio_id;
	return M_PROPERTY_OK;
    case M_PROPERTY_PRINT:
	if (!mpctx->sh_audio)
	    return M_PROPERTY_UNAVAILABLE;
	if (!arg)
	    return M_PROPERTY_ERROR;

	if (audio_id < 0)
	    *(char **) arg = strdup(MSGTR_Disabled);
	else {
	    char lang[40] = MSGTR_Unknown;
	    if (mpctx->demuxer->type == DEMUXER_TYPE_MATROSKA)
		demux_mkv_get_audio_lang(mpctx->demuxer, audio_id, lang, 9);
#ifdef USE_DVDREAD
	    else if (mpctx->stream->type == STREAMTYPE_DVD) {
		int code = dvd_lang_from_aid(mpctx->stream, audio_id);
		if (code) {
		    lang[0] = code >> 8;
		    lang[1] = code;
		    lang[2] = 0;
		}
	    }
#endif

#ifdef USE_DVDNAV
	    else if (mpctx->stream->type == STREAMTYPE_DVDNAV)
		dvdnav_lang_from_aid(mpctx->stream, audio_id, lang);
#endif
	    *(char **) arg = malloc(64);
	    snprintf(*(char **) arg, 64, "(%d) %s", audio_id, lang);
	}
	return M_PROPERTY_OK;

    case M_PROPERTY_STEP_UP:
    case M_PROPERTY_SET:
	if (action == M_PROPERTY_SET && arg)
	    tmp = *((int *) arg);
	else
	    tmp = -1;
	current_id = mpctx->demuxer->audio->id;
	audio_id = demuxer_switch_audio(mpctx->demuxer, tmp);
	if (audio_id == -2
	    || (audio_id > -1
		&& mpctx->demuxer->audio->id != current_id && current_id != -2))
	    uninit_player(INITED_AO | INITED_ACODEC);
	if (audio_id > -1 && mpctx->demuxer->audio->id != current_id) {
	    sh_audio_t *sh2;
	    sh2 = mpctx->demuxer->a_streams[mpctx->demuxer->audio->id];
	    if (sh2) {
		sh2->ds = mpctx->demuxer->audio;
		mpctx->sh_audio = sh2;
		reinit_audio_chain();
	    }
	}
	mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_AUDIO_TRACK=%d\n", audio_id);
	return M_PROPERTY_OK;
    default:
	return M_PROPERTY_NOT_IMPLEMENTED;
    }

}

/// Selected video id (RW)
static int mp_property_video(m_option_t * prop, int action, void *arg,
			     MPContext * mpctx)
{
    int current_id = -1, tmp;

    switch (action) {
    case M_PROPERTY_GET:
	if (!mpctx->sh_video)
	    return M_PROPERTY_UNAVAILABLE;
	if (!arg)
	    return M_PROPERTY_ERROR;
	*(int *) arg = video_id;
	return M_PROPERTY_OK;
    case M_PROPERTY_PRINT:
	if (!mpctx->sh_video)
	    return M_PROPERTY_UNAVAILABLE;
	if (!arg)
	    return M_PROPERTY_ERROR;

	if (video_id < 0)
	    *(char **) arg = strdup(MSGTR_Disabled);
	else {
	    char lang[40] = MSGTR_Unknown;
	    *(char **) arg = malloc(64);
	    snprintf(*(char **) arg, 64, "(%d) %s", video_id, lang);
	}
	return M_PROPERTY_OK;

    case M_PROPERTY_STEP_UP:
    case M_PROPERTY_SET:
	current_id = mpctx->demuxer->video->id;
	if (action == M_PROPERTY_SET && arg)
	    tmp = *((int *) arg);
	else
	    tmp = -1;
	video_id = demuxer_switch_video(mpctx->demuxer, tmp);
	if (video_id == -2
	    || (video_id > -1 && mpctx->demuxer->video->id != current_id
		&& current_id != -2))
	    uninit_player(INITED_VCODEC |
			  (fixed_vo && video_id != -2 ? 0 : INITED_VO));
	if (video_id > -1 && mpctx->demuxer->video->id != current_id) {
	    sh_video_t *sh2;
	    sh2 = mpctx->demuxer->v_streams[mpctx->demuxer->video->id];
	    if (sh2) {
		sh2->ds = mpctx->demuxer->video;
		mpctx->sh_video = sh2;
		reinit_video_chain();
	    }
	}
	mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_VIDEO_TRACK=%d\n", video_id);
	return M_PROPERTY_OK;

    default:
	return M_PROPERTY_NOT_IMPLEMENTED;
    }
}

static int mp_property_program(m_option_t * prop, int action, void *arg,
			       MPContext * mpctx)
{
    demux_program_t prog;

    switch (action) {
    case M_PROPERTY_STEP_UP:
    case M_PROPERTY_SET:
	if (action == M_PROPERTY_SET && arg)
	    prog.progid = *((int *) arg);
	else
	    prog.progid = -1;
	if (demux_control
	    (mpctx->demuxer, DEMUXER_CTRL_IDENTIFY_PROGRAM,
	     &prog) == DEMUXER_CTRL_NOTIMPL)
	    return M_PROPERTY_ERROR;

	mp_property_do("switch_audio", M_PROPERTY_SET, &prog.aid, mpctx);
	mp_property_do("switch_video", M_PROPERTY_SET, &prog.vid, mpctx);
	return M_PROPERTY_OK;

    default:
	return M_PROPERTY_NOT_IMPLEMENTED;
    }
}

///@}

/// \defgroup VideoProperties Video properties
/// \ingroup Properties
///@{

/// Fullscreen state (RW)
static int mp_property_fullscreen(m_option_t * prop, int action, void *arg,
				  MPContext * mpctx)
{

    if (!mpctx->video_out)
	return M_PROPERTY_UNAVAILABLE;

    switch (action) {
    case M_PROPERTY_SET:
	if (!arg)
	    return M_PROPERTY_ERROR;
	M_PROPERTY_CLAMP(prop, *(int *) arg);
	if (vo_fs == !!*(int *) arg)
	    return M_PROPERTY_OK;
    case M_PROPERTY_STEP_UP:
    case M_PROPERTY_STEP_DOWN:
#ifdef HAVE_NEW_GUI
	if (use_gui)
	    guiGetEvent(guiIEvent, (char *) MP_CMD_GUI_FULLSCREEN);
	else
#endif
	if (vo_config_count)
	    mpctx->video_out->control(VOCTRL_FULLSCREEN, 0);
	return M_PROPERTY_OK;
    default:
	return m_property_flag(prop, action, arg, &vo_fs);
    }
}

static int mp_property_deinterlace(m_option_t * prop, int action,
				   void *arg, MPContext * mpctx)
{
    int deinterlace;
    vf_instance_t *vf;
    if (!mpctx->sh_video || !mpctx->sh_video->vfilter)
	return M_PROPERTY_UNAVAILABLE;
    vf = mpctx->sh_video->vfilter;
    switch (action) {
    case M_PROPERTY_GET:
	if (!arg)
	    return M_PROPERTY_ERROR;
	vf->control(vf, VFCTRL_GET_DEINTERLACE, arg);
	return M_PROPERTY_OK;
    case M_PROPERTY_SET:
	if (!arg)
	    return M_PROPERTY_ERROR;
	M_PROPERTY_CLAMP(prop, *(int *) arg);
	vf->control(vf, VFCTRL_SET_DEINTERLACE, arg);
	return M_PROPERTY_OK;
    case M_PROPERTY_STEP_UP:
    case M_PROPERTY_STEP_DOWN:
	vf->control(vf, VFCTRL_GET_DEINTERLACE, &deinterlace);
	deinterlace = !deinterlace;
	vf->control(vf, VFCTRL_SET_DEINTERLACE, &deinterlace);
	return M_PROPERTY_OK;
    }
    return M_PROPERTY_NOT_IMPLEMENTED;
}

/// Panscan (RW)
static int mp_property_panscan(m_option_t * prop, int action, void *arg,
			       MPContext * mpctx)
{

    if (!mpctx->video_out
	|| mpctx->video_out->control(VOCTRL_GET_PANSCAN, NULL) != VO_TRUE)
	return M_PROPERTY_UNAVAILABLE;

    switch (action) {
    case M_PROPERTY_SET:
	if (!arg)
	    return M_PROPERTY_ERROR;
	M_PROPERTY_CLAMP(prop, *(float *) arg);
	vo_panscan = *(float *) arg;
	mpctx->video_out->control(VOCTRL_SET_PANSCAN, NULL);
	return M_PROPERTY_OK;
    case M_PROPERTY_STEP_UP:
    case M_PROPERTY_STEP_DOWN:
	vo_panscan += (arg ? *(float *) arg : 0.1) *
	    (action == M_PROPERTY_STEP_DOWN ? -1 : 1);
	if (vo_panscan > 1)
	    vo_panscan = 1;
	else if (vo_panscan < 0)
	    vo_panscan = 0;
	mpctx->video_out->control(VOCTRL_SET_PANSCAN, NULL);
	return M_PROPERTY_OK;
    default:
	return m_property_float_range(prop, action, arg, &vo_panscan);
    }
}

/// Helper to set vo flags.
/** \ingroup PropertyImplHelper
 */
static int mp_property_vo_flag(m_option_t * prop, int action, void *arg,
			       int vo_ctrl, int *vo_var, MPContext * mpctx)
{

    if (!mpctx->video_out)
	return M_PROPERTY_UNAVAILABLE;

    switch (action) {
    case M_PROPERTY_SET:
	if (!arg)
	    return M_PROPERTY_ERROR;
	M_PROPERTY_CLAMP(prop, *(int *) arg);
	if (*vo_var == !!*(int *) arg)
	    return M_PROPERTY_OK;
    case M_PROPERTY_STEP_UP:
    case M_PROPERTY_STEP_DOWN:
	if (vo_config_count)
	    mpctx->video_out->control(vo_ctrl, 0);
	return M_PROPERTY_OK;
    default:
	return m_property_flag(prop, action, arg, vo_var);
    }
}

/// Window always on top (RW)
static int mp_property_ontop(m_option_t * prop, int action, void *arg,
			     MPContext * mpctx)
{
    return mp_property_vo_flag(prop, action, arg, VOCTRL_ONTOP, &vo_ontop,
			       mpctx);
}

/// Display in the root window (RW)
static int mp_property_rootwin(m_option_t * prop, int action, void *arg,
			       MPContext * mpctx)
{
    return mp_property_vo_flag(prop, action, arg, VOCTRL_ROOTWIN,
			       &vo_rootwin, mpctx);
}

/// Show window borders (RW)
static int mp_property_border(m_option_t * prop, int action, void *arg,
			      MPContext * mpctx)
{
    return mp_property_vo_flag(prop, action, arg, VOCTRL_BORDER,
			       &vo_border, mpctx);
}

/// Framedropping state (RW)
static int mp_property_framedropping(m_option_t * prop, int action,
				     void *arg, MPContext * mpctx)
{

    if (!mpctx->sh_video)
	return M_PROPERTY_UNAVAILABLE;

    switch (action) {
    case M_PROPERTY_PRINT:
	if (!arg)
	    return M_PROPERTY_ERROR;
	*(char **) arg = strdup(frame_dropping == 1 ? MSGTR_Enabled :
				(frame_dropping == 2 ? MSGTR_HardFrameDrop :
				 MSGTR_Disabled));
	return M_PROPERTY_OK;
    default:
	return m_property_choice(prop, action, arg, &frame_dropping);
    }
}

/// Color settings, try to use vf/vo then fall back on TV. (RW)
static int mp_property_gamma(m_option_t * prop, int action, void *arg,
			     MPContext * mpctx)
{
    int *gamma = prop->priv, r;

    if (!mpctx->sh_video)
	return M_PROPERTY_UNAVAILABLE;

    if (gamma[0] == 1000) {
	gamma[0] = 0;
	get_video_colors(mpctx->sh_video, prop->name, gamma);
    }

    switch (action) {
    case M_PROPERTY_SET:
	if (!arg)
	    return M_PROPERTY_ERROR;
	M_PROPERTY_CLAMP(prop, *(int *) arg);
	*gamma = *(int *) arg;
	r = set_video_colors(mpctx->sh_video, prop->name, *gamma);
	if (r <= 0)
	    break;
	return r;
    case M_PROPERTY_GET:
	if (!arg)
	    return M_PROPERTY_ERROR;
	r = get_video_colors(mpctx->sh_video, prop->name, arg);
	if (r <= 0)
	    break;
	return r;
    case M_PROPERTY_STEP_UP:
    case M_PROPERTY_STEP_DOWN:
	*gamma += (arg ? *(int *) arg : 1) *
	    (action == M_PROPERTY_STEP_DOWN ? -1 : 1);
	M_PROPERTY_CLAMP(prop, *gamma);
	r = set_video_colors(mpctx->sh_video, prop->name, *gamma);
	if (r <= 0)
	    break;
	return r;
    default:
	return M_PROPERTY_NOT_IMPLEMENTED;
    }

#ifdef USE_TV
    if (mpctx->demuxer->type == DEMUXER_TYPE_TV) {
	int l = strlen(prop->name);
	char tv_prop[3 + l + 1];
	sprintf(tv_prop, "tv_%s", prop->name);
	return mp_property_do(tv_prop, action, arg, mpctx);
    }
#endif

    return M_PROPERTY_UNAVAILABLE;
}

/// VSync (RW)
static int mp_property_vsync(m_option_t * prop, int action, void *arg,
			     MPContext * mpctx)
{
    return m_property_flag(prop, action, arg, &vo_vsync);
}

/// Video codec tag (RO)
static int mp_property_video_format(m_option_t * prop, int action,
				    void *arg, MPContext * mpctx)
{
    if (!mpctx->sh_video)
	return M_PROPERTY_UNAVAILABLE;
    return m_property_int_ro(prop, action, arg, mpctx->sh_video->format);
}

/// Video bitrate (RO)
static int mp_property_video_bitrate(m_option_t * prop, int action,
				     void *arg, MPContext * mpctx)
{
    if (!mpctx->sh_video)
	return M_PROPERTY_UNAVAILABLE;
    return m_property_int_ro(prop, action, arg, mpctx->sh_video->i_bps);
}

/// Video display width (RO)
static int mp_property_width(m_option_t * prop, int action, void *arg,
			     MPContext * mpctx)
{
    if (!mpctx->sh_video)
	return M_PROPERTY_UNAVAILABLE;
    return m_property_int_ro(prop, action, arg, mpctx->sh_video->disp_w);
}

/// Video display height (RO)
static int mp_property_height(m_option_t * prop, int action, void *arg,
			      MPContext * mpctx)
{
    if (!mpctx->sh_video)
	return M_PROPERTY_UNAVAILABLE;
    return m_property_int_ro(prop, action, arg, mpctx->sh_video->disp_h);
}

/// Video fps (RO)
static int mp_property_fps(m_option_t * prop, int action, void *arg,
			   MPContext * mpctx)
{
    if (!mpctx->sh_video)
	return M_PROPERTY_UNAVAILABLE;
    return m_property_float_ro(prop, action, arg, mpctx->sh_video->fps);
}

/// Video aspect (RO)
static int mp_property_aspect(m_option_t * prop, int action, void *arg,
			      MPContext * mpctx)
{
    if (!mpctx->sh_video)
	return M_PROPERTY_UNAVAILABLE;
    return m_property_float_ro(prop, action, arg, mpctx->sh_video->aspect);
}

///@}

/// \defgroup SubProprties Subtitles properties
/// \ingroup Properties
///@{

/// Text subtitle position (RW)
static int mp_property_sub_pos(m_option_t * prop, int action, void *arg,
			       MPContext * mpctx)
{
    if (!mpctx->sh_video)
	return M_PROPERTY_UNAVAILABLE;

    switch (action) {
    case M_PROPERTY_SET:
	if (!arg)
	    return M_PROPERTY_ERROR;
    case M_PROPERTY_STEP_UP:
    case M_PROPERTY_STEP_DOWN:
	vo_osd_changed(OSDTYPE_SUBTITLE);
    default:
	return m_property_int_range(prop, action, arg, &sub_pos);
    }
}

/// Selected subtitles (RW)
static int mp_property_sub(m_option_t * prop, int action, void *arg,
			   MPContext * mpctx)
{
    demux_stream_t *const d_sub = mpctx->d_sub;
    const int global_sub_size = mpctx->global_sub_size;
    int source = -1, reset_spu = 0;
    char *sub_name;

    if (global_sub_size <= 0)
	return M_PROPERTY_UNAVAILABLE;

    switch (action) {
    case M_PROPERTY_GET:
	if (!arg)
	    return M_PROPERTY_ERROR;
	*(int *) arg = mpctx->global_sub_pos;
	return M_PROPERTY_OK;
    case M_PROPERTY_PRINT:
	if (!arg)
	    return M_PROPERTY_ERROR;
	*(char **) arg = malloc(64);
	(*(char **) arg)[63] = 0;
	sub_name = 0;
	if (subdata)
	    sub_name = subdata->filename;
#ifdef USE_ASS
	if (ass_track && ass_track->name)
	    sub_name = ass_track->name;
#endif
	if (sub_name) {
	    char *tmp, *tmp2;
	    tmp = sub_name;
	    if ((tmp2 = strrchr(tmp, '/')))
		tmp = tmp2 + 1;

	    snprintf(*(char **) arg, 63, "(%d) %s%s",
		     mpctx->set_of_sub_pos + 1,
		     strlen(tmp) < 20 ? "" : "...",
		     strlen(tmp) < 20 ? tmp : tmp + strlen(tmp) - 19);
	    return M_PROPERTY_OK;
	}
#ifdef USE_DVDNAV
	if (mpctx->stream->type == STREAMTYPE_DVDNAV) {
	    if (vo_spudec && dvdsub_id >= 0) {
		unsigned char lang[3];
		if (dvdnav_lang_from_sid(mpctx->stream, dvdsub_id, lang)) {
		    snprintf(*(char **) arg, 63, "(%d) %s", dvdsub_id, lang);
		    return M_PROPERTY_OK;
		}
	    }
	}
#endif

	if (mpctx->demuxer->type == DEMUXER_TYPE_MATROSKA && dvdsub_id >= 0) {
	    char lang[40] = MSGTR_Unknown;
	    demux_mkv_get_sub_lang(mpctx->demuxer, dvdsub_id, lang, 9);
	    snprintf(*(char **) arg, 63, "(%d) %s", dvdsub_id, lang);
	    return M_PROPERTY_OK;
	}
#ifdef HAVE_OGGVORBIS
	if (mpctx->demuxer->type == DEMUXER_TYPE_OGG && d_sub && dvdsub_id >= 0) {
	    char *lang = demux_ogg_sub_lang(mpctx->demuxer, dvdsub_id);
	    if (!lang)
		lang = MSGTR_Unknown;
	    snprintf(*(char **) arg, 63, "(%d) %s", dvdsub_id, lang);
	    return M_PROPERTY_OK;
	}
#endif
	if (vo_vobsub && vobsub_id >= 0) {
	    const char *language = MSGTR_Unknown;
	    language = vobsub_get_id(vo_vobsub, (unsigned int) vobsub_id);
	    snprintf(*(char **) arg, 63, "(%d) %s",
		     vobsub_id, language ? language : MSGTR_Unknown);
	    return M_PROPERTY_OK;
	}
#ifdef USE_DVDREAD
	if (vo_spudec && mpctx->stream->type == STREAMTYPE_DVD
	    && dvdsub_id >= 0) {
	    char lang[3];
	    int code = dvd_lang_from_sid(mpctx->stream, dvdsub_id);
	    lang[0] = code >> 8;
	    lang[1] = code;
	    lang[2] = 0;
	    snprintf(*(char **) arg, 63, "(%d) %s", dvdsub_id, lang);
	    return M_PROPERTY_OK;
	}
#endif
	if (dvdsub_id >= 0) {
	    snprintf(*(char **) arg, 63, "(%d) %s", dvdsub_id, MSGTR_Unknown);
	    return M_PROPERTY_OK;
	}
	snprintf(*(char **) arg, 63, MSGTR_Disabled);
	return M_PROPERTY_OK;

    case M_PROPERTY_SET:
	if (!arg)
	    return M_PROPERTY_ERROR;
	if (*(int *) arg < -1)
	    *(int *) arg = -1;
	else if (*(int *) arg >= global_sub_size)
	    *(int *) arg = global_sub_size - 1;
	mpctx->global_sub_pos = *(int *) arg;
	break;
    case M_PROPERTY_STEP_UP:
	mpctx->global_sub_pos += 2;
	mpctx->global_sub_pos =
	    (mpctx->global_sub_pos % (global_sub_size + 1)) - 1;
	break;
    case M_PROPERTY_STEP_DOWN:
	mpctx->global_sub_pos += global_sub_size + 1;
	mpctx->global_sub_pos =
	    (mpctx->global_sub_pos % (global_sub_size + 1)) - 1;
	break;
    default:
	return M_PROPERTY_NOT_IMPLEMENTED;
    }

    if (mpctx->global_sub_pos >= 0)
	source = sub_source(mpctx);

    mp_msg(MSGT_CPLAYER, MSGL_DBG3,
	   "subtitles: %d subs, (v@%d s@%d d@%d), @%d, source @%d\n",
	   global_sub_size,
	   mpctx->global_sub_indices[SUB_SOURCE_VOBSUB],
	   mpctx->global_sub_indices[SUB_SOURCE_SUBS],
	   mpctx->global_sub_indices[SUB_SOURCE_DEMUX],
	   mpctx->global_sub_pos, source);

    mpctx->set_of_sub_pos = -1;
    subdata = NULL;
    vo_sub_last = vo_sub = NULL;

    vobsub_id = -1;
    dvdsub_id = -1;
    if (d_sub) {
	if (d_sub->id > -2)
	    reset_spu = 1;
	d_sub->id = -2;
    }
#ifdef USE_ASS
    ass_track = 0;
#endif

    if (source == SUB_SOURCE_VOBSUB) {
	vobsub_id =
	    mpctx->global_sub_pos -
	    mpctx->global_sub_indices[SUB_SOURCE_VOBSUB];
    } else if (source == SUB_SOURCE_SUBS) {
	mpctx->set_of_sub_pos =
	    mpctx->global_sub_pos - mpctx->global_sub_indices[SUB_SOURCE_SUBS];
#ifdef USE_ASS
	if (ass_enabled && mpctx->set_of_ass_tracks[mpctx->set_of_sub_pos])
	    ass_track = mpctx->set_of_ass_tracks[mpctx->set_of_sub_pos];
	else
#endif
	{
	    subdata = mpctx->set_of_subtitles[mpctx->set_of_sub_pos];
	    vo_osd_changed(OSDTYPE_SUBTITLE);
	}
    } else if (source == SUB_SOURCE_DEMUX) {
	dvdsub_id =
	    mpctx->global_sub_pos - mpctx->global_sub_indices[SUB_SOURCE_DEMUX];
	if (d_sub) {
#ifdef USE_DVDREAD
	    if (vo_spudec && mpctx->stream->type == STREAMTYPE_DVD) {
		d_sub->id = dvdsub_id;
	    }
#endif

#ifdef USE_DVDNAV
	    if (vo_spudec && mpctx->stream->type == STREAMTYPE_DVDNAV) {
		d_sub->id = dvdsub_id;
	    }
#endif
	    if (mpctx->stream->type != STREAMTYPE_DVD
		&& mpctx->stream->type != STREAMTYPE_DVDNAV) {
		int i = 0;
		for (d_sub->id = 0; d_sub->id < MAX_S_STREAMS; d_sub->id++) {
		    if (mpctx->demuxer->s_streams[d_sub->id]) {
			if (i == dvdsub_id)
			    break;
			i++;
		    }
		}
		d_sub->sh = mpctx->demuxer->s_streams[d_sub->id];
	    }
	    if (mpctx->demuxer->type == DEMUXER_TYPE_MATROSKA)
		d_sub->id = demux_mkv_change_subs(mpctx->demuxer, dvdsub_id);
	    if (d_sub->sh && d_sub->id >= 0) {
		sh_sub_t *sh = d_sub->sh;
		if (sh->type == 'v')
		    init_vo_spudec();
#ifdef USE_ASS
		else if (ass_enabled && sh->type == 'a')
		    ass_track = sh->ass_track;
#endif
	    }
	}
    }
#ifdef USE_DVDREAD
    if (vo_spudec
	&& (mpctx->stream->type == STREAMTYPE_DVD
	    || mpctx->stream->type == STREAMTYPE_DVDNAV)
	&& dvdsub_id < 0 && reset_spu) {
	dvdsub_id = -2;
	d_sub->id = dvdsub_id;
    }
#endif
    update_subtitles(mpctx->sh_video, d_sub, 1);

    return M_PROPERTY_OK;
}

/// Subtitle delay (RW)
static int mp_property_sub_delay(m_option_t * prop, int action, void *arg,
				 MPContext * mpctx)
{
    if (!mpctx->sh_video)
	return M_PROPERTY_UNAVAILABLE;
    return m_property_delay(prop, action, arg, &sub_delay);
}

/// Alignment of text subtitles (RW)
static int mp_property_sub_alignment(m_option_t * prop, int action,
				     void *arg, MPContext * mpctx)
{
    char *name[] = { MSGTR_Top, MSGTR_Center, MSGTR_Bottom };

    if (!mpctx->sh_video || mpctx->global_sub_pos < 0
	|| sub_source(mpctx) != SUB_SOURCE_SUBS)
	return M_PROPERTY_UNAVAILABLE;

    switch (action) {
    case M_PROPERTY_PRINT:
	if (!arg)
	    return M_PROPERTY_ERROR;
	M_PROPERTY_CLAMP(prop, sub_alignment);
	*(char **) arg = strdup(name[sub_alignment]);
	return M_PROPERTY_OK;
    case M_PROPERTY_SET:
	if (!arg)
	    return M_PROPERTY_ERROR;
    case M_PROPERTY_STEP_UP:
    case M_PROPERTY_STEP_DOWN:
	vo_osd_changed(OSDTYPE_SUBTITLE);
    default:
	return m_property_choice(prop, action, arg, &sub_alignment);
    }
}

/// Subtitle visibility (RW)
static int mp_property_sub_visibility(m_option_t * prop, int action,
				      void *arg, MPContext * mpctx)
{
    if (!mpctx->sh_video)
	return M_PROPERTY_UNAVAILABLE;

    switch (action) {
    case M_PROPERTY_SET:
	if (!arg)
	    return M_PROPERTY_ERROR;
    case M_PROPERTY_STEP_UP:
    case M_PROPERTY_STEP_DOWN:
	vo_osd_changed(OSDTYPE_SUBTITLE);
	if (vo_spudec)
	    vo_osd_changed(OSDTYPE_SPU);
    default:
	return m_property_flag(prop, action, arg, &sub_visibility);
    }
}

/// Show only forced subtitles (RW)
static int mp_property_sub_forced_only(m_option_t * prop, int action,
				       void *arg, MPContext * mpctx)
{
    if (!vo_spudec)
	return M_PROPERTY_UNAVAILABLE;

    switch (action) {
    case M_PROPERTY_SET:
	if (!arg)
	    return M_PROPERTY_ERROR;
    case M_PROPERTY_STEP_UP:
    case M_PROPERTY_STEP_DOWN:
	m_property_flag(prop, action, arg, &forced_subs_only);
	spudec_set_forced_subs_only(vo_spudec, forced_subs_only);
	return M_PROPERTY_OK;
    default:
	return m_property_flag(prop, action, arg, &forced_subs_only);
    }

}

///@}

/// \defgroup TVProperties TV properties
/// \ingroup Properties
///@{

#ifdef USE_TV

/// TV color settings (RW)
static int mp_property_tv_color(m_option_t * prop, int action, void *arg,
				MPContext * mpctx)
{
    int r, val;
    tvi_handle_t *tvh = mpctx->demuxer->priv;
    if (mpctx->demuxer->type != DEMUXER_TYPE_TV || !tvh)
	return M_PROPERTY_UNAVAILABLE;

    switch (action) {
    case M_PROPERTY_SET:
	if (!arg)
	    return M_PROPERTY_ERROR;
	M_PROPERTY_CLAMP(prop, *(int *) arg);
	return tv_set_color_options(tvh, (int) prop->priv, *(int *) arg);
    case M_PROPERTY_GET:
	return tv_get_color_options(tvh, (int) prop->priv, arg);
    case M_PROPERTY_STEP_UP:
    case M_PROPERTY_STEP_DOWN:
	if ((r = tv_get_color_options(tvh, (int) prop->priv, &val)) >= 0) {
	    if (!r)
		return M_PROPERTY_ERROR;
	    val += (arg ? *(int *) arg : 1) *
		(action == M_PROPERTY_STEP_DOWN ? -1 : 1);
	    M_PROPERTY_CLAMP(prop, val);
	    return tv_set_color_options(tvh, (int) prop->priv, val);
	}
	return M_PROPERTY_ERROR;
    }
    return M_PROPERTY_NOT_IMPLEMENTED;
}

#endif

///@}

/// All properties available in MPlayer.
/** \ingroup Properties
 */
static m_option_t mp_properties[] = {
    // General
    { "osdlevel", mp_property_osdlevel, CONF_TYPE_INT,
     M_OPT_RANGE, 0, 3, NULL },
    { "speed", mp_property_playback_speed, CONF_TYPE_FLOAT,
     M_OPT_RANGE, 0.01, 100.0, NULL },
    { "filename", mp_property_filename, CONF_TYPE_STRING,
     0, 0, 0, NULL },
    { "path", mp_property_path, CONF_TYPE_STRING,
     0, 0, 0, NULL },
    { "demuxer", mp_property_demuxer, CONF_TYPE_STRING,
     0, 0, 0, NULL },
    { "stream_pos", mp_property_stream_pos, CONF_TYPE_POSITION,
     M_OPT_MIN, 0, 0, NULL },
    { "stream_start", mp_property_stream_start, CONF_TYPE_POSITION,
     M_OPT_MIN, 0, 0, NULL },
    { "stream_end", mp_property_stream_end, CONF_TYPE_POSITION,
     M_OPT_MIN, 0, 0, NULL },
    { "stream_length", mp_property_stream_length, CONF_TYPE_POSITION,
     M_OPT_MIN, 0, 0, NULL },
    { "length", mp_property_length, CONF_TYPE_DOUBLE,
     0, 0, 0, NULL },

    // Audio
    { "volume", mp_property_volume, CONF_TYPE_FLOAT,
     M_OPT_RANGE, 0, 100, NULL },
    { "mute", mp_property_mute, CONF_TYPE_FLAG,
     M_OPT_RANGE, 0, 1, NULL },
    { "audio_delay", mp_property_audio_delay, CONF_TYPE_FLOAT,
     M_OPT_RANGE, -100, 100, NULL },
    { "audio_format", mp_property_audio_format, CONF_TYPE_INT,
     0, 0, 0, NULL },
    { "audio_bitrate", mp_property_audio_bitrate, CONF_TYPE_INT,
     0, 0, 0, NULL },
    { "samplerate", mp_property_samplerate, CONF_TYPE_INT,
     0, 0, 0, NULL },
    { "channels", mp_property_channels, CONF_TYPE_INT,
     0, 0, 0, NULL },
    { "switch_audio", mp_property_audio, CONF_TYPE_INT,
     CONF_RANGE, -2, MAX_A_STREAMS - 1, NULL },

    // Video
    { "fullscreen", mp_property_fullscreen, CONF_TYPE_FLAG,
     M_OPT_RANGE, 0, 1, NULL },
    { "deinterlace", mp_property_deinterlace, CONF_TYPE_FLAG,
     M_OPT_RANGE, 0, 1, NULL },
    { "ontop", mp_property_ontop, CONF_TYPE_FLAG,
     M_OPT_RANGE, 0, 1, NULL },
    { "rootwin", mp_property_rootwin, CONF_TYPE_FLAG,
     M_OPT_RANGE, 0, 1, NULL },
    { "border", mp_property_border, CONF_TYPE_FLAG,
     M_OPT_RANGE, 0, 1, NULL },
    { "framedropping", mp_property_framedropping, CONF_TYPE_INT,
     M_OPT_RANGE, 0, 2, NULL },
    { "gamma", mp_property_gamma, CONF_TYPE_INT,
     M_OPT_RANGE, -100, 100, &vo_gamma_gamma },
    { "brightness", mp_property_gamma, CONF_TYPE_INT,
     M_OPT_RANGE, -100, 100, &vo_gamma_brightness },
    { "contrast", mp_property_gamma, CONF_TYPE_INT,
     M_OPT_RANGE, -100, 100, &vo_gamma_contrast },
    { "saturation", mp_property_gamma, CONF_TYPE_INT,
     M_OPT_RANGE, -100, 100, &vo_gamma_saturation },
    { "hue", mp_property_gamma, CONF_TYPE_INT,
     M_OPT_RANGE, -100, 100, &vo_gamma_hue },
    { "panscan", mp_property_panscan, CONF_TYPE_FLOAT,
     M_OPT_RANGE, 0, 1, NULL },
    { "vsync", mp_property_vsync, CONF_TYPE_FLAG,
     M_OPT_RANGE, 0, 1, NULL },
    { "video_format", mp_property_video_format, CONF_TYPE_INT,
     0, 0, 0, NULL },
    { "video_bitrate", mp_property_video_bitrate, CONF_TYPE_INT,
     0, 0, 0, NULL },
    { "width", mp_property_width, CONF_TYPE_INT,
     0, 0, 0, NULL },
    { "height", mp_property_height, CONF_TYPE_INT,
     0, 0, 0, NULL },
    { "fps", mp_property_fps, CONF_TYPE_FLOAT,
     0, 0, 0, NULL },
    { "aspect", mp_property_aspect, CONF_TYPE_FLOAT,
     0, 0, 0, NULL },
    { "switch_video", mp_property_video, CONF_TYPE_INT,
     CONF_RANGE, -2, MAX_V_STREAMS - 1, NULL },
    { "switch_program", mp_property_program, CONF_TYPE_INT,
     CONF_RANGE, -1, 65535, NULL },

    // Subs
    { "sub", mp_property_sub, CONF_TYPE_INT,
     M_OPT_MIN, -1, 0, NULL },
    { "sub_delay", mp_property_sub_delay, CONF_TYPE_FLOAT,
     0, 0, 0, NULL },
    { "sub_pos", mp_property_sub_pos, CONF_TYPE_INT,
     M_OPT_RANGE, 0, 100, NULL },
    { "sub_alignment", mp_property_sub_alignment, CONF_TYPE_INT,
     M_OPT_RANGE, 0, 2, NULL },
    { "sub_visibility", mp_property_sub_visibility, CONF_TYPE_FLAG,
     M_OPT_RANGE, 0, 1, NULL },
    { "sub_forced_only", mp_property_sub_forced_only, CONF_TYPE_FLAG,
     M_OPT_RANGE, 0, 1, NULL },

#ifdef USE_TV
    { "tv_brightness", mp_property_tv_color, CONF_TYPE_INT,
     M_OPT_RANGE, -100, 100, (void *) TV_COLOR_BRIGHTNESS },
    { "tv_contrast", mp_property_tv_color, CONF_TYPE_INT,
     M_OPT_RANGE, -100, 100, (void *) TV_COLOR_CONTRAST },
    { "tv_saturation", mp_property_tv_color, CONF_TYPE_INT,
     M_OPT_RANGE, -100, 100, (void *) TV_COLOR_SATURATION },
    { "tv_hue", mp_property_tv_color, CONF_TYPE_INT,
     M_OPT_RANGE, -100, 100, (void *) TV_COLOR_HUE },
#endif

    { NULL, NULL, NULL, 0, 0, 0, NULL }
};


m_option_t *mp_property_find(const char *name)
{
    return m_option_list_find(mp_properties, name);
}

int mp_property_do(const char *name, int action, void *val, void *ctx)
{
    m_option_t *p = mp_property_find(name);
    if (!p)
	return M_PROPERTY_UNAVAILABLE;
    return m_property_do(p, action, val, ctx);
}

char *property_expand_string(MPContext * mpctx, char *str)
{
    return m_properties_expand_string(mp_properties, str, mpctx);
}

void property_print_help(void)
{
    m_properties_print_help_list(mp_properties);
}


///@}
// Properties group


/**
 * \defgroup Command2Property Command to property bridge
 *
 * It is used to handle most commands that just set a property
 * and optionally display something on the OSD.
 * Two kinds of commands are handled: adjust or toggle.
 *
 * Adjust commands take 1 or 2 parameters: <value> <abs>
 * If <abs> is non-zero the property is set to the given value
 * otherwise it is adjusted.
 *
 * Toggle commands take 0 or 1 parameters. With no parameter
 * or a value less than the property minimum it just steps the
 * property to its next value. Otherwise it sets it to the given
 * value.
 *
 *@{
 */

/// List of the commands that can be handled by setting a property.
static struct {
    /// property name
    const char *name;
    /// cmd id
    int cmd;
    /// set/adjust or toggle command
    int toggle;
    /// progressbar type
    int osd_progbar;
    /// osd msg id if it must be shared
    int osd_id;
    /// osd msg template
    const char *osd_msg;
} set_prop_cmd[] = {
    // audio
    { "volume", MP_CMD_VOLUME, 0, OSD_VOLUME, -1, MSGTR_Volume },
    { "mute", MP_CMD_MUTE, 1, 0, -1, MSGTR_MuteStatus },
    { "audio_delay", MP_CMD_AUDIO_DELAY, 0, 0, -1, MSGTR_AVDelayStatus },
    { "switch_audio", MP_CMD_SWITCH_AUDIO, 1, 0, -1, MSGTR_OSDAudio },
    // video
    { "fullscreen", MP_CMD_VO_FULLSCREEN, 1, 0, -1, NULL },
    { "panscan", MP_CMD_PANSCAN, 0, OSD_PANSCAN, -1, MSGTR_Panscan },
    { "ontop", MP_CMD_VO_ONTOP, 1, 0, -1, MSGTR_OnTopStatus },
    { "rootwin", MP_CMD_VO_ROOTWIN, 1, 0, -1, MSGTR_RootwinStatus },
    { "border", MP_CMD_VO_BORDER, 1, 0, -1, MSGTR_BorderStatus },
    { "framedropping", MP_CMD_FRAMEDROPPING, 1, 0, -1, MSGTR_FramedroppingStatus },
    { "gamma", MP_CMD_GAMMA, 0, OSD_BRIGHTNESS, -1, MSGTR_Gamma },
    { "brightness", MP_CMD_BRIGHTNESS, 0, OSD_BRIGHTNESS, -1, MSGTR_Brightness },
    { "contrast", MP_CMD_CONTRAST, 0, OSD_CONTRAST, -1, MSGTR_Contrast },
    { "saturation", MP_CMD_SATURATION, 0, OSD_SATURATION, -1, MSGTR_Saturation },
    { "hue", MP_CMD_HUE, 0, OSD_HUE, -1, MSGTR_Hue },
    { "vsync", MP_CMD_SWITCH_VSYNC, 1, 0, -1, MSGTR_VSyncStatus },
	// subs
    { "sub", MP_CMD_SUB_SELECT, 1, 0, -1, MSGTR_SubSelectStatus },
    { "sub_pos", MP_CMD_SUB_POS, 0, 0, -1, MSGTR_SubPosStatus },
    { "sub_alignment", MP_CMD_SUB_ALIGNMENT, 1, 0, -1, MSGTR_SubAlignStatus },
    { "sub_delay", MP_CMD_SUB_DELAY, 0, 0, OSD_MSG_SUB_DELAY, MSGTR_SubDelayStatus },
    { "sub_visibility", MP_CMD_SUB_VISIBILITY, 1, 0, -1, MSGTR_SubVisibleStatus },
    { "sub_forced_only", MP_CMD_SUB_FORCED_ONLY, 1, 0, -1, MSGTR_SubForcedOnlyStatus },
#ifdef USE_TV
    { "tv_brightness", MP_CMD_TV_SET_BRIGHTNESS, 0, OSD_BRIGHTNESS, -1, MSGTR_Brightness },
    { "tv_hue", MP_CMD_TV_SET_HUE, 0, OSD_HUE, -1, MSGTR_Hue },
    { "tv_saturation", MP_CMD_TV_SET_SATURATION, 0, OSD_SATURATION, -1, MSGTR_Saturation },
    { "tv_contrast", MP_CMD_TV_SET_CONTRAST, 0, OSD_CONTRAST, -1, MSGTR_Contrast },
#endif
    { NULL, 0, 0, 0, -1, NULL }
};


/// Handle commands that set a property.
static int set_property_command(MPContext * mpctx, mp_cmd_t * cmd)
{
    int i, r;
    m_option_t *prop;

    // look for the command
    for (i = 0; set_prop_cmd[i].name; i++)
	if (set_prop_cmd[i].cmd == cmd->id)
	    break;
    if (!set_prop_cmd[i].name)
	return 0;

    // get the property
    prop = mp_property_find(set_prop_cmd[i].name);
    if (!prop)
	return 0;

    // toggle command
    if (set_prop_cmd[i].toggle) {
	// set to value
	if (cmd->nargs > 0 && cmd->args[0].v.i >= prop->min)
	    r = m_property_do(prop, M_PROPERTY_SET, &cmd->args[0].v.i, mpctx);
	else
	    r = m_property_do(prop, M_PROPERTY_STEP_UP, NULL, mpctx);
    } else if (cmd->args[1].v.i)	//set
	r = m_property_do(prop, M_PROPERTY_SET, &cmd->args[0].v, mpctx);
    else			// adjust
	r = m_property_do(prop, M_PROPERTY_STEP_UP, &cmd->args[0].v, mpctx);

    if (r <= 0)
	return 1;

    if (set_prop_cmd[i].osd_progbar) {
	if (prop->type == CONF_TYPE_INT) {
	    if (m_property_do(prop, M_PROPERTY_GET, &r, mpctx) > 0)
		set_osd_bar(set_prop_cmd[i].osd_progbar,
			    set_prop_cmd[i].osd_msg, prop->min, prop->max, r);
	} else if (prop->type == CONF_TYPE_FLOAT) {
	    float f;
	    if (m_property_do(prop, M_PROPERTY_GET, &f, mpctx) > 0)
		set_osd_bar(set_prop_cmd[i].osd_progbar,
			    set_prop_cmd[i].osd_msg, prop->min, prop->max, f);
	} else
	    mp_msg(MSGT_CPLAYER, MSGL_ERR,
		   "Property use an unsupported type.\n");
	return 1;
    }

    if (set_prop_cmd[i].osd_msg) {
	char *val = m_property_print(prop, mpctx);
	if (val) {
	    set_osd_msg(set_prop_cmd[i].osd_id >=
			0 ? set_prop_cmd[i].osd_id : OSD_MSG_PROPERTY + i,
			1, osd_duration, set_prop_cmd[i].osd_msg, val);
	    free(val);
	}
    }
    return 1;
}


int run_command(MPContext * mpctx, mp_cmd_t * cmd)
{
    sh_audio_t * const sh_audio = mpctx->sh_audio;
    sh_video_t * const sh_video = mpctx->sh_video;
    int brk_cmd = 0;
    if (!set_property_command(mpctx, cmd))
	switch (cmd->id) {
	case MP_CMD_SEEK:{
		float v;
		int abs;
		if (sh_video)
		    mpctx->osd_show_percentage = sh_video->fps;
		v = cmd->args[0].v.f;
		abs = (cmd->nargs > 1) ? cmd->args[1].v.i : 0;
		if (abs == 2) {	/* Absolute seek to a specific timestamp in seconds */
		    abs_seek_pos = 1;
		    if (sh_video)
			mpctx->osd_function =
			    (v > sh_video->pts) ? OSD_FFW : OSD_REW;
		    rel_seek_secs = v;
		} else if (abs) {	/* Absolute seek by percentage */
		    abs_seek_pos = 3;
		    if (sh_video)
			mpctx->osd_function = OSD_FFW;	// Direction isn't set correctly
		    rel_seek_secs = v / 100.0;
		} else {
		    rel_seek_secs += v;
		    mpctx->osd_function = (v > 0) ? OSD_FFW : OSD_REW;
		}
		brk_cmd = 1;
	    }
	    break;

	case MP_CMD_SET_PROPERTY:{
		m_option_t *prop = mp_property_find(cmd->args[0].v.s);
		if (!prop)
		    mp_msg(MSGT_CPLAYER, MSGL_WARN,
			   "Unknown property: '%s'\n", cmd->args[0].v.s);
		else if (m_property_parse(prop, cmd->args[1].v.s, mpctx) <= 0)
		    mp_msg(MSGT_CPLAYER, MSGL_WARN,
			   "Failed to set property '%s' to '%s'.\n",
			   cmd->args[0].v.s, cmd->args[1].v.s);
	    }
	    break;

	case MP_CMD_STEP_PROPERTY:{
		m_option_t *prop = mp_property_find(cmd->args[0].v.s);
		float arg = cmd->args[1].v.f;
		if (!prop)
		    mp_msg(MSGT_CPLAYER, MSGL_WARN,
			   "Unknown property: '%s'\n", cmd->args[0].v.s);
		else if (m_property_do
			 (prop, M_PROPERTY_STEP_UP,
			  arg ? &arg : NULL, mpctx) <= 0)
		    mp_msg(MSGT_CPLAYER, MSGL_WARN,
			   "Failed to increment property '%s' by %f.\n",
			   cmd->args[0].v.s, arg);
	    }
	    break;

	case MP_CMD_GET_PROPERTY:{
		m_option_t *prop;
		void *val;
		char *tmp;
		prop = mp_property_find(cmd->args[0].v.s);
		if (!prop) {
		    mp_msg(MSGT_CPLAYER, MSGL_WARN,
			   "Unknown property: '%s'\n", cmd->args[0].v.s);
		    break;
		}
		/* Use m_option_print directly to get easily parseable values. */
		val = calloc(1, prop->type->size);
		if (m_property_do(prop, M_PROPERTY_GET, val, mpctx) <= 0) {
		    mp_msg(MSGT_CPLAYER, MSGL_WARN,
			   "Failed to get value of property '%s'.\n",
			   cmd->args[0].v.s);
		    break;
		}
		tmp = m_option_print(prop, val);
		if (!tmp || tmp == (char *) -1) {
		    mp_msg(MSGT_CPLAYER, MSGL_WARN,
			   "Failed to print value of property '%s'.\n",
			   cmd->args[0].v.s);
		    break;
		}
		mp_msg(MSGT_GLOBAL, MSGL_INFO, "ANS_%s=%s\n",
		       cmd->args[0].v.s, tmp);
		free(tmp);
	    }
	    break;

	case MP_CMD_EDL_MARK:
	    if (edl_fd) {
		float v = sh_video ? sh_video->pts :
		    playing_audio_pts(sh_audio, mpctx->d_audio,
				      mpctx->audio_out);

		if (mpctx->begin_skip == MP_NOPTS_VALUE) {
		    mpctx->begin_skip = v;
		    mp_msg(MSGT_CPLAYER, MSGL_INFO, MSGTR_EdloutStartSkip);
		} else {
		    if (mpctx->begin_skip > v)
			mp_msg(MSGT_CPLAYER, MSGL_WARN, MSGTR_EdloutBadStop);
		    else {
			fprintf(edl_fd, "%f %f %d\n", mpctx->begin_skip, v, 0);
			mp_msg(MSGT_CPLAYER, MSGL_INFO, MSGTR_EdloutEndSkip);
		    }
		    mpctx->begin_skip = MP_NOPTS_VALUE;
		}
	    }
	    break;

	case MP_CMD_SWITCH_RATIO:
	    if (cmd->nargs == 0 || cmd->args[0].v.f == -1)
		movie_aspect = (float) sh_video->disp_w / sh_video->disp_h;
	    else
		movie_aspect = cmd->args[0].v.f;
	    mpcodecs_config_vo(sh_video, sh_video->disp_w, sh_video->disp_h, 0);
	    break;

	case MP_CMD_SPEED_INCR:{
		float v = cmd->args[0].v.f;
		playback_speed += v;
		build_afilter_chain(sh_audio, &ao_data);
		set_osd_msg(OSD_MSG_SPEED, 1, osd_duration, MSGTR_OSDSpeed,
			    playback_speed);
	    } break;

	case MP_CMD_SPEED_MULT:{
		float v = cmd->args[0].v.f;
		playback_speed *= v;
		build_afilter_chain(sh_audio, &ao_data);
		set_osd_msg(OSD_MSG_SPEED, 1, osd_duration, MSGTR_OSDSpeed,
			    playback_speed);
	    } break;

	case MP_CMD_SPEED_SET:{
		float v = cmd->args[0].v.f;
		playback_speed = v;
		build_afilter_chain(sh_audio, &ao_data);
		set_osd_msg(OSD_MSG_SPEED, 1, osd_duration, MSGTR_OSDSpeed,
			    playback_speed);
	    } break;

	case MP_CMD_FRAME_STEP:
	case MP_CMD_PAUSE:
	    cmd->pausing = 1;
	    brk_cmd = 1;
	    break;

	case MP_CMD_FILE_FILTER:
	    file_filter = cmd->args[0].v.i;
	    break;

	case MP_CMD_QUIT:
	    exit_player_with_rc(MSGTR_Exit_quit,
				(cmd->nargs > 0) ? cmd->args[0].v.i : 0);

	case MP_CMD_PLAY_TREE_STEP:{
		int n = cmd->args[0].v.i == 0 ? 1 : cmd->args[0].v.i;
		int force = cmd->args[1].v.i;

#ifdef HAVE_NEW_GUI
		if (use_gui) {
		    int i = 0;
		    if (n > 0)
			for (i = 0; i < n; i++)
			    mplNext();
		    else
			for (i = 0; i < -1 * n; i++)
			    mplPrev();
		} else
#endif
		{
		    if (!force && mpctx->playtree_iter) {
			play_tree_iter_t *i =
			    play_tree_iter_new_copy(mpctx->playtree_iter);
			if (play_tree_iter_step(i, n, 0) ==
			    PLAY_TREE_ITER_ENTRY)
			    mpctx->eof =
				(n > 0) ? PT_NEXT_ENTRY : PT_PREV_ENTRY;
			play_tree_iter_free(i);
		    } else
			mpctx->eof = (n > 0) ? PT_NEXT_ENTRY : PT_PREV_ENTRY;
		    if (mpctx->eof)
			mpctx->play_tree_step = n;
		    brk_cmd = 1;
		}
	    }
	    break;

	case MP_CMD_PLAY_TREE_UP_STEP:{
		int n = cmd->args[0].v.i > 0 ? 1 : -1;
		int force = cmd->args[1].v.i;

		if (!force && mpctx->playtree_iter) {
		    play_tree_iter_t *i =
			play_tree_iter_new_copy(mpctx->playtree_iter);
		    if (play_tree_iter_up_step(i, n, 0) == PLAY_TREE_ITER_ENTRY)
			mpctx->eof = (n > 0) ? PT_UP_NEXT : PT_UP_PREV;
		    play_tree_iter_free(i);
		} else
		    mpctx->eof = (n > 0) ? PT_UP_NEXT : PT_UP_PREV;
		brk_cmd = 1;
	    }
	    break;

	case MP_CMD_PLAY_ALT_SRC_STEP:
	    if (mpctx->playtree_iter && mpctx->playtree_iter->num_files > 1) {
		int v = cmd->args[0].v.i;
		if (v > 0
		    && mpctx->playtree_iter->file <
		    mpctx->playtree_iter->num_files)
		    mpctx->eof = PT_NEXT_SRC;
		else if (v < 0 && mpctx->playtree_iter->file > 1)
		    mpctx->eof = PT_PREV_SRC;
	    }
	    brk_cmd = 1;
	    break;

	case MP_CMD_SUB_STEP:
	    if (sh_video) {
		int movement = cmd->args[0].v.i;
		step_sub(subdata, sh_video->pts, movement);
#ifdef USE_ASS
		if (ass_track)
		    sub_delay +=
			ass_step_sub(ass_track,
				     (sh_video->pts +
				      sub_delay) * 1000 + .5, movement) / 1000.;
#endif
		set_osd_msg(OSD_MSG_SUB_DELAY, 1, osd_duration,
			    MSGTR_OSDSubDelay, ROUND(sub_delay * 1000));
	    }
	    break;

	case MP_CMD_SUB_LOG:
	    log_sub();
	    break;

	case MP_CMD_OSD:{
		int v = cmd->args[0].v.i;
		int max = (term_osd
			   && !sh_video) ? MAX_TERM_OSD_LEVEL : MAX_OSD_LEVEL;
		if (osd_level > max)
		    osd_level = max;
		if (v < 0)
		    osd_level = (osd_level + 1) % (max + 1);
		else
		    osd_level = v > max ? max : v;
		/* Show OSD state when disabled, but not when an explicit
		   argument is given to the OSD command, i.e. in slave mode. */
		if (v == -1 && osd_level <= 1)
		    set_osd_msg(OSD_MSG_OSD_STATUS, 0, osd_duration,
				MSGTR_OSDosd,
				osd_level ? MSGTR_OSDenabled :
				MSGTR_OSDdisabled);
		else
		    rm_osd_msg(OSD_MSG_OSD_STATUS);
	    }
	    break;

	case MP_CMD_OSD_SHOW_TEXT:
	    set_osd_msg(OSD_MSG_TEXT, cmd->args[2].v.i,
			(cmd->args[1].v.i <
			 0 ? osd_duration : cmd->args[1].v.i),
			"%-.63s", cmd->args[0].v.s);
	    break;

	case MP_CMD_OSD_SHOW_PROPERTY_TEXT:{
		char *txt = m_properties_expand_string(mp_properties,
						       cmd->args[0].v.s,
						       mpctx);
		/* if no argument supplied take default osd_duration, else <arg> ms. */
		if (txt) {
		    set_osd_msg(OSD_MSG_TEXT, cmd->args[2].v.i,
				(cmd->args[1].v.i <
				 0 ? osd_duration : cmd->args[1].v.i),
				"%-.63s", txt);
		    free(txt);
		}
	    }
	    break;

	case MP_CMD_LOADFILE:{
		play_tree_t *e = play_tree_new();
		play_tree_add_file(e, cmd->args[0].v.s);

		if (cmd->args[1].v.i)	// append
		    play_tree_append_entry(mpctx->playtree, e);
		else {
		    // Go back to the starting point.
		    while (play_tree_iter_up_step
			   (mpctx->playtree_iter, 0, 1) != PLAY_TREE_ITER_END)
			/* NOP */ ;
		    play_tree_free_list(mpctx->playtree->child, 1);
		    play_tree_set_child(mpctx->playtree, e);
		    play_tree_iter_step(mpctx->playtree_iter, 0, 0);
		    mpctx->eof = PT_NEXT_SRC;
		}
		brk_cmd = 1;
	    }
	    break;

	case MP_CMD_LOADLIST:{
		play_tree_t *e = parse_playlist_file(cmd->args[0].v.s);
		if (!e)
		    mp_msg(MSGT_CPLAYER, MSGL_ERR,
			   MSGTR_PlaylistLoadUnable, cmd->args[0].v.s);
		else {
		    if (cmd->args[1].v.i)	// append
			play_tree_append_entry(mpctx->playtree, e);
		    else {
			// Go back to the starting point.
			while (play_tree_iter_up_step
			       (mpctx->playtree_iter, 0, 1)
			       != PLAY_TREE_ITER_END)
			    /* NOP */ ;
			play_tree_free_list(mpctx->playtree->child, 1);
			play_tree_set_child(mpctx->playtree, e);
			play_tree_iter_step(mpctx->playtree_iter, 0, 0);
			mpctx->eof = PT_NEXT_SRC;
		    }
		}
		brk_cmd = 1;
	    }
	    break;

#ifdef USE_RADIO
	case MP_CMD_RADIO_STEP_CHANNEL:
	    if (mpctx->demuxer->stream->type == STREAMTYPE_RADIO) {
		int v = cmd->args[0].v.i;
		if (v > 0)
		    radio_step_channel(mpctx->demuxer->stream,
				       RADIO_CHANNEL_HIGHER);
		else
		    radio_step_channel(mpctx->demuxer->stream,
				       RADIO_CHANNEL_LOWER);
		if (radio_get_channel_name(mpctx->demuxer->stream)) {
		    set_osd_msg(OSD_MSG_RADIO_CHANNEL, 1, osd_duration,
				MSGTR_OSDChannel,
				radio_get_channel_name(mpctx->demuxer->stream));
		}
	    }
	    break;

	case MP_CMD_RADIO_SET_CHANNEL:
	    if (mpctx->demuxer->stream->type == STREAMTYPE_RADIO) {
		radio_set_channel(mpctx->demuxer->stream, cmd->args[0].v.s);
		if (radio_get_channel_name(mpctx->demuxer->stream)) {
		    set_osd_msg(OSD_MSG_RADIO_CHANNEL, 1, osd_duration,
				MSGTR_OSDChannel,
				radio_get_channel_name(mpctx->demuxer->stream));
		}
	    }
	    break;

	case MP_CMD_RADIO_SET_FREQ:
	    if (mpctx->demuxer->stream->type == STREAMTYPE_RADIO)
		radio_set_freq(mpctx->demuxer->stream, cmd->args[0].v.f);
	    break;

	case MP_CMD_RADIO_STEP_FREQ:
	    if (mpctx->demuxer->stream->type == STREAMTYPE_RADIO)
		radio_step_freq(mpctx->demuxer->stream, cmd->args[0].v.f);
	    break;
#endif

#ifdef USE_TV
	case MP_CMD_TV_SET_FREQ:
	    if (mpctx->file_format == DEMUXER_TYPE_TV)
		tv_set_freq((tvi_handle_t *) (mpctx->demuxer->priv),
			    cmd->args[0].v.f * 16.0);
	    break;

	case MP_CMD_TV_STEP_FREQ:
	    if (mpctx->file_format == DEMUXER_TYPE_TV)
		tv_step_freq((tvi_handle_t *) (mpctx->demuxer->priv),
			    cmd->args[0].v.f * 16.0);
	    break;

	case MP_CMD_TV_SET_NORM:
	    if (mpctx->file_format == DEMUXER_TYPE_TV)
		tv_set_norm((tvi_handle_t *) (mpctx->demuxer->priv),
			    cmd->args[0].v.s);
	    break;

	case MP_CMD_TV_STEP_CHANNEL:{
		if (mpctx->file_format == DEMUXER_TYPE_TV) {
		    int v = cmd->args[0].v.i;
		    if (v > 0) {
			tv_step_channel((tvi_handle_t *) (mpctx->
							  demuxer->priv),
					TV_CHANNEL_HIGHER);
		    } else {
			tv_step_channel((tvi_handle_t *) (mpctx->
							  demuxer->priv),
					TV_CHANNEL_LOWER);
		    }
		    if (tv_channel_list) {
			set_osd_msg(OSD_MSG_TV_CHANNEL, 1, osd_duration,
				    MSGTR_OSDChannel, tv_channel_current->name);
			//vo_osd_changed(OSDTYPE_SUBTITLE);
		    }
		}
	    }
#ifdef HAS_DVBIN_SUPPORT
	    if ((mpctx->stream->type == STREAMTYPE_DVB)
		&& mpctx->stream->priv) {
		dvb_priv_t *priv = (dvb_priv_t *) mpctx->stream->priv;
		if (priv->is_on) {
		    int dir;
		    int v = cmd->args[0].v.i;

		    mpctx->last_dvb_step = v;
		    if (v > 0)
			dir = DVB_CHANNEL_HIGHER;
		    else
			dir = DVB_CHANNEL_LOWER;


		    if (dvb_step_channel(priv, dir))
			mpctx->eof = mpctx->dvbin_reopen = 1;
		}
	    }
#endif				/* HAS_DVBIN_SUPPORT */
	    break;

	case MP_CMD_TV_SET_CHANNEL:
	    if (mpctx->file_format == DEMUXER_TYPE_TV) {
		tv_set_channel((tvi_handle_t *) (mpctx->demuxer->priv),
			       cmd->args[0].v.s);
		if (tv_channel_list) {
		    set_osd_msg(OSD_MSG_TV_CHANNEL, 1, osd_duration,
				MSGTR_OSDChannel, tv_channel_current->name);
		    //vo_osd_changed(OSDTYPE_SUBTITLE);
		}
	    }
	    break;

#ifdef HAS_DVBIN_SUPPORT
	case MP_CMD_DVB_SET_CHANNEL:
	    if ((mpctx->stream->type == STREAMTYPE_DVB)
		&& mpctx->stream->priv) {
		dvb_priv_t *priv = (dvb_priv_t *) mpctx->stream->priv;
		if (priv->is_on) {
		    if (priv->list->current <= cmd->args[0].v.i)
			mpctx->last_dvb_step = 1;
		    else
			mpctx->last_dvb_step = -1;

		    if (dvb_set_channel
			(priv, cmd->args[1].v.i, cmd->args[0].v.i))
			mpctx->eof = mpctx->dvbin_reopen = 1;
		}
	    }
	    break;
#endif				/* HAS_DVBIN_SUPPORT     */

	case MP_CMD_TV_LAST_CHANNEL:
	    if (mpctx->file_format == DEMUXER_TYPE_TV) {
		tv_last_channel((tvi_handle_t *) (mpctx->demuxer->priv));
		if (tv_channel_list) {
		    set_osd_msg(OSD_MSG_TV_CHANNEL, 1, osd_duration,
				MSGTR_OSDChannel, tv_channel_current->name);
		    //vo_osd_changed(OSDTYPE_SUBTITLE);
		}
	    }
	    break;

	case MP_CMD_TV_STEP_NORM:
	    if (mpctx->file_format == DEMUXER_TYPE_TV)
		tv_step_norm((tvi_handle_t *) (mpctx->demuxer->priv));
	    break;

	case MP_CMD_TV_STEP_CHANNEL_LIST:
	    if (mpctx->file_format == DEMUXER_TYPE_TV)
		tv_step_chanlist((tvi_handle_t *) (mpctx->demuxer->priv));
	    break;
#endif				/* USE_TV */

	case MP_CMD_SUB_LOAD:
	    if (sh_video) {
		int n = mpctx->set_of_sub_size;
		add_subtitles(cmd->args[0].v.s, sh_video->fps, 0);
		if (n != mpctx->set_of_sub_size) {
		    if (mpctx->global_sub_indices[SUB_SOURCE_SUBS] < 0)
			mpctx->global_sub_indices[SUB_SOURCE_SUBS] =
			    mpctx->global_sub_size;
		    ++mpctx->global_sub_size;
		}
	    }
	    break;

	case MP_CMD_SUB_REMOVE:
	    if (sh_video) {
		int v = cmd->args[0].v.i;
		sub_data *subd;
		if (v < 0) {
		    for (v = 0; v < mpctx->set_of_sub_size; ++v) {
			subd = mpctx->set_of_subtitles[v];
			mp_msg(MSGT_CPLAYER, MSGL_STATUS,
			       MSGTR_RemovedSubtitleFile, v + 1,
			       filename_recode(subd->filename));
			sub_free(subd);
			mpctx->set_of_subtitles[v] = NULL;
		    }
		    mpctx->global_sub_indices[SUB_SOURCE_SUBS] = -1;
		    mpctx->global_sub_size -= mpctx->set_of_sub_size;
		    mpctx->set_of_sub_size = 0;
		    if (mpctx->set_of_sub_pos >= 0) {
			mpctx->global_sub_pos = -2;
			vo_sub_last = vo_sub = NULL;
			vo_osd_changed(OSDTYPE_SUBTITLE);
			vo_update_osd(sh_video->disp_w, sh_video->disp_h);
			mp_input_queue_cmd(mp_input_parse_cmd("sub_select"));
		    }
		} else if (v < mpctx->set_of_sub_size) {
		    subd = mpctx->set_of_subtitles[v];
		    mp_msg(MSGT_CPLAYER, MSGL_STATUS,
			   MSGTR_RemovedSubtitleFile, v + 1,
			   filename_recode(subd->filename));
		    sub_free(subd);
		    if (mpctx->set_of_sub_pos == v) {
			mpctx->global_sub_pos = -2;
			vo_sub_last = vo_sub = NULL;
			vo_osd_changed(OSDTYPE_SUBTITLE);
			vo_update_osd(sh_video->disp_w, sh_video->disp_h);
			mp_input_queue_cmd(mp_input_parse_cmd("sub_select"));
		    } else if (mpctx->set_of_sub_pos > v) {
			--mpctx->set_of_sub_pos;
			--mpctx->global_sub_pos;
		    }
		    while (++v < mpctx->set_of_sub_size)
			mpctx->set_of_subtitles[v - 1] =
			    mpctx->set_of_subtitles[v];
		    --mpctx->set_of_sub_size;
		    --mpctx->global_sub_size;
		    if (mpctx->set_of_sub_size <= 0)
			mpctx->global_sub_indices[SUB_SOURCE_SUBS] = -1;
		    mpctx->set_of_subtitles[mpctx->set_of_sub_size] = NULL;
		}
	    }
	    break;

	case MP_CMD_GET_SUB_VISIBILITY:
	    if (sh_video) {
		mp_msg(MSGT_GLOBAL, MSGL_INFO,
		       "ANS_SUB_VISIBILITY=%d\n", sub_visibility);
	    }
	    break;

	case MP_CMD_SCREENSHOT:
	    if (vo_config_count) {
		mp_msg(MSGT_CPLAYER, MSGL_INFO, "sending VFCTRL_SCREENSHOT!\n");
		if (CONTROL_OK !=
		    ((vf_instance_t *) sh_video->vfilter)->
		    control(sh_video->vfilter, VFCTRL_SCREENSHOT,
			    &cmd->args[0].v.i))
		    mpctx->video_out->control(VOCTRL_SCREENSHOT, NULL);
	    }
	    break;

	case MP_CMD_VF_CHANGE_RECTANGLE:
	    set_rectangle(sh_video, cmd->args[0].v.i, cmd->args[1].v.i);
	    break;

	case MP_CMD_GET_TIME_LENGTH:{
		mp_msg(MSGT_GLOBAL, MSGL_INFO, "ANS_LENGTH=%.2lf\n",
		       demuxer_get_time_length(mpctx->demuxer));
	    }
	    break;

	case MP_CMD_GET_FILENAME:{
		mp_msg(MSGT_GLOBAL, MSGL_INFO, "ANS_FILENAME='%s'\n",
		       get_metadata(META_NAME));
	    }
	    break;

	case MP_CMD_GET_VIDEO_CODEC:{
		char *inf = get_metadata(META_VIDEO_CODEC);
		if (!inf)
		    inf = strdup("");
		mp_msg(MSGT_GLOBAL, MSGL_INFO, "ANS_VIDEO_CODEC='%s'\n", inf);
		free(inf);
	    }
	    break;

	case MP_CMD_GET_VIDEO_BITRATE:{
		char *inf = get_metadata(META_VIDEO_BITRATE);
		if (!inf)
		    inf = strdup("");
		mp_msg(MSGT_GLOBAL, MSGL_INFO, "ANS_VIDEO_BITRATE='%s'\n", inf);
		free(inf);
	    }
	    break;

	case MP_CMD_GET_VIDEO_RESOLUTION:{
		char *inf = get_metadata(META_VIDEO_RESOLUTION);
		if (!inf)
		    inf = strdup("");
		mp_msg(MSGT_GLOBAL, MSGL_INFO,
		       "ANS_VIDEO_RESOLUTION='%s'\n", inf);
		free(inf);
	    }
	    break;

	case MP_CMD_GET_AUDIO_CODEC:{
		char *inf = get_metadata(META_AUDIO_CODEC);
		if (!inf)
		    inf = strdup("");
		mp_msg(MSGT_GLOBAL, MSGL_INFO, "ANS_AUDIO_CODEC='%s'\n", inf);
		free(inf);
	    }
	    break;

	case MP_CMD_GET_AUDIO_BITRATE:{
		char *inf = get_metadata(META_AUDIO_BITRATE);
		if (!inf)
		    inf = strdup("");
		mp_msg(MSGT_GLOBAL, MSGL_INFO, "ANS_AUDIO_BITRATE='%s'\n", inf);
		free(inf);
	    }
	    break;

	case MP_CMD_GET_AUDIO_SAMPLES:{
		char *inf = get_metadata(META_AUDIO_SAMPLES);
		if (!inf)
		    inf = strdup("");
		mp_msg(MSGT_GLOBAL, MSGL_INFO, "ANS_AUDIO_SAMPLES='%s'\n", inf);
		free(inf);
	    }
	    break;

	case MP_CMD_GET_META_TITLE:{
		char *inf = get_metadata(META_INFO_TITLE);
		if (!inf)
		    inf = strdup("");
		mp_msg(MSGT_GLOBAL, MSGL_INFO, "ANS_META_TITLE='%s'\n", inf);
		free(inf);
	    }
	    break;

	case MP_CMD_GET_META_ARTIST:{
		char *inf = get_metadata(META_INFO_ARTIST);
		if (!inf)
		    inf = strdup("");
		mp_msg(MSGT_GLOBAL, MSGL_INFO, "ANS_META_ARTIST='%s'\n", inf);
		free(inf);
	    }
	    break;

	case MP_CMD_GET_META_ALBUM:{
		char *inf = get_metadata(META_INFO_ALBUM);
		if (!inf)
		    inf = strdup("");
		mp_msg(MSGT_GLOBAL, MSGL_INFO, "ANS_META_ALBUM='%s'\n", inf);
		free(inf);
	    }
	    break;

	case MP_CMD_GET_META_YEAR:{
		char *inf = get_metadata(META_INFO_YEAR);
		if (!inf)
		    inf = strdup("");
		mp_msg(MSGT_GLOBAL, MSGL_INFO, "ANS_META_YEAR='%s'\n", inf);
		free(inf);
	    }
	    break;

	case MP_CMD_GET_META_COMMENT:{
		char *inf = get_metadata(META_INFO_COMMENT);
		if (!inf)
		    inf = strdup("");
		mp_msg(MSGT_GLOBAL, MSGL_INFO, "ANS_META_COMMENT='%s'\n", inf);
		free(inf);
	    }
	    break;

	case MP_CMD_GET_META_TRACK:{
		char *inf = get_metadata(META_INFO_TRACK);
		if (!inf)
		    inf = strdup("");
		mp_msg(MSGT_GLOBAL, MSGL_INFO, "ANS_META_TRACK='%s'\n", inf);
		free(inf);
	    }
	    break;

	case MP_CMD_GET_META_GENRE:{
		char *inf = get_metadata(META_INFO_GENRE);
		if (!inf)
		    inf = strdup("");
		mp_msg(MSGT_GLOBAL, MSGL_INFO, "ANS_META_GENRE='%s'\n", inf);
		free(inf);
	    }
	    break;

	case MP_CMD_GET_VO_FULLSCREEN:
	    if (mpctx->video_out && vo_config_count)
		mp_msg(MSGT_GLOBAL, MSGL_INFO, "ANS_VO_FULLSCREEN=%d\n", vo_fs);
	    break;

	case MP_CMD_GET_PERCENT_POS:
	    mp_msg(MSGT_GLOBAL, MSGL_INFO, "ANS_PERCENT_POSITION=%d\n",
		   demuxer_get_percent_pos(mpctx->demuxer));
	    break;

	case MP_CMD_GET_TIME_POS:{
		float pos = 0;
		if (sh_video)
		    pos = sh_video->pts;
		else if (sh_audio && mpctx->audio_out)
		    pos =
			playing_audio_pts(sh_audio, mpctx->d_audio,
					  mpctx->audio_out);
		mp_msg(MSGT_GLOBAL, MSGL_INFO, "ANS_TIME_POSITION=%.1f\n", pos);
	    }
	    break;

	case MP_CMD_RUN:
#ifndef __MINGW32__
	    if (!fork()) {
		execl("/bin/sh", "sh", "-c", cmd->args[0].v.s, NULL);
		exit(0);
	    }
#endif
	    break;

	case MP_CMD_KEYDOWN_EVENTS:
	    mplayer_put_key(cmd->args[0].v.i);
	    break;

	case MP_CMD_SEEK_CHAPTER:{
		int seek = cmd->args[0].v.i;
		int abs = (cmd->nargs > 1) ? cmd->args[1].v.i : 0;
		int chap;
		float next_pts = 0;
		int num_chapters;
		char *chapter_name;

		rel_seek_secs = 0;
		abs_seek_pos = 0;
		chap =
		    demuxer_seek_chapter(mpctx->demuxer, seek, abs,
					 &next_pts, &num_chapters,
					 &chapter_name);
		if (chap != -1) {
		    if (next_pts > -1.0) {
			abs_seek_pos = 1;
			rel_seek_secs = next_pts;
		    }
		    if (chapter_name) {
			set_osd_msg(OSD_MSG_TEXT, 1, osd_duration,
				    MSGTR_OSDChapter, chap + 1, chapter_name);
			free(chapter_name);
		    }
		} else {
		    if (seek > 0)
			rel_seek_secs = 1000000000.;
		    else
			set_osd_msg(OSD_MSG_TEXT, 1, osd_duration,
				    MSGTR_OSDChapter, 0, MSGTR_Unknown);
		}
		break;
	    }
	    break;

	case MP_CMD_SET_MOUSE_POS:{
		int button = -1, pointer_x, pointer_y;
		double dx, dy;
		pointer_x = cmd->args[0].v.i;
		pointer_y = cmd->args[1].v.i;
		rescale_input_coordinates(pointer_x, pointer_y, &dx, &dy);
#ifdef USE_DVDNAV
		if (mpctx->stream->type == STREAMTYPE_DVDNAV
		    && dx > 0.0 && dy > 0.0) {
		    pointer_x = (int) (dx * (double) sh_video->disp_w);
		    pointer_y = (int) (dy * (double) sh_video->disp_h);
		    mp_dvdnav_update_mouse_pos(mpctx->stream,
					       pointer_x, pointer_y, &button);
		    if (button > 0)
			set_osd_msg(OSD_MSG_TEXT, 1, osd_duration,
				    "Selected button number %d", button);
		}
#endif
	    }
	    break;

#ifdef USE_DVDNAV
	case MP_CMD_DVDNAV:{
		int button = -1;
		if (mpctx->stream->type != STREAMTYPE_DVDNAV)
		    break;

		if (mp_dvdnav_handle_input
		    (mpctx->stream, cmd->args[0].v.i, &button)) {
		    uninit_player(INITED_ALL - (INITED_STREAM | INITED_INPUT |
				   (fixed_vo ? INITED_VO : 0)));
		    brk_cmd = 2;
		} else if (button > 0)
		    set_osd_msg(OSD_MSG_TEXT, 1, osd_duration,
				"Selected button number %d", button);
	    }
	    break;
#endif

	default:
#ifdef HAVE_NEW_GUI
	    if ((use_gui) && (cmd->id > MP_CMD_GUI_EVENTS))
		guiGetEvent(guiIEvent, (char *) cmd->id);
	    else
#endif
		mp_msg(MSGT_CPLAYER, MSGL_V,
		       "Received unknown cmd %s\n", cmd->name);
	}

    switch (cmd->pausing) {
    case 1:	// "pausing"
	mpctx->osd_function = OSD_PAUSE;
	break;
    case 3:	// "pausing_toggle"
	mpctx->was_paused = !mpctx->was_paused;
	// fall through
    case 2:	// "pausing_keep"
	if (mpctx->was_paused)
	    mpctx->osd_function = OSD_PAUSE;
    }
    return brk_cmd;
}
