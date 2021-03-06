#include "ProgramGlobals.h"
#include <iostream>
#include "InputNg.h"
#include "InputCheck.h"
#include "ParametersDmrgSolver.h"
#include "ToolBox.h"

#ifndef USE_FLOAT
typedef double RealType;
#else
typedef float RealType;
#endif
typedef PsimagLite::InputNg<Dmrg::InputCheck> InputNgType;
typedef Dmrg::ParametersDmrgSolver<RealType,InputNgType::Readable>
ParametersDmrgSolverType;
typedef Dmrg::ToolBox<ParametersDmrgSolverType> ToolBoxType;

void usage(const char* name)
{
	std::cerr<<"USAGE is "<<name<<" -f filename -a action [-s] [-p precision]\n";
}

int main(int argc,char *argv[])
{
	using namespace Dmrg;

	PsimagLite::String filename;
	PsimagLite::String action;
	PsimagLite::String extraOptions = "lowest eigenvalue";
	int opt = 0;
	int precision = 6;
	bool shortoption = false;
	bool versionOnly = false;
	while ((opt = getopt(argc, argv,"f:p:a:E:sV")) != -1) {
		switch (opt) {
		case 'f':
			filename = optarg;
			break;
		case 'p':
			precision = atoi(optarg);
			std::cout.precision(precision);
			std::cerr.precision(precision);
			break;
		case 'a':
			action = optarg;
			break;
		case 'E':
			extraOptions = optarg;
			break;
		case 's':
			shortoption = true;
			break;
		case 'V':
			versionOnly = true;
			break;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	//sanity checks here
	if (filename=="" || action == "") {
		if (!versionOnly) {
			usage(argv[0]);
			return 1;
		}
	}

	typedef PsimagLite::Concurrency ConcurrencyType;
	ConcurrencyType concurrency(&argc,&argv,1);

	// print license
	if (ConcurrencyType::root()) {
		std::cerr<<ProgramGlobals::license;
		Provenance provenance;
		std::cout<<provenance;
	}

	if (versionOnly) return 0;

	InputCheck inputCheck;

	if (action == "files") {
		if (extraOptions == "") extraOptions = "list";
		inputCheck.checkFileOptions(extraOptions);
	}

	InputNgType::Writeable ioWriteable(filename,
	                                   inputCheck,
	                                   "#InputStartsHere",
	                                   "#InputEndsHere");
	InputNgType::Readable io(ioWriteable);

	//! Read the parameters for this run
	bool earlyExit = true;
	ParametersDmrgSolverType dmrgSolverParams(io,earlyExit);

	ConcurrencyType::npthreads = dmrgSolverParams.nthreads;
	ToolBoxType::ParametersForGrepType params(extraOptions, shortoption);
	if (ToolBoxType::actionCanonical(action) == ToolBoxType::ACTION_GREP) {
		ToolBoxType::printGrep(filename, dmrgSolverParams.filename,params);
	} else if (ToolBoxType::actionCanonical(action) == ToolBoxType::ACTION_FILES) {
		ToolBoxType::files(filename, dmrgSolverParams,extraOptions);
	} else if (ToolBoxType::actionCanonical(action) == ToolBoxType::ACTION_INPUT) {
		std::cout<<io.data()<<"\n";
	} else {
		std::cerr<<argv[0]<<": Unknown action "<<action<<"\n";
		std::cerr<<"\tSupported actions are "<<ToolBoxType::actions()<<"\n";
		return 1;
	}
} // main

