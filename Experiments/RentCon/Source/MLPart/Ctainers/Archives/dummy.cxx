#include "priElt.h"
#include "prioritizer.h"

void dummyPrioritizerInstantiatorDontEvenThinkAboutUsing(void)
    {
    vector<const Empty*> v; 
    vector<unsigned> u; 
    Prioritizer<Empty> p(v,u);
    vector<const Empty*>::iterator iE = v.begin();
    p.reQueueToHead(*iE,7);
    p.reQueueToTail(*iE,2);
    p.erase(*iE);

    p.reQueueToHead(4,2);
    p.reQueueToTail(1,3);
    p.erase(7);
    p.top(); 
    }
 