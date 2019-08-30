#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>

#ifdef _WIN32
#include <winsock2.h>
#endif

static uint32_t
_bigendian(uint32_t i)
{
    uint16_t c = 0xaabb;
    return c >> 8 == 0xbb ? i :
        (i << 24 & 0xff000000)
        | (i << 8 & 0x00ff0000)
        | (i >> 8  & 0x0000ff00)
        | (i >> 24 & 0x000000ff);
}

/*
uuid consisting of a 4-byte timestamp (seconds since epoch), a 3-byte machine id, a 2-byte process id, and a 3-byte counter, time, pid, inc is big endian

 time machine pid inc
 0-3  4-6     7-8 9-11

 reference: http://www.mongodb.org/display/DOCS/Optimizing+Object+IDs
            https://github.com/ashun/mongo-c-driver/blob/master/src/bson.c
*/
void
uuidgen(char uuid[12])
{
    static uint32_t inc = 0;
    char hostname[256], *p;

    uint32_t _time;
    uint32_t _pid;
    uint32_t _inc;
    uint32_t _hostname;

    inc = (inc + 1) % 16777216;

    gethostname(hostname, sizeof(hostname));
    for (_hostname = 0, p = hostname; *p; p++) {
        _hostname = 31 * _hostname + *p; /* BKDR hash */
    }

    _time     = _bigendian((uint32_t)time(NULL));
    _pid      = _bigendian((uint32_t)getpid());
    _inc      = _bigendian((uint32_t)inc);
    _hostname = _bigendian((uint32_t)_hostname);

    memcpy(uuid,     (char *)&_time, 4);
    memcpy(uuid + 4, (char *)&_hostname + 1, 3);
    memcpy(uuid + 7, (char *)&_pid + 2, 2);
    memcpy(uuid + 9, (char *)&_inc + 1, 3);
}


void uuid_from_string(char hexstr[24], char uuid[12])
{
    int i;
    for (i=0; i<12; i++) {
        uuid[i] = hexstr[2*i]<<4 | hexstr[2*i+1];
    }
}


void uuid_to_string(char uuid[12], char hexstr[24])
{
    static char ch[] = "0123456789abcdef";
    int i = 0;
    for (i = 0; i < 12; i++) {
        hexstr[2*i]   = ch[uuid[i] >> 4 & 0x0f];
        hexstr[2*i+1] = ch[uuid[i] & 0x0f];
    }
}


time_t
uuid_get_time(char uuid[12])
{
    return (time_t)_bigendian(*(uint32_t *)uuid);
}


void uuidgen2(char hexstr[24])
{
    char uuid[12];
    uuidgen(uuid);
    uuid_to_string(uuid, hexstr);
}
