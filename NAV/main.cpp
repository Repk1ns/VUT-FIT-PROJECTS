#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <esp_wifi.h>
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <esp_http_server.h>
#include "cJSON.h"
#include "esp_netif.h"
#include <sys/param.h>

#include "lwip/err.h"
#include "lwip/sys.h"

#define TFT_DC 2
#define TFT_CS 5
#define TFT_MOSI 23
#define TFT_CLK 18
#define TFT_RST 4

#define UPPER_LINE_PIXEL_VALUE 35
#define END_OF_STRING "        "
#define WIDTH_BY_CHARS 10

#define ESP_AP_WIFI_SSID "NAV-Project"
#define ESP_AP_WIFI_PASS ""
#define ESP_AP_WIFI_CHANNEL 6
#define ESP_AP_WIFI_MAX_STA_CONN 3

static const char *TAG = "NAV";

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

String text = "Zpravu zadejte na 192.168.4.1";

float p = 3.1415926;

static void wifiAccessPointEventHandle(void* arg, esp_event_base_t eventBase, int32_t eventID, void* eventData)
{
    if (eventID == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) eventData;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
    } else if (eventID == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) eventData;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}


void wifiAccessPointInit(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiAccessPointEventHandle, NULL, NULL));

    wifi_config_t wifiConfig = {};
    strcpy((char *)wifiConfig.ap.ssid, ESP_AP_WIFI_SSID);
    wifiConfig.ap.ssid_len = strlen(ESP_AP_WIFI_SSID);
    wifiConfig.ap.channel = ESP_AP_WIFI_CHANNEL;
    strcpy((char *)wifiConfig.ap.password, ESP_AP_WIFI_PASS);
    wifiConfig.ap.max_connection = ESP_AP_WIFI_MAX_STA_CONN;
    wifiConfig.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    if (strlen(ESP_AP_WIFI_PASS) == 0) {
        wifiConfig.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(
        TAG,
        "Initialization of Wi-Fi Access Point was finished - SSID:%s Password:%s Channel:%d",
        ESP_AP_WIFI_SSID,
        ESP_AP_WIFI_PASS,
        ESP_AP_WIFI_CHANNEL
    );
}


void drawText(String text, uint16_t color, int size, int x, int y) {
      tft.setCursor(x, y);
      tft.setTextColor(color);
      tft.setTextSize(size);
      tft.setTextWrap(false);
      tft.print(text);
}

void showSlidedText(String text)
{
      text = text + END_OF_STRING;
      const int width = tft.width();

      for (int offset = 0; offset < text.length(); offset++)
      {     
            
            String t = "";
            tft.fillRect(0, tft.height()/2-10, tft.width(), tft.height()/2, ST77XX_BLACK);
            for (int i = 0; i < width; i++)
            {
                  t += text.charAt((offset + i) % text.length());
            }
            
            drawText(t, ST77XX_YELLOW, 2, 0, tft.height()/2);

            delay(300);
      }
}


void drawBackground()
{
    tft.fillScreen(ST77XX_BLACK);
    drawText("NAV", ST77XX_BLUE, 2, tft.width()/2 - 15, 5);
    drawText("Project", ST7735_CYAN, 1, tft.width()/3 + 3, 25);

    tft.drawLine(0, UPPER_LINE_PIXEL_VALUE, tft.width(), UPPER_LINE_PIXEL_VALUE, ST77XX_WHITE);
    drawText("Zprava:", ST77XX_RED, 1, 5, 40);
    tft.drawLine(0, tft.height()-10, tft.width(), tft.height()-10, ST77XX_WHITE);
}


void introAnimation()
{
    bool tmp = false;
    tft.fillScreen(ST77XX_BLACK);
    for (int i = 0; i < tft.height(); i++) {
        if (i == tft.height()/2) {
            tmp = true;
        }
        if (tmp == true) {
            tft.drawLine(0, i, tft.width(), i, ST77XX_BLACK);
            tft.drawLine(0, tft.height()-i, tft.width(), tft.height()-i, ST77XX_BLACK);
        } else {
            tft.drawLine(0, i, tft.width(), i, ST77XX_CYAN);
            tft.drawLine(0, tft.height()-i, tft.width(), tft.height()-i, ST77XX_BLUE);
        }
        
        delay(50);
    }

    tft.fillScreen(ST77XX_BLACK);
    drawText("Loading: 0%", ST77XX_WHITE, 1, tft.width()/3 - 15, 15);
    delay(700);
    tft.fillScreen(ST77XX_BLACK);
    drawText("Loading: 15%", ST77XX_WHITE, 1, tft.width()/3 - 15, 15);
    delay(700);
    tft.fillScreen(ST77XX_BLACK);
    drawText("Loading: 27%", ST77XX_WHITE, 1, tft.width()/3 - 15, 15);
    delay(700);
    tft.fillScreen(ST77XX_BLACK);
    drawText("Loading: 49%", ST77XX_WHITE, 1, tft.width()/3 - 15, 15);
    delay(700);
    tft.fillScreen(ST77XX_BLACK);
    drawText("Loading: 65%", ST77XX_WHITE, 1, tft.width()/3 - 15, 15);
    delay(700);
    tft.fillScreen(ST77XX_BLACK);
    drawText("Loading: 83%", ST77XX_WHITE, 1, tft.width()/3 - 15, 15);
    delay(700);
    tft.fillScreen(ST77XX_BLACK);
    drawText("Loading: 94%", ST77XX_WHITE, 1, tft.width()/3 - 15, 15);
    delay(700);
    tft.fillScreen(ST77XX_BLACK);
    drawText("Loading: 100%", ST77XX_WHITE, 1, tft.width()/3 - 15, 15);
    delay(1000);
}


bool checkTextLength(String text)
{
      int width = WIDTH_BY_CHARS;

      if(text.length() < width) {
            return true;
      }

      return false;
}


static esp_err_t homepageGetHandler(httpd_req_t *request)
{
    httpd_resp_sendstr_chunk(request, "<!DOCTYPE html<html>");
    httpd_resp_sendstr_chunk(request, "<head>");
    httpd_resp_sendstr_chunk(request, "<style>");
    httpd_resp_sendstr_chunk(request, "form {display: grid; padding: 1em; background: #f9F9F9; border: 1px solid #c1c1c1; margin: 2rem auto 0 auto; max-width: 400px; padding: 1em;}}>");
    httpd_resp_sendstr_chunk(request, "form input {background: #fff;border: 1px solid #9c9c9c;}");
    httpd_resp_sendstr_chunk(request, "form button {background: lightgrey; padding: 0.7em;width: 100%; border: 0;");
    httpd_resp_sendstr_chunk(request, "label {padding: 0.5em 0.5em 0.5em 0;}");
    httpd_resp_sendstr_chunk(request, "input {padding: 0.7em;margin-bottom: 0.5rem;}");
    httpd_resp_sendstr_chunk(request, "input:focus {outline: 10px solid gold;}");
    httpd_resp_sendstr_chunk(request, "@media (min-width: 300px) {form {grid-template-columns: 200px 1fr; grid-gap: 16px;} label { text-align: right; grid-column: 1 / 2; } input, button { grid-column: 2 / 3; }}");
    httpd_resp_sendstr_chunk(request, "</style>");
    httpd_resp_sendstr_chunk(request, "</head>");

    httpd_resp_sendstr_chunk(request, "<body>");
    httpd_resp_sendstr_chunk(request, "<form class=\"form1\" id=\"loginForm\" action=\"\">");

    httpd_resp_sendstr_chunk(request, "<label for=\"MESSAGE\">Zobrazeni zpravy</label>");
    httpd_resp_sendstr_chunk(request, "<input id=\"message\" type=\"text\" name=\"message\" maxlength=\"99\" minlength=\"0\">");

    httpd_resp_sendstr_chunk(request, "<button>Odeslat</button>");
    httpd_resp_sendstr_chunk(request, "</form>");

    httpd_resp_sendstr_chunk(request, "<script>");
    httpd_resp_sendstr_chunk(request, "document.getElementById(\"loginForm\").addEventListener(\"submit\", (e) => {e.preventDefault(); const formData = new FormData(e.target); const data = Array.from(formData.entries()).reduce((memo, pair) => ({...memo, [pair[0]]: pair[1],  }), {}); var xhr = new XMLHttpRequest(); xhr.open(\"POST\", \"http://192.168.4.1/connect\", true); xhr.setRequestHeader('Content-Type', 'application/json'); xhr.send(JSON.stringify(data)); document.getElementById(\"output\").innerHTML = JSON.stringify(data);});");
    httpd_resp_sendstr_chunk(request, "</script>");

    httpd_resp_sendstr_chunk(request, "</body></html>");

    httpd_resp_send_chunk(request, NULL, 0);

    return ESP_OK;
}


static esp_err_t messagePostHandler(httpd_req_t *request)
{
    char buf[128];
    int ret, remaining = request->content_len;

    while (remaining > 0)
    {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(request, buf, MIN(remaining, sizeof(buf)))) <= 0)
        {
            if (ret == 0)
            {
                ESP_LOGI(TAG, "No content received please try again ...");
            }
            else if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            {

                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        cJSON *root = cJSON_Parse(buf);

        char messageBuffer[100];
        sprintf(messageBuffer, "%s", cJSON_GetObjectItem(root, "message")->valuestring);
        text = messageBuffer;

        Serial.println(text);

        remaining -= ret;
    }

    // End response
    httpd_resp_send_chunk(request, NULL, 0);

    return ESP_OK;
}


static const httpd_uri_t homepage = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = homepageGetHandler,
    .user_ctx = NULL
    };



static const httpd_uri_t fetchMessage = {
    .uri = "/connect",
    .method = HTTP_POST,
    .handler = messagePostHandler,
    .user_ctx = NULL
    };


static httpd_handle_t startWebserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG, "Starting WebServer, PORT: '%d'", config.server_port);

    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "WebServer start was successful");
        ESP_LOGI(TAG, "Registering WebServer URI handlers");

        httpd_register_uri_handler(server, &homepage);
        httpd_register_uri_handler(server, &fetchMessage);

        ESP_LOGI(TAG, "WebServer URI handlers was registered");

        return server;
    }

    ESP_LOGI(TAG, "Error: WebServer was not started");

    return NULL;
}


void setup(void) 
{
      Serial.begin(115200);

      ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
      wifiAccessPointInit();

      httpd_handle_t server = NULL;
      server = startWebserver();
      
      tft.initR(INITR_144GREENTAB);
 
      uint16_t time = millis();
      tft.fillScreen(ST77XX_BLACK);
      time = millis() - time;

      Serial.println(time, DEC);
      delay(500);

      introAnimation();
}

void loop() 
{
      drawBackground();

      if(checkTextLength(text)) {
            drawText(text, ST77XX_YELLOW, 2, 0, tft.height()/2-10);
            delay(1000);
      } else {
            showSlidedText(text);
      }

      delay(1000);

}

