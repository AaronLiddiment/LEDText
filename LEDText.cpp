/*
LEDText V6 class by Aaron Liddiment (c) 2015

Uses my LEDMatrix class and especially the 

FastLED v3.1 library by Daniel Garcia and Mark Kriegsmann.
Written & tested on a Teensy 3.1 using Arduino V1.6.3 & teensyduino V1.22

Even the basic examples need 12k rom & 4k ram 
*/

#include <FastLED.h>
#include <LEDMatrix.h>
#include <LEDText.h>

#define  BACKGND_MASK    (BACKGND_ERASE | BACKGND_LEAVE | BACKGND_DIMMING)
#define  CHAR_MASK       (CHAR_UP | CHAR_DOWN | CHAR_LEFT | CHAR_RIGHT)
#define  SCROLL_MASK     (SCROLL_LEFT | SCROLL_RIGHT | SCROLL_UP | SCROLL_DOWN)
#define  COLR_MASK       (COLR_RGB | COLR_HSV | COLR_SINGLE | COLR_GRAD | COLR_CHAR | COLR_AREA | COLR_VERT | COLR_HORI | COLR_EMPTY | COLR_DIMMING)

#define  UC_CHAR_UP            0xd8
#define  UC_CHAR_DOWN          0xd9
#define  UC_CHAR_LEFT          0xda
#define  UC_CHAR_RIGHT         0xdb

#define  UC_SCROLL_LEFT        0xdc
#define  UC_SCROLL_RIGHT       0xdd
#define  UC_SCROLL_UP          0xde
#define  UC_SCROLL_DOWN        0xdf

#define  UC_RGB                0xe0
#define  UC_HSV                0xe1
#define  UC_RGB_CV             0xe2
#define  UC_HSV_CV             0xe3
#define  UC_RGB_AV             0xe6
#define  UC_HSV_AV             0xe7
#define  UC_RGB_CH             0xea
#define  UC_HSV_CH             0xeb
#define  UC_RGB_AH             0xee
#define  UC_HSV_AH             0xef
#define  UC_COLR_EMPTY         0xf0
#define  UC_COLR_DIMMING       0xf1

#define  UC_BACKGND_ERASE      0xf4
#define  UC_BACKGND_LEAVE      0xf5
#define  UC_BACKGND_DIMMING    0xf6

#define  UC_FRAME_RATE         0xf8
#define  UC_DELAY_FRAMES       0xf9
#define  UC_CUSTOM_RC          0xfa


void cLEDText::SetFont(const uint8_t *FontData)
{
  m_FontHeight = FontData[1];
  m_FontBase = FontData[2];
  m_FontUpper = FontData[3];
  m_FontData = &FontData[4];
  if ((FontData[0] & FONT_PROPORTIONAL) == FONT_PROPORTIONAL)
  {
    m_FontWidth = FontData[0] & 0x7f;
    m_FCBytes = 1;
    m_FProp = true;
  }
  else
  {
    m_FontWidth = FontData[0];
    m_FCBytes = 0;
    m_FProp = false;
  }
  m_FWBytes = (m_FontWidth + 7) / 8;
  m_FCBytes += (m_FWBytes * m_FontHeight);
}


void cLEDText::Init(cLEDMatrixBase *Matrix, uint16_t Width, uint16_t Height, int16_t OriginX, int16_t OriginY)
{
  m_Matrix = Matrix;
  m_XMin = OriginX;
  m_YMin = OriginY;
  m_XMax = OriginX + Width - 1;
  m_YMax = OriginY + Height - 1;
  m_Options = (BACKGND_ERASE | CHAR_UP | SCROLL_LEFT | COLR_RGB);
  m_Col1[0] = m_Col1[1] = m_Col1[2] = 255;
  m_LastDelayTP = m_LastCustomRCTP = m_DelayCounter = 0;
  m_FrameRate = 0;
  Initialised = true;
}


void cLEDText::SetBackgroundMode(uint16_t Options, uint8_t Dimming)
{
  m_Options = (m_Options & (~BACKGND_MASK)) | (Options & BACKGND_MASK);
  if ((m_Options & BACKGND_MASK) == BACKGND_DIMMING)
    m_BackDim = Dimming;
}


void cLEDText::SetScrollDirection(uint16_t Options)
{
  m_Options = (m_Options & (~SCROLL_MASK)) | (Options & SCROLL_MASK);
}


void cLEDText::SetTextDirection(uint16_t Options)
{
  m_Options = (m_Options & (~CHAR_MASK)) | (Options & CHAR_MASK);
}


void cLEDText::SetTextColrOptions(uint16_t Options, uint8_t ColA1, uint8_t ColA2, uint8_t ColA3, uint8_t ColB1, uint8_t ColB2, uint8_t ColB3)
{
  m_Options = (m_Options & (~COLR_MASK)) | (Options & COLR_MASK);
  if ((m_Options & COLR_EMPTY) != COLR_EMPTY)
  {
    if ((m_Options & COLR_DIMMING) == COLR_DIMMING)
      m_ColDim = ColA1;
    else
    {
      if ((m_Options & COLR_GRAD) == COLR_GRAD)
      {
        m_Col2[0] = ColB1;
        m_Col2[1] = ColB2;
        m_Col2[2] = ColB3;
      }
      m_Col1[0] = ColA1;
      m_Col1[1] = ColA2;
      m_Col1[2] = ColA3;
    }
  }
}


void cLEDText::SetFrameRate(uint8_t Rate)
{
  m_FrameRate = Rate;
}


void cLEDText::SetOptionsChangeMode(uint16_t Options)
{
  m_Options = (m_Options & (~INSTANT_OPTIONS_MODE)) | (Options & INSTANT_OPTIONS_MODE);
}


void cLEDText::SetText(unsigned char *Txt, uint16_t TxtSize)
{
  m_pText = Txt;
  m_pSize = TxtSize;
  m_TextPos = m_EOLtp = m_XBitPos = m_YBitPos = 0;
  m_LastDelayTP = m_LastCustomRCTP = 0;
  Initialised = true;
}


void cLEDText::DecodeOptions(uint16_t *tp, uint16_t *opt, uint8_t *backDim, uint8_t *col1, uint8_t *col2, uint8_t *colDim, uint8_t *RC)
{
  switch (m_pText[*tp])
  {
    case UC_BACKGND_ERASE:
    case UC_BACKGND_LEAVE:
      *opt = (*opt & (~BACKGND_MASK)) | ((uint16_t)m_pText[*tp] & BACKGND_MASK);
      break;
    case UC_BACKGND_DIMMING:
      *opt = (*opt & (~BACKGND_MASK)) | ((uint16_t)m_pText[*tp] & BACKGND_MASK);
      *backDim = (uint8_t)m_pText[*tp + 1];
      *tp += 1;
      break;
    case UC_FRAME_RATE:
      m_FrameRate = (uint8_t)m_pText[*tp + 1];
      *tp += 1;
      break;
    case UC_DELAY_FRAMES:
      if (m_LastDelayTP < *tp)
      {
        m_LastDelayTP = *tp;
        m_DelayCounter = (m_pText[*tp + 1] << 8) + m_pText[*tp + 2];
      }
      else if ( (m_LastDelayTP == *tp) && (m_DelayCounter == 0) )
        m_LastDelayTP++;
      *tp += 2;
      break;
    case UC_CUSTOM_RC:
      if (m_LastCustomRCTP < *tp)
      {
        m_LastCustomRCTP = *tp;
        *RC = m_pText[*tp + 1];
      }
      *tp += 1;
      break;
    case UC_CHAR_UP:
    case UC_CHAR_DOWN:
    case UC_CHAR_LEFT:
    case UC_CHAR_RIGHT:
      *opt = (*opt & (~CHAR_MASK)) | ((((uint16_t)m_pText[*tp] & 0x03) << 2) & CHAR_MASK);
      break;
    case UC_SCROLL_LEFT:
    case UC_SCROLL_RIGHT:
    case UC_SCROLL_UP:
    case UC_SCROLL_DOWN:
      *opt = (*opt & (~SCROLL_MASK)) | ((((uint16_t)m_pText[*tp] & 0x03) << 4) & SCROLL_MASK);
      break;
    case UC_RGB:
    case UC_HSV:
    case UC_RGB_CV:
    case UC_HSV_CV:
    case UC_RGB_AV:
    case UC_HSV_AV:
    case UC_RGB_CH:
    case UC_HSV_CH:
    case UC_RGB_AH:
    case UC_HSV_AH:
      *opt = (*opt & (~COLR_MASK)) | ((((uint16_t)m_pText[*tp] & 0x0f) << 6) & COLR_MASK);
      col1[0] = m_pText[*tp + 1];
      col1[1] = m_pText[*tp + 2];
      col1[2] = m_pText[*tp + 3];
      *tp += 3;
      if ((*opt & COLR_GRAD) == COLR_GRAD)
      {
        col2[0] = m_pText[*tp + 1];
        col2[1] = m_pText[*tp + 2];
        col2[2] = m_pText[*tp + 3];
        *tp += 3;
      }
      break;
    case UC_COLR_EMPTY:
      *opt = (*opt & (~COLR_MASK)) | COLR_EMPTY;
      break;
    case UC_COLR_DIMMING:
      *opt = (*opt & (~COLR_MASK)) | COLR_DIMMING;
      *colDim = (uint8_t)m_pText[*tp + 1];
      *tp += 1;
      break;
   }
}


int cLEDText::UpdateText()
{
  uint8_t bDim, cDim, c1[3], c2[3], xbp, RC;
  int16_t x, y, MinY, MaxY;
  uint16_t opt, tp, MfractAV, MfractAH;

  RC = 0;
  if (m_TextPos >= m_pSize)
    return(-1);
  MfractAV = 65535 / ((m_YMax - m_YMin) + 1);
  MfractAH = 65535 / ((m_XMax - m_XMin) + 1);
  if (m_DelayCounter == 0)
  {
    if (Initialised == true)
      Initialised = false;
    else
    {
      uint8_t fw;
      if (m_FProp == true)
        fw = m_FontData[(m_pText[m_TextPos] - m_FontBase) * m_FCBytes];
      else
      	fw = m_FontWidth;
      if (((m_Options & SCROLL_MASK) == SCROLL_LEFT) || ((m_Options & SCROLL_MASK) == SCROLL_RIGHT))
      {
        m_XBitPos++;
        if ( ((m_Options & CHAR_MASK) == CHAR_UP) || ((m_Options & CHAR_MASK) == CHAR_DOWN) )
        {
          if (m_XBitPos > fw)
            m_XBitPos = 0;
        }
        else
        {
          if (m_XBitPos > m_FontHeight)
            m_XBitPos = 0;
        }
        if (m_XBitPos == 0)
          m_EOLtp = m_TextPos + 1;
      }
      else if (((m_Options & SCROLL_MASK) == SCROLL_UP) || ((m_Options & SCROLL_MASK) == SCROLL_DOWN))
      {
        m_YBitPos++;
        if ( ((m_Options & CHAR_MASK) == CHAR_UP) || ((m_Options & CHAR_MASK) == CHAR_DOWN) )
        {
          if (m_YBitPos > m_FontHeight)
            m_YBitPos = 0;
        }
        else
        {
          if (m_YBitPos > fw)
            m_YBitPos = 0;
        }
      }
    }
    if ((m_XBitPos == 0) && (m_YBitPos == 0))
    {
      while ( (m_TextPos < m_pSize) && ((m_TextPos < m_EOLtp) || (m_pText[m_TextPos] > m_FontUpper)) )
      {
        if (m_pText[m_TextPos] > m_FontUpper)
          DecodeOptions(&m_TextPos, &m_Options, &m_BackDim, m_Col1, m_Col2, &m_ColDim, &RC);
        m_TextPos++;
      }
    }
  }
  if ( ((m_Options & SCROLL_MASK) != SCROLL_RIGHT) && (m_TextPos >= m_pSize) )
    return(-1);
  tp = m_TextPos;
  opt = m_Options;
  bDim = m_BackDim;
  memcpy(c1, m_Col1, sizeof(c1));
  memcpy(c2, m_Col2, sizeof(c2));
  cDim = m_ColDim;
  if ((opt & SCROLL_MASK) == SCROLL_DOWN)
    MinY = MaxY = m_YMin - m_YBitPos;
  else
    MaxY = MinY = m_YMax + m_YBitPos;
  m_EOLtp = 0;
  do
  {
    xbp = m_XBitPos;
    if ((opt & SCROLL_MASK) == SCROLL_RIGHT)
      x = m_XMax;
    else
      x = m_XMin;
    do
    {
      if ( (tp < m_pSize) && ((m_pText[tp] < m_FontBase) || (m_pText[tp] > m_FontUpper)) )
      {
        uint16_t oldopt = opt;
        DecodeOptions(&tp, &opt, &bDim, c1, c2, &cDim, &RC);
        tp++;
        if ( (tp == (m_LastDelayTP + 3)) && (m_DelayCounter > 0) )
        { // Fix to stop processing codes until delay expired
          tp = m_pSize;
          x = m_XMax + 1;
        }
        else if ((m_Options & INSTANT_OPTIONS_MODE) == INSTANT_OPTIONS_MODE)
          opt = (opt & (~SCROLL_MASK)) | (oldopt & SCROLL_MASK);
        else if ((oldopt & SCROLL_MASK) != (opt & SCROLL_MASK))
        {
          if (m_EOLtp == 0)
            m_EOLtp = tp;
          tp = m_pSize;
          x = m_XMax + 1;
          opt = oldopt;
        }
      }
      else
      {
        uint8_t fw, xbpmax;
        uint16_t fdo = (m_pText[tp] - m_FontBase) * m_FCBytes;
        if (m_FProp == true)
          fw = m_FontData[fdo++];
        else
          fw = m_FontWidth;
        if ((x < 0) || (x >= (*m_Matrix).Width()))
        {
          if ( ((opt & CHAR_MASK) == CHAR_UP) || ((opt & CHAR_MASK) == CHAR_DOWN) )
            xbpmax = fw;
          else
            xbpmax = m_FontHeight;
        }
        else
        {
          uint8_t bf, xgap;
          if ( ((opt & CHAR_MASK) == CHAR_UP) || ((opt & CHAR_MASK) == CHAR_DOWN) )
          {
            if ((opt & SCROLL_MASK) == SCROLL_DOWN)
              MaxY = MinY + m_FontHeight;
            else
              MinY = MaxY - m_FontHeight;
            xbpmax = fw;
            if ((opt & CHAR_MASK) == CHAR_UP)
            {
              if ((opt & SCROLL_MASK) == SCROLL_RIGHT)
              {
                bf = 0x80 >> ((fw - xbp) % 8);
                fdo += ((fw - xbp) / 8);
              }
              else
              {
                bf = 0x80 >> (xbp % 8);
                fdo += (xbp / 8);
              }
              fdo += ((m_FontHeight - 1) * m_FWBytes);
            }
            else
            {
              if ((opt & SCROLL_MASK) == SCROLL_RIGHT)
              {
                bf = 0x80 >> ((xbp - 1) % 8);
                fdo += ((xbp - 1) / 8);
              }
              else
              {
                bf = 0x80 >> (((fw - xbp) - 1) % 8);
                fdo += (((fw - xbp) - 1) / 8);
              }
            }
          }
          else
          {
            if ((opt & SCROLL_MASK) == SCROLL_DOWN)
              MaxY = MinY + fw;
            else
              MinY = MaxY - fw;
            xbpmax = m_FontHeight;
            if ((opt & CHAR_MASK) == CHAR_LEFT)
            {
              bf = 0x80;
              if ((opt & SCROLL_MASK) == SCROLL_RIGHT)
                fdo += ((m_FontHeight - xbp) * m_FWBytes);
              else
                fdo += (xbp * m_FWBytes);
            }
            else
            {
              bf = 0x80 >> ((fw - 1) % 8);
              fdo += ((fw - 1) / 8);
              if ((opt & SCROLL_MASK) == SCROLL_RIGHT)
                fdo += ((xbp - 1) * m_FWBytes);
              else
                fdo += ((m_FontHeight - xbp - 1) * m_FWBytes);
            }
          }
          if ( ((opt & SCROLL_MASK) == SCROLL_RIGHT) || (tp >= m_pSize) )
            xgap = 0;
          else
            xgap = xbpmax;
          uint16_t MfractCV = 65535 / (MaxY - MinY);
          uint16_t MfractCH = 65535 / xbpmax;
          y = MinY - 1;
          while (y <= MaxY)
          {
            if ((y >= 0) && (y < (*m_Matrix).Height()))
            {
              if ( (xbp != xgap) && (y >= MinY) && (y < MaxY) && ((m_FontData[fdo] & bf) != 0x00) )
              {
                if ((opt & COLR_MASK) != COLR_EMPTY)
                {
                  if ((opt & COLR_MASK) == COLR_DIMMING)
                    (*m_Matrix)(x, y).nscale8(cDim);
                  else
                  {
                    uint8_t v[3];
                    if ((opt & COLR_GRAD) == COLR_SINGLE)
                      memcpy(v, c1, sizeof(v));
                    else
                    {
                      uint16_t fract;
                      if ((opt & (COLR_AREA | COLR_HORI)) == (COLR_CHAR | COLR_VERT))
                        fract = (y - MinY) * MfractCV;
                      else if ((opt & (COLR_AREA | COLR_HORI)) == (COLR_AREA | COLR_VERT))
                        fract = (y - m_YMin) * MfractAV;
                      else if ((opt & (COLR_AREA | COLR_HORI)) == (COLR_CHAR | COLR_HORI))
                        fract = xbp * MfractCH;
                      else /* if ((opt & (COLR_AREA | COLR_HORI)) == (COLR_AREA | COLR_HORI)) */
                        fract = (x - m_XMin) * MfractAH;
                      for (int i=0; i<3; i++)
                      {
                        if (c1[i] <= c2[i])
                          v[i] = lerp16by16(c1[i]<<8, c2[i]<<8, fract) >> 8;
                        else
                          v[i] = lerp16by16(c2[i]<<8, c1[i]<<8, ~fract) >> 8;
                      }
                    }
                    if ((opt & COLR_HSV) == COLR_RGB)
                      (*m_Matrix)(x, y) = CRGB(v[0], v[1], v[2]);
                    else
                      (*m_Matrix)(x, y) = CHSV(v[0], v[1], v[2]);
                  }
                }
              }
              else if ( (((opt & SCROLL_MASK) == SCROLL_DOWN) && ( (MinY <= m_YMin) || (y >= MinY)))
                      || (((opt & SCROLL_MASK) != SCROLL_DOWN) && ( (MaxY >= m_YMax) || (y < MaxY))) )
              // Fix for double dimming/blanking of blank vertical gap lines
              {
                if ((opt & BACKGND_MASK) == BACKGND_ERASE)
                  (*m_Matrix)(x, y) = CRGB(0, 0, 0);
                else if ((opt & BACKGND_MASK) == BACKGND_DIMMING)
                  (*m_Matrix)(x, y).nscale8(bDim);
              }
            }
            if ((y >= MinY) && (xbp != xgap))
            {
              if ((opt & CHAR_MASK) == CHAR_UP)
                fdo -= m_FWBytes;
              else if ((opt & CHAR_MASK) == CHAR_DOWN)
                fdo += m_FWBytes;
              else if ((opt & CHAR_MASK) == CHAR_LEFT)
              {
                if (bf == 0x01)
                {
                  bf = 0x80;
                  ++fdo;
                }
                else
                  bf >>= 1;
              }
              else if ((opt & CHAR_MASK) == CHAR_RIGHT)
              {
                if (bf == 0x80)
                {
                  bf = 0x01;
                  --fdo;
                }
                else
                  bf <<= 1;
              }
            }
            ++y;
          }
        }
        ++xbp;
        if (xbp > xbpmax)
        {
          xbp = 0;
          ++tp;
        }
        if ((opt & SCROLL_MASK) != SCROLL_RIGHT)
        {
          ++x;
          if (tp == m_pSize)
            ++tp;
        }
        else
        {
          --x;
          if ((tp == m_pSize) && (xbp > 0))
            ++tp;
        }
      }
    }
    while ((x >= m_XMin) && (x <= m_XMax) && (tp <= m_pSize));
    if (xbp != 0)
      ++tp;
    if (m_EOLtp == 0)
      m_EOLtp = tp;
    if ((opt & SCROLL_MASK) == SCROLL_DOWN)
      y = MinY = MaxY + 1;
    else if ((opt & SCROLL_MASK) == SCROLL_UP)
      y = MaxY = MinY - 1;
    else
      y = m_YMin - 1;
  }
  while ((y >= m_YMin) && (y <= m_YMax) && (tp < m_pSize));
  if (m_DelayCounter > 0)
    m_DelayCounter--;
  else if (m_FrameRate > 0)
    m_DelayCounter = m_FrameRate;
  if ( (m_Options & INSTANT_OPTIONS_MODE) && (tp > m_pSize) && ( ((y >= m_YMin) && (y <= m_YMax)) || ((x >= m_XMin) && (x <= m_XMax)) ) )
    return(-1);
  return(RC);
}
