#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "OLED.h"
#include "tf.h"

static const char *TAG = "Main";
#define MOUNT_POINT "/sdcard"

void app_main()
{
    OLED_Init();
    TF_Init();
    // FILE *f = fopen(MOUNT_POINT"/hello.txt","w");
    // if (f == NULL) {
    //     ESP_LOGE(TAG, "Failed to open file for writing");
    //     return;
    // }
    // fprintf(f,"Hello SD!");
    // fclose(f);
    // ESP_LOGI(TAG, "File written");

    FILE *f = fopen(MOUNT_POINT"/hello.txt","r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    
    // Read a line from file
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);

    // Strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);
    
    // All done, unmount partition and disable SPI peripheral
    TF_Deinit();
}
