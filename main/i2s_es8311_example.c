/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier:  CC0-1.0
 */

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "esp_system.h"
#include "esp_check.h"
#include "es8311.h"
#include "example_config.h"

static const char *TAG = "i2s_es8311";

/* Fehlermeldungen für Timeout oder ungültige Parameter */
static const char err_reason[][30] = {
    "input param is invalid",
    "operation timeout"
};

static i2s_chan_handle_t tx_handle = NULL;   // I2S TX-Kanal (Audio-Ausgabe)
static i2s_chan_handle_t rx_handle = NULL;   // I2S RX-Kanal (Audio-Eingabe)

/* Musikdatei als Binärdaten einbinden (nur im Musikmodus) */
#if CONFIG_EXAMPLE_MODE_MUSIC
extern const uint8_t music_pcm_start[] asm("_binary_canon_pcm_start");
extern const uint8_t music_pcm_end[]   asm("_binary_canon_pcm_end");
#endif

/* ---------------------------------------------------------
 * GPIO-Initialisierung
 * Aktiviert den externen Audio-Verstärker (PA-Pin)
 * --------------------------------------------------------- */
static void gpio_init(void)
{
    // GPIO48 als Ausgang konfigurieren
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_OUTPUT_PA), // GPIO48 auswählen
        .mode = GPIO_MODE_OUTPUT,                 // Ausgangsmodus
        .pull_down_en = GPIO_PULLDOWN_DISABLE,    // Kein Pull-Down
        .pull_up_en = GPIO_PULLUP_DISABLE,        // Kein Pull-Up
        .intr_type = GPIO_INTR_DISABLE            // Keine Interrupts
    };
    gpio_config(&io_conf);

    // GPIO48 auf HIGH setzen → Verstärker einschalten
    gpio_set_level(GPIO_OUTPUT_PA, 1);
}

/* ---------------------------------------------------------
 * ES8311 Audio-Codec initialisieren
 * - I2C konfigurieren
 * - Codec konfigurieren (Sample-Rate, MCLK, Lautstärke)
 * - Mikrofon aktivieren
 * --------------------------------------------------------- */
static esp_err_t es8311_codec_init(void)
{
#if !defined(CONFIG_EXAMPLE_BSP)
    /* I2C-Bus konfigurieren */
    const i2c_config_t es_i2c_cfg = {
        .sda_io_num = I2C_SDA_IO,
        .scl_io_num = I2C_SCL_IO,
        .mode = I2C_MODE_MASTER,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000, // 100 kHz
    };
    ESP_RETURN_ON_ERROR(i2c_param_config(I2C_NUM, &es_i2c_cfg), TAG, "config i2c failed");
    ESP_RETURN_ON_ERROR(i2c_driver_install(I2C_NUM, I2C_MODE_MASTER, 0, 0, 0), TAG, "install i2c driver failed");
#else
    ESP_ERROR_CHECK(bsp_i2c_init());
#endif

    /* ES8311-Codec erstellen */
    es8311_handle_t es_handle = es8311_create(I2C_NUM, ES8311_ADDRRES_0);
    ESP_RETURN_ON_FALSE(es_handle, ESP_FAIL, TAG, "es8311 create failed");

    /* Clock-Konfiguration für den Codec */
    const es8311_clock_config_t es_clk = {
        .mclk_inverted = false,
        .sclk_inverted = false,
        .mclk_from_mclk_pin = true,
        .mclk_frequency = EXAMPLE_MCLK_FREQ_HZ,
        .sample_frequency = EXAMPLE_SAMPLE_RATE
    };

    /* Codec initialisieren */
    ESP_ERROR_CHECK(es8311_init(es_handle, &es_clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16));

    /* Sample-Frequenz einstellen */
    ESP_RETURN_ON_ERROR(
        es8311_sample_frequency_config(es_handle,
                                       EXAMPLE_SAMPLE_RATE * EXAMPLE_MCLK_MULTIPLE,
                                       EXAMPLE_SAMPLE_RATE),
        TAG, "set es8311 sample frequency failed");

    /* Lautstärke setzen */
    ESP_RETURN_ON_ERROR(es8311_voice_volume_set(es_handle, EXAMPLE_VOICE_VOLUME, NULL),
                        TAG, "set es8311 volume failed");

    /* Mikrofon aktivieren */
    ESP_RETURN_ON_ERROR(es8311_microphone_config(es_handle, false),
                        TAG, "set es8311 microphone failed");

#if CONFIG_EXAMPLE_MODE_ECHO
    /* Mikrofonverstärkung setzen (nur im Echomodus) */
    ESP_RETURN_ON_ERROR(es8311_microphone_gain_set(es_handle, EXAMPLE_MIC_GAIN),
                        TAG, "set es8311 microphone gain failed");
#endif

    return ESP_OK;
}

/* ---------------------------------------------------------
 * I2S-Treiber initialisieren
 * - TX und RX Kanäle erstellen
 * - Philips-Standardmodus
 * - 16-Bit Stereo
 * - Pins zuweisen
 * --------------------------------------------------------- */
static esp_err_t i2s_driver_init(void)
{
#if !defined(CONFIG_EXAMPLE_BSP)
    /* Standard I2S-Kanal-Konfiguration */
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true; // DMA-Puffer automatisch leeren
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle));

    /* Standard I2S-Konfiguration */
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(EXAMPLE_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
                        I2S_DATA_BIT_WIDTH_16BIT,
                        I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_MCK_IO,
            .bclk = I2S_BCK_IO,
            .ws = I2S_WS_IO,
            .dout = I2S_DO_IO,
            .din = I2S_DI_IO,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    std_cfg.clk_cfg.mclk_multiple = EXAMPLE_MCLK_MULTIPLE;

    /* TX- und RX-Kanal aktivieren */
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_handle));

#else
    /* BSP übernimmt die Hardware-Konfiguration */
    ESP_LOGI(TAG, "Using BSP for HW configuration");
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(EXAMPLE_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
                        I2S_DATA_BIT_WIDTH_16BIT,
                        I2S_SLOT_MODE_STEREO),
        .gpio_cfg = BSP_I2S_GPIO_CFG,
    };
    std_cfg.clk_cfg.mclk_multiple = EXAMPLE_MCLK_MULTIPLE;

    ESP_ERROR_CHECK(bsp_audio_init(&std_cfg, &tx_handle, &rx_handle));
    ESP_ERROR_CHECK(bsp_audio_poweramp_enable(true));
#endif

    return ESP_OK;
}

#if CONFIG_EXAMPLE_MODE_MUSIC
/* ---------------------------------------------------------
 *  Musikmodus
 * - PCM-Daten werden abgespielt
 * - Endlosschleife
 * --------------------------------------------------------- */
static void i2s_music(void *args)
{
    esp_err_t ret = ESP_OK;
    size_t bytes_write = 0;
    uint8_t *data_ptr = (uint8_t *)music_pcm_start;

    /* TX-Kanal deaktivieren und Daten vorladen,
       damit sofort gültige Daten gesendet werden */
    ESP_ERROR_CHECK(i2s_channel_disable(tx_handle));
    ESP_ERROR_CHECK(i2s_channel_preload_data(
        tx_handle, data_ptr, music_pcm_end - data_ptr, &bytes_write));

    data_ptr += bytes_write; // Datenzeiger weiter bewegen

    /* TX-Kanal aktivieren */
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));

    while (1) {
        /* Musikdaten an Kopfhörer ausgeben */
        ret = i2s_channel_write(tx_handle,
                                data_ptr,
                                music_pcm_end - data_ptr,
                                &bytes_write,
                                portMAX_DELAY);

        if (ret != ESP_OK) {
            /* portMAX_DELAY → Timeout nur möglich, wenn man ihn ändert */
            ESP_LOGE(TAG, "[music] i2s write failed, %s",
                     err_reason[ret == ESP_ERR_TIMEOUT]);
            abort();
        }

        if (bytes_write > 0) {
            ESP_LOGI(TAG, "[music] %d Bytes abgespielt.", bytes_write);
        } else {
            ESP_LOGE(TAG, "[music] Wiedergabe fehlgeschlagen.");
            abort();
        }

        /* Wieder von vorne beginnen */
        data_ptr = (uint8_t *)music_pcm_start;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

#else
/* ---------------------------------------------------------
 * Echomodus
 * - Mikrofon einlesen
 * - Direkt wieder ausgeben
 * - Echtzeit-Echo
 * --------------------------------------------------------- */
static void i2s_echo(void *args)
{
    int *mic_data = malloc(EXAMPLE_RECV_BUF_SIZE);
    if (!mic_data) {
        ESP_LOGE(TAG, "[echo] Kein Speicher für Puffer");
        abort();
    }

    esp_err_t ret = ESP_OK;
    size_t bytes_read = 0;
    size_t bytes_write = 0;

    ESP_LOGI(TAG, "[echo] Echo gestartet");

    while (1) {
        memset(mic_data, 0, EXAMPLE_RECV_BUF_SIZE);

        /* Mikrofon-Daten lesen */
        ret = i2s_channel_read(rx_handle,
                               mic_data,
                               EXAMPLE_RECV_BUF_SIZE,
                               &bytes_read,
                               1000);

        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "[echo] i2s read failed, %s",
                     err_reason[ret == ESP_ERR_TIMEOUT]);
            abort();
        }

        /* Daten direkt wieder ausgeben */
        ret = i2s_channel_write(tx_handle,
                                mic_data,
                                EXAMPLE_RECV_BUF_SIZE,
                                &bytes_write,
                                1000);

        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "[echo] i2s write failed, %s",
                     err_reason[ret == ESP_ERR_TIMEOUT]);
            abort();
        }

        if (bytes_read != bytes_write) {
            ESP_LOGW(TAG, "[echo] %d Bytes gelesen, aber nur %d geschrieben",
                     bytes_read, bytes_write);
        }
    }

    vTaskDelete(NULL);
}
#endif

/* ---------------------------------------------------------
 * Hauptprogramm
 * - GPIO initialisieren
 * - I2S starten
 * - Codec starten
 * - Musik- oder Echo-Task starten
 * --------------------------------------------------------- */
void app_main(void)
{
    gpio_init();
    printf("i2s es8311 codec example start\n-----------------------------\n");

    /* I2S initialisieren */
    if (i2s_driver_init() != ESP_OK) {
        ESP_LOGE(TAG, "i2s driver init failed");
        abort();
    } else {
        ESP_LOGI(TAG, "i2s driver init success");
    }

    /* Codec initialisieren */
    if (es8311_codec_init() != ESP_OK) {
        ESP_LOGE(TAG, "es8311 codec init failed");
        abort();
    } else {
        ESP_LOGI(TAG, "es8311 codec init success");
    }

#if CONFIG_EXAMPLE_MODE_MUSIC
    /* Musikmodus starten */
    xTaskCreate(i2s_music, "i2s_music", 4096, NULL, 5, NULL);
#else
    /* Echomodus starten */
    xTaskCreate(i2s_echo, "i2s_echo", 8192, NULL, 5, NULL);
#endif
}
