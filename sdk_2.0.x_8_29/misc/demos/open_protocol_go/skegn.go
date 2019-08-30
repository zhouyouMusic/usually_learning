/**

 */
package skegn

import (
	"github.com/bitly/go-simplejson"
	"github.com/gorilla/websocket"
	"crypto/sha1"
	"encoding/hex"
	"time"
	"fmt"
	"net/url"
	"errors"
	"os"
	"io"
)

type Skegn struct {
	appKey string
	secretKey string
	server string
	client *websocket.Conn
	coreType string
	isConnect bool
	canFeed bool
}

const skegn_sdk_version  = 16777472

// 创建引擎实例
func NewSkegn(cfg string) (*Skegn, error) {
	s := new(Skegn)

	js_cfg , err := simplejson.NewJson([]byte(cfg))
	if err!=nil {
		return nil ,err
	}

	//init some params
	appKey := js_cfg.Get("appKey").MustString()
	if len(appKey) == 0 {
		return nil ,errors.New("auth failed, no appKey ")
	}
	s.appKey = appKey

	secretKey := js_cfg.Get("secretKey").MustString()
	if len(secretKey) == 0 {
		return nil ,errors.New("auth failed, no secretKey ")
	}
	s.secretKey = secretKey

	server := js_cfg.Get("server").MustString()
	if len(server) == 0 {
		return nil ,errors.New("no server ")
	}
	s.server = server

	//init some status
	s.canFeed = false
	s.isConnect = false

	return s ,nil

}

//连接评分服务器
func (engine *Skegn) connectServer(coreType string) (error) {

	//switch or connect core connection
	//if core type is not change ,eq previous one
	if engine.coreType == coreType && engine.isConnect{
		return nil
	}

	//set coreType
	engine.coreType = coreType

	js , _ := simplejson.NewJson([]byte(`{"cmd":"connect","param":{"sdk":{"version":16777472,"source":4,"protocol":1},"app":{"applicationId":"AA","sig":"BB","timestamp":0}}}`))

	//microsecond time
	timestamp := fmt.Sprintf("%d",time.Now().UnixNano()/1000000)

	js.Get("param").Get("sdk").Set("version",skegn_sdk_version)
	js.Get("param").Get("app").Set("applicationId",engine.appKey)

	//create sig format ：appKey+timestamp+secretKey
	sig := signParams(fmt.Sprintf("%s%s%s", engine.appKey, timestamp,engine.secretKey))
	js.Get("param").Get("app").Set("sig",sig)
	js.Get("param").Get("app").Set("timestamp",timestamp)

	//connect to server
	u := url.URL{Scheme: "ws", Host: engine.server, Path: "/"+ engine.coreType,RawQuery:"e=0&t=0"}
	c, _, err := websocket.DefaultDialer.Dial(u.String(), nil)
	if err != nil {
		return err
	}
	engine.client = c

	//convert to json string
	s ,err := js.MarshalJSON()
	if err != nil {
		return err
	}

	//send connect cmd
	err =c.WriteMessage(websocket.TextMessage,s)
	if err != nil {
		return err
	}

	//change connect status
	engine.isConnect = true
	return nil
}

//断开连接
func (engine *Skegn) Skegn_delete() error  {
	engine.isConnect = false
	engine.canFeed = false
	return engine.client.Close()
}

//传入评分参数
func (engine *Skegn) Skegn_start(param string) (error) {
	//convert to json object
	js_param , err := simplejson.NewJson([]byte(param))
	if err!=nil {
		return err
	}
	//get core type for connect different server
	coreType := js_param.Get("request").Get("coreType").MustString()
	if len(coreType) == 0 {
		return errors.New("no coreType")
	}

	//connect to server and send connect cmd
	err = engine.connectServer(coreType)
	if err != nil {
		return err
	}

	//user id need random or trues, not recommend const val
	userId := js_param.Get("app").Get("userId").MustString()

	//microsecond time
	timestamp := fmt.Sprintf("%d",time.Now().UnixNano()/1000000)
	js ,err := simplejson.NewJson([]byte(`{"cmd":"start","param":{"app":{"applicationId":"","sig":"","userId":"","timestamp":0},"audio":"","request":""}}`))

	js.Get("param").Get("app").Set("applicationId",engine.appKey)
	js.Get("param").Get("app").Set("timestamp",timestamp)
	js.Get("param").Get("app").Set("userId",userId)

	//create sig format ：appKey+timestamp+userId+secretKey
	js.Get("param").Get("app").Set( "sig", signParams(fmt.Sprintf("%s%s%s%s", engine.appKey, timestamp,userId,engine.secretKey)))

	//audio and request see document
	js.Get("param").Set("audio",js_param.Get("audio"))
	js.Get("param").Set("request",js_param.Get("request"))

	//to json string
	s ,err := js.MarshalJSON()
	if err != nil {
		return err
	}

	//send start cmd
	err =engine.client.WriteMessage(websocket.TextMessage,s)
	if err != nil {
		return err
	}

	//must started after feed audio
	engine.canFeed = true
	return nil
}

//feed audio
func (engine *Skegn) Skegn_feed(buf []byte) error  {
	if engine.canFeed == false {
		return errors.New("must started")
	}
	//opcode is  BinaryMessage
	return engine.client.WriteMessage(websocket.BinaryMessage,buf)
}

//feed file
func (engine *Skegn) Skegn_feed_file(filepath string) error  {
	if engine.canFeed == false {
		return errors.New("must started")
	}
	f ,err:= os.OpenFile(filepath,os.O_RDONLY,0666)
	if err != nil {
		return err
	}
	//1024 byte size buf
	buf := make([]byte, 1024)
	for {
		n, err := f.Read(buf)
		//not eof err
		if err != nil && err != io.EOF{
			return err
		}
		//read to end
		if n == 0 {
			break
		}
		err = engine.client.WriteMessage(websocket.BinaryMessage,buf[:n])
		if err != nil {
			return err
		}
	}
	return nil
}

//stop feed and get result
func (engine *Skegn) Skegn_stop() (string , error) {
	//can't feed audio anymore when stop
	engine.canFeed = false
	err := engine.client.WriteMessage(websocket.BinaryMessage,make([]byte, 0))
	if err != nil {
		return "", err
	}
	//receive score result
	_, message, err := engine.client.ReadMessage()
	if err != nil {
		return "", err
	}
	return string(message) ,nil
}

//sign params with sha1
func signParams(s string) string {
	h := sha1.New()
	h.Write([]byte(s))
	return hex.EncodeToString(h.Sum(nil))
}
