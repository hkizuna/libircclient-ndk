LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libircclient-prebuilt
LOCAL_SRC_FILES := libircclient/src/libircclient.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/libircclient/include

include $(BUILD_STATIC_LIBRARY)

#####################

include $(CLEAR_VARS)

LOCAL_MODULE := ircclient
LOCAL_SRC_FILES := IRCClient.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/libircclient/include
LOCAL_STATIC_LIBRARIES := libircclient-prebuilt

include $(BUILD_SHARED_LIBRARY)
