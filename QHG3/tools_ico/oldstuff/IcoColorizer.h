#ifndef __ICOCOLORIZER_H__
#define __ICOCOLORIZER_H__

#ifndef NULL 
  #define NULL 0
#endif


class LookUp;

class IcoColorizer {
public:
    IcoColorizer() : m_bUseColor(true), m_pLookUp(NULL) {};
    void setLookUp(LookUp *pLookUp) { m_pLookUp = pLookUp;};

    virtual void getCol(double dLon, double dLat, float fCol[4])=0; 

    void setUseColor(bool bUseColor) { m_bUseColor = bUseColor;};
    bool getUseColor() { return m_bUseColor; };

    bool m_bUseColor;
    LookUp *m_pLookUp;

};

#endif

