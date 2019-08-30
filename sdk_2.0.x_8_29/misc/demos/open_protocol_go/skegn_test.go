package skegn

import (
	"testing"
	"log"
)

func TestNewSkegn(t *testing.T) {
	cfg := `{"appKey": "17KouyuTestAppKey","secretKey": "17KouyuTestSecretKey","server": "api.17kouyu.com:8080"}`
	engine ,err := NewSkegn(cfg)
	if err !=nil{
		log.Panicf("NewSkegn:",err)
	}
	param := `{"app":{"userId":"test-userId"},"request":{"getParam":1,"coreType":"sent.eval","refText":"how are you","dict_type":"KK","phoneme_output":1,"refAudio":"","tokenId":""},"audio":{"audioType":"amr","sampleRate":8000,"channel":1,"sampleBytes":2}}`
	err = engine.Skegn_start(param)
	if err !=nil{
		log.Panicf("Skegn_start:",err)
	}
	filepath := "temp_audio.amr"
	err = engine.Skegn_feed_file(filepath)
	if err !=nil{
		log.Panicf("Skegn_feed_file:",err)
	}
	result ,err := engine.Skegn_stop()
	if err !=nil{
		log.Panicf("Skegn_stop:",err)
	}
	log.Printf("ret:%s",result)
	err = engine.Skegn_delete()
	if err !=nil{
		log.Panicf("Skegn_stop:",err)
	}
}

func BenchmarkNewSkegn(b *testing.B) {
	for i:=0 ; i<b.N ; i++ {
		cfg := `{"appKey": "17KouyuTestAppKey","secretKey": "17KouyuTestSecretKey","server": "api.17kouyu.com:8080"}`
		engine ,err := NewSkegn(cfg)
		if err !=nil{
			log.Panicf("NewSkegn:",err)
		}
		param := `{"app":{"userId":"test-userId"},"request":{"getParam":1,"coreType":"sent.eval","refText":"how are you","dict_type":"KK","phoneme_output":1,"refAudio":"","tokenId":""},"audio":{"audioType":"amr","sampleRate":8000,"channel":1,"sampleBytes":2}}`
		err = engine.Skegn_start(param)
		if err !=nil{
			log.Panicf("Skegn_start:",err)
		}
		filepath := "temp_audio.amr"
		err = engine.Skegn_feed_file(filepath)
		if err !=nil{
			log.Panicf("Skegn_feed_file:",err)
		}
		_ ,err = engine.Skegn_stop()
		if err !=nil{
			log.Panicf("Skegn_stop:",err)
		}
		//log.Printf("ret:%s",result)
		err = engine.Skegn_delete()
		if err !=nil{
			log.Panicf("Skegn_stop:",err)
		}
	}

}