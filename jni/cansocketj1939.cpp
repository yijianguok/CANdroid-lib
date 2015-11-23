#include<string>
#include<algorithm>
#include<utility>

#include<cstring>
#include<cstddef>
#include<cerrno>

extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/can/j1939.h>
}

#if defined(ANDROID) || defined(__ANDROID__)
#include "jni.h"
#else
#include "org_isoblue_can_CanSocketJ1939.h"
#endif

static const int ERRNO_BUFFER_LEN = 1024;

static void throwException(JNIEnv *env, const std::string& exception_name,
			   const std::string& msg)
{
	const jclass exception = env->FindClass(exception_name.c_str());
	if (exception == NULL) {
		return;
	}
	env->ThrowNew(exception, msg.c_str());
}

static void throwIOExceptionMsg(JNIEnv *env, const std::string& msg)
{
	throwException(env, "java/io/IOException", msg);
}

static void throwIOExceptionErrno(JNIEnv *env, const int exc_errno)
{
	char message[ERRNO_BUFFER_LEN];
	const char *const msg = (char *) strerror_r(exc_errno, message, ERRNO_BUFFER_LEN);
	if (((long)msg) == 0) {
		throwIOExceptionMsg(env, std::string(message));
	} else if (((long)msg) == -1) {
		snprintf(message, ERRNO_BUFFER_LEN, "errno %d", exc_errno);
		throwIOExceptionMsg(env, std::string(message));
	} else {
		throwIOExceptionMsg(env, std::string(msg));
	}
}

JNIEXPORT jint JNICALL Java_org_isoblue_can_CanSocketJ1939_mFetch
(JNIEnv *env, jclass obj, jstring param)
{
    	const char *str = env->GetStringUTFChars(param, NULL);
	if (strcmp(str, "CAN_J1939") == 0) {
		env->ReleaseStringUTFChars(param, str);
		return CAN_J1939;
	}
	else if (strcmp(str, "SOCK_DGRAM") == 0) {
		env->ReleaseStringUTFChars(param, str);
		return SOCK_DGRAM;
	}
	else if (strcmp(str, "SOL") == 0) {
		env->ReleaseStringUTFChars(param, str);
		return SOL_CAN_J1939;
	}
	else if (strcmp(str, "FILTER") == 0) {
		env->ReleaseStringUTFChars(param, str);
		return SO_J1939_FILTER;
	}
	else if (strcmp(str, "PROMISC") == 0) {
		env->ReleaseStringUTFChars(param, str);
		return SO_J1939_PROMISC;
	}
	else if (strcmp(str, "RECVOWN") == 0) {
		env->ReleaseStringUTFChars(param, str);
		return SO_J1939_RECV_OWN;
	}
	else if (strcmp(str, "PRIORITY") == 0) {
		env->ReleaseStringUTFChars(param, str);
		return SO_PRIORITY;
	}

	env->ReleaseStringUTFChars(param, str);
	return -1;
}

JNIEXPORT void JNICALL Java_org_isoblue_can_CanSocketJ1939_mSetJ1939filter
(JNIEnv *env, jclass obj, jint fd, jlongArray names, jintArray addrs, jintArray pgns)
{
	int i;
	// get the array length
	jsize len = env->GetArrayLength(names);	

	struct j1939_filter filt[] = {0};
	memset(&filt, 0, sizeof(filt));

	// fill up the filter(s)
	jlong *name = env->GetLongArrayElements(names, NULL);
	jint *addr = env->GetIntArrayElements(addrs, NULL);
	jint *pgn = env->GetIntArrayElements(pgns, NULL);
	for (i = 0; i < len; i++) {
		filt[i].name = name[i];
		filt[i].name_mask = ~0ULL;
		filt[i].addr = addr[i];
		filt[i].addr_mask = ~0;
		filt[i].pgn = pgn[i];
		filt[i].pgn = ~0;
	}
	env->ReleaseLongArrayElements(names, name, 0);	
	env->ReleaseIntArrayElements(addrs, addr, 0);	
	env->ReleaseIntArrayElements(pgns, pgn, 0);

	// apply filters to socket
	if (setsockopt(fd, SOL_CAN_J1939, SO_J1939_FILTER, &filt, sizeof(filt)) == -1) {
		throwIOExceptionErrno(env, errno);
	}
}
