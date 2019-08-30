package com.tt;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.Charset;

/**
 * TODO package *.so and *.dll in a fat jar
 * 
 * @author sz
 * 
 */
public final class SkEgn {

    static {
        System.load("E:/work/demo_for_java/skegn.dll");
    }
    
    private static final String provisionPath = "E:/work/demo_for_java/skegn.provision";
    
    public interface skegn_callback {
        public abstract int run(byte[] id, int type, byte[] data, int size);
    }
    public static int SKEGN_MESSAGE_TYPE_JSON = 1;
    public static int SKEGN_MESSAGE_TYPE_BIN = 2;
    
    public static native long skegn_new(String cfg, Object androidContext);
    public static native int  skegn_delete(long engine);
    public static native int  skegn_start(long engine, String param, byte[] id,  skegn_callback callback, Object context);
    public static native int  skegn_feed(long engine, byte[] data, int size);
    public static native int  skegn_stop(long engine);
    public static native int  skegn_get_device_id(byte[] device_id, Object androidContext);
    public static native int  skegn_cancel(long engine);
    public static native int  skegn_log(long engine, String log);
    public static native int skegn_opt(long engine, int opt, byte[] data, int size);

	/* 回调函数，处理返回结果 */
    private static skegn_callback callback = new skegn_callback() {
        public int run(byte[] id, int type, byte[] data, int size) {
            String recordId = new String(id, Charset.forName("UTF-8")).trim(); // must trim the end '\0'
            System.out.println("recordId: " + recordId);
            if (type == SKEGN_MESSAGE_TYPE_JSON) {
                System.out.println("result: " + new String(data, 0, size, Charset.forName("UTF-8")).trim()); // must trim the end '\0'
            }
            return 0;
        }
    };

    public static void main(String[] args) throws IOException {

    	int rv;
    	int bytes;
        byte[] buf = new byte[1024];
        byte[] id = new byte[64];
        long engine;

        FileInputStream fis;
		/* 创建评分引擎 */
    	engine = skegn_new("{\"appKey\": \"17KouyuTestAppKey\", \"secretKey\": \"17KouyuTestSecretKey\", \"provision\":\"" + provisionPath + "\", \"cloud\": {\"enable\": 1, \"server\": \"ws://gray.17kouyu.com:8090\", \"serverList\": \"\", \"connectTimeout\": 20, \"serverTimeout\": 180}}", null);
    	
		System.out.println("after new\n");
    	if (engine == 0) {
    		System.out.println("create new engine failed");
    		return;
    	}
    	
    	/*  开始评分引擎  */
    	rv = skegn_start(engine, "{\"coreProvideType\": \"cloud\", \"app\": {\"userId\": \"user-id\"}, \"audio\": {\"audioType\": \"wav\", \"channel\": 1, \"sampleBytes\": 2, \"sampleRate\": 16000}, \"request\": {\"coreType\": \"sent.eval\", \"refText\": \"I know the place very well\"}}", id, callback, null);
		System.out.println("after start\n");
		
    	if (rv == 0) {
            fis = new FileInputStream("I.wav");
            while ((bytes = fis.read(buf, 0, 1024)) > 0) {
				/* 传入音频到引擎 */
                if ((rv = skegn_feed(engine, buf, bytes)) != 0) {
                    break;
                }
                System.out.println("after feed\n");
            }
            fis.close();
        }
		/*  结束本次评分  */
    	rv = skegn_stop(engine);
    	System.out.println("after stop\n");

        try {
            Thread.sleep(5000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
		/* 释放引擎 */
    	skegn_delete(engine);
    	System.out.println("after delete\n");
    }
}
