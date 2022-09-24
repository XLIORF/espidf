#include <iostream>
#include "epd1in54.h"
#include "epdpaint.h"
#include "imagedata.h"
#include "esp_log.h"

#define COLORED     0
#define UNCOLORED   1

static char TAG[] = "main";

/**
  * Due to RAM not enough in Arduino UNO, a frame buffer is not allowed.
  * In this case, a smaller image buffer is allocated and you have to 
  * update a partial display several times.
  * 1 byte = 8 pixels, therefore you have to set 8*N pixels at a time.
  */
unsigned char image[1024];
Paint paint(image, 0, 0);    // width should be the multiple of 8 
Epd epd;
unsigned long time_start_ms;
unsigned long time_now_s;

void setup() {
  // put your setup code here, to run once:
  if (epd.Init(lut_full_update) != 0) {
      ESP_LOGE(TAG,"e-Paper init failed");
      return;
  }

  /** 
   *  there are 2 memory areas embedded in the e-paper display
   *  and once the display is refreshed, the memory area will be auto-toggled,
   *  i.e. the next action of SetFrameMemory will set the other memory area
   *  therefore you have to clear the frame memory twice.
   */
  epd.ClearFrameMemory(0x00);   // bit set = white, bit reset = black
  epd.DisplayFrame();
  epd.ClearFrameMemory(0x00);   // bit set = white, bit reset = black
  epd.DisplayFrame();
  ESP_LOGI(TAG,"0");
  paint.SetRotate(ROTATE_0);//指定显示区域
  paint.SetWidth(200);
  paint.SetHeight(200);

  // /* For simplicity, the arguments are explicit numerical coordinates */
  paint.Clear(COLORED);
  paint.DrawStringAt(30, 2, "Perseverance", &Font16, UNCOLORED);//位置，内容
  epd.SetFrameMemory(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
  
  paint.Clear(UNCOLORED);//白色区域内容显示
  paint.DrawStringAt(30, 2, "e-Paper Demo", &Font16, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 0, 16, paint.GetWidth(), paint.GetHeight());

  paint.Clear(COLORED);
  paint.DrawStringAt(20, 2, "Hello world!", &Font16, UNCOLORED);//位置，内容
  epd.SetFrameMemory(paint.GetImage(), 0, 32, paint.GetWidth(), paint.GetHeight());

  paint.SetWidth(64);//指定显示区域
  paint.SetHeight(64);
  
  paint.Clear(UNCOLORED);
  paint.DrawRectangle(0, 0, 40, 50, COLORED);//绘制矩形
  paint.DrawLine(0, 0, 40, 50, COLORED);//绘制线条
  paint.DrawLine(40, 0, 0, 50, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 16, 60, paint.GetWidth(), paint.GetHeight());//显示位置

  paint.Clear(UNCOLORED);
  paint.DrawCircle(32, 32, 30, COLORED);//绘制圆
  epd.SetFrameMemory(paint.GetImage(), 120, 60, paint.GetWidth(), paint.GetHeight());//显示位置

  paint.Clear(UNCOLORED);
  paint.DrawFilledRectangle(0, 0, 40, 50, COLORED);//填充矩形
  epd.SetFrameMemory(paint.GetImage(), 16, 130, paint.GetWidth(), paint.GetHeight());//显示位置

  paint.Clear(UNCOLORED);
  paint.DrawFilledCircle(32, 32, 30, COLORED);//填充圆
  epd.SetFrameMemory(paint.GetImage(), 120, 130, paint.GetWidth(), paint.GetHeight());//显示位置
  epd.DisplayFrame();//显示内容
  ESP_LOGI(TAG,"1");
  // delay(2000);

  // if (epd.Init(lut_partial_update) != 0) {
  //     Serial.print("e-Paper init failed");
  //     return;
  // }

  // /** 
  //  *  there are 2 memory areas embedded in the e-paper display
  //  *  and once the display is refreshed, the memory area will be auto-toggled,
  //  *  i.e. the next action of SetFrameMemory will set the other memory area
  //  *  therefore you have to set the frame memory and refresh the display twice.
  //  */
  epd.SetFrameMemory(IMAGE_DATA);
  epd.DisplayFrame();
  epd.SetFrameMemory(IMAGE_DATA);
  epd.DisplayFrame();
ESP_LOGI(TAG,"2");

}

void loop() {
  // put your main code here, to run repeatedly:
  // time_now_s = (millis() - time_start_ms) / 1000;
  // char time_string[] = {'0', '0', ':', '0', '0', '\0'};
  // time_string[0] = time_now_s / 60 / 10 + '0';
  // time_string[1] = time_now_s / 60 % 10 + '0';
  // time_string[3] = time_now_s % 60 / 10 + '0';
  // time_string[4] = time_now_s % 60 % 10 + '0';

  // paint.SetWidth(32);
  // paint.SetHeight(96);
  // paint.SetRotate(ROTATE_270);

  // paint.Clear(UNCOLORED);
  // paint.DrawStringAt(0, 4, time_string, &Font24, COLORED);
  // epd.SetFrameMemory(paint.GetImage(), 80, 72, paint.GetWidth(), paint.GetHeight());
  // epd.DisplayFrame();

  // paint.Clear(COLORED);
  // epd.SetFrameMemory(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
  // epd.DisplayFrame();

}


extern "C" void app_main(void)
{
    setup();
    // while (true)
    // {
    //     loop();
    //     vTaskDelay(10/portTICK_PERIOD_MS);
    // }
    
}


