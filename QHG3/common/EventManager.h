/*============================================================================
| EventManager
| 
|  Manages QHG EventData associated with Trigger objects determining their
|  activation.
| 
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __EVENTMANAGER_H__
#define __EVENTMANAGER_H__

#include <map>
#include <vector>
#include <string>

#include "types.h"
#include "Triggers.h"
#include "EventData.h"

typedef std::map<EventData *, Triggers *, EDCompare> evttrig;


class EventManager {
public:
    EventManager();
   ~EventManager();

    void start();
    void clear();
    int   loadEventItem(EventData *pSD, Triggers *pT, float fCurTime=-1);
    
    bool  hasNewEvent(float fT);

    const std::vector<EventData*> &getEventData();
    const std::vector<EventData*> &force();
    
    void forwardTo(float fStartTime);
    void triggerAll(float fTime);
    void findFinalEvents();

    void toString(std::vector<std::string> &vsCur) const;
protected:
    double findNextEvent();
    void   updateEventList();

    std::vector<EventData*> m_vEDSet;
    
    double m_dNextTime;

    evttrig m_vEDT;
    bool m_bStarted;
};

#endif
