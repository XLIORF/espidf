#ifndef LCD
#define LCD

#include "spi.h"
#include "u8g2.h"
#include "u8g2_luat_fonts.h"

#define WHITE            0xFFFF
#define BLACK            0x0000
#define BLUE             0x001F
#define BRED             0XF81F
#define GRED             0XFFE0
#define GBLUE            0X07FF
#define RED              0xF800
#define MAGENTA          0xF81F
#define GREEN            0x07E0
#define CYAN             0x7FFF
#define YELLOW           0xFFE0
#define BROWN            0XBC40
#define BRRED            0XFC07
#define GRAY             0X8430
#define GRAY175          0XAD75
#define GRAY151          0X94B2
#define GRAY187          0XBDD7
#define GRAY240          0XF79E

#ifndef LCD_COLOR_DEPTH
#define LCD_COLOR_DEPTH 16
#endif

#if (LCD_COLOR_DEPTH == 32)
#define color_t uint32_t
#elif (LCD_COLOR_DEPTH == 16)
#define color_t uint16_t
#elif (LCD_COLOR_DEPTH == 8)
#define color_t uint8_t
#else
#error "no supprt color depth"
#endif

#define LCD_SPI_DEVICE 255

struct lcd_opts;

typedef struct lcd_conf {
    uint8_t port;
    uint8_t pin_dc;
    uint8_t pin_pwr;
    uint8_t pin_rst;

    uint32_t w;
    uint32_t h;
    uint32_t buffer_size;
    uint32_t dc_delay_us;
    uint8_t xoffset;//偏移
    uint8_t yoffset;//偏移
    uint8_t auto_flush;
    uint8_t direction;//方向
    u8g2_t lcd_u8g2 ;
    struct lcd_opts* opts;
    spi_device_t* lcd_spi_device;
    int lcd_spi_ref;
    void* userdata;

    // buff 相关
    color_t* buff;
    int buff_ref;
    uint16_t flush_y_min;
    uint16_t flush_y_max;
    uint8_t is_init_done;
} lcd_conf_t;

typedef struct lcd_opts {
    const char* name;
    int (*init)(lcd_conf_t* conf);
} lcd_opts_t;

typedef struct lcd_reg {
  const char *name;
  const lcd_opts_t *lcd_opts;
}lcd_reg_t;

lcd_conf_t* lcd_setup(spi_device_t *spi_device, char* tp, int pin_dc, int pin_pwr, int pin_rst, int direction, int w, int h, int xoffset, int yoffset);
int lcd_write_cmd(lcd_conf_t* conf,const uint8_t cmd);
int lcd_write_data(lcd_conf_t* conf,const uint8_t data);
int lcd_write_half_word(lcd_conf_t* conf,const color_t da);

lcd_conf_t* lcd_get_default(void);
const char* lcd_name(lcd_conf_t* conf);
int lcd_init(lcd_conf_t* conf);
int lcd_close(lcd_conf_t* conf);
int lcd_display_on(lcd_conf_t* conf);
int lcd_display_off(lcd_conf_t* conf);
int lcd_sleep(lcd_conf_t* conf);
int lcd_wakeup(lcd_conf_t* conf);
int lcd_inv_off(lcd_conf_t* conf);
int lcd_inv_on(lcd_conf_t* conf);
int lcd_set_address(lcd_conf_t* conf,uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
int lcd_set_color(color_t back, color_t fore);
color_t color_swap(color_t color);
int lcd_draw(lcd_conf_t* conf, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t* color);
int lcd_flush(lcd_conf_t* conf);
int lcd_draw_no_block(lcd_conf_t* conf, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t* color, uint8_t last_flush);
int lcd_clear(lcd_conf_t* conf,color_t color);
int lcd_draw_fill(lcd_conf_t* conf,uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,color_t color);
int lcd_draw_point(lcd_conf_t* conf, uint16_t x, uint16_t y, color_t color);
int lcd_draw_line(lcd_conf_t* conf,uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t color);
int lcd_draw_vline(lcd_conf_t* conf, uint16_t x, uint16_t y,uint16_t h, color_t color);
int lcd_draw_hline(lcd_conf_t* conf, uint16_t x, uint16_t y,uint16_t h, color_t color);
int lcd_draw_rectangle(lcd_conf_t* conf,uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t color);
int lcd_draw_circle(lcd_conf_t* conf,uint16_t x0, uint16_t y0, uint8_t r, color_t color);

int lcd_set_font(lcd_conf_t* conf, const uint8_t *ptr);
int lcd_draw_str(lcd_conf_t* conf, int x, int y, char* str, color_t color);
int lcd_draw_qr_code(lcd_conf_t* conf, int x, int y, char* str, int size);
int lcd_draw_xbm(lcd_conf_t* conf, int x, int y, int w, int h, char* data, int data_len);

void lcd_execute_cmds(lcd_conf_t* conf, uint32_t* cmds, uint32_t count);

typedef struct lcd_custom {
    size_t init_cmd_count;
    uint32_t *initcmd; // 实际命令长度与init_cmd_count相同
}lcd_custom_t;

#endif

