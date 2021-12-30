#ifndef LEDText_h
#define LEDText_h

#define  FONT_PROPORTIONAL  0x80

#define  BACKGND_ERASE   0x0000
#define  BACKGND_LEAVE   0x0001
#define  BACKGND_DIMMING 0x0002

#define  CHAR_UP         0x0000
#define  CHAR_DOWN       0x0004
#define  CHAR_LEFT       0x0008
#define  CHAR_RIGHT      0x000c

#define  SCROLL_LEFT     0x0000
#define  SCROLL_RIGHT    0x0010
#define  SCROLL_UP       0x0020
#define  SCROLL_DOWN     0x0030

#define  COLR_RGB        0x0000
#define  COLR_HSV        0x0040

#define  COLR_SINGLE     0x0000
#define  COLR_GRAD       0x0080
#define  COLR_CHAR       0x0000
#define  COLR_AREA       0x0100
#define  COLR_VERT       0x0000
#define  COLR_HORI       0x0200
// Pre combined defines to make life easier
#define  COLR_GRAD_CV    (COLR_GRAD | COLR_CHAR | COLR_VERT)
#define  COLR_GRAD_AV    (COLR_GRAD | COLR_AREA | COLR_VERT)
#define  COLR_GRAD_CH    (COLR_GRAD | COLR_CHAR | COLR_HORI)
#define  COLR_GRAD_AH    (COLR_GRAD | COLR_AREA | COLR_HORI)

#define  COLR_EMPTY      0x0400
#define  COLR_DIMMING    0x0800

#define  INSTANT_OPTIONS_MODE    0x1000

#define  EFFECT_CHAR_UP          "\xd8"
#define  EFFECT_CHAR_DOWN        "\xd9"
#define  EFFECT_CHAR_LEFT        "\xda"
#define  EFFECT_CHAR_RIGHT       "\xdb"

#define  EFFECT_SCROLL_LEFT      "\xdc"
#define  EFFECT_SCROLL_RIGHT     "\xdd"
#define  EFFECT_SCROLL_UP        "\xde"
#define  EFFECT_SCROLL_DOWN      "\xdf"

#define  EFFECT_RGB              "\xe0"
#define  EFFECT_HSV              "\xe1"
#define  EFFECT_RGB_CV           "\xe2"
#define  EFFECT_HSV_CV           "\xe3"
#define  EFFECT_RGB_AV           "\xe6"
#define  EFFECT_HSV_AV           "\xe7"
#define  EFFECT_RGB_CH           "\xea"
#define  EFFECT_HSV_CH           "\xeb"
#define  EFFECT_RGB_AH           "\xee"
#define  EFFECT_HSV_AH           "\xef"
#define  EFFECT_COLR_EMPTY       "\xf0"
#define  EFFECT_COLR_DIMMING     "\xf1"

#define  EFFECT_BACKGND_ERASE    "\xf4"
#define  EFFECT_BACKGND_LEAVE    "\xf5"
#define  EFFECT_BACKGND_DIMMING  "\xf6"

#define  EFFECT_FRAME_RATE       "\xf8"
#define  EFFECT_DELAY_FRAMES     "\xf9"
#define  EFFECT_CUSTOM_RC        "\xfa"


/* Binary constant generator macro By Tom Torfs - donated to the public domain */
/* All macro's evaluate to compile-time constants */
#ifndef HEX__
  #define HEX__(n) 0x##n##LU
#endif
#ifndef B8__
  #define B8__(x) ((x&0x0000000FLU)?1:0) +((x&0x000000F0LU)?2:0) +((x&0x00000F00LU)?4:0) +((x&0x0000F000LU)?8:0) \
                  +((x&0x000F0000LU)?16:0) +((x&0x00F00000LU)?32:0) +((x&0x0F000000LU)?64:0) +((x&0xF0000000LU)?128:0)
  #define B8(d1)        ((uint8_t)B8__(HEX__(d1)))
  #define B16(d1,d2)    ((uint8_t)B8__(HEX__(d1))),((uint8_t)B8__(HEX__(d2)))
  #define B24(d1,d2,d3) ((uint8_t)B8__(HEX__(d1))),((uint8_t)B8__(HEX__(d2))),((uint8_t)B8__(HEX__(d3))))
#endif


class cLEDText
{
  public:
    void SetFont(const uint8_t *FontData);
    void Init(cLEDMatrixBase *Matrix, uint16_t Width, uint16_t Height, int16_t OriginX = 0, int16_t OriginY = 0);
    void SetBackgroundMode(uint16_t Options, uint8_t Dimming = 0x00);
    void SetScrollDirection(uint16_t Options);
    void SetTextDirection(uint16_t Options);
    void SetTextColrOptions(uint16_t Options, uint8_t ColA1 = 0xff, uint8_t ColA2 = 0xff, uint8_t ColA3 = 0xff, uint8_t ColB1 = 0xff, uint8_t ColB2 = 0xff, uint8_t ColB3 = 0xff);
    void SetFrameRate(uint8_t Rate);
    void SetOptionsChangeMode(uint16_t Options);
    void SetText(unsigned char *Txt, uint16_t TxtSize);
    int UpdateText();
    uint8_t FontWidth()  { return(m_FontWidth); };
    uint8_t FontHeight() { return(m_FontHeight); };
  private:
    void DecodeOptions(uint16_t *tp, uint16_t *opt, uint8_t *backDim, uint8_t *col1, uint8_t *col2, uint8_t *colDim, uint8_t *RC);

    cLEDMatrixBase *m_Matrix;
    uint8_t m_FontWidth, m_FontHeight, m_FontBase, m_FontUpper, m_FWBytes;
    uint16_t m_FCBytes;
    const uint8_t *m_FontData;
    int16_t m_XMin, m_XMax, m_YMin, m_YMax;
    unsigned char *m_pText;
    uint16_t m_pSize, m_TextPos, m_Options, m_EOLtp;
    uint8_t m_XBitPos, m_YBitPos;
    uint8_t m_BackDim, m_ColDim, m_Col1[3], m_Col2[3], m_FrameRate;
    uint16_t m_LastDelayTP, m_LastCustomRCTP, m_DelayCounter;
    bool m_FProp, Initialised;
};

#endif
