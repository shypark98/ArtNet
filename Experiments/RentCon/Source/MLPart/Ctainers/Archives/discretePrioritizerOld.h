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
// discretePrioritizer.h: interface for the DiscretePrioritizer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_DISCRETE_PRIORITIZER_H_INCLUDED_)
#define _DISCRETE_PRIORITIZER_H_INCLUDED_

#include <map.h>
#include <vector.h>

#define DiscretePrioritizerVecType vector<DiscretePriorityElement<T>* >

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "discPriElt.h"
 
template<class T>
class DiscretePrioritizer  
    {
    protected:

        DiscretePrioritizerVecType _eltVec;
        
        // The following arrays are parallel.  _heads[i] points to
        // the beginning of the linked list with priority i; _tails[i]
        // points to the last element of that list.
        vector<DiscretePriorityElement<T>*> _heads;
        vector<DiscretePriorityElement<T>*> _tails;
        int                         _maxPriority;

        DiscretePriorityElement<T> *_disconnect(unsigned elementIdx,
                                                int   newPriority);
    public:
        DiscretePrioritizer(vector<const T*> elements,
                            vector<unsigned> priorities);

        virtual ~DiscretePrioritizer();

        const T *top() const 
            {
            if (_maxPriority<0) return NULL;
            else return _heads[_maxPriority] ->getBareElement();
            }

        unsigned topEltIndex() const {if (_maxPriority<0) return UINT_MAX;
                                else return _heads[_maxPriority]->getIndex();}
                               

        bool reQueueToHead(unsigned elementIdx, int newPriority);
        bool reQueueToTail(unsigned elementIdx, int newPriority);
        bool erase(unsigned elementIdx);

        unsigned getEltIndex(const T *element) const;
    };

#ifdef _MSC_VER
#define _INSIDE_DISCRETE_PRIORITIZER_H
#include "discretePrioritizer.cxx"
#undef _INSIDE_DISCRETE_PRIORITIZER_H
#endif

#endif // !defined(_DISCRETE_PRIORITIZER_H_INCLUDED_)
