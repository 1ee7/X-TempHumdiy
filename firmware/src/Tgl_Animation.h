/**********************
   显示小飞人
 ***********************/

 #include <TJpg_Decoder.h>
 
#include "img/pangzi/i0.h"
#include "img/pangzi/i1.h"
#include "img/pangzi/i2.h"
#include "img/pangzi/i3.h"
#include "img/pangzi/i4.h"
#include "img/pangzi/i5.h"
#include "img/pangzi/i6.h"
#include "img/pangzi/i7.h"
#include "img/pangzi/i8.h"
#include "img/pangzi/i9.h"

int Anim = 0;
int AprevTime = 0;
int x = 160, y = 160;
 
void MyShow_ImgAnim()
{ 
  if (millis() - AprevTime > 37) //x ms切换一次
  {
    Anim++;
    AprevTime = millis();
  }
  if (Anim == 10)
    Anim = 0;

  switch (Anim)
  {
    case 0:
      TJpgDec.drawJpg(x, y, i0, sizeof(i0));
      break;
    case 1:
      TJpgDec.drawJpg(x, y, i1, sizeof(i1));
      break;
    case 2:
      TJpgDec.drawJpg(x, y, i2, sizeof(i2));
      break;
    case 3:
      TJpgDec.drawJpg(x, y, i3, sizeof(i3));
      break;
    case 4:
      TJpgDec.drawJpg(x, y, i4, sizeof(i4));
      break;
    case 5:
      TJpgDec.drawJpg(x, y, i5, sizeof(i5));
      break;
    case 6:
      TJpgDec.drawJpg(x, y, i6, sizeof(i6));
      break;
    case 7:
      TJpgDec.drawJpg(x, y, i7, sizeof(i7));
      break;
    case 8:
      TJpgDec.drawJpg(x, y, i8, sizeof(i8));
      break;
    case 9:
      TJpgDec.drawJpg(x, y, i9, sizeof(i9));
      break;
    default:
      Serial.println("显示Anim错误");
      break;
  }
}
