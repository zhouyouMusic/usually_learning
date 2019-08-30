# -*- coding: utf-8 -*-
from ctypes import *
import time
import io

 
class Skegn:
	'评分引擎'
	def __init__(self):
#		self.engine = cdll.LoadLibrary('./skegn.dll')
#		self.engine = WinDLL('./skegn.dll')
		self.engine = WinDLL('./skegn_x86_64.dll')
#		self.engine = WinDLL('./skegn_i686.dll')
#		self.engine = cdll.LoadLibrary('./skegn_i686.dll')
	def skegn_new(self,cfg):
		return self.engine.skegn_new(cfg)
	
	def skegn_start(self, p_skegn, params, id, _callback):
		return self.engine.skegn_start(p_skegn, params, id, _callback, None)
	
	def skegn_feed(self,skegn,data,size):
		return self.engine.skegn_feed(skegn, data, size)
		
	def skegn_stop(self, skegn):
		return self.engine.skegn_stop(skegn)
	
	def skegn_delete(self,skegn):
		return self.engine.skegn_delete(skegn)
	
	def skegn_cancel(self,skegn):
		return self.engine.skegn_cancel(skegn)


def callback(usrdata, id, type, data, size):
    print (data); #输出结果
    return 0

#创建评分引擎
cfg='{"appKey":"1533182404000077","secretKey":"eec6a15cb27b89b27a877ec55a017b65","provision":"skegn.provision","cloud":{"enable":1,"connectTimeout":20,"serverTimeout":60,"server":"ws://192.168.0.12:8090","serverList":""}}';
#cfg={\"appKey\":\"1533182404000077\",\"secretKey\":\"eec6a15cb27b89b27a877ec55a017b65\",\"provision\":\"skegn.provision\",\"cloud\":{\"enable\":1,\"connectTimeout\":20,\"serverTimeout\":60,\"server\":\"ws://192.168.0.12:8090\",\"serverList\":\"\"}}";
print(cfg)
t = Skegn()
p_skegn= t.skegn_new(cfg)
print(p_skegn)

#开始
CharArray64 = c_char * 64;
id=CharArray64()
params="{\"coreProvideType\":\"cloud\",\"app\":{\"userId\":\"xxx\"},\"audio\":{\"audioType\":\"wav\",\"sampleRate\":16000,\"channel\":1,\"sampleBytes\":2},\"request\":{\"coreType\":\"sent.eval\",\"refText\":\"I konw the place very well\",\"phoneme_output\":0}}"
CMPFUNC = CFUNCTYPE(c_int, c_void_p, c_char_p, c_int, c_char_p, c_int)
_callback  = CMPFUNC(callback)
print (t.skegn_start(p_skegn, params, id, _callback))

#传入音频
fo = io.open("I.wav", 'rb')
fo.seek(44)
try:
	while True:
		data=fo.read(1024)
		if not data:
			break
		t.skegn_feed(p_skegn, data, len(data))
finally:
     fo.close( )

#停止
t.skegn_stop(p_skegn)

#等待结果
time.sleep(3)

#释放引擎
t.skegn_delete(p_skegn)
