#include "quiet-jni.h"

jvm_pointer jvm_opaque_pointer(void *p) { return (jvm_pointer)(intptr_t)p; }

void *recover_pointer(jvm_pointer p) { return (void *)(intptr_t)p; }

void throw_error(JNIEnv *env, jclass exc_class, const char *err_fmt, ...) {
    char *err_msg;
    va_list err_args;
    va_start(err_args, err_fmt);
    vasprintf(&err_msg, err_fmt, err_args);
    va_end(err_args);
    (*env)->ThrowNew(env, exc_class, err_msg);
    free(err_msg);
}

void convert_stereoopensl2monofloat(const opensl_sample_t *stereo_buf, float *mono_f,
                                    size_t num_frames, unsigned int num_channels) {
    for (size_t i = 0; i < num_frames; i++) {
        // just skip every other sample e.g. ignore samples from right channel
#ifdef QUIET_JNI_USE_FLOAT
        mono_f[i] = stereo_buf[i * num_channels];
#else
        mono_f[i] = stereo_buf[i * num_channels] / (float)(SHRT_MAX);
#endif
    }
}

void convert_monofloat2stereoopensl(const float *mono_f, opensl_sample_t *stereo_buf,
                                    size_t num_frames) {
    for (size_t i = 0; i < num_frames; i++) {
        float temp = mono_f[i];
        temp = (temp > 1.0f) ? 1.0f : temp;
        temp = (temp < -1.0f) ? -1.0f : temp;
        // just skip every other sample e.g. leave the right channel empty
#ifdef QUIET_JNI_USE_FLOAT
        stereo_buf[i * num_playback_channels] = temp;
#else
        stereo_buf[i * num_playback_channels] = temp * SHRT_MAX;
#endif
    }
}

const char *opensl_engine_error_format = "failed to initialize opensl engine, opensl error code=%04d";
const char *opensl_playback_error_format = "failed to initialize opensl playback, opensl error code=%04d";
const char *opensl_recorder_error_format = "failed to initialize opensl recorder, opensl error code=%04d";
const char *encoder_profile_error_format = "invalid quiet encoder profile, quiet error code=%04d";
const char *decoder_profile_error_format = "invalid quiet decoder profile, quiet error code=%04d";
const char *encoder_error_format = "failed to initialize quiet encoder, quiet error code=%04d";
const char *decoder_error_format = "failed to initialize quiet decoder, quiet error code=%04d";

void android_system_destroy(quiet_android_system *android_sys) {
    if (android_sys->opensl_sys) {
        quiet_opensl_system_destroy(android_sys->opensl_sys);
    }
    free(android_sys);
}

quiet_android_system *android_system_create(JNIEnv *env) {
    return calloc(1, sizeof(quiet_android_system));
}

java_cache cache;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    // classes
    jclass localInterrupted =
        (*env)->FindClass(env, "java/io/InterruptedIOException");

    cache.java.interrupted_io_exception_klass = (jclass)((*env)->NewGlobalRef(env, localInterrupted));
    (*env)->DeleteLocalRef(env, localInterrupted);

    jclass localEOF =
        (*env)->FindClass(env, "java/io/EOFException");

    cache.java.eof_exception_klass = (jclass)((*env)->NewGlobalRef(env, localEOF));
    (*env)->DeleteLocalRef(env, localEOF);

    jclass localIO =
        (*env)->FindClass(env, "java/io/IOException");

    cache.java.io_exception_klass = (jclass)((*env)->NewGlobalRef(env, localIO));
    (*env)->DeleteLocalRef(env, localIO);

    jclass localIllegalArg =
        (*env)->FindClass(env, "java/lang/IllegalArgumentException");
    cache.java.illegal_arg_klass = (jclass)((*env)->NewGlobalRef(env, localIllegalArg));
    (*env)->DeleteLocalRef(env, localIllegalArg);

    jclass localSystemClass =
        (*env)->FindClass(env, "org/quietmodem/Quiet/QuietSystem");

    cache.system.klass = (jclass)((*env)->NewGlobalRef(env, localSystemClass));
    (*env)->DeleteLocalRef(env, localSystemClass);

    jclass localSystemInitExcClass =
        (*env)->FindClass(env, "org/quietmodem/Quiet/ModemException");
    cache.system.init_exc_klass = (jclass)((*env)->NewGlobalRef(env, localSystemInitExcClass));
    (*env)->DeleteLocalRef(env, localSystemInitExcClass);

    jclass localEncoderProfileClass =
        (*env)->FindClass(env, "org/quietmodem/Quiet/FrameTransmitterConfig");
    cache.encoder_profile.klass = (jclass)((*env)->NewGlobalRef(env, localEncoderProfileClass));
    (*env)->DeleteLocalRef(env, localEncoderProfileClass);

    jclass localDecoderProfileClass =
        (*env)->FindClass(env, "org/quietmodem/Quiet/FrameReceiverConfig");
    cache.decoder_profile.klass = (jclass)((*env)->NewGlobalRef(env, localDecoderProfileClass));
    (*env)->DeleteLocalRef(env, localDecoderProfileClass);

    jclass localEncoderClass =
        (*env)->FindClass(env, "org/quietmodem/Quiet/BaseFrameTransmitter");

    cache.encoder.klass = (jclass)((*env)->NewGlobalRef(env, localEncoderClass));
    (*env)->DeleteLocalRef(env, localEncoderClass);

    jclass localDecoderClass =
        (*env)->FindClass(env, "org/quietmodem/Quiet/BaseFrameReceiver");

    cache.decoder.klass = (jclass)((*env)->NewGlobalRef(env, localDecoderClass));
    (*env)->DeleteLocalRef(env, localDecoderClass);

    jclass localComplexClass =
        (*env)->FindClass(env, "org/quietmodem/Quiet/Complex");
    cache.complex.klass = (jclass)((*env)->NewGlobalRef(env, localComplexClass));
    (*env)->DeleteLocalRef(env, localComplexClass);

    jclass localFrameStatsClass =
        (*env)->FindClass(env, "org/quietmodem/Quiet/FrameStats");
    cache.frame_stats.klass = (jclass)((*env)->NewGlobalRef(env, localFrameStatsClass));
    (*env)->DeleteLocalRef(env, localFrameStatsClass);

    cache.system.ptr =
        (*env)->GetFieldID(env, cache.system.klass, "sys_ptr", "J");

    cache.encoder_profile.ptr =
        (*env)->GetFieldID(env, cache.encoder_profile.klass, "profile_ptr", "J");

    cache.encoder_profile.num_bufs =
        (*env)->GetFieldID(env, cache.encoder_profile.klass, "numBuffers", "J");

    cache.encoder_profile.buf_len =
        (*env)->GetFieldID(env, cache.encoder_profile.klass, "bufferLength", "J");

    cache.decoder_profile.ptr =
        (*env)->GetFieldID(env, cache.decoder_profile.klass, "profile_ptr", "J");

    cache.decoder_profile.num_bufs =
        (*env)->GetFieldID(env, cache.decoder_profile.klass, "numBuffers", "J");

    cache.decoder_profile.buf_len =
        (*env)->GetFieldID(env, cache.decoder_profile.klass, "bufferLength", "J");

    cache.encoder.ptr =
        (*env)->GetFieldID(env, cache.encoder.klass, "enc_ptr", "J");

    cache.decoder.ptr =
        (*env)->GetFieldID(env, cache.decoder.klass, "dec_ptr", "J");

    cache.complex.ctor =
        (*env)->GetMethodID(env, cache.complex.klass, "<init>", "(FF)V");

    cache.frame_stats.ctor =
        (*env)->GetMethodID(env, cache.frame_stats.klass, "<init>", "([Lorg/quietmodem/Quiet/Complex;FFZ)V");
		
    cache.input_stream.fd = (*env)->GetFieldID(env, cache.input_stream.klass, "fd", "I");

    cache.output_stream.fd = (*env)->GetFieldID(env, cache.output_stream.klass, "fd", "I");

    return JNI_VERSION_1_4;
}

JNIEXPORT jvm_pointer JNICALL
Java_org_quietmodem_Quiet_QuietSystem_nativeOpen(JNIEnv *env,
        jobject This) {
    return jvm_opaque_pointer(android_system_create(env));
}

JNIEXPORT void JNICALL
Java_org_quietmodem_Quiet_QuietSystem_nativeClose(JNIEnv *env, jobject This) {
    jvm_pointer j_sys = (*env)->GetLongField(env, This, cache.system.ptr);
    quiet_android_system *sys = (quiet_android_system *)recover_pointer(j_sys);
    android_system_destroy(sys);
}

JNIEXPORT void JNICALL
Java_org_quietmodem_Quiet_QuietSystem_nativeOpenOpenSL(JNIEnv *env,
                                                          jobject This) {
    jvm_pointer j_sys = (*env)->GetLongField(env, This, cache.system.ptr);
    quiet_android_system *sys = (quiet_android_system *)recover_pointer(j_sys);
    SLresult res = quiet_opensl_system_create(&sys->opensl_sys);

    if (res != SL_RESULT_SUCCESS) {
        quiet_opensl_system_destroy(sys->opensl_sys);
        throw_error(env, cache.system.init_exc_klass, opensl_engine_error_format, res);
        return;
    }
}
