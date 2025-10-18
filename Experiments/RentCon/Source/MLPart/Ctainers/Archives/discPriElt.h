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
//
//////////////////////////////////////////////////////////////////////
// discPriElt.h: interface for the DiscretePriorityElement class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_DISCPRIELT_H__INCLUDED_)
#define _DISCPRIELT_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <abkcommon.h>

template <class T>
class DiscretePriorityElement  
    {
    protected:
        const unsigned   _index;
        const T         *_bareElement;
        DiscretePriorityElement *_prev;
        DiscretePriorityElement *_next;
        int              _priority;
    public:
        DiscretePriorityElement(unsigned         index,
                                const T         *bareElement,
                                int              priority,
                                DiscretePriorityElement *prevElt=NULL);
        virtual ~DiscretePriorityElement();

        unsigned getPriority() const {return _priority;}
        void     setPriority(int pr){_priority=pr;}

        unsigned getIndex() const {return _index;}

        const T *getBareElement() const {return _bareElement;}

        void disconnect(DiscretePriorityElement *&oldPrev,
                        DiscretePriorityElement *&oldNext);

        void insertBefore(DiscretePriorityElement *nextElt);
        void insertAfter(DiscretePriorityElement  *prevElt);
    };
#ifdef _MSC_VER
#define _INSIDE_DISCPRIELT_H
#include "discPriElt.cxx"
#undef _INSIDE_DISCPRIELT_H
#endif

#endif // !defined(_DISCPRIELT_H__INCLUDED_)

