#include <limits.h>
// #include <unistd.h>
#include <sys/endian.h>
#include <assert.h>
#include <pthread.h>
#include <errno.h>

#include <jni.h>
#include <quiet.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "quiet", __VA_ARGS__)

static const unsigned int num_playback_channels = 2;
static const unsigned int num_record_channels = 2;

#if 0 && defined(SL_ANDROID_PCM_REPRESENTATION_FLOAT)
#define QUIET_JNI_USE_FLOAT 1
typedef float opensl_sample_t;
#else
typedef int16_t opensl_sample_t;
#endif

typedef jlong jvm_pointer;

jvm_pointer jvm_opaque_pointer(void *p);
void *recover_pointer(jvm_pointer p);
void throw_error(JNIEnv *env, jclass exc_class, const char *err_fmt, ...);

void convert_stereoopensl2monofloat(const opensl_sample_t *stereo_buf, float *mono_f,
                                    size_t num_frames, unsigned int num_channels);
void convert_monofloat2stereoopensl(const float *mono_f, opensl_sample_t *stereo_buf,
                                    size_t num_frames);
typedef struct {
    SLObjectItf engine;
    SLObjectItf output_mix;
    SLEngineItf engine_itf;
} quiet_opensl_system;

SLresult quiet_opensl_system_create(quiet_opensl_system **opensl_sys_dest);
void quiet_opensl_system_destroy(quiet_opensl_system *opensl_sys);

typedef struct {
    opensl_sample_t **buf;
    // num_buf and buf_idx tell us how many buf there are and which we're about
    // to use
    size_t num_buf;
    ptrdiff_t buf_idx;
    // buf_len is the number of frames in each buf
    // number of samples is num_playback_channels * buf_len
    size_t num_frames;
    // scratch is a mono floating point buffer that we can give to
    // quiet_decoder_consume
    float *scratch;
    ssize_t (*produce)(void *, float *, size_t);
    void *produce_arg;
} quiet_opensl_producer;

quiet_opensl_producer *opensl_producer_create(size_t num_buf,
                                              size_t num_frames);
void opensl_producer_destroy(quiet_opensl_producer *p);

typedef struct {
    opensl_sample_t **buf;
    // num_buf and buf_idx tell us how many buf there are and which we're about
    // to use
    size_t num_buf;
    ptrdiff_t buf_idx;
    // buf_len is the number of frames in each buf
    // number of samples is num_record_channels * buf_len
    size_t num_frames;
    // scratch is a mono floating point buffer that we can give to
    // quiet_decoder_consume
    float *scratch;
    void (*consume)(void *, const float *, size_t);
    void *consume_arg;
} quiet_opensl_consumer;

quiet_opensl_consumer *opensl_consumer_create(size_t num_buf,
                                              size_t num_frames);
void opensl_consumer_destroy(quiet_opensl_consumer *c);

typedef struct {
    SLObjectItf player;
    SLAndroidSimpleBufferQueueItf buffer_queue;
    SLPlayItf play;
    quiet_opensl_producer *producer;
} quiet_opensl_player;

SLresult quiet_opensl_create_player(quiet_opensl_system *sys,
                                    quiet_opensl_producer *p,
                                    quiet_opensl_player **player_dest);
// for fully graceful shutdown, close the producer
// that will eventually cause callback to shut everything down
// otherwise, just call stop_player which will immediately halt the player
SLresult quiet_opensl_stop_player(quiet_opensl_player *player);
void quiet_opensl_destroy_player(quiet_opensl_player *player);

typedef struct {
    SLObjectItf recorder;
    SLAndroidSimpleBufferQueueItf buffer_queue;
    SLRecordItf record;
    quiet_opensl_consumer *consumer;
} quiet_opensl_recorder;

SLresult quiet_opensl_create_recorder(quiet_opensl_system *sys,
                                      quiet_opensl_consumer *c,
                                      quiet_opensl_recorder **recorder_dest);
// in theory, there could be buffers inflight that we might care to decode
// for now, there is no way to consume those as well
// so, when calling stop_recorder, you should also close the consumer
SLresult quiet_opensl_stop_recorder(quiet_opensl_recorder *recorder);
void quiet_opensl_destroy_recorder(quiet_opensl_recorder *recorder);

typedef struct {
    quiet_opensl_system *opensl_sys;
} quiet_android_system;

typedef struct {
    quiet_encoder *enc;
    quiet_opensl_producer *producer;
    quiet_opensl_player *player;
} quiet_android_encoder;

typedef struct {
    quiet_decoder *dec;
    quiet_opensl_consumer *consumer;
    quiet_opensl_recorder *recorder;
} quiet_android_decoder;

typedef struct {
    jclass interrupted_io_exception_klass;
    jfieldID interrupted_bytes;
    jclass eof_exception_klass;
    jclass io_exception_klass;
    jclass out_of_memory_error_klass;
    jclass illegal_arg_klass;
} java_java_cache;

typedef struct {
    jclass klass;
    jclass init_exc_klass;
    jfieldID ptr;
} java_system_cache;

typedef struct {
    jclass klass;
    jfieldID ptr;
    jfieldID num_bufs;
    jfieldID buf_len;
} java_encoder_profile_cache;

typedef struct {
    jclass klass;
    jfieldID ptr;
    jfieldID num_bufs;
    jfieldID buf_len;
} java_decoder_profile_cache;

typedef struct {
    jclass klass;
    jfieldID ptr;
} java_encoder_cache;

typedef struct {
    jclass klass;
    jfieldID ptr;
} java_decoder_cache;

typedef struct {
    jclass klass;
    jmethodID ctor;
} java_complex_cache;

typedef struct {
    jclass klass;
    jmethodID ctor;
} java_frame_stats_cache;

typedef struct {
    jclass klass;
    jfieldID fd;
} java_input_stream_cache;

typedef struct {
    jclass klass;
    jfieldID fd;
} java_output_stream_cache;

typedef struct {
    java_java_cache java;
    java_system_cache system;
    java_encoder_profile_cache encoder_profile;
    java_decoder_profile_cache decoder_profile;
    java_encoder_cache encoder;
    java_decoder_cache decoder;
    java_complex_cache complex;
    java_frame_stats_cache frame_stats;
} java_cache;

extern java_cache cache;

extern const char *opensl_engine_error_format;
extern const char *opensl_playback_error_format;
extern const char *opensl_recorder_error_format;
extern const char *encoder_profile_error_format;
extern const char *decoder_profile_error_format;
extern const char *encoder_error_format;
extern const char *decoder_error_format;