/**
 ****************************************************************************************************
 * @file        app_web.c
 * @author      Autumn
 * @version     V1.0
 * @date        2026-06-17
 * @brief       Web控制页面 + HTTP请求处理
 ****************************************************************************************************
 * @attention
 *
 * 控制4个窗帘: 0x0201, 0x0202, 0x0203, 0x0204
 * API: /c1/open, /c1/close, /c2/open, /c2/close, ...
 *
 ****************************************************************************************************
 */

#include "app_web.h"
#include "bsp_esp01s.h"
#include "bsp_curtain.h"
#include "bsp_oled.h"
#include <stdio.h>
#include <string.h>

/******************************************************************************************/
/* WiFi配置 */

#define WIFI_SSID       "Xiaomi15 By Autumn"
#define WIFI_PASS       "........"
#define WEB_PORT        80

/******************************************************************************************/
/* 窗帘地址表 */

static const uint8_t curtain_addr_h[4] = { 0x02, 0x02, 0x02, 0x02 };
static const uint8_t curtain_addr_l[4] = { 0x01, 0x02, 0x03, 0x04 };

/******************************************************************************************/
/* HTML页面 (存储在Flash中) */

static const char html_page[] =
"<!DOCTYPE html><html><head>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
"<title>Curtain Control</title>"
"<style>"
"*{box-sizing:border-box;margin:0;padding:0}"
"body{font-family:Arial,sans-serif;background:#f0f2f5;padding:16px}"
"h1{text-align:center;color:#333;margin-bottom:16px;font-size:22px}"
".g{display:grid;grid-template-columns:1fr 1fr;gap:12px;max-width:400px;margin:0 auto}"
".c{background:#fff;border-radius:12px;padding:16px;box-shadow:0 2px 8px rgba(0,0,0,.1)}"
".c h3{font-size:14px;color:#666;margin-bottom:12px;text-align:center}"
".b{display:block;width:100%;padding:14px;margin:6px 0;border:none;border-radius:8px;"
"font-size:16px;font-weight:bold;color:#fff;cursor:pointer;transition:opacity .2s}"
".b:active{opacity:.7}"
".o{background:#4CAF50}"
".x{background:#f44336}"
"#m{text-align:center;margin-top:16px;color:#666;font-size:14px}"
"</style></head><body>"
"<h1>Curtain Control</h1>"
"<div class=\"g\">"
"<div class=\"c\"><h3>Curtain 1 (0201)</h3>"
"<button class=\"b o\" onclick=\"c('/c1/open')\">OPEN</button>"
"<button class=\"b x\" onclick=\"c('/c1/close')\">CLOSE</button></div>"
"<div class=\"c\"><h3>Curtain 2 (0202)</h3>"
"<button class=\"b o\" onclick=\"c('/c2/open')\">OPEN</button>"
"<button class=\"b x\" onclick=\"c('/c2/close')\">CLOSE</button></div>"
"<div class=\"c\"><h3>Curtain 3 (0203)</h3>"
"<button class=\"b o\" onclick=\"c('/c3/open')\">OPEN</button>"
"<button class=\"b x\" onclick=\"c('/c3/close')\">CLOSE</button></div>"
"<div class=\"c\"><h3>Curtain 4 (0204)</h3>"
"<button class=\"b o\" onclick=\"c('/c4/open')\">OPEN</button>"
"<button class=\"b x\" onclick=\"c('/c4/close')\">CLOSE</button></div>"
"</div>"
"<div id=\"m\"></div>"
"<script>"
"function c(p){fetch(p).then(function(r){return r.text()}).then("
"function(t){document.getElementById('m').textContent=t})}"
"</script></body></html>";

static const char http_200_header[] =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html\r\n"
"Connection: close\r\n\r\n";

static const char http_200_text[] =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/plain\r\n"
"Connection: close\r\n\r\n";

static const char http_404[] =
"HTTP/1.1 404 Not Found\r\n"
"Content-Type: text/plain\r\n"
"Connection: close\r\n\r\nNot Found";

/******************************************************************************************/
/* 状态 */

static uint8_t s_web_initialized = 0;
static uint8_t s_wifi_connected = 0;

/******************************************************************************************/
/* HTTP请求处理回调 */

static void on_http_request(uint8_t link_id, const char *method, const char *path)
{
    char msg[32];

    printf("[WEB] %d %s %s\r\n", link_id, method, path);

    /* 解析窗帘控制命令 */
    if (strncmp(path, "/c", 2) == 0 && strlen(path) >= 4)
    {
        uint8_t idx = path[2] - '1';  /* 0~3 */
        const char *action = path + 4;

        if (idx < 4)
        {
            /* 设置目标窗帘地址 */
            g_curtain_use_custom = 1;
            g_curtain_custom_addr_h = curtain_addr_h[idx];
            g_curtain_custom_addr_l = curtain_addr_l[idx];

            if (strcmp(action, "/open") == 0)
            {
                bsp_curtain_open();
                snprintf(msg, sizeof(msg), "C%d OPEN sent", idx + 1);
                printf("[WEB] -> Curtain %d OPEN (0x%02X%02X)\r\n",
                       idx + 1, curtain_addr_h[idx], curtain_addr_l[idx]);
            }
            else if (strcmp(action, "/close") == 0)
            {
                bsp_curtain_close();
                snprintf(msg, sizeof(msg), "C%d CLOSE sent", idx + 1);
                printf("[WEB] -> Curtain %d CLOSE (0x%02X%02X)\r\n",
                       idx + 1, curtain_addr_h[idx], curtain_addr_l[idx]);
            }
            else if (strcmp(action, "/stop") == 0)
            {
                bsp_curtain_stop();
                snprintf(msg, sizeof(msg), "C%d STOP sent", idx + 1);
            }
            else
            {
                snprintf(msg, sizeof(msg), "Unknown action");
            }

            bsp_esp01s_send_response(link_id, http_200_text, msg);
            return;
        }
    }

    /* 首页 */
    if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0)
    {
        bsp_esp01s_send_response(link_id, http_200_header, html_page);
        return;
    }

    /* 404 */
    bsp_esp01s_send_response(link_id, http_404, NULL);
}

/******************************************************************************************/
/* 公开接口 */

/**
 * @brief       Web控制初始化
 */
void app_web_init(void)
{
    uint8_t i;

    /* 在OLED上显示初始化状态 */
    oled_clear();
    oled_show_string(0, 0, "WiFi Init...", 12);
    oled_refresh_gram();

    printf("[WEB] Initializing ESP01S...\r\n");
    bsp_esp01s_init();

    /* 连接WiFi */
    printf("[WEB] Connecting to %s...\r\n", WIFI_SSID);
    oled_clear();
    oled_show_string(0, 0, "Connecting WiFi", 12);
    oled_show_string(0, 14, WIFI_SSID, 12);
    oled_refresh_gram();

    for (i = 0; i < 3; i++)
    {
        if (bsp_esp01s_connect_wifi(WIFI_SSID, WIFI_PASS))
        {
            s_wifi_connected = 1;
            printf("[WEB] WiFi connected!\r\n");
            break;
        }
        printf("[WEB] WiFi retry %d/3...\r\n", i + 1);
    }

    if (!s_wifi_connected)
    {
        printf("[WEB] WiFi failed!\r\n");
        oled_clear();
        oled_show_string(0, 24, "WiFi Failed!", 12);
        oled_refresh_gram();
        return;
    }

    /* 启动Web服务器 */
    oled_clear();
    oled_show_string(0, 0, "Starting server", 12);
    oled_refresh_gram();

    if (bsp_esp01s_start_server(WEB_PORT))
    {
        s_web_initialized = 1;
        printf("[WEB] Server started on port %d\r\n", WEB_PORT);

        /* 注册HTTP回调 */
        bsp_esp01s_set_http_callback(on_http_request);

        /* 显示成功信息 */
        oled_clear();
        oled_show_string(0, 0, "WiFi OK!", 12);
        oled_show_string(0, 14, "Web Server ON", 12);
        oled_show_string(0, 28, "Port: 80", 12);
        oled_show_string(0, 42, "4 Curtains Ready", 12);
        oled_refresh_gram();
    }
    else
    {
        printf("[WEB] Server failed!\r\n");
        oled_clear();
        oled_show_string(0, 24, "Server Failed!", 12);
        oled_refresh_gram();
    }
}

/**
 * @brief       Web服务轮询 (主循环调用)
 */
void app_web_process(void)
{
    if (s_web_initialized)
    {
        bsp_esp01s_poll();
    }
}
