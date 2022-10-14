#include "lcd.h"
#include "gpio.h"
#include "spi.h"
#include "malloc.h"
#include "delay.h"
#include "qrcodegen.h"

#include "esp_log.h"

#define TAG "lcd"

static lcd_conf_t *default_conf = NULL;
color_t BACK_COLOR = WHITE, FORE_COLOR = BLACK;

extern const lcd_opts_t lcd_opts_st7735;
extern const lcd_opts_t lcd_opts_st7735v;
extern const lcd_opts_t lcd_opts_st7735s;
extern const lcd_opts_t lcd_opts_st7789;
extern const lcd_opts_t lcd_opts_st7796;
extern const lcd_opts_t lcd_opts_gc9a01;
extern const lcd_opts_t lcd_opts_gc9106l;
extern const lcd_opts_t lcd_opts_gc9306x;
extern const lcd_opts_t lcd_opts_ili9341;
extern const lcd_opts_t lcd_opts_ili9486;
extern const lcd_opts_t lcd_opts_ili9488;
extern const lcd_opts_t lcd_opts_custom;

static const lcd_reg_t lcd_regs[] = {
    {"custom",  &lcd_opts_custom},   //0 固定为第零个
    {"st7735",  &lcd_opts_st7735},
    {"st7735v", &lcd_opts_st7735v},
    {"st7735s", &lcd_opts_st7735s},
    {"st7789",  &lcd_opts_st7789},
    {"st7796", &lcd_opts_st7796},
    {"gc9a01",  &lcd_opts_gc9a01},
    {"gc9106l", &lcd_opts_gc9106l},
    {"gc9306x", &lcd_opts_gc9306x},
    {"gc9306",  &lcd_opts_gc9306x},  //gc9306是gc9306x的别名
    {"ili9341", &lcd_opts_ili9341},
    {"ili9486", &lcd_opts_ili9486},
    {"ili9488", &lcd_opts_ili9488},
    {"", NULL} // 最后一个必须是空字符串
};

// lcd.init("st7735s",{port = "device",pin_dc = 6, pin_pwr = 11,pin_rst = 10,direction = 2,w = 160,h = 80, xoffset = 1,yoffset = 26},spi_lcd)
lcd_conf_t * lcd_setup(spi_device_t *spi_device, char* tp, int pin_dc, int pin_pwr, int pin_rst, int direction, int w, int h, int xoffset, int yoffset)
{
    lcd_conf_t *conf = malloc(sizeof(lcd_conf_t));
    memset(conf, 0, sizeof(lcd_conf_t)); // 填充0,保证无脏数据
    
    conf->pin_pwr = 255;
    conf->lcd_spi_device = spi_device;
    conf->port = LCD_SPI_DEVICE;

    // const char* tp = "st7735s";
    int16_t s_index = -1;   //第几个屏幕，-1表示没匹配到
    for(int i = 0; i < 100; i++){
        if (strlen(lcd_regs[i].name) == 0)
          break;
        if(strcmp(lcd_regs[i].name,tp) == 0){
            s_index = i;
            break;
        }
    }
    if (s_index != -1) {
        ESP_LOGI(TAG, "ic support: %s",tp);
        
        conf->opts = (struct lcd_opts *)lcd_regs[s_index].lcd_opts;
        conf->port = LCD_SPI_DEVICE;
        conf->pin_dc = pin_dc;
        conf->pin_pwr = pin_pwr;
        conf->pin_rst = pin_rst;
        conf->direction = direction;
        conf->w = w;
        conf->h = h;

        conf->buffer_size = (conf->w * conf->h) * 2;
        conf->xoffset = xoffset;
        conf->yoffset = yoffset;
            

        if (s_index == 0){
            // lcd_custom_t *cst = heap_malloc(sizeof(lcd_custom_t));
            // lua_pushstring(L, "initcmd");
            // lua_gettable(L, 2);
            // if (lua_istable(L, -1)) {
            //   cst->init_cmd_count = lua_rawlen(L, -1);
            //   cst->initcmd = heap_malloc(cst->init_cmd_count * sizeof(uint32_t));
            //   for (size_t i = 1; i <= cst->init_cmd_count; i++){
            //       lua_geti(L, -1, i);
            //       cst->initcmd[i-1] = luaL_checkinteger(L, -1);
            //       lua_pop(L, 1);
            //   }
            // }else if(lua_isstring(L, -1)){
            //   size_t  len,cmd;
            //   const char *fail_name = luaL_checklstring(L, -1, &len);
            //   FILE* fd = (FILE *)fs_fopen(fail_name, "rb");
            //   cst->init_cmd_count = 0;
            //   if (fd){
            //       #define INITCMD_BUFF_SIZE 128
            //       char init_cmd_buff[INITCMD_BUFF_SIZE] ;
            //       cst->initcmd = heap_malloc(sizeof(uint32_t));
            //       while (1) {
            //           memset(init_cmd_buff, 0, INITCMD_BUFF_SIZE);
            //           int readline_len = fs_readline(init_cmd_buff, INITCMD_BUFF_SIZE-1, fd);
            //           if (readline_len < 1)
            //               break;
            //           if (memcmp(init_cmd_buff, "#", 1)==0){
            //               continue;
            //           }
            //           char *token = strtok(init_cmd_buff, ",");
            //           if (sscanf(token,"%x",&cmd) < 1){
            //               continue;
            //           }
            //           cst->init_cmd_count = cst->init_cmd_count + 1;
            //           cst->initcmd = heap_realloc(cst->initcmd,cst->init_cmd_count * sizeof(uint32_t));
            //           cst->initcmd[cst->init_cmd_count-1]=cmd;
            //           while( token != NULL ) {
            //               token = strtok(NULL, ",");
            //               if (sscanf(token,"%x",&cmd) < 1){
            //                   break;
            //               }
            //               cst->init_cmd_count = cst->init_cmd_count + 1;
            //               cst->initcmd = heap_realloc(cst->initcmd,cst->init_cmd_count * sizeof(uint32_t));
            //               cst->initcmd[cst->init_cmd_count-1]=cmd;
            //           }
            //       }
            //       cst->initcmd[cst->init_cmd_count]= 0;
            //       fs_fclose(fd);
            //   }else{
            //       LLOGE("init_cmd fail open error");
            //   }
            // }
            // lua_pop(L, 1);
            // conf->userdata = cst;
        }
        // 默认自动flush,即使没有buff
        conf->auto_flush = 1;

#ifdef USE_LCD_SDL2
        extern const lcd_opts_t lcd_opts_sdl2;
        conf->opts = &lcd_opts_sdl2;
#endif
        int ret = lcd_init(conf);
        if (ret != 0) {
            ESP_LOGE(TAG, "lcd init fail %d", ret);
            free(conf);
            return NULL;
        }
        // 初始化OK, 配置额外的参数
        default_conf = conf;
        u8g2_SetFont(&(conf->lcd_u8g2), u8g2_font_opposansm8);
        u8g2_SetFontMode(&(conf->lcd_u8g2), 0);
        u8g2_SetFontDirection(&(conf->lcd_u8g2), 0);
        return conf;
    }

    ESP_LOGE(TAG, "ic not support: %s",tp);
    free(conf);
    return NULL;
}

#define LCD_CONF_COUNT (1)
static lcd_conf_t* confs[LCD_CONF_COUNT] = {0};

color_t color_swap(color_t color) {
    color_t tmp = (color >> 8) + ((color & 0xFF) << 8);
    return tmp;
}

void lcd_execute_cmds(lcd_conf_t* conf, uint32_t* cmds, uint32_t count) {
    uint32_t cmd = 0;
    for (size_t i = 0; i < count; i++)
    {
        cmd = cmds[i];
        switch(((cmd >> 16) & 0xFFFF)) {
            case 0x0000 :
                lcd_write_cmd(conf, (const uint8_t)(cmd & 0xFF));
                break;
            case 0x0001 :
                delay_ms(cmd & 0xFF);
                break;
            case 0x0002 :
                lcd_write_cmd(conf, (const uint8_t)(cmd & 0xFF));
                break;
            case 0x0003 :
                lcd_write_data(conf, (const uint8_t)(cmd & 0xFF));
                break;
            default:
                break;
        }
    }
}


int lcd_write_cmd(lcd_conf_t* conf, const uint8_t cmd){
    size_t len;
    gpio_set(conf->pin_dc, GPIO_LOW);
#ifdef LCD_CMD_DELAY_US
    if (conf->dc_delay_us){
    	timer_us_delay(conf->dc_delay_us);
    }
#endif
    if (conf->port == LCD_SPI_DEVICE){
        len = spi_device_send((spi_device_t*)(conf->lcd_spi_device),  (const char*)&cmd, 1);
    }else{
        len = spi_send(conf->port, (const char*)&cmd, 1);
    }
    gpio_set(conf->pin_dc, GPIO_HIGH);
    if (len != 1){
        ESP_LOGI(TAG, "lcd_write_cmd error. %d", len);
        return -1;
    }else{
        #ifdef LCD_CMD_DELAY_US
        if (conf->dc_delay_us){
        	timer_us_delay(conf->dc_delay_us);
        }
        #endif
        return 0;
    }
}

int lcd_write_data(lcd_conf_t* conf, const uint8_t data){
    size_t len;
    if (conf->port == LCD_SPI_DEVICE){
        len = spi_device_send((spi_device_t*)(conf->lcd_spi_device),  (const char*)&data, 1);
    }else{
        len = spi_send(conf->port,  (const char*)&data, 1);
    }
    if (len != 1){
        ESP_LOGI(TAG, "lcd_write_data error. %d", len);
        return -1;
    }else{
        return 0;
    }
}

lcd_conf_t* lcd_get_default(void) {
    for (size_t i = 0; i < LCD_CONF_COUNT; i++){
        if (confs[i] != NULL) {
            return confs[i];
        }
    }
    return NULL;
}

const char* lcd_name(lcd_conf_t* conf) {
    return conf->opts->name;
}

int lcd_init(lcd_conf_t* conf) {
	conf->is_init_done = 0;
    int ret = conf->opts->init(conf);
    if (ret == 0) {
    	conf->is_init_done = 1;
        for (size_t i = 0; i < LCD_CONF_COUNT; i++)
        {
            if (confs[i] == NULL) {
                confs[i] = conf;
                break;
            }
        }
    }
    return ret;
}

int lcd_close(lcd_conf_t* conf) {
    if (conf->pin_pwr != 255)
        gpio_set(conf->pin_pwr, GPIO_LOW);
    return 0;
}

int lcd_display_off(lcd_conf_t* conf) {
    if (conf->pin_pwr != 255)
        gpio_set(conf->pin_pwr, GPIO_LOW);
    lcd_write_cmd(conf,0x28);
    return 0;
}

int lcd_display_on(lcd_conf_t* conf) {
    if (conf->pin_pwr != 255)
        gpio_set(conf->pin_pwr, GPIO_HIGH);
    lcd_write_cmd(conf,0x29);
    return 0;
}

int lcd_sleep(lcd_conf_t* conf) {
    if (conf->pin_pwr != 255)
        gpio_set(conf->pin_pwr, GPIO_LOW);
    delay_ms(5);
    lcd_write_cmd(conf,0x10);
    return 0;
}

int lcd_wakeup(lcd_conf_t* conf) {
    if (conf->pin_pwr != 255)
        gpio_set(conf->pin_pwr, GPIO_HIGH);
    delay_ms(5);
    lcd_write_cmd(conf,0x11);
    return 0;
}

int lcd_inv_off(lcd_conf_t* conf) {
    lcd_write_cmd(conf,0x20);
    return 0;
}

int lcd_inv_on(lcd_conf_t* conf) {
    lcd_write_cmd(conf,0x21);
    return 0;
}

int lcd_set_address(lcd_conf_t* conf,uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    lcd_write_cmd(conf,0x2a);
    lcd_write_data(conf,(x1+conf->xoffset)>>8);
    lcd_write_data(conf,x1+conf->xoffset);
    lcd_write_data(conf,(x2+conf->xoffset)>>8);
    lcd_write_data(conf,x2+conf->xoffset);
    lcd_write_cmd(conf,0x2b);
    lcd_write_data(conf,(y1+conf->yoffset)>>8);
    lcd_write_data(conf,y1+conf->yoffset);
    lcd_write_data(conf,(y2+conf->yoffset)>>8);
    lcd_write_data(conf,y2+conf->yoffset);
    lcd_write_cmd(conf,0x2C);
    return 0;
}

int lcd_set_color(color_t back, color_t fore){
    BACK_COLOR = back;
    FORE_COLOR = fore;
    return 0;
}

#ifndef USE_LCD_CUSTOM_DRAW
int lcd_flush(lcd_conf_t* conf) {
    if (conf->buff == NULL) {
        return 0;
    }
    //LLOGD("lcd_flush range %d %d", conf->flush_y_min, conf->flush_y_max);
    if (conf->flush_y_max < conf->flush_y_min) {
        // 没有需要刷新的内容,直接跳过
        //LLOGD("lcd_flush no need");
        return 0;
    }
    uint32_t size = conf->w * (conf->flush_y_max - conf->flush_y_min + 1) * 2;
    lcd_set_address(conf, 0, conf->flush_y_min, conf->w - 1, conf->flush_y_max);
    const char* tmp = (const char*)(conf->buff + conf->flush_y_min * conf->w);
	if (conf->port == LCD_SPI_DEVICE){
		spi_device_send((spi_device_t*)(conf->lcd_spi_device), tmp, size);
	}else{
		spi_send(conf->port, tmp, size);
	}

    // 重置为不需要刷新的状态
    conf->flush_y_max = 0;
    conf->flush_y_min = conf->h;
    
    return 0;
}

int lcd_draw(lcd_conf_t* conf, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t* color) {
    // ESP_LOGI(TAG, "lcd_draw conf={port=%d, w=%d, h=%d, buff=0x%x}", conf->port, conf->w, conf->h, conf->buff);
    // 直接刷屏模式
    if (conf->buff == NULL) {
        uint32_t size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
        lcd_set_address(conf, x1, y1, x2, y2);
	    if (conf->port == LCD_SPI_DEVICE){
		    spi_device_send((spi_device_t*)(conf->lcd_spi_device), (const char*)color, size);
	    }else{
		    spi_send(conf->port, (const char*)color, size);
	    }
        return 0;
    }
    // buff模式
    if (x1 > conf->w || y1 > conf->h) {
        ESP_LOGE(TAG, "out of lcd range");
        return -1;
    }
    uint16_t x_end = x2 > conf->w?conf->w:x2;
    uint16_t y_end = y2 > conf->h?conf->h:y2;
    color_t* dst = (conf->buff + x1 + conf->w * y1);
    color_t* src = (color);
    size_t lsize = (x_end - x1 + 1);
    for (size_t i = y1; i <= y_end; i++) {
        memcpy(dst, src, lsize * sizeof(color_t));
        dst += conf->w;  // 移动到下一行
        src += lsize;    // 移动数据
        if (x2 > conf->w){
            src+=x2 - conf->w;
        }
    }
    // 存储需要刷新的区域
    if (y1 < conf->flush_y_min)
        conf->flush_y_min = y1;
    if (y_end > conf->flush_y_max)
        conf->flush_y_max = y_end;
    return 0;
}
#endif

int lcd_draw_point(lcd_conf_t* conf, uint16_t x, uint16_t y, color_t color) {
    // 注意, 这里需要把颜色swap了
    color_t tmp = color_swap(color);
    return lcd_draw(conf, x, y, x, y, &tmp);
}

int lcd_clear(lcd_conf_t* conf, color_t color){
    lcd_draw_fill(conf, 0, 0, conf->w, conf->h, color);
    return 0;
}

int lcd_draw_fill(lcd_conf_t* conf,uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2, color_t color) {          
	uint16_t i;
	for(i=y1;i<y2;i++)
	{
		lcd_draw_line(conf, x1, i, x2, i, color);
	}
    return 0;			  	    
}

int lcd_draw_vline(lcd_conf_t* conf, uint16_t x, uint16_t y,uint16_t h, color_t color) {
    if (h==0) return 0;
    return lcd_draw_line(conf, x, y, x, y + h - 1, color);
}

int lcd_draw_hline(lcd_conf_t* conf, uint16_t x, uint16_t y,uint16_t w, color_t color) {
    if (w==0) return 0;
    return lcd_draw_line(conf, x, y, x + w - 1, y, color);
}

int lcd_draw_line(lcd_conf_t* conf,uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,color_t color) {
    uint16_t t;
    uint32_t i = 0;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, row, col;
    if (x1 == x2 || y1 == y2) // 直线
    {
        size_t dots = (x2 - x1 + 1) * (y2 - y1 + 1);//点数量
        color_t* line_buf = (color_t*) malloc(dots * sizeof(color_t));
        // 颜色swap
        color_t tmp = color_swap(color);
        if (line_buf) {
            for (i = 0; i < dots; i++)
            {
                line_buf[i] = tmp;
            }
            lcd_draw(conf, x1, y1, x2, y2, line_buf);
            free(line_buf);
            return 0;
        }
    }

    delta_x = x2 - x1;
    delta_y = y2 - y1;
    row = x1;
    col = y1;
    if (delta_x > 0)incx = 1;
    else if (delta_x == 0)incx = 0;
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }
    if (delta_y > 0)incy = 1;
    else if (delta_y == 0)incy = 0;
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }
    if (delta_x > delta_y)distance = delta_x;
    else distance = delta_y;
    for (t = 0; t <= distance + 1; t++)
    {
        lcd_draw_point(conf,row, col,color);
        xerr += delta_x ;
        yerr += delta_y ;
        if (xerr > distance)
        {
            xerr -= distance;
            row += incx;
        }
        if (yerr > distance)
        {
            yerr -= distance;
            col += incy;
        }
    }
    return 0;
}

int lcd_draw_rectangle(lcd_conf_t* conf,uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t color){
    lcd_draw_line(conf,x1, y1, x2, y1, color);
    lcd_draw_line(conf,x1, y1, x1, y2, color);
    lcd_draw_line(conf,x1, y2, x2, y2, color);
    lcd_draw_line(conf,x2, y1, x2, y2, color);
    return 0;
}

int lcd_draw_circle(lcd_conf_t* conf,uint16_t x0, uint16_t y0, uint8_t r, color_t color){
    int a, b;
    int di;
    a = 0;
    b = r;
    di = 3 - (r << 1);
    while (a <= b)
    {
        lcd_draw_point(conf,x0 - b, y0 - a,color);
        lcd_draw_point(conf,x0 + b, y0 - a,color);
        lcd_draw_point(conf,x0 - a, y0 + b,color);
        lcd_draw_point(conf,x0 - b, y0 - a,color);
        lcd_draw_point(conf,x0 - a, y0 - b,color);
        lcd_draw_point(conf,x0 + b, y0 + a,color);
        lcd_draw_point(conf,x0 + a, y0 - b,color);
        lcd_draw_point(conf,x0 + a, y0 + b,color);
        lcd_draw_point(conf,x0 - b, y0 + a,color);
        a++;
        //Bresenham
        if (di < 0)di += 4 * a + 6;
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
        lcd_draw_point(conf,x0 + a, y0 + b,color);
    }
    return 0;
}


extern int8_t u8g2_font_decode_get_signed_bits(u8g2_font_decode_t *f, uint8_t cnt);
extern uint8_t u8g2_font_decode_get_unsigned_bits(u8g2_font_decode_t *f, uint8_t cnt);
static color_t lcd_str_fg_color,lcd_str_bg_color;

static void u8g2_draw_hv_line(u8g2_t *u8g2, int16_t x, int16_t y, int16_t len, uint8_t dir, uint16_t color){
  switch(dir)
  {
    case 0:
      lcd_draw_hline(default_conf,x,y,len,color);
      break;
    case 1:
      lcd_draw_vline(default_conf,x,y,len,color);
      break;
    case 2:
        lcd_draw_hline(default_conf,x-len+1,y,len,color);
      break;
    case 3:
      lcd_draw_vline(default_conf,x,y-len+1,len,color);
      break;
  }
}

static void u8g2_font_decode_len(u8g2_t *u8g2, uint8_t len, uint8_t is_foreground){
  uint8_t cnt;  /* total number of remaining pixels, which have to be drawn */
  uint8_t rem;  /* remaining pixel to the right edge of the glyph */
  uint8_t current;  /* number of pixels, which need to be drawn for the draw procedure */
    /* current is either equal to cnt or equal to rem */
  /* local coordinates of the glyph */
  uint8_t lx,ly;
  /* target position on the screen */
  int16_t x, y;
  u8g2_font_decode_t *decode = &(u8g2->font_decode);
  cnt = len;
  /* get the local position */
  lx = decode->x;
  ly = decode->y;
  for(;;){
    /* calculate the number of pixel to the right edge of the glyph */
    rem = decode->glyph_width;
    rem -= lx;
    /* calculate how many pixel to draw. This is either to the right edge */
    /* or lesser, if not enough pixel are left */
    current = rem;
    if ( cnt < rem )
      current = cnt;
    /* now draw the line, but apply the rotation around the glyph target position */
    //u8g2_font_decode_draw_pixel(u8g2, lx,ly,current, is_foreground);
    // printf("lx:%d,ly:%d,current:%d, is_foreground:%d \r\n",lx,ly,current, is_foreground);
    /* get target position */
    x = decode->target_x;
    y = decode->target_y;
    /* apply rotation */
    x = u8g2_add_vector_x(x, lx, ly, decode->dir);
    y = u8g2_add_vector_y(y, lx, ly, decode->dir);
    /* draw foreground and background (if required) */
    if ( current > 0 )		/* avoid drawing zero length lines, issue #4 */
    {
      if ( is_foreground )
      {
	    u8g2_draw_hv_line(u8g2, x, y, current, decode->dir, lcd_str_fg_color);
      }
      // else if ( decode->is_transparent == 0 )
      // {
	    // u8g2_draw_hv_line(u8g2, x, y, current, decode->dir, lcd_str_bg_color);
      // }
    }
    /* check, whether the end of the run length code has been reached */
    if ( cnt < rem )
      break;
    cnt -= rem;
    lx = 0;
    ly++;
  }
  lx += cnt;
  decode->x = lx;
  decode->y = ly;
}
static void u8g2_font_setup_decode(u8g2_t *u8g2, const uint8_t *glyph_data)
{
  u8g2_font_decode_t *decode = &(u8g2->font_decode);
  decode->decode_ptr = glyph_data;
  decode->decode_bit_pos = 0;

  /* 8 Nov 2015, this is already done in the glyph data search procedure */
  /*
  decode->decode_ptr += 1;
  decode->decode_ptr += 1;
  */

  decode->glyph_width = u8g2_font_decode_get_unsigned_bits(decode, u8g2->font_info.bits_per_char_width);
  decode->glyph_height = u8g2_font_decode_get_unsigned_bits(decode,u8g2->font_info.bits_per_char_height);

}
static int8_t u8g2_font_decode_glyph(u8g2_t *u8g2, const uint8_t *glyph_data){
  uint8_t a, b;
  int8_t x, y;
  int8_t d;
  int8_t h;
  u8g2_font_decode_t *decode = &(u8g2->font_decode);
  u8g2_font_setup_decode(u8g2, glyph_data);
  h = u8g2->font_decode.glyph_height;
  x = u8g2_font_decode_get_signed_bits(decode, u8g2->font_info.bits_per_char_x);
  y = u8g2_font_decode_get_signed_bits(decode, u8g2->font_info.bits_per_char_y);
  d = u8g2_font_decode_get_signed_bits(decode, u8g2->font_info.bits_per_delta_x);

  if ( decode->glyph_width > 0 )
  {
    decode->target_x = u8g2_add_vector_x(decode->target_x, x, -(h+y), decode->dir);
    decode->target_y = u8g2_add_vector_y(decode->target_y, x, -(h+y), decode->dir);
    //u8g2_add_vector(&(decode->target_x), &(decode->target_y), x, -(h+y), decode->dir);
    /* reset local x/y position */
    decode->x = 0;
    decode->y = 0;
    /* decode glyph */
    for(;;){
      a = u8g2_font_decode_get_unsigned_bits(decode, u8g2->font_info.bits_per_0);
      b = u8g2_font_decode_get_unsigned_bits(decode, u8g2->font_info.bits_per_1);
      do{
        u8g2_font_decode_len(u8g2, a, 0);
        u8g2_font_decode_len(u8g2, b, 1);
      } while( u8g2_font_decode_get_unsigned_bits(decode, 1) != 0 );
      if ( decode->y >= h )
        break;
    }
  }
  return d;
}
const uint8_t *u8g2_font_get_glyph_data(u8g2_t *u8g2, uint16_t encoding);
static int16_t u8g2_font_draw_glyph(u8g2_t *u8g2, int16_t x, int16_t y, uint16_t encoding){
  int16_t dx = 0;
  u8g2->font_decode.target_x = x;
  u8g2->font_decode.target_y = y;
  const uint8_t *glyph_data = u8g2_font_get_glyph_data(u8g2, encoding);
  if ( glyph_data != NULL ){
    dx = u8g2_font_decode_glyph(u8g2, glyph_data);
  }
  return dx;
}

/*
设置字体
@api lcd.setFont(font)
@int font lcd.font_opposansm8 lcd.font_opposansm10 lcd.font_opposansm16  lcd.font_opposansm18  lcd.font_opposansm20  lcd.font_opposansm22  lcd.font_opposansm24 lcd.font_opposansm32 lcd.font_opposansm12_chinese lcd.font_opposansm16_chinese lcd.font_opposansm24_chinese lcd.font_opposansm32_chinese
@usage
-- 设置为字体,对之后的drawStr有效,调用lcd.drawStr前一定要先设置
-- 使用中文字体需在conf_bsp.h中开启相对应的宏
lcd.setFont(lcd.font_opposansm12)
lcd.drawStr(40,10,"drawStr")
sys.wait(2000)
lcd.setFont(lcd.font_opposansm12_chinese)
lcd.drawStr(40,40,"drawStr测试")
*/
int lcd_set_font(lcd_conf_t* conf, const uint8_t *ptr) {
    u8g2_SetFont(&(conf->lcd_u8g2), ptr);
    return ESP_OK;
}

static uint8_t utf8_state;
static uint16_t encoding;
static uint16_t utf8_next(uint8_t b)
{
  if ( b == 0 )  /* '\n' terminates the string to support the string list procedures */
    return 0x0ffff; /* end of string detected, pending UTF8 is discarded */
  if ( utf8_state == 0 )
  {
    if ( b >= 0xfc )  /* 6 byte sequence */
    {
      utf8_state = 5;
      b &= 1;
    }
    else if ( b >= 0xf8 )
    {
      utf8_state = 4;
      b &= 3;
    }
    else if ( b >= 0xf0 )
    {
      utf8_state = 3;
      b &= 7;
    }
    else if ( b >= 0xe0 )
    {
      utf8_state = 2;
      b &= 15;
    }
    else if ( b >= 0xc0 )
    {
      utf8_state = 1;
      b &= 0x01f;
    }
    else
    {
      /* do nothing, just use the value as encoding */
      return b;
    }
    encoding = b;
    return 0x0fffe;
  }
  else
  {
    utf8_state--;
    /* The case b < 0x080 (an illegal UTF8 encoding) is not checked here. */
    encoding<<=6;
    b &= 0x03f;
    encoding |= b;
    if ( utf8_state != 0 )
      return 0x0fffe; /* nothing to do yet */
  }
  return encoding;
}

/*
显示字符串
@api lcd.drawStr(x,y,str,fg_color)
@int x 横坐标
@int y 竖坐标  注意:此(x,y)为左下起始坐标
@string str 文件内容
@int fg_color str颜色 注意:此参数可选，如不填写则使用之前设置的颜色，绘制只会绘制字体部分，背景需要自己清除
@usage
-- 显示之前先设置为中文字体,对之后的drawStr有效,使用中文字体需在conf_bsp.h.h开启#define USE_U8G2_OPPOSANSMxx_CHINESE xx代表字号
lcd.setFont(lcd.font_opposansm12)
lcd.drawStr(40,10,"drawStr")
sys.wait(2000)
lcd.setFont(lcd.font_opposansm16_chinese)
lcd.drawStr(40,40,"drawStr测试")
*/
int lcd_draw_str(lcd_conf_t* conf, int x, int y, char* str, color_t color)
{
    uint16_t e;
    int16_t delta;
    utf8_state = 0;

    for(;;){
        e = utf8_next((uint8_t)*str);
        if ( e == 0x0ffff )
        break;
        str++;
        if ( e != 0x0fffe ){
            delta = u8g2_font_draw_glyph(&(conf->lcd_u8g2), x, y, e);
            switch(conf->lcd_u8g2.font_decode.dir){
                case 0:
                x += delta;
                break;
                case 1:
                y += delta;
                break;
                case 2:
                x -= delta;
                break;
                case 3:
                y -= delta;
                break;
            }
        }
    }
    lcd_flush(conf);
    return ESP_OK;
}

/**
缓冲区绘制QRCode
@api lcd.drawQrcode(x, y, str, size)
@int x坐标
@int y坐标
@string 二维码的内容
@int 可选,显示大小,不可小于21,默认21
@return nil 无返回值
*/
int lcd_draw_qr_code(lcd_conf_t* conf, int x, int y, char* str, int size)
{
    uint8_t *qrcode = malloc(qrcodegen_BUFFER_LEN_MAX);
    uint8_t *tempBuffer = malloc(qrcodegen_BUFFER_LEN_MAX);
    if (qrcode == NULL || tempBuffer == NULL) {
        if (qrcode)
            free(qrcode);
        if (tempBuffer)
            free(tempBuffer);
        ESP_LOGE(TAG, "qrcode out of memory");
        return 0;
    }
    bool ok = qrcodegen_encodeText(str, tempBuffer, qrcode, qrcodegen_Ecc_MEDIUM,
        qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
    if (ok){
        int qr_size = qrcodegen_getSize(qrcode);
        int scale = size / qr_size ;
        int margin = (size - qr_size * scale) / 2;
        lcd_draw_fill(default_conf,x,y,x+size,y+size,BACK_COLOR);
        x+=margin;
        y+=margin;
        for (int j = 0; j < qr_size; j++) {
            for (int i = 0; i < qr_size; i++) {
                if (qrcodegen_getModule(qrcode, i, j))
                    lcd_draw_fill(default_conf,x+i*scale,y+j*scale,x+(i+1)*scale,y+(j+1)*scale,FORE_COLOR);
            }
        }
    }
    if (qrcode)
        free(qrcode);
    if (tempBuffer)
        free(tempBuffer);
    lcd_flush(default_conf);
    return ESP_OK;
}

/*
绘制位图
@api lcd.drawXbm(x, y, w, h, data)
@int X坐标
@int y坐标
@int 位图宽
@int 位图高
@int 位图数据,每一位代表一个像素
@usage
-- 取模使用PCtoLCD2002软件即可 阴码 逐行 逆向
-- 在(0,0)为左上角,绘制 16x16 "今" 的位图
lcd.drawXbm(0, 0, 16,16, string.char(
    0x80,0x00,0x80,0x00,0x40,0x01,0x20,0x02,0x10,0x04,0x48,0x08,0x84,0x10,0x83,0x60,
    0x00,0x00,0xF8,0x0F,0x00,0x08,0x00,0x04,0x00,0x04,0x00,0x02,0x00,0x01,0x80,0x00
))
*/
int lcd_draw_xbm(lcd_conf_t* conf, int x, int y, int w, int h, char* data, int data_len)
{
    uint8_t mask = 1;
    if (h < 1) return 0; // 行数必须大于0
    if (data_len*8/h < w) return 0; // 起码要填满一行
    int w1 = w/8;
    if (w%8)w1++;
    if (data_len != h*w1)return 0;
    color_t* color_w = malloc(sizeof(color_t) * w);
    for (size_t b = 0; b < h; b++){
      size_t a = 0;
      while (a < w){
        for (size_t c = 0; c < 8; c++){
          if (*data&(mask<<c)){
            color_w[a]=FORE_COLOR;
          }else{
            color_w[a]=BACK_COLOR;
          }
          a++;
          if (a == w)break;
        }
        data++;
      }
      lcd_draw(conf, x, y+b, x+w-1, y+b, color_w);
    }
    free(color_w);
    lcd_flush(conf);
    return ESP_OK;
}


