///////////////////////////////////////////////////////////////////////////
//
// BSD 3-Clause License
//
// Copyright (c) 2025, Seonghyeon Park and the Regents of the University of California
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <cmath>

#include "ang/artNetGen.h"

namespace ang {

using std::abs;
using std::ceil;
using std::cout;
using std::endl;
using std::lower_bound;
using std::max;
using std::min;
using std::rand;
using std::random_shuffle;
using std::string;
using std::vector;

DistUnit::~DistUnit()
{
  clear();
}

void DistUnit::clear()
{
  value_ = 0;
  target_ = 0;
  current_ = 0;
  ratio_ = 0;
}

Distrib::Distrib()
{
}

Distrib::Distrib(Distrib* dist)
{
  /*
   min_ = dist->getMin();
   max_ = dist->getMax();

   totCnt_ = dist->getTotCnt();

   for(auto dit : dist) {

       int val = dit.getValue();
       int cur = dit.getCurrent();
       int tar = dit.getTarget();
       double rat = dit.getRatio();
       units_.push_back(DistUnit(val, targ, cur, rat));
   }
   */
}

Distrib::~Distrib()
{
  clear();
}

// initialize distribution
void Distrib::init(vector<DistUnit> units)
{
  int min = INT_MAX;
  int max = INT_MIN;
  int totCnt = 0;

  int i = 0;

  for (auto unit : units) {
    int value = unit.getValue();
    int target = unit.getTarget();
    int current = unit.getCurrent();
    double ratio = unit.getRatio();

    min = std::min(min, target);
    max = std::max(max, target);
    totCnt += target;

    while (i < value) {
      units_.push_back(DistUnit(i, 0, 0, 0.0));
      i++;
    }

    units_.push_back(DistUnit(value, target, current, ratio));
    i++;
  }

  min_ = min;
  max_ = max;
  totCnt_ = totCnt;
}

void Distrib::refine(int totCnt)
{
  if (totCnt != totCnt_) {
    int total = 0;
    for (auto unit : units_) {
      int target = int(unit.getTarget() * (1.0 * totCnt / totCnt_));
      total += target;
    }
    if (total > totCnt_) {
      while (total != totCnt_) {
        auto it = units_.begin();
        std::advance(it, rand() % units_.size());
        if (it->getTarget() > 0) {
          int value = it->getValue();
          it->tarDecr(value);
          total--;
        }
      }
    } else {
      while (total != totCnt_) {
        auto it = units_.begin();
        std::advance(it, rand() % units_.size());
        if (it->getTarget() > 0) {
          int value = it->getValue();
          it->tarDecr(value);
          total--;
        }
      }
    }

    totCnt_ = totCnt;
  }

  for (auto unit : units_) {
    double ratio = unit.getTarget() / totCnt_;
  }
}

void Distrib::clear()
{
  min_ = 0;
  max_ = 0;
  totCnt_ = 0;
  units_.clear();
}

// get average value of target distribution
double Distrib::getTarAvg() const
{
  double avg = 0.0;

  for (auto& it : units_) {
    avg += it.getValue() * it.getTarget();
  }

  avg /= totCnt_;

  return avg;
}

// get average value of current distribution
double Distrib::getCurAvg() const
{
  double avg = 0.0;

  for (auto& it : units_) {
    avg += it.getValue() * it.getCurrent();
  }

  avg /= totCnt_;

  return avg;
}

// get target ratio of the given value
double Distrib::getTarRatio(int x) const
{
  int targetCnt = getTarget(x);
  double ratio = 1.0 * targetCnt / totCnt_;
  return ratio;
}

// get current ratio of the given value
double Distrib::getCurRatio(int x) const
{
  int currentCnt = getCurrent(x);
  double ratio = 1.0 * currentCnt / totCnt_;
  return ratio;
}

// increase target value
void Distrib::tarIncr(int x, int cnt)
{
  units_[x].tarIncr(cnt);
}

// decrease target value
void Distrib::tarDecr(int x, int cnt)
{
  units_[x].tarDecr(cnt);
}

// increase current value
void Distrib::curIncr(int x, int cnt)
{
  units_[x].curIncr(cnt);
}

// decrease current value
void Distrib::curDecr(int x, int cnt)
{
  units_[x].curDecr(cnt);
}

double Distrib::ratioError(int value) const
{
  double tarRatio = 1.0 * getTarget(value) / totCnt_;
  double curRatio = 1.0 * getCurrent(value) / totCnt_;

  return curRatio - tarRatio;
}

double Distrib::delRatio(int value, int delta) const
{
  int curCnt = getCurrent(value);
  int nextCnt = getCurrent(value) + delta;
  int tarCnt = getTarget(value);

  double curRatio = 1.0 * curCnt / totCnt_;
  double nextRatio = 1.0 * nextCnt / totCnt_;
  double tarRatio = 1.0 * tarCnt / totCnt_;

  double curError = abs(curRatio - tarRatio);
  double nextError = abs(nextRatio - tarRatio);

  double delError = curError - nextError;

  return delError;
}

vector<int> Distrib::rouletteVector()
{
  vector<int> rVector;

  for (int value = 0; value <= max_; ++value) {
    int tarCnt = getTarget(value);
    rVector.insert(rVector.end(), tarCnt, value);
  }

  random_shuffle(rVector.begin(), rVector.end());

  return rVector;
}

}  // namespace ang
