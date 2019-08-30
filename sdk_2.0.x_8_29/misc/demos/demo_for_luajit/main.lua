local ffi = require("ffi")
local skegn = ffi.load("skegn")
local bit = require("bit")

ffi.cdef[[
struct skegn;

typedef int ( *skegn_callback)(const void *usrdata, const char *id, int type, const void *message, int size);
struct skegn * skegn_new(const char *cfg);
int skegn_delete(struct skegn *engine);
int skegn_start(struct skegn *engine, const char *param, char id[64], skegn_callback callback, const void *usrdata);
int skegn_feed(struct skegn *engine, const void *data, int size);
int skegn_stop(struct skegn *engine);
int skegn_get_device_id(char device_id[64]);
int skegn_cancel(struct skegn *engine);
int skegn_opt(struct skegn *engine, int opt, char *data, int size);

]]

local function callback(userdata, id, type, message, size)
      print(ffi.string(message, size))
      print(ffi.string(id, 64))
--      coroutine.yield()
      return 0
end

local cb = ffi.cast("skegn_callback", callback)

local function loop(egn)

    local params = "{\"coreProvideType\":\"cloud\",\"app\":{\"userId\":\"xxx\"},\"audio\":{\"audioType\":\"mp3\",\"sampleRate\":44100,\"channel\":1,\"sampleBytes\":2},\"request\":{\"attachAudioUrl\": 1,\"coreType\":\"en.sent.score\",\"refText\":\"Hello, I'm Mike.\",\"phoneme_output\":0}}"
    local id = ffi.new("uint8_t[?]", 64)
    local rt = skegn.skegn_start(egn, params, id, cb, nil)
    local s_id = ffi.string(id, 64)
    print(s_id)
    local audio = io.open("3_1_1_1_3.mp3", "rb")
    --audio:seek("set", 44)
    local audiodata = audio:read("*a")
    audio:close()
    print(#audiodata)
    rt = skegn.skegn_feed(egn, audiodata, #audiodata)
    rt = skegn.skegn_stop(egn)
    os.execute("sleep " .. tonumber(10))
    --rt = skegn.skegn_delete(egn)

end


local function eval()
    local cfg = "{\"appKey\": \"17KouyuTestAppKey\",\"secretKey\": \"17KouyuTestSecretKey\",\"provision\": \"skegn.provision\",\"cloud\": {\"server\": \"ws://gray.17kouyu.com:8090\",\"serverList\": \"\"} }"
    local egn = skegn.skegn_new(cfg)
    local cor = coroutine.create(loop)
    print(coroutine.resume(cor, egn))
    skegn.skegn_delete(egn)
end


eval()
--
--local function main1()
--  print("a\n")
--  coroutine.yield()
--  print("b\n")
--  coroutine.yield()
--  print("c\n")
--  coroutine.yield()
--end
--
--local function main2()
--  print("a1\n")
--  coroutine.yield()
--  print("b2\n")
--  coroutine.yield()
--  print("c3\n")
--  coroutine.yield()
--end
--
--
--local function test_coroutine()
--  local co1 = coroutine.create(main1)
--  local co2 = coroutine.create(main2)
--  coroutine.resume(co1)
--  coroutine.resume(co2)
--  coroutine.resume(co1)
--  coroutine.resume(co2)
--end


--test_coroutine()
