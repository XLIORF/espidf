#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
void TF_Init(void);
void TF_Deinit(void);
sdmmc_card_t * TF_Get_Card(void);
