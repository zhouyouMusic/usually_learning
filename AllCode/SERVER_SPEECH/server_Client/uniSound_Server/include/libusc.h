#ifndef _LIBUSC_H_
#define _LIBUSC_H_


// ���������ʽ
#define AUDIO_FORMAT_PCM_16K         "pcm16k"
#define AUDIO_FORMAT_PCM_8K          "pcm8k"

// ʶ������
#define RECOGNITION_FIELD_GENERAL    "general"
#define RECOGNITION_FIELD_POI        "poi"
#define RECOGNITION_FIELD_SONG       "song"
#define RECOGNITION_FIELD_MEDICAL    "medical"
#define RECOGNITION_FIELD_MOVIETV    "movietv"
#define RECOGNITION_FIELD_FRIDGE	"fridge"

// ״̬����
#define USC_ENABLE					"true"
#define USC_DISABLED				"false"

#define LANGUAGE_ENGLISH             "english"
#define LANGUAGE_CANTONESE           "cantonese"
#define LANGUAGE_CHINESE             "chinese"

enum {
    // ʶ������
    USC_ASR_OK                       = 0,

    // �н������
    USC_RECOGNIZER_PARTIAL_RESULT    = 2,

    // ��⵽������ʼ
    USC_RECOGNIZER_SPEAK_BEGIN       = 100,

    // ��⵽��������
    USC_RECOGNIZER_SPEAK_END         = 101,

    // ʶ��������
	USC_ASR_NO_HANDLE_INPUT			 = -91138,

	// ����ID����
	USC_ASR_INVALID_ID				 = -91151,

	// ��������
	USC_ASR_INVALID_PARAMETERS		 = -91152,

	// �������ݸ�ʽ����
	USC_ASR_INVALID_INPUT_DATA		 = -91157,

};


enum {
    // ����ΪAPP_KEY
    USC_OPT_ASR_APP_KEY              = 9,

    // ����Ϊ�û�ID
    USC_OPT_ASRUSER_ID               = 14,

    // ѡ��ʶ������ 
    USC_OPT_RECOGNITION_FIELD        = 18,

	//����ѡ��
	USC_OPT_LANGUAGE_SELECT          = 20,   
	
	USC_OPT_ASR_ENGINE_PARAMETER	 = 104,

	// �����������
	USC_OPT_NLU_PARAMETER			 = 201,

	// ����Ϊ�û�secret
	USC_OPT_USER_SECRET				 = 204,

    // �������������ʽ
    USC_OPT_INPUT_AUDIO_FORMAT       = 1001,

	// ʶ�����ı����Ƿ�ʹ�ñ�����
	USC_OPT_PUNCTUATION_ENABLED      = 1002,

	// ��������֡�ֽڳ���
	USC_OPT_DECODE_FRAME_SIZE        = 1003,

	// �������ݹ���
	USC_OPT_NOISE_FILTER			 = 1004,
	
	USC_OPT_RESULT_FORMAT			 = 1006,

	USC_SERVICE_STATUS_SELECT		 = 1015,

};

// ����ʶ����
typedef long long USC_HANDLE;

#ifdef WIN32
	#ifdef LIBUSC_EXPORTS
		#ifdef JNI_EXPORTS
			#define USC_API
		#else
			#define USC_API extern "C" __declspec(dllexport)
		#endif
	#else
		#define USC_API extern "C" __declspec(dllimport)
	#endif
#else
//	#define USC_API extern "C" __attribute__ ((visibility("default")))
#endif


extern __attribute__ ((visibility("default"))) int usc_create_service(USC_HANDLE* handle);

extern __attribute__ ((visibility("default"))) int usc_create_service_ext(USC_HANDLE* handle, const char* host, const unsigned short port);

extern __attribute__ ((visibility("default"))) void usc_vad_set_timeout(USC_HANDLE handle, int frontSil,int backSil);

extern __attribute__ ((visibility("default"))) int usc_login_service(USC_HANDLE handle);

extern __attribute__ ((visibility("default"))) int usc_start_recognizer(USC_HANDLE handle);

extern __attribute__ ((visibility("default"))) int usc_stop_recognizer(USC_HANDLE handle);

extern __attribute__ ((visibility("default"))) int usc_feed_buffer(USC_HANDLE handle, const char* buffer, int len);

extern __attribute__ ((visibility("default"))) const char* usc_get_result(USC_HANDLE handle);

extern __attribute__ ((visibility("default"))) void usc_release_service(USC_HANDLE handle);

extern __attribute__ ((visibility("default"))) int usc_set_option(USC_HANDLE handle, int option_id, const char* value);

extern __attribute__ ((visibility("default"))) int usc_set_option_str(USC_HANDLE handle, const char* key, const char* value);

extern __attribute__ ((visibility("default"))) int usc_set_nlu_option_str(USC_HANDLE handle, const char* key, const char* value);

extern __attribute__ ((visibility("default"))) const char* usc_get_version();

extern __attribute__ ((visibility("default"))) int usc_get_result_begin_time(USC_HANDLE handle);

extern __attribute__ ((visibility("default"))) int usc_get_result_end_time(USC_HANDLE handle);

extern __attribute__ ((visibility("default"))) const char * usc_get_option(USC_HANDLE handle, int option_id);

extern __attribute__ ((visibility("default"))) void usc_clear_option(USC_HANDLE handle, int option_id);

extern __attribute__ ((visibility("default"))) int usc_cancel_recognizer(USC_HANDLE handle);


#endif

