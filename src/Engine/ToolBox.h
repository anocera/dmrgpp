/*
Copyright (c) 2009-2015, UT-Battelle, LLC
All rights reserved

[DMRG++, Version 3.0]
[by G.A., Oak Ridge National Laboratory]

UT Battelle Open Source Software License 11242008

OPEN SOURCE LICENSE

Subject to the conditions of this License, each
contributor to this software hereby grants, free of
charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), a
perpetual, worldwide, non-exclusive, no-charge,
royalty-free, irrevocable copyright license to use, copy,
modify, merge, publish, distribute, and/or sublicense
copies of the Software.

1. Redistributions of Software must retain the above
copyright and license notices, this list of conditions,
and the following disclaimer.  Changes or modifications
to, or derivative works of, the Software should be noted
with comments and the contributor and organization's
name.

2. Neither the names of UT-Battelle, LLC or the
Department of Energy nor the names of the Software
contributors may be used to endorse or promote products
derived from this software without specific prior written
permission of UT-Battelle.

3. The software and the end-user documentation included
with the redistribution, with or without modification,
must include the following acknowledgment:

"This product includes software produced by UT-Battelle,
LLC under Contract No. DE-AC05-00OR22725  with the
Department of Energy."

*********************************************************
DISCLAIMER

THE SOFTWARE IS SUPPLIED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER, CONTRIBUTORS, UNITED STATES GOVERNMENT,
OR THE UNITED STATES DEPARTMENT OF ENERGY BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.

NEITHER THE UNITED STATES GOVERNMENT, NOR THE UNITED
STATES DEPARTMENT OF ENERGY, NOR THE COPYRIGHT OWNER, NOR
ANY OF THEIR EMPLOYEES, REPRESENTS THAT THE USE OF ANY
INFORMATION, DATA, APPARATUS, PRODUCT, OR PROCESS
DISCLOSED WOULD NOT INFRINGE PRIVATELY OWNED RIGHTS.

*********************************************************

*/
/** \ingroup DMRG */
/*@{*/

/*! \file ToolBox.h
 *
 *
 */

#ifndef DMRG_TOOLBOX_H
#define DMRG_TOOLBOX_H
#include "TarPack.h"
#include "Vector.h"
#include "ProgramGlobals.h"
#include "ArchiveFiles.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace Dmrg {

template<typename DmrgParametersType>
class ToolBox  {

	typedef ArchiveFiles<DmrgParametersType> ArchiveFilesType;

	class GrepForLabel {

		typedef PosixTarHeader::LongType LongType;

		struct InternalName {
			InternalName(PsimagLite::String label_, bool cooked_)
			    : cooked(cooked_),label(label_)
			{}

			bool cooked;
			PsimagLite::String label;
		}; // struct InternalName

	public:

		typedef InternalName ParametersType;

		static void hook(std::ifstream& fin,
		                 PsimagLite::String,
		                 LongType len,
		                 const ParametersType& params)
		{
			LongType len2 = len;
			LongType bufferLen = 1;
			std::stringstream ss;
			char *buffer = new char[bufferLen];
			while (len2 >= bufferLen && !fin.eof()) {
				fin.read(buffer,bufferLen);
				ss<<buffer[0];
				if (buffer[0] == '\n') {
					procLine(ss.str(),params);
					ss.str("");
				}

				if (len > 1) len2 -= bufferLen;
			}

			delete [] buffer;
		}

	private:

		static void procLine(PsimagLite::String line, const ParametersType& params)
		{
			if (line.find(params.label) == PsimagLite::String::npos) return;
			if (params.cooked)
				cookThisLine(line);
			else
				std::cout<<line;
		}

		static void cookThisLine(PsimagLite::String line)
		{
			size_t index = line.find(" after");
			PsimagLite::String line2 = line;
			if (index != PsimagLite::String::npos)
				line2 = line.erase(index,line.length());
			line = line2;
			PsimagLite::String magic = "eigenvalue= ";
			index = line.find(magic);
			if (index != PsimagLite::String::npos)
				line.erase(0,index + magic.length());
			std::cout<<line<<"\n";
		}
	}; // GrepForLabel

public:

	enum ActionEnum {ACTION_UNKNOWN, ACTION_GREP, ACTION_FILES, ACTION_INPUT};

	typedef typename GrepForLabel::ParametersType ParametersForGrepType;

	static ActionEnum actionCanonical(PsimagLite::String action)
	{
		if (action == "energy" || action == "Energy" || action == "energies"
		        || action == "Energies" || action == "grep") return ACTION_GREP;
		if (action == "files") return ACTION_FILES;
		if (action == "input") return ACTION_INPUT;

		return ACTION_UNKNOWN;
	}

	static PsimagLite::String actions()
	{
		return "energies | grep | files | input";
	}

	static void printGrep(PsimagLite::String inputfile,
	                      PsimagLite::String datafile,
	                      ParametersForGrepType params)
	{

		PsimagLite::String tarname = ArchiveFilesType::rootName(datafile) + ".tar";
		PsimagLite::String coutName = ArchiveFilesType::coutName(inputfile);
		struct stat *buf = 0;
		int ret = stat(tarname.c_str(), buf);
		if (ret != 0) return printGrepNoTar(coutName,datafile,params);

		UnTarPack untarpack(tarname);
		bool rewind = false;
		untarpack.extract<GrepForLabel>(coutName,rewind,params);
	}

	static void files(PsimagLite::String inputfile,
	                  const DmrgParametersType& solverParams,
	                  PsimagLite::String extraOptions)
	{
		PsimagLite::String coutName = ArchiveFilesType::coutName(inputfile);
		ArchiveFilesType af(solverParams,inputfile,false,coutName);

		if (extraOptions == "DELETE" || extraOptions == "list") {
			af.listOrClear(inputfile,extraOptions);
		} else {
			PsimagLite::String str("ToolBox: action=files: ");
			str += "extra option= " + extraOptions + " not understood\n";
			throw PsimagLite::RuntimeError(str);
		}
	}

private:

	static void printGrepNoTar(PsimagLite::String inputfile,
	                      PsimagLite::String,
	                      ParametersForGrepType params)
	{
		std::ifstream fin(inputfile.c_str());
		GrepForLabel::hook(fin,"",1,params);
	}
}; //class ToolBox

} // namespace Dmrg
/*@}*/
#endif

