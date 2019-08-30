#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "sgn_secure_code.h"

static unsigned long2str(unsigned* v, unsigned lv, unsigned w, char** buf) {
	unsigned n;
	unsigned m;
	unsigned i;
    n = (lv - 1) * 4;
	*buf = (char*)malloc(n + 4);
    if(w) {
        m = v[lv - 1];
        if ((m < n - 3) || (m > n)) {
			return 0;
		}
        n = m;
    }else{
		n = lv * 4;
	}
    for(i = 0; i < lv; i++) {
		*(((unsigned*)(*buf)) + i) = v[i];
    }
	return n;
}
static unsigned str2long(char* s, unsigned ls, unsigned w, unsigned** res) {
	unsigned add;
	char* buf;
	unsigned i;
	unsigned lr;
	if(w){
		w = 1;
	}else{
		w = 0;
	}
	add = (4 - ls % 4) & 3;
	if(add){
		buf = (char*)malloc(ls + add);
		memcpy(buf, s, ls);
		memset(buf + ls, 0, add);
	}else{
		buf = s;
	}
	lr = (ls + add)/4 + w;
	*res = (unsigned*)malloc(lr * sizeof(unsigned));
	for(i = 0; i < lr - w; i++){
		(*res)[i] = *(((unsigned*)buf) + i);
	}
    if(w) {
        (*res)[lr - 1] = ls;
    }
	if(add){
		free(buf);
	}
    return lr;
}

static unsigned _encode(char* str, unsigned lstr, char* key, unsigned lkey, char** res){
	unsigned* v;
	unsigned lv;
	unsigned* k;
	unsigned lk;
	unsigned lres;
	unsigned* tmp;
	unsigned i;
	unsigned n;
	unsigned z;
	unsigned y;
	volatile unsigned delta, delta2;
	unsigned q;
	unsigned sum;
	unsigned e;
	unsigned p;
	unsigned mx;
    if (lstr == 0 ) {
        return 0;
    }
    lv = str2long(str, lstr, 1, &v);
    lk = str2long(key, lkey, 0, &k);
    delta2 = 0x27226599;
    if(lk < 4) {
		tmp = (unsigned*)malloc(4 * sizeof(unsigned));
        for(i = 0; i < lk; i++) {
            tmp[i] = k[i];
        }
        for(i = lk; i < 4; i++) {
            tmp[i] = 0;
        }
		free(k);
		k = tmp;
    }
    n = lv - 1;
    z = v[n];
    y = v[0];
    delta = 0x4FF2AE87 + (delta2<<1);   /* 隐藏特征常数 */
    q = 6 + 52 / (n + 1);
    sum = 0;
    while(0 < q--) {
        sum = sum + delta;
        e = (sum >> 2) & 3;
        for(p = 0; p < n; p++) {
            y = v[p + 1];
            mx = (
				((z >> 5 & 0x07ffffff) ^ y << 2)
				+ ((y >> 3 & 0x1fffffff) ^ z << 4)
				)
				^ ((sum ^ y) + ( k[(p & 3) ^ e]	^ z	)
				);
            z = v[p] = v[p] + mx;
        }
        y = v[0];
        mx = (
			((z >> 5 & 0x07ffffff) ^ y << 2)
			+ ((y >> 3 & 0x1fffffff) ^ z << 4)
			) ^ ((sum ^ y) + (k[(p & 3) ^ e] ^ z));
        z = v[n] = (v[n] + mx);
    }
    lres = long2str(v, lv, 0, res);
	free(v);
	free(k);
	return lres;
}
static unsigned _decode(char* str, unsigned lstr, char* key, unsigned lkey, char** res){
	unsigned* v;
	unsigned lv;
	unsigned* k;
	unsigned lk;
	unsigned lres;
	unsigned* tmp;
	unsigned i;
	unsigned n;
	unsigned z;
	unsigned y;
	volatile unsigned delta, delta2;
	unsigned q;
	unsigned sum;
	unsigned e;
	unsigned p;
	unsigned mx;
    if (lstr == 0 ) {
        return 0;
    }
    lv = str2long(str, lstr, 0, &v);
    lk = str2long(key, lkey, 0, &k);
    delta2 = 0x27226599;
    if(lk < 4) {
		tmp = (unsigned*)malloc(4 * sizeof(unsigned));
        for(i = 0; i < lk; i++) {
            tmp[i] = k[i];
        }
        for(i = lk; i < 4; i++) {
            tmp[i] = 0;
        }
		free(k);
		k = tmp;
    }

    n = lv - 1;

    z = v[n];
    y = v[0];
    delta = 0x4FF2AE87 + (delta2<<1);   /* 隐藏特征常数 */
    q = 6 + 52 / (n + 1);
    sum = q * delta;
    while(sum != 0) {
        e = sum >> 2 & 3;
        for(p = n; p > 0; p--) {
            z = v[p - 1];
            mx = (((z >> 5 & 0x07ffffff) ^ y << 2)
				+ ((y >> 3 & 0x1fffffff) ^ z << 4))
				^ ((sum ^ y) + (k[(p & 3) ^ e] ^ z));
            y = v[p] = (v[p] - mx);
        }
        z = v[n];
        mx = (((z >> 5 & 0x07ffffff) ^ y << 2)
			+ ((y >> 3 & 0x1fffffff) ^ z << 4))
			^ ((sum ^ y) + (k[(p & 3) ^ e] ^ z));
        y = v[0] = v[0] - mx;
        sum = sum - delta;
    }
    lres = long2str(v, lv, 1, res);
	free(v);
	free(k);
	return lres;
}

unsigned sgn_secure_code(char* str, unsigned lstr, char* key, unsigned lkey, char** res, char opt)
{
    if('e' == opt)
    {
        return _encode(str, lstr, key, lkey, res);
    }
    else if('d' == opt)
    {
        return _decode(str, lstr, key, lkey, res);
    }
    else
        return 0;
}
