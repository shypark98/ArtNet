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
//! CONTACTS=" Mike Igor ABK"
// discPriElt.cxx: implementation of the DiscretePriorityElement class.
//
//////////////////////////////////////////////////////////////////////
#if (!defined(_MSC_VER) || defined(_INSIDE_DISCPRIELT_H))

#ifndef _INSIDE_DISCPRIELT_H
#include "discPriElt.h"
#endif //_INSIDE_DISCPRIELT_H

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

template<class T>
DiscretePriorityElement<T>::DiscretePriorityElement(unsigned index,
                                    const T *bareElement,
                                    int      priority,
                                    DiscretePriorityElement *prevElt):
_index(index),
_bareElement(bareElement),
_prev(prevElt),
_next(NULL),
_priority(priority)
    {
    if (prevElt)
        prevElt->_next = this;
    }

template<class T>
DiscretePriorityElement<T>::~DiscretePriorityElement()
    {    
    }

template<class T>
void DiscretePriorityElement<T>::disconnect(DiscretePriorityElement *&oldPrev,
                                    DiscretePriorityElement *&oldNext)
    {
    oldPrev=_prev;oldNext=_next;
    if (_prev)
        _prev->_next = _next;
    if (_next)
        _next->_prev = _prev;
    _prev=_next=NULL;
    }

template<class T>
void DiscretePriorityElement<T>::insertBefore(DiscretePriorityElement *nextElt)
    {
    abkfatal(!_next && !_prev,"attempt to insert already-connected "
                              "element");

    _next = nextElt;

    if (nextElt)
        {
        _prev = nextElt->_prev;
        nextElt->_prev = this;
        }

    if (_prev)
        _prev->_next = this;
    }

template<class T>
void DiscretePriorityElement<T>::insertAfter(DiscretePriorityElement  *prevElt)
    {
    abkfatal(!_next && !_prev,"attempt to insert already-connected "
                              "element");

    _prev = prevElt;

    if (prevElt)
        {
        _next = prevElt->_next;
        prevElt->_next = this;
        }

    if (_next)
        _next->_prev = this;
    }

#endif // !defined(_MSC_VER) || defined(_INSIDE_DISCPRIELT_H)
