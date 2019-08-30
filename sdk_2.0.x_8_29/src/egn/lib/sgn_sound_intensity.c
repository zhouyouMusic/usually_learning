/*
 * sgn_msg_queue.c
 *
 *  Created on: 2018年8月23日
 *      Author: weicong.liu
 */

float sgn_vad_sound_intensity(const void *data, int size)
{
    int i, vol;
    short tmp = 0, min = 0, max = 0;
    for (i = 0; i < size / 2; ++i) {
        tmp = ((short *)data)[i];
        max = tmp > max ? tmp : max;
        min = tmp < min ? tmp : min;
    }
    vol = 0.5 * (max - min) / 320;
    vol = vol > 100 ? 100 : vol;
    return vol;
}
