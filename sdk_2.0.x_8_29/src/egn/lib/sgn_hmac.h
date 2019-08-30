#ifdef USE_NATIVE
/*******************************************************************************
* Copyright (C),  
* 
* FileName    : sgn_hmac.h
* Author      :  
*  
* Description : hmac加密算法接口 
*               算法介绍：http://baike.baidu.com/view/1136366.htm
*               rfc: http://www.ietf.org/rfc/rfc2104.txt
*------------------------------------------------------------------------------*
* Record      : 
*******************************************************************************/
#ifndef SGN_HMAC_H_
#define SGN_HMAC_H_


#ifdef __cplusplus
extern "C" {
#endif

/* out的内存大小必须大于等于40个字节 */
void sgn_hmac(char *key, int k_len, char *txt, int t_len, char *out);

#ifdef __cplusplus
}
#endif
#endif

#endif //#if !(defined DISABLE_PROVISION) && defined USE_NATIVE
