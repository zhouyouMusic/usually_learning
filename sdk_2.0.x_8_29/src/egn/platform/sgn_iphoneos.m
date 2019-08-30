#ifdef __IPHONE_OS__

#include "sgn_iphoneos.h"
#include "sgn_common.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#import "iOS/BPXLUUIDHandler/SGNBPXLUUIDHandler.h"
#import "iOS/Reachability/SGNReachability.h"
#import <UIKit/UIDevice.h>
#import <Foundation/Foundation.h>

int
sgn_get_device_id(char device_id[64])
{
    NSString *uuid = [SGNBPXLUUIDHandler UUID];
    memcpy(device_id, [uuid UTF8String], [uuid length]);
    *(device_id + [uuid length]) = '\0';
    return 0;
}

int sgn_is_wifi_work(void) {
    return ([[SGNReachability reachabilityForLocalWiFi] currentReachabilityStatus] != NotReachable)? 1 : 0;
}

int sgn_select_product(char ret_product[64], const char arg[64])
{
    if(!strcmp(arg, "iPhone6,1") || !strcmp(arg, "iPhone6,2"))
    {
        strcpy(ret_product, "iPhone 5s");    
    }
    else if(!strcmp(arg, "iPhone5,1") || !strcmp(arg, "iPhone5,2"))
    {
        strcpy(ret_product, "iPhone 5");
    }
    else if(!strcmp(arg, "iPhone7,1"))
    {
        strcpy(ret_product, "iPhone 6 Plus");   
    }
    else if(!strcmp(arg, "iPhone7,2"))
    {
        strcpy(ret_product, "iPhone 6");    
    }
    else if(!strcmp(arg, "iPhone5,3") || !strcmp(arg, "iPhone5,4"))
    {
        strcpy(ret_product, "iPhone 5c");    
    }
    else if(!strcmp(arg, "iPad4,1") || !strcmp(arg, "iPad4,2") || !strcmp(arg, "iPad4,3"))
    {
        strcpy(ret_product, "iPad Air");    
    }
    else if(!strcmp(arg, "iPad4,4") || !strcmp(arg, "iPad4,5") || !strcmp(arg, "iPad4,6"))
    {
        strcpy(ret_product, "iPad mini 2");    
    }
    else if(!strcmp(arg, "iPad2,5") || !strcmp(arg, "iPad2,6") || !strcmp(arg, "iPad2,7"))
    {
        strcpy(ret_product, "iPad mini 1");    
    }
    else if(!strcmp(arg, "iPad3,4") || !strcmp(arg, "iPad3,5") || !strcmp(arg, "iPad3,6"))
    {
        strcpy(ret_product, "iPad 4");    
    }
    else if(!strcmp(arg, "iPhone4,1"))
    {
        strcpy(ret_product, "iPhone 4S");    
    }
    else if(!strcmp(arg, "iPad3,1") || !strcmp(arg, "iPad3,2") || !strcmp(arg, "iPad3,3"))
    {
        strcpy(ret_product, "iPad 3");    
    }
    else if(!strcmp(arg, "iPad2,1") || !strcmp(arg, "iPad2,2") || !strcmp(arg, "iPad2,3") || !strcmp(arg, "iPad2,4"))
    {
        strcpy(ret_product, "iPad 2");    
    }
    else if(!strcmp(arg, "iPhone3,1") || !strcmp(arg, "iPhone3,2") || !strcmp(arg, "iPhone3,3"))
    {
        strcpy(ret_product, "iPhone 4");
    }
    else if(!strncmp(arg, "iPod", 4))
    {
        strcpy(ret_product, "iPod touch");
    }
    else
    {
        strcpy(ret_product, "unknown_Apple_product");
    }
    
    return 0;
}



int sgn_select_arch(char ret_arch[64], const char arg[64])
{
    if(!strcmp(arg, "iPhone 5s") || !strcmp(arg, "iPhone 6") || !strcmp(arg, "iPhone 6 Plus") || !strcmp(arg, "iPad mini 2")|| !strcmp(arg, "iPad Air"))    
    {
        strcpy(ret_arch, "arm64");
    }
    else if(!strcmp(arg, "iPhone 5") || !strcmp(arg, "iPhone 5c") || !strcmp(arg, "iPad 4"))
    {
        strcpy(ret_arch, "arm7s");
    }
    else
    {
        strcpy(ret_arch, "arm7");
    }
    return 0;
}

int sgn_get_system_info(sgn_system_info_t *info)
{
    
    info->version = sgn_get_full_version();
    info->source = SOURCE;
    info->protocol = PROTOCOL;
    
    NSString *tmp = [[UIDevice currentDevice] systemName];
    
    memcpy(info->os, [tmp UTF8String], [tmp length]);
    
    tmp = [[UIDevice currentDevice] systemVersion];
    memcpy(info->os_version, [tmp UTF8String], [tmp length]);
    
/*     char platform[64] = {0};
    sgn_get_arch(platform);
    sgn_select_product(info->product,  platform);
    sgn_select_arch(info->arch, info->product); */
	
	sgn_get_arch(info->arch);
	strcpy(info->product, info->arch);
    
    return 0;
}

int sgn_get_app_path(char path[1024])
{
    NSString *_path = [NSHomeDirectory() stringByAppendingPathComponent:@"/Documents/"];
    memcpy(path, [_path UTF8String], [_path length]);
    memcpy(path+[_path length], "/", 1);
    return 0;
}

#endif
