#ifdef _SPEECH_RECONGNITION_H_
#define _SPEECH_RECONGNITION_H_

void accessID_token(char *apiKey,char *secretKey,char **tokenBuff);
void speech_recongnize(char *cuid,char *accessId,int rate,int pcmLength,char *pcmBuff,char **speechBuf);

#endif
