#include "bluetooth_uart.h"
#include "core/config.h"
#include "core/display.h"
#include "core/netserver.h"
#include "core/telnet.h"
#include <string.h>
#include <ctype.h>

static void trim(char *s)
{
    // trim leading
    char *p = s;
    while (*p && isspace((unsigned char)*p))
        p++;
    if (p != s)
        memmove(s, p, strlen(p) + 1);
    // trim trailing
    int l = strlen(s);
    while (l > 0 && isspace((unsigned char)s[l - 1]))
    {
        s[l - 1] = '\0';
        l--;
    }
}

void bluetooth_handle_line(const char *line)
{
    if (!line)
        return;
    if (strncmp(line, "BT:", 3) != 0)
        return;
    char cmd[32] = {0};
    char value[256] = {0};
    const char *p = line + 3;
    const char *colon = strchr(p, ':');
    if (!colon)
    {
        strncpy(cmd, p, sizeof(cmd) - 1);
        trim(cmd);
    }
    else
    {
        size_t cmdlen = colon - p;
        if (cmdlen >= sizeof(cmd))
            cmdlen = sizeof(cmd) - 1;
        strncpy(cmd, p, cmdlen);
        cmd[cmdlen] = '\0';
        strncpy(value, colon + 1, sizeof(value) - 1);
        trim(value);
        trim(cmd);
    }

    if (strcmp(cmd, "CONNECTED") == 0)
    {
        if (btMetaMutex)
            xSemaphoreTake(btMetaMutex, pdMS_TO_TICKS(100));
        btMeta.connected = true;
        memset(btMeta.artist, 0, sizeof(btMeta.artist));
        memset(btMeta.title, 0, sizeof(btMeta.title));
        if (btMetaMutex)
            xSemaphoreGive(btMetaMutex);
        Serial.println("BT: Connected set to true");
        if (config.getMode() == PM_BLUETOOTH)
            display.putRequest(NEWTITLE);
        return;
    }
    if (strcmp(cmd, "DISCONNECTED") == 0)
    {
        if (btMetaMutex)
            xSemaphoreTake(btMetaMutex, pdMS_TO_TICKS(100));
        btMeta.connected = false;
        memset(btMeta.deviceName, 0, sizeof(btMeta.deviceName));
        memset(btMeta.deviceMAC, 0, sizeof(btMeta.deviceMAC));
        memset(btMeta.artist, 0, sizeof(btMeta.artist));
        memset(btMeta.title, 0, sizeof(btMeta.title));
        if (btMetaMutex)
            xSemaphoreGive(btMetaMutex);
        if (config.getMode() == PM_BLUETOOTH)
            display.putRequest(NEWTITLE);
        return;
    }
    if (strcmp(cmd, "NAME") == 0)
    {
        if (btMetaMutex)
            xSemaphoreTake(btMetaMutex, pdMS_TO_TICKS(100));
        if (!btMeta.connected)
        {
            btMeta.connected = true;
            Serial.println("BT: Connected set to true from NAME");
        }
        strlcpy(btMeta.deviceName, value, sizeof(btMeta.deviceName));
        if (btMetaMutex)
            xSemaphoreGive(btMetaMutex);
        if (config.getMode() == PM_BLUETOOTH)
            display.putRequest(NEWTITLE);
        return;
    }
    if (strcmp(cmd, "MAC") == 0)
    {
        if (btMetaMutex)
            xSemaphoreTake(btMetaMutex, pdMS_TO_TICKS(100));
        strlcpy(btMeta.deviceMAC, value, sizeof(btMeta.deviceMAC));
        if (btMetaMutex)
            xSemaphoreGive(btMetaMutex);
        return;
    }
    if (strcmp(cmd, "ARTIST") == 0)
    {
        if (btMetaMutex)
            xSemaphoreTake(btMetaMutex, pdMS_TO_TICKS(100));
        if (!btMeta.connected)
        {
            btMeta.connected = true;
            Serial.println("BT: Connected set to true from ARTIST");
        }
        strlcpy(btMeta.artist, value, sizeof(btMeta.artist));
        if (btMetaMutex)
            xSemaphoreGive(btMetaMutex);
        if (config.getMode() == PM_BLUETOOTH)
            display.putRequest(NEWTITLE);
        return;
    }
    if (strcmp(cmd, "TITLE") == 0)
    {
        if (btMetaMutex)
            xSemaphoreTake(btMetaMutex, pdMS_TO_TICKS(100));
        if (!btMeta.connected)
        {
            btMeta.connected = true;
            Serial.println("BT: Connected set to true from TITLE");
        }
        strlcpy(btMeta.title, value, sizeof(btMeta.title));
        char localArtist[sizeof(btMeta.artist)];
        strlcpy(localArtist, btMeta.artist, sizeof(localArtist));
        if (btMetaMutex)
            xSemaphoreGive(btMetaMutex);
        if (config.getMode() == PM_BLUETOOTH)
        {
            display.putRequest(NEWTITLE);
            if (strlen(localArtist) > 0 && strlen(btMeta.title) > 0)
            {
                char meta[256];
                snprintf(meta, sizeof(meta), "%s - %s", localArtist, btMeta.title);
                strlcpy(config.station.title, meta, sizeof(config.station.title));
                netserver.requestOnChange(TITLE, 0);
                telnet.printf("##CLI.META#: %s\r\n", meta);
            }
        }
        return;
    }
    if (strcmp(cmd, "PLAYING") == 0)
    {
        if (btMetaMutex)
            xSemaphoreTake(btMetaMutex, pdMS_TO_TICKS(100));
        btMeta.playing = true;
        if (btMetaMutex)
            xSemaphoreGive(btMetaMutex);
        return;
    }
    if (strcmp(cmd, "STOPPED") == 0)
    {
        if (btMetaMutex)
            xSemaphoreTake(btMetaMutex, pdMS_TO_TICKS(100));
        btMeta.playing = false;
        if (btMetaMutex)
            xSemaphoreGive(btMetaMutex);
        if (config.getMode() == PM_BLUETOOTH)
        {
            memset(config.station.title, 0, sizeof(config.station.title));
            netserver.requestOnChange(TITLE, 0);
            telnet.printf("##CLI.META#: \r\n");
        }
        return;
    }
}
