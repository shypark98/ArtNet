/**************************************************************************
***    
***    Copyright (c) 1995-1999 by ABKGroup, UCLA VLSI CAD Laboratory,
***    UCLA Computer Science Department, Los Angeles, CA 90095-1596 USA, 
***    and by the Regents of the University of California.    
***    All rights reserved.   
***
***    No portion of this package may be used, copied, or transmitted for
***    any purpose, including any research or commercial purpose,
***    without the express written consent of Professor Andrew B. Kahng,
***    UCLA Computer Science Department, 3713 Boelter Hall, Los Angeles,
***    CA 90095-1596 USA.   Any copy made of this file must include this
***    notice.    ABKGroup and the Regents of the University of California
***    retain title to all copies of this file and/or its compiled
***    derivatives.
***
***************************************************************************/
//! author="Mike Oliver 1999.4.27"
// discretePrioritizer.cxx: implementation of the DiscretePrioritizer class.
//
//////////////////////////////////////////////////////////////////////
#if (!defined(_MSC_VER) || defined(_INSIDE_DISCRETE_PRIORITIZER_H))

#ifndef _INSIDE_DISCRETE_PRIORITIZER_H
#include "discretePrioritizer.h"
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
template <class T>
DiscretePrioritizer<T>::DiscretePrioritizer(vector<const T*> elements,
                            vector<unsigned> priorities):
                            _eltVec(elements.size(),
                              static_cast<DiscretePriorityElement<T>*>(NULL))
    {
    abkfatal(elements.size()==priorities.size(),"Mismatched vector sizes");

    // scan for max priority
    _maxPriority=-1;
    unsigned i;
    for (i=0;i<priorities.size();i++)
        {
        if (signed(priorities[i])>_maxPriority)
            _maxPriority=priorities[i];
        }

    _heads.clear();
    _tails.clear();
    _heads.insert(_heads.end(),_maxPriority+1,
        static_cast<DiscretePriorityElement<T>*>(NULL));
    _tails.insert(_tails.end(),_maxPriority+1,
        static_cast<DiscretePriorityElement<T>*>(NULL));

    for (i=0;i<elements.size();i++)
        {
        unsigned priority = priorities[i];
        const T *pOrigElt = elements[i];
        DiscretePriorityElement<T> *pPrElt = new DiscretePriorityElement<T>(i,
                                                          pOrigElt,
                                                          priority,
                                                          _tails[priority]);
        if (!_heads[priority])
            _heads[priority] = pPrElt;

        _tails[priority] = pPrElt;

        _eltVec[i] = pPrElt;
        }
    
    }

template <class T>
DiscretePrioritizer<T>::~DiscretePrioritizer()
    {
    DiscretePrioritizerVecType::iterator iM = _eltVec.begin();
    for (;iM!=_eltVec.end();iM++)
        {
        delete (*iM);
        }
    
    }


template <class T>
DiscretePriorityElement<T> *DiscretePrioritizer<T>::_disconnect
                                               (unsigned elementIdx,
                                                int      newPriority)
    {
    DiscretePriorityElement<T> *pPrElt = _eltVec[elementIdx];

    if (!pPrElt)
        return static_cast<DiscretePriorityElement<T>*>(NULL);

    unsigned oldPriority = pPrElt->getPriority();

    DiscretePriorityElement<T> *oldPrev,*oldNext;
    pPrElt->disconnect(oldPrev,oldNext);

    if (!oldPrev)
        _heads[oldPriority] = oldNext;

    if (!oldNext)
        _tails[oldPriority] = oldPrev;

    if (newPriority > _maxPriority)
        {
        _heads.insert(_heads.end(),newPriority-_maxPriority+1,
            static_cast<DiscretePriorityElement<T>*>(NULL));
        _tails.insert(_tails.end(),newPriority-_maxPriority+1,
            static_cast<DiscretePriorityElement<T>*>(NULL));
        }

    else
        {  // if we emptied this priority level, scan down for new max
        while (_maxPriority>newPriority && !_heads[_maxPriority])
            --_maxPriority;
        }

    return pPrElt;
    }

template <class T>
bool DiscretePrioritizer<T>::reQueueToHead(unsigned elementIdx,
                                           int      newPriority)
    {
    DiscretePriorityElement<T> *pPrElt = _disconnect(elementIdx,newPriority);

    if (!pPrElt)
        return false; // element not found

    pPrElt->setPriority(newPriority);

    pPrElt->insertBefore(_heads[newPriority]);
    _heads[newPriority] = pPrElt;

    return true;
    }
template <class T>
bool DiscretePrioritizer<T>::reQueueToTail(unsigned elementIdx,
                                   int      newPriority)
    {
    DiscretePriorityElement<T> *pPrElt = _disconnect(elementIdx,newPriority);

    if (!pPrElt)
        return false; // element not found

    pPrElt->setPriority(newPriority);

    pPrElt->insertAfter(_tails[newPriority]);
    _tails[newPriority] = pPrElt;

    return true;
    }

template <class T>
bool DiscretePrioritizer<T>::erase(unsigned elementIdx)
    {
    DiscretePriorityElement<T> *pPrElt = _disconnect(elementIdx,-1);

    if (!pPrElt)
        return false; // element not found

    delete pPrElt;

    _eltVec[elementIdx]=NULL;


    return true;
    }

#endif // !defined(_MSC_VER) || defined(_INSIDE_DISCRETE_PRIORITIZER_H)

