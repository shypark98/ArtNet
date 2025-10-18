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
#include <fstream>
#include <ctime>
#include <cstdio>
#include <iomanip>
#include <unistd.h>
#include <cstring>

#include "Gate.h"
#include "LDIO.h"
#include "Design.h"
#include <new>

#include "ABKCommon/abkcommon.h"
#include "ABKCommon/abktimer.h"
#include "ABKCommon/abkmessagebuf.h"

using namespace DESIGN;
using namespace std;

void print_usage()
{
	cout <<
    " Rent parameter computation program parameters: " << endl <<
    "\t-f <aux file> - auxiliary file for LEF and DEF file names\n"
    "\t-noCP - do not use circuit partitioning method\n"
    "\t-noGT - do not use graph traversal method\n"
    "\t-noRS - do not use rectangle sampling method\n"
    "\t-seedCP <unsigned> - seed for the random number generator used in circuit partitioning method (default: 123)\n"
    "\t-seedGT <unsigned> - seed for the random number generator used in graph traversal method (default: 123)\n"
    "\t-seedRS <unsigned> - seed for the random number generator used in rectangle sampling method (default: 123)\n"
    "\t-verb <1_1_1|1_2_3|etc> - verbosity: <For actions>_<For system resources>_<For major stats>\n"
    "\t-startsPerRun <unsigned> - number of solutions for each run of circuit partitioning process (default: 1)\n"
    "\t-totalRuns <unsigned> - total number of runs of circuit partitioning process (default: 1)\n"
    "\t-sampleNumGT <unsigned> - number of samples for each round in the graph traversal method (default: 3)\n"
    "\t-sampleNumRS <unsigned> - number of samples for each round in the rectangle sampling method (default: 3)\n"
    "\t-sampleSizeNum <unsigned> - total number of rounds in the graph traversal and rectangle sampling methods (default: 30)\n"
    "\t-log <filename>  - print all output to a file instead of the screen\n"
    "\t-h|-help  - print this message\n"
    << endl;
}

int main(int argc, const char *argv[])
{
	int startsPerRun = 1;
	int totalRuns = 1;
	int sampleNumGT = 3;
	int sampleNumRS = 3;
	int sampleSizeNum = 30;
	unsigned seedCP = 123, seedGT = 123, seedRS = 123;
	bool noCP = false, noGT = false, noRS = false;
	
  LDReader reader;
  Design design;
	
	BoolParam hCheck("h", argc, argv);
	BoolParam helpCheck("help", argc, argv);
	if(hCheck.found() || helpCheck.found())
	{
		print_usage();
		exit(0);
	}
  
	BoolParam verbCheck("verb", argc, argv);
	if(verbCheck.found())
	{
		Verbosity verb(argc, argv);
		Verbosity & v = design.getVerb();
    v.setForActions(verb.getForActions());
    v.setForSysRes(verb.getForSysRes());
    v.setForMajStats(verb.getForMajStats());
    if(verb[0] == 0 && verb[1] == 0 && verb[2] == 0)
    {
      fclose(stdout);
      fclose(stderr);
    }
		
		cout<<v<<endl;
	}
  
	Verbosity & v = design.getVerb();
	cout<<"Verbosity: "<<v<<endl;
	
	if (v.getForActions() > 0)
	{
		cout<<"Start to read the aux file"<<endl;
	}

  StringParam log("log", argc, argv);
  if(log.found())
  {
    FILE *logfile = fopen(log, "w");
    abkfatal(logfile != NULL, "Could not open logfile");
    fclose(logfile);
    cout<<"Redirect all stdandard output and error messages to log file"<<endl;
    freopen(log, "a", stdout);
    freopen(log, "a", stderr);
  }
	
	StringParam    auxFileName    ("f",   argc,argv);
	abkfatal(auxFileName.found(),"Usage: prog -f filename.aux \n"
	                              " See -help for more params");
  string auxName(auxFileName);
	if (v.getForMajStats() > 0)
	{
	  cout<<"aux file: "<<auxName<<endl;
	}
  design.readAUX(auxName.c_str());
	if (v.getForActions() > 0)
	{
		cout<<"Finished reading aux file"<<endl;
	}
  
  try
  {
		if (v.getForActions() > 0)
		{
			cout<<"Start to read in command line parameters"<<endl;
		}
		for(int i = 1; i < argc; i++)
		{
			if(strcmp(argv[i], "-startsPerRun") == 0)
			{
				sscanf(argv[++i], "%d", &startsPerRun);
				abkassert(startsPerRun >= 0, "Error in command line parameters: startsPerRun for circuit partitioning method should be a positive integer value");
			}
			if(strcmp(argv[i], "-totalRuns") == 0)
			{
				sscanf(argv[++i], "%d", &totalRuns);
				abkassert(totalRuns >= 0, "Error in command line parameters: totalRuns for circuit partitioning method should be a positive integer value");
			}
			if(strcmp(argv[i], "-sampleNumGT") == 0)
			{
				sscanf(argv[++i], "%d", &sampleNumGT);
				abkassert(sampleNumGT >= 0, "Error in command line parameters: number of samples for graph traversal method should be a positive integer value");
			}
			if(strcmp(argv[i], "-sampleNumRS") == 0)
			{
				sscanf(argv[++i], "%d", &sampleNumRS);
				abkassert(sampleNumRS >= 0, "Error in command line parameters: number of samples for rectangle sampling method should be a positive integer value");
			}
			if(strcmp(argv[i], "-sampleSizeNum") == 0)
			{
				sscanf(argv[++i], "%d", &sampleSizeNum);
				abkassert(sampleSizeNum >= 0, "Error in command line parameters: sampleSizeNum (granularities) for graph traversal/rectangle sampling method should be a positive integer value");
			}
			if(strcmp(argv[i], "-seedCP") == 0)
			{
				int seed;
				sscanf(argv[++i], "%d", &seed);
				abkassert(seed >= 0, "Error in command line parameters: seed for circuit partitioning method should be an unsigned value");
				seedCP = (unsigned)seed;
			}
			if(strcmp(argv[i], "-seedGT") == 0)
			{
				int seed;
				sscanf(argv[++i], "%d", &seed);
				abkassert(seed >= 0, "Error in command line parameters: seed for graph traversal method should be an unsigned value");
				seedGT = (unsigned)seed;
			}
			if(strcmp(argv[i], "-seedRS") == 0)
			{
				int seed;
				sscanf(argv[++i], "%d", &seed);
				abkassert(seed >= 0, "Error in command line parameters: seed for rectangle sampling method should be an unsigned value");
				seedRS = (unsigned)seed;
			}
			if(strcmp(argv[i], "-noCP") == 0)
			{
				noCP = true;
			}
			if(strcmp(argv[i], "-noGT") == 0)
			{
				noGT = true;
			}
			if(strcmp(argv[i], "-noRS") == 0)
			{
				noRS = true;
			}
		}

		if (v.getForMajStats() > 0)
		{
			fprintf(stdout, "number of solutions for each run of circuit partitioning process: %d\n", startsPerRun);
			fprintf(stdout, "total nubmer of runs of for the circuit partitioning process: %d\n", totalRuns);
			fprintf(stdout, "number of samples for each round in the graph traversal method: %d\n", sampleNumGT);
			fprintf(stdout, "number of samples for each round in the rectangle sampling method: %d\n", sampleNumRS);
			fprintf(stdout, "total number of rounds in the graph traversal and rectangle sampling methods: %d\n", sampleSizeNum);
			fprintf(stdout, "Seed for circuit partitioning method: %u\n", seedCP);
			fprintf(stdout, "Seed for graph traversal method: %u\n", seedGT);
			fprintf(stdout, "Seed for rectangle sampling method: %u\n", seedRS);
			if(noCP)
			{
				fprintf(stdout, "No circuit partitioning method\n");
			}
			if(noGT)
			{
				fprintf(stdout, "No graph traversal method\n");
			}
			if(noRS)
			{
				fprintf(stdout, "No rectangle sampling method\n");
			}
			if (noCP && noGT && noRS)
			{
				fprintf(stdout, "Warning: none of the available methods is choosen for Rent's parameter evaluation\nExisting ...\n");
				exit(0);
			}
		}
		
		if (v.getForActions() > 0)
		{
			cout<<"Finished reading command line parameters"<<endl;
		  cout<<"########## Rent's parameter evaluation process starts ##########"<<endl;
		}
		design.setStartsPerRun(startsPerRun);
		design.setTotalRuns(totalRuns);
		design.setSampleNumGT(sampleNumGT);
		design.setSampleNumRS(sampleNumRS);
		design.setSampleSizeNum(sampleSizeNum);
		design.setSeedCP(seedCP);
		design.setSeedGT(seedGT);
		design.setSeedRS(seedRS);
		design.setNoCP(noCP);
		design.setNoGT(noGT);
		design.setNoRS(noRS);
		
		if(v.getForSysRes() > 0)
		{
			design.reportMem();
    }
		
	  Timer totalTime;
		if (v.getForActions() > 0)
		{
		  cout<<"########## Starts to read in the design ##########"<<endl;
		}
	  reader.readDesign(&design);
	  totalTime.stop();
		if (v.getForActions() > 0)
		{
		  cout<<"########## Design successfully read in "<<totalTime.getUserTime()<<" seconds ##########"<<endl;
		}
		
		if(v.getForSysRes() > 0)
		{
			design.reportMem();
    }
		
		totalTime.start();
	  if (design.getNetNum() == 0)
	  {
			cout<<"No nets read from the design. Please check the input LEF/DEF files. \nExiting ..."<<endl;
			exit(-1);
	  }
	  
		if (v.getForActions() > 0)
		{
		  cout<<"########## Starts to evaluate the Rent parameter ##########"<<endl;
		}
		design.evaluateRent();
	 	totalTime.stop();
		if (v.getForActions() > 0)
		{
		  cout<<"########## Rent parameter evaluation finished in "<<totalTime.getUserTime()<<" seconds ##########"<<endl;
		}
		
		if(v.getForSysRes() > 0)
		{
			design.reportMem();
    }
  }
  catch(const std::bad_alloc &e)
  {
    cout << endl << e.what() << endl;
    cout << "MetaPlacer has run out of memory. Exiting." << endl;
  }

	return 0;
}
