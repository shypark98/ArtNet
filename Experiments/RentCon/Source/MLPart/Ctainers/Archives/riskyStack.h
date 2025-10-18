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

//! author="Mike Oliver 03/14/99 "      

#if !defined(RISKYSTACK_H__INCLUDED_)
#define RISKYSTACK_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector.h>
#include <abkassert.h>

//: this container is intended to be used as a *stack*, 
// not a general sequence like a vector.
// It's like a vector but
//        i) doesn't do bounds checking.  That's the whole
//        point, because bounds checking takes time, and this
//        is intended for tight loops.
//
//        ii) doesn't have the full interface of vector.  this
//        is intentional, as those interfaces could invalidate
//        pointers.  basically, you're allowed to push_back() and
//        pop_back() and look at/modify back() (the top element of the
//        stack), and you can also look at/modify other elements
//        using operator[], but you can't get any iterators
//        to the underlying vector, so you can't sort it or
//        anything like that
template<class T>
class RiskyStack  
    {
//      vector<T> _meat;
//      vector<T>::iterator _end;
        T*        _begin;
        T*        _end;
    public:
        RiskyStack():_meat(2),_end(_meat.begin()){}
        RiskyStack(unsigned maxSize):_meat(maxSize),_end(_meat.begin()){}
        RiskyStack(RiskyStack const &rhs):_meat(rhs._meat),
                                          _end(_meat.begin()+rhs.size()){}
        inline void push_back(const T &val){*(_end++)=val;
                                     abkassert(_end<=_meat.end(),"overrun");}
        inline void pop_back(){_end--;}
        inline size_t size() const {return _end-_meat.begin();}
        inline bool empty() const {return _end==_meat.begin();}
        inline T  operator[](unsigned idx) const {return _meat[idx];}
        inline T &operator[](unsigned idx)       {return _meat[idx];}
        inline T  back() const {return *(_end-1);}
        inline T &back()       {return *(_end-1);}
        inline void reserve(unsigned n);
        inline RiskyStack &operator=(RiskyStack const &rhs)
            {_meat=rhs._meat;_end=_meat.begin()+rhs.size();return *this;}

    };

template<class T>
inline void RiskyStack<T>::reserve(unsigned n)
    {
    unsigned s=_meat.size();
    unsigned oldStackSize=size();
    if (n>s)
        {
        T defaultVal;
        _meat.insert(_meat.end(),n-s,defaultVal);
        _end=_meat.begin()+oldStackSize;
        }
    }

#endif // !defined(RISKYSTACK_H__INCLUDED_)
