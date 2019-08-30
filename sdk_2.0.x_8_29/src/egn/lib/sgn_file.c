#include "sgn_file.h"

int sgn_is_file_exist(const char *filepath)
{
    int ret = 0;

    FILE *fp = fopen(filepath, "r");
    ret = (NULL != fp) ? 1 : 0;
    if (NULL != fp)
    {
        fclose(fp);
    }

    return ret;
}

#ifdef __WIN32__
void sgn_utf8_to_ansi(const char* utf8, char *buf)
{
    int         wlen, alen;
    LPWSTR  pszw = 0;

    if(utf8 == NULL)
    {
        buf[0] = 0;
        return;
    }

    wlen = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, NULL);
    pszw = (LPWSTR)malloc((wlen+1) * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, pszw, wlen);
    pszw[wlen] = 0;

    alen = WideCharToMultiByte(CP_ACP, 0, pszw, -1, NULL, 0, NULL, NULL);
    buf[0] = 0;
    WideCharToMultiByte(CP_ACP, 0, pszw, -1, buf, alen, NULL, NULL);
    if(alen < PATH_MAX)
    {
        buf[alen] = 0;
    }
    else
    {
        buf[PATH_MAX-1] = 0;
    }

    free(pszw);
}
#endif
