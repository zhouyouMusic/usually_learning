import hashlib
import hmac
import base64

message = bytes("GET\n/v3/iot/management/device/LIGHT_REAL/accessDetail\nhost:iotdm.gz.baidubce.com").encode('utf-8')
secret = bytes("1shMAEpi+C/HrcMBtowprrtvGuaiVrKSfE6b5JPBnyU=").encode('utf-8')

signature = base64.b64encode(hmac.new(secret, message, digestmod=hashlib.sha256).digest())
print(signature)
