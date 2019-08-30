#ifdef __WIN32__

#include <stdio.h>
#include <string.h>

#include <windows.h>
#include <winioctl.h>

/* #include <ddk/ntddndis.h> */
/* it's defined in WDK/inc/netcfgx.h, http://msdn.microsoft.com/en-us/library/windows/hardware/ff547832(v=vs.85).aspx */
#undef  NCF_VIRTUAL
#define NCF_VIRTUAL 0x01
#undef  NCF_SOFTWARE_ENUMERATED
#define NCF_SOFTWARE_ENUMERATED 0x02
#undef  NCF_PHYSICAL
#define NCF_PHYSICAL 0x04

/* it's defined in vc's ntddndis.h, https://research.microsoft.com/en-us/um/redmond/projects/invisible/src/drivers/net/simnic/NTDDNDIS.H.htm */
#define IOCTL_NDIS_QUERY_GLOBAL_STATS  CTL_CODE(FILE_DEVICE_PHYSICAL_NETCARD, 0, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#undef  OID_802_3_PERMANENT_ADDRESS 
#define OID_802_3_PERMANENT_ADDRESS       0x01010101


typedef long LSTATUS;


static int
_get_pnp_instance_id(const char *netcfg_instance_id, char pnp_instance_id[MAX_PATH])
{

    char key_name[MAX_PATH];
    DWORD key_type = REG_SZ;
    DWORD value_len = MAX_PATH;
    LRESULT rv;

    HKEY network_adapter_key = NULL;

    sprintf(key_name, "SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\%s\\Connection", netcfg_instance_id);
    rv = RegOpenKeyExA(HKEY_LOCAL_MACHINE, key_name, 0, KEY_READ, &network_adapter_key);
    if (rv != ERROR_SUCCESS) {
        goto end;
    }

    rv = RegQueryValueExA(network_adapter_key, "PnpInstanceID", 0, &key_type, (BYTE *)pnp_instance_id, &value_len);

end:
    if (network_adapter_key) {
        RegCloseKey(network_adapter_key);
    }

    return rv == ERROR_SUCCESS ? 0 : -1;
}


static int
_get_permanent_mac(const char *netcfg_instance_id, char mac[12])
{
    int rv = -1;
    BOOL b;
    DWORD oid;

    char buf[16];
    char *p, pnp_instance_id[MAX_PATH], device_path[MAX_PATH];
    HANDLE	device_handle = INVALID_HANDLE_VALUE;

    unsigned char value[6];
    DWORD value_len = sizeof(value);

    if (_get_pnp_instance_id(netcfg_instance_id, pnp_instance_id) != 0) {
        goto end;
    }

    sprintf(device_path, "\\\\.\\%s#{ad498944-762f-11d0-8dcb-00c04fc3358c}", pnp_instance_id);
    for (p = device_path + 4; *p; p++) {
        if (*p == '\\') {
            *p = '#';
        }
    }

    device_handle = CreateFileA(device_path, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if(device_handle == INVALID_HANDLE_VALUE) {
        goto end;
    }

    oid = OID_802_3_PERMANENT_ADDRESS;
    b = DeviceIoControl(device_handle, IOCTL_NDIS_QUERY_GLOBAL_STATS, &oid, sizeof(oid), value, sizeof(value), &value_len, NULL);
    if (!b || value_len != 6) {
        goto end;
    }

    sprintf(buf, "%02x%02x%02x%02x%02x%02x", value[0], value[1], value[2], value[3], value[4], value[5]);
    if (strstr(buf, "000000") || strstr(buf, "ffffff")) {
        goto end;
    }
    sprintf(mac, "%s", buf);

#ifdef DEBUG_DEVICE_ID
    printf(" - %s", mac);
#endif

    rv = 0;

end:
    if (device_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(device_handle);
    }

    return rv;
}


/*
 * get the physical ethernet adapters' permanent mac address
 * return 0 on success, otherwise -1 on failed, multiple mac was seperated by ','
 */
int
agn_win32_getmac(char mac[256])
{
    int c = 0;
    LSTATUS rv;
    HKEY class_key = NULL;
    HKEY class_adapter_key = NULL;

    char key_name[256];
    DWORD key_name_len = sizeof(key_name) - 1;
    DWORD key_type = REG_DWORD;

    char value[256];
    DWORD value_len = sizeof(value) - 1;

    int i = 0;

    rv = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002bE10318}", 0, KEY_READ, &class_key);
    if (rv != ERROR_SUCCESS) {
        goto end;
    }

    /* enumerate all network adapters */
    for (;;i++) {

        key_name_len = sizeof(key_name) - 1;
        rv = RegEnumKeyExA(class_key, i, key_name, &key_name_len, NULL, NULL, NULL, NULL);
        if (rv == ERROR_NO_MORE_ITEMS) {
            break;
        }

#ifdef DEBUG_DEVICE_ID
        printf("\n#%d - %s", i, key_name);
#endif
        if (rv != ERROR_SUCCESS) {
            continue;
        }

        rv = RegOpenKeyExA(class_key, key_name, 0, KEY_READ, &class_adapter_key);
        if (rv != ERROR_SUCCESS) {
            continue;
        }
        key_type = REG_DWORD;
        value_len = sizeof(value) - 1;
        rv = RegQueryValueExA(class_adapter_key, "Characteristics", 0, &key_type, (BYTE *)value, &value_len);

        if (rv == ERROR_SUCCESS) {
            if ((*((DWORD *)value) & NCF_PHYSICAL) == NCF_PHYSICAL) { /* if it's physical adapter */
#ifdef DEBUG_DEVICE_ID
                printf(" - NCF_PHYSICAL");
#endif
                key_type = REG_SZ;
                value_len = sizeof(value) - 1;
                rv = RegQueryValueExA(class_adapter_key, "NetCfgInstanceId", 0, &key_type, (BYTE *)value, &value_len);
                if (_get_permanent_mac(value, mac) == 0) {
                    *(mac + 12) = ',';
                    *(mac + 13) = '\0';
                    mac += 13;
                    c++;
                }
            }
#ifdef DEBUG_DEVICE_ID
            else if ((*((DWORD *)value) & NCF_VIRTUAL) == NCF_VIRTUAL) { /* virtual adapter */
                printf(" - NCF_VIRTUAL");
            } else if ((*((DWORD *)value) & NCF_SOFTWARE_ENUMERATED) == NCF_SOFTWARE_ENUMERATED) { /* enumerated adapter */
                printf(" - NCF_SOFTWARE_ENUMERATED");
            }
#endif
        }

        RegCloseKey(class_adapter_key);
        if (c >= 16) {
            break;
        }
    }

    RegCloseKey(class_key);

end:
    if (c > 0) {
        *(mac - 1) = '\0';
    }

    return c == 0 ? -1 : 0;
}

/*
rv = _wmic("wmic nicconfig where ipenabled=true get macaddress", udidinfo->mac, 255);
if (rv) {
    sprintf(udidinfo->mac, "N/A");
}
*/
/*
static int
agn_win32_getmac(char *output, int size)
{
    int rv;
    char buf[4096];

    output[0] = '\0';
    rv = _system("getmac /nh /fo table", buf, sizeof(buf) - 1);
    if (!rv) {
        int is_first_value, is_blank_line;
        char *p, *line_start, *line_end, *value_start, *value_end, *buf_end;

        is_first_value = 1;
        for (line_start = buf, buf_end = buf + strlen(buf) - 1;
                line_start <= buf_end;
                line_start = line_end + 2) {

            is_blank_line = 1;
            line_end = strchr(line_start, '\n');
            line_end = line_end ? --line_end : buf_end;

            p = strchr(line_start, '-');
            is_blank_line = !p || p == line_start || p >= line_end;

            if (!is_blank_line) {
                value_start = line_start;
                value_end = strchr(value_start, '\t');
                value_end = value_end ? value_end : strchr(value_start, ' ');
                if (!value_end) {
                    continue;
                }

                --value_end;

                if ((size - strlen(output)) < (value_end - value_start + 2)) {
                    break;
                }

                if (is_first_value) {
                    is_first_value = 0;
                } else {
                    strcat(output, ",");
                }
                p = output + strlen(output);
                strncat(p, value_start, value_end - value_start + 1);
                _strnorm(p);

            }
        }
    }

    return 0;
}
*/

#endif
