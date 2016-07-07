#include "IRCClient.h"
#include <libircclient.h>
#include <string.h>

irc_session_t *session;
irc_callbacks_t callbacks;

JavaVM *g_jvm;

jstring
get_j_string(JNIEnv *env, const char *cstr) {
  return (*env)->NewStringUTF(env, cstr);
}

const char *
get_c_string(JNIEnv *env, jstring jstr) {
  return (*env)->GetStringUTFChars(env, jstr, 0);
}

void
free_c_string(JNIEnv *env, jstring jstr, const char *cstr) {
  (*env)->ReleaseStringUTFChars(env, jstr, cstr);
}

void
get_jni_env(JNIEnv **env) {
  (*g_jvm)->GetEnv(g_jvm, (void **)env, JNI_VERSION_1_6); 
}

jobject
get_irc_client_instance(JNIEnv *env, jclass cls) {
  jmethodID mid = (*env)->GetStaticMethodID(env, cls, "getInstance", "()LIRCClient;");
  return (*env)->CallStaticObjectMethod(env, cls, mid);
}

void
event_connect(irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count) {
  JNIEnv *env;
  get_jni_env(&env);
  jclass cls = (*env)->FindClass(env, "IRCClient");
  jobject obj = get_irc_client_instance(env, cls);
  jmethodID mid = (*env)->GetMethodID(env, cls, "onConnect", "()V");
  (*env)->CallVoidMethod(env, obj, mid);
}

void
event_channel(irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count) {
  JNIEnv *env;
  get_jni_env(&env);
  jclass cls = (*env)->FindClass(env, "IRCClient");
  jobject obj = get_irc_client_instance(env, cls);
  jmethodID mid = (*env)->GetMethodID(env, cls, "onChannel", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
  jstring person = get_j_string(env, origin);
  jstring channel = get_j_string(env, params[0]);
  jstring message = get_j_string(env, params[1]);
  (*env)->CallVoidMethod(env, obj, mid, person, channel, message);
}

void
Java_IRCClient_initialize(JNIEnv *env, jobject obj) {
  if (!session) {
    memset(&callbacks, 0, sizeof(callbacks));

    callbacks.event_connect = event_connect;
    callbacks.event_channel = event_channel;
    session = irc_create_session(&callbacks);
  }
  
  if(!g_jvm) {
    (*env)->GetJavaVM(env, &g_jvm);
  }
}

jint
Java_IRCClient_connect(JNIEnv *env, jobject obj, jstring server, jint port, jstring password, jstring nickname, jstring username, jstring realname) {
  const char *cc_server = get_c_string(env, server);
  unsigned short us_port = (unsigned short) port;
  const char *cc_password = (*env)->GetStringLength(env, password) > 0 ? get_c_string(env, password) : NULL;
  const char *cc_nickname = get_c_string(env, nickname);
  const char *cc_username = get_c_string(env, username);
  const char *cc_realname = get_c_string(env, realname);

  if (irc_connect(session, cc_server, us_port, cc_password, cc_nickname, cc_username, cc_realname)) {
    free_c_string(env, server, cc_server);
    free_c_string(env, password, cc_password);
    free_c_string(env, nickname, cc_nickname);
    free_c_string(env, username, cc_username);
    free_c_string(env, realname, cc_realname);
    return irc_errno(session);
  }
  if (irc_run(session)) {
    free_c_string(env, server, cc_server);
    free_c_string(env, password, cc_password);
    free_c_string(env, nickname, cc_nickname);
    free_c_string(env, username, cc_username);
    free_c_string(env, realname, cc_realname);
    return irc_errno(session);
  }
  free_c_string(env, server, cc_server);
  free_c_string(env, password, cc_password);
  free_c_string(env, nickname, cc_nickname);
  free_c_string(env, username, cc_username);
  free_c_string(env, realname, cc_realname);
  return 0;
}

void
Java_IRCClient_disconnect(JNIEnv *env, jobject obj) {
  if (session) {
    irc_destroy_session(session);
  }
}

jint
Java_IRCClient_join(JNIEnv *env, jobject obj, jstring channel) {
  const char *cc_channel = get_c_string(env, channel);
  if (irc_cmd_join(session, cc_channel, 0)) {
    free_c_string(env, channel, cc_channel);
    return irc_errno(session); 
  }
  free_c_string(env, channel, cc_channel);
  return 0;
}

jint
Java_IRCClient_message(JNIEnv *env, jobject obj, jstring channel, jstring content) {
  const char *cc_channel = get_c_string(env, channel);
  const char *cc_content = get_c_string(env, content);
  if (irc_cmd_msg(session, cc_channel, cc_content)) {
    free_c_string(env, channel, cc_channel);
    free_c_string(env, content, cc_content);
    return irc_errno(session);
  }
  free_c_string(env, channel, cc_channel);
  free_c_string(env, content, cc_content);
  return 0;
}
