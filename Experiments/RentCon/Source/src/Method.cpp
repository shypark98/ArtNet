/**************************************************************************
***    
*** Copyright (c) 2008 Regents of the University of California,
***               Andrew B. Kahng, Kwangok Jeong and Hailong Yao
***
***  Contact author(s): abk@cs.ucsd.edu, kjeong@vlsicad.ucsd.edu, hailong@cs.ucsd.edu
***  Original Affiliation:   UCSD, Computer Science and Engineering Department,
***                          La Jolla, CA 92093-0404 USA
***
***  Permission is hereby granted, free of charge, to any person obtaining 
***  a copy of this software and associated documentation files (the
***  "Software"), to deal in the Software without restriction, including
***  without limitation 
***  the rights to use, copy, modify, merge, publish, distribute, sublicense, 
***  and/or sell copies of the Software, and to permit persons to whom the 
***  Software is furnished to do so, subject to the following conditions:
***
***  The above copyright notice and this permission notice shall be included
***  in all copies or substantial portions of the Software.
***
*** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
*** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
*** OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
*** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
*** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
*** OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
*** THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***
***
***************************************************************************/

/***************************
*** Created by Hailong Yao
*** hailong@cs.ucsd.edu
*** Date: Jul. 30, 2008
***************************/

#include <iostream>
#include <cstdio>
#include <fstream>
#include <ctime>
#include <cmath>
#include <gsl/gsl_fit.h>
// Mateus@180515
#include <algorithm>
//--------------

#include "Method.h"
#include "ABKCommon/abkassert.h"

using namespace std;

namespace DESIGN {

  void Method::print(FILE *file)
  {
    fprintf(file, "Method %d : %s\n", id, msg.c_str());
    for (int i = 0; i < modules.size(); ++ i)
    {
    	Module *m = modules[i];
    	fprintf(file, "Module %d, arithmetic avg gate: %f, k: %f, T1: %f, T2: %f, geometric avg gate: %f, k: %f, T1: %f, T2: %f\n", i, m->getAvgG(), avgK, m->getAvgT1(), m->getAvgT2(), m->getGeomAvgG(), geomAvgK, m->getGeomAvgT1(), m->getGeomAvgT2());
    }
    fprintf(file, "Arithmetic avg pins per gate: %f, geometric avg pins per gate: %f\n", avgK, geomAvgK);
    fprintf(file, "----------------- Arithmetic Rent's parameters -----------------\n");
    fprintf(file, "(1) Type 1 pin counting: %f, (2) Type 2 pin counting: %f\n", rentP1, rentP2);
    fprintf(file, "----------------- Geometric Rent's parameters -----------------\n");
    fprintf(file, "(1) Type 1 pin counting: %f, (2) Type 2 pin counting: %f\n\n", geomRentP1, geomRentP2);
  }

  void Method::printRents(FILE *file)
  {
    fprintf(file, "Method %d : %s\n", id, msg.c_str());
    fprintf(file, "Arithmetic avg pins per gate: %f, geometric avg pins per gate: %f\n", avgK, geomAvgK);
    fprintf(file, "----------------- Arithmetic Rent's parameters -----------------\n");
    fprintf(file, "(1) Type 1 pin counting: %f, (2) Type 2 pin counting: %f, (3) Type 3 pin counting: %f\n", rentP1, rentP2, rentP3);
    fprintf(file, "----------------- Geometric Rent's parameters -----------------\n");
    fprintf(file, "(1) Type 1 pin counting: %f, (2) Type 2 pin counting: %f, (3) Type 3 pin counting: %f\n\n", geomRentP1, geomRentP2, geomRentP3);
  }
  
  void Method::printModules(FILE *file)
  {
    fprintf(file, "Method %d : %s\n", id, msg.c_str());
    fprintf(file, "----------------- Total %d modules -----------------\n", modules.size());
    for (int i = 0; i < modules.size(); ++ i)
    {
    	Module *m = modules[i];
    	fprintf(file, "Module %d, arithmetic avg gate: %f, k: %f, T1: %f, T2: %f, T3: %f, geometric avg gate: %f, k: %f, T1: %f, T2: %f, T3: %f\n", i, m->getAvgG(), avgK, m->getAvgT1(), m->getAvgT2(), m->getAvgT3(),m->getGeomAvgG(), geomAvgK, m->getGeomAvgT1(), m->getGeomAvgT2(), m->getGeomAvgT3());
    }
  }

	void Method::clearMem()
	{
	}

	/*
	* y(x)-b = p * x
	*/
	double Method::fitRent(double *x, double *y, int n)
	{
		int minPntNum = (int)(n*0.75);
	  double bestRent;
	  int bestN = n;
	  double rentP, cov11, sumsq;
	  
	  fprintf(stdout, "Points to fit: \n");
	  for (int i = 0; i < n; ++ i)
	  {
	  	fprintf(stdout, "[%f, %f]\n", x[i], y[i]);
	  }
	  
		fprintf(stdout, "Minimum number of data points = %d\n", minPntNum);
		gsl_fit_mul(x, 1, y, 1, n, &rentP, &cov11, &sumsq);
		bestRent = rentP;
		//compute the standard deviation of the residuals
		abkassert(n > 0 && sumsq >= 0, "Error in fitRent()");
		double oldDev = sqrt(sumsq/n);
		fprintf(stdout, "#points = %d, Rent's p = %f, standard deviation of the residuals: %f\n", n, rentP, oldDev);
		
		while (n > minPntNum)
		{
			n --;
			gsl_fit_mul(x, 1, y, 1, n, &rentP, &cov11, &sumsq);
			//compute the standard deviation of the residuals
			double newDev = sqrt(sumsq/n);
			fprintf(stdout, "#points = %d, Rent's p = %f, standard deviation of the residuals: %f\n", n, rentP, newDev);
			if (newDev > oldDev)
				break;
			else
			{
				oldDev = newDev;
				bestN = n;
				bestRent = rentP;
			}
		}
	  
		fprintf(stdout, "Final #points = %d, Rent's p = %f\n", n, rentP);
	  
	  return rentP;
	}

	/*
	 * T = k * m^p => log(T) = log(k) + p*log(m)
	 */
	void Method::linCurvFit()
	{
		int n = modules.size();
	  double *x = new double[n];
	  double *y1 = new double[n];
	  double *y2 = new double[n];
	  double *y3 = new double[n];
		
		sort(modules.begin(), modules.end(), ModuleAvgGS2L());
    double b = log(avgK);
    for (int i = 0; i < n; i ++)
    {
    	Module *m = modules[i];
//    	fprintf(stdout, "Module %d, Gate: %f, k: %f, T1: %f, T2: %f\n", i, m->getAvgG(), avgK, m->getAvgT1(), m->getAvgT2());
			abkassert(m->getAvgG() > 0 && m->getAvgT1() > 0 && m->getAvgT2() > 0 && m->getAvgT3() > 0, "Error in linCurvFit()");
    	x[i] = log(m->getAvgG());
    	y1[i] = log(m->getAvgT1()) - b;
    	y2[i] = log(m->getAvgT2()) - b;
    	y3[i] = log(m->getAvgT3()) - b;
    }
    
    fprintf(stdout, "%s\n", msg.c_str());
    fprintf(stdout, "Start to fit arithmetic Rent's parameter in Region I ...\n");
    fprintf(stdout, "-------- Type I Rent's parameter --------\n");
		rentP1 = fitRent(x, y1, n);
    fprintf(stdout, "-------- Type II Rent's parameter --------\n");
		rentP2 = fitRent(x, y2, n);
    fprintf(stdout, "-------- Type III Rent's parameter --------\n");
		rentP3 = fitRent(x, y3, n);
//		fprintf(stdout, "Rent1: %f, Rent2: %f, Rent3: %f, Rent4: %f\n", rentP1, rentP2, rentP3, rentP4);
		
		sort(modules.begin(), modules.end(), ModuleGeomAvgGS2L());
    b = log(geomAvgK);
    for (int i = 0; i < n; i ++)
    {
    	Module *m = modules[i];
//    	fprintf(stdout, "Module %d, Gate: %f, k: %f, T1: %f, T2: %f\n", i, m->getGeomAvgG(), geomAvgK, m->getGeomAvgT1(), m->getGeomAvgT2());
			abkassert(m->getGeomAvgG() > 0 && m->getGeomAvgT1() > 0 && m->getGeomAvgT2() > 0 && m->getGeomAvgT3() > 0, "Error in linCurvFit()");
    	x[i] = log(m->getGeomAvgG());
    	y1[i] = log(m->getGeomAvgT1()) - b;
    	y2[i] = log(m->getGeomAvgT2()) - b;
    	y3[i] = log(m->getGeomAvgT3()) - b;
    }
    
    fprintf(stdout, "Start to fit geometric Rent's parameter in Region I ...\n");
    fprintf(stdout, "-------- Type I Rent's parameter --------\n");
		geomRentP1 = fitRent(x, y1, n);
    fprintf(stdout, "-------- Type II Rent's parameter --------\n");
		geomRentP2 = fitRent(x, y2, n);
    fprintf(stdout, "-------- Type III Rent's parameter --------\n");
		geomRentP3 = fitRent(x, y3, n);
//		fprintf(stdout, "Geom Rent1: %f, Geom Rent2: %f, Geom Rent3: %f, Geom Rent4: %f\n", geomRentP1, geomRentP2, geomRentP3, geomRentP4);
		
		delete x;
		delete y1;
		delete y2;
		delete y3;
  }
}
