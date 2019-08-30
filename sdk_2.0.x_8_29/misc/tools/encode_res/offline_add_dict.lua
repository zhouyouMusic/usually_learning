--[[
readme
功能:按约定格式增加离线词库到离线资源
使用:
    需要修改序号变量:num
    输入文件格式,单词必须全是小写:
        pupae/pjuːpiː/
        compound/kɔmpaʊnd/
        milkweed/mɪlkwiːd/
    输出: 3个文件redw_words.txt\redw_align_lexicon.int\AH.variation
]]

local ipa882CMU = {
    ["ʌ"] = "AH1",
    ["e"] = "EH",
    ["ɪ"] = "IH",
    ["ʊ"] = "UH",
    ["ə"] = "AH",
    ["æ"] = "AE",
    ["p"] = "P",
    ["b"] = "B",
    ["t"] = "T",
    ["d"] = "D",
    ["k"] = "K",
    ["g"] = "G",
    ["f"] = "F",
    ["v"] = "V",
    ["θ"] = "TH",
    ["ð"] = "DH",
    ["s"] = "S",
    ["z"] = "Z",
    ["ʃ"] = "SH",
    ["ʒ"] = "ZH",
    ["h"] = "HH",
    ["m"] = "M",
    ["n"] = "N",
    ["ŋ"] = "NG",
    ["l"] = "L",
    ["r"] = "R",
    ["w"] = "W",
    ["j"] = "Y"
}

local ipa882CMU2 = {
    ["ɔː"] = "AO",
    ["ɑː"] = "AA",
    ["iː"] = "IY",
    ["uː"] = "UW",
    ["ɜː"] = "ER",
    ["ər"] = "AH R",
    ["eɪ"] = "EY",
    ["aɪ"] = "AY",
    ["oʊ"] = "OW",
    ["aʊ"] = "AW",
    ["ɔɪ"] = "OY",
    ["tʃ"] = "CH",
    ["dʒ"] = "JH"
}

local kk2CMU = {
    ["ɒ"] = "AO",
    ["ɔ"] = "AO",
    ["ɑ"] = "AA",
    ["i"] = "IY",
    ["u"] = "UW",
    ["ɛ"] = "EH",
    ["ɪ"] = "IH",
    ["ʊ"] = "UH",
    ["ə"] = "AH",
    ["æ"] = "AE",
    ["e"] = "EY",
    ["o"] = "OW",
    ["p"] = "P",
    ["b"] = "B",
    ["t"] = "T",
    ["d"] = "D",
    ["k"] = "K",
    ["g"] = "G",
    ["f"] = "F",
    ["v"] = "V",
    ["θ"] = "TH",
    ["ð"] = "DH",
    ["s"] = "S",
    ["z"] = "Z",
    ["ʃ"] = "SH",
    ["ʒ"] = "ZH",
    ["h"] = "HH",
    ["m"] = "M",
    ["n"] = "N",
    ["ŋ"] = "NG",
    ["l"] = "L",
    ["r"] = "R",
    ["ɝ"] = "ER",
    ["ɚ"] = "AH R",
    ["w"] = "W",
    ["j"] = "Y"
}

local kk2CMU2 = {
    ["tʃ"] = "CH",
    ["dʒ"] = "JH",
    ["aʊ"] = "AW",
    ["ɔɪ"] = "OY",
    ["aɪ"] = "AY"
}

local CMU2number = {
    ["F"] = "21",
    ["G"] = "22",
    ["K"] = "27",
    ["L"] = "28",
    ["M"] = "29",
    ["N"] = "30",
    ["B"] = "14",
    ["P"] = "34",
    ["R"] = "35",
    ["S"] = "36",
    ["D"] = "16",
    ["T"] = "38",
    ["V"] = "42",
    ["W"] = "43",
    ["Y"] = "44",
    ["Z"] = "45"
}

local CMU2number2 = {
    ["SIL"] = "1",
    ["BRH"] = "2",
    ["CGH"] = "3",
    ["NSN"] = "4",
    ["SMK"] = "5",
    ["UM"] = "6",
    ["UHH"] = "7",
    ["AA"] = "8",
    ["AE"] = "9",
    ["AH"] = "10",
    ["AO"] = "11",
    ["AW"] = "12",
    ["AY"] = "13",
    ["ZH"] = "46",
    ["TH"] = "39",
    ["UH"] = "40",
    ["UW"] = "41",
    ["SH"] = "37",
    ["HH"] = "23",
    ["IH"] = "24",
    ["IY"] = "25",
    ["JH"] = "26",
    ["CH"] = "15",
    ["DH"] = "17",
    ["EH"] = "18",
    ["ER"] = "19",
    ["EY"] = "20",
    ["NG"] = "31",
    ["OW"] = "32",
    ["OY"] = "33"
}

local num = 41824
local redw_words = io.open("redw_words.txt", "w+")
local align = io.open("redw_align_lexicon.int", "w")
local AH = io.open("AH.variation", "w")
local listfile = io.open("wordlist.txt")

local phone= ""
local CMUphonetic = ""
local numphonetic = ""
local ah = ""
local i,j = ""
local word = ""
local phonetic = ""
local linedata = listfile:read("*l")

while linedata ~= nil do

    i,j = string.find(linedata, "/")
    word = string.sub(linedata, 1, i-1)
    phonetic = string.sub(linedata, i+1, -3)

    --写入单词序号
    redw_words:write(word)
    redw_words:write(" ")
    redw_words:write(num)
    redw_words:write("\n")

    --写入音标代号
        --获取CMU音素表
    CMUphonetic = phonetic
    for k,v in pairs(ipa882CMU2) do
        CMUphonetic = string.gsub(CMUphonetic, k, v .. " ")
    end
    for k,v in pairs(kk2CMU2) do
        CMUphonetic = string.gsub(CMUphonetic, k, v .. " ")
    end
    for k,v in pairs(ipa882CMU) do
        CMUphonetic = string.gsub(CMUphonetic, k, v .. " ")
    end
    for k,v in pairs(kk2CMU) do
        CMUphonetic = string.gsub(CMUphonetic, k, v .. " ")
    end
    print(CMUphonetic)
        --获取数字音素表
    numphonetic = CMUphonetic
    numphonetic = string.gsub(numphonetic, "AH1", "10")
    numphonetic = string.gsub(numphonetic, "AH1", "10")
    for k,v in pairs(CMU2number2) do
        numphonetic = string.gsub(numphonetic, k, v)
    end
    for k,v in pairs(CMU2number) do
        numphonetic = string.gsub(numphonetic, k, v)
    end
    numphonetic = string.sub(numphonetic, 1, -2)
    print(numphonetic)
    align:write(num)
    align:write(" ")
    align:write(num)
    align:write(" ")
    align:write(numphonetic)
    align:write("\n")

    --标识AH0和AH1
    i,j = string.find(CMUphonetic, "AH")
    if i then
        ah = string.gsub(numphonetic, " ", "_")
        ah = ah .. " "
        while true do
            if string.sub(CMUphonetic, i, j+1) == "AH1" then 
                ah = ah .. "1"
            else
                ah = ah .. "0"
            end
            ah = ah .. " "
            i,j = string.find(CMUphonetic, "AH", j)
            if i == nil then
                ah = string.sub(ah, 1, -2)
                break
            end
        end
        AH:write(num)
        AH:write("_")
        AH:write(ah)
        AH:write("\n")
    end

    linedata = listfile:read("*l")
    num = num + 1
end

redw_words:close()
align:close()
AH:close()
listfile:close()