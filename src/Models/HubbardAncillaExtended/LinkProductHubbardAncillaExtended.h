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

/*! \file LinkProductHubbardAncillaExtended.h
 *
 *  A class to represent product of operators that form a link or
 *  bond for this model
 *
 */
#ifndef DMRG_LINKPROD_HUBBARD_ANCILLA_EXTENDED_H
#define DMRG_LINKPROD_HUBBARD_ANCILLA_EXTENDED_H
#include "../Models/FeAsModel/LinkProductFeAs.h"
#include "ProgramGlobals.h"

namespace Dmrg {

template<typename ModelHelperType>
class LinkProductHubbardAncillaExtended {

	typedef typename ModelHelperType::SparseMatrixType SparseMatrixType;
	typedef typename SparseMatrixType::value_type SparseElementType;
	typedef std::pair<SizeType,SizeType> PairType;
	typedef LinkProductFeAs<ModelHelperType> LinkProductFeAsType;
	typedef LinkProductHeisenberg<ModelHelperType> LinkProductHeisenbergType;
	typedef typename ModelHelperType::BasisType BasisType;
	typedef typename ModelHelperType::OperatorType OperatorType;

	enum {TERM_HOPPING, TERM_LAMBDA, TERM_SPLUS, TERM_SZ, TERM_PAIR, TERM_NINJ};

public:

	typedef typename ModelHelperType::RealType RealType;

	template<typename SomeStructType>
	static SizeType dofs(SizeType term,const SomeStructType&)
	{
		if (term == TERM_HOPPING || term == TERM_LAMBDA)
			return 2;

		return 1;
	}

	// has only dependence on orbital
	template<typename SomeStructType>
	static PairType connectorDofs(SizeType,
	                              SizeType,
	                              const SomeStructType&)
	{
		return PairType(0,0);
	}

	template<typename SomeStructType>
	static void setLinkData(SizeType term,
	                        SizeType dofs,
	                        bool,
	                        SizeType& fermionOrBoson,
	                        PairType& ops,
	                        std::pair<char,char>& mods,
	                        SizeType& angularMomentum,
	                        RealType& angularFactor,
	                        SizeType& category,
	                        const SomeStructType&)
	{
		if (term==TERM_HOPPING) {
			fermionOrBoson = ProgramGlobals::FERMION;
			ops = PairType(dofs,dofs);
			angularFactor = 1;
			if (dofs == 1) angularFactor = -1;
			angularMomentum = 1;
			category = dofs;
			return;
		}

		if (term==TERM_LAMBDA) {
			fermionOrBoson = ProgramGlobals::BOSON;
			SizeType offset1 = 2;
			ops = PairType(dofs + offset1,dofs + offset1);
			return;
		}

		if (term==TERM_SPLUS) {
			fermionOrBoson = ProgramGlobals::BOSON;
			SizeType offset1 = 4;
			assert(dofs == 0);
			// S+ S- which includes also S- S+
			angularFactor = -1;
			category = 2;
			angularMomentum = 2;
			ops = PairType(offset1,offset1);
			return;
		}

		if (term==TERM_SZ) {
			fermionOrBoson = ProgramGlobals::BOSON;
			SizeType offset1 = 5;
			angularFactor = 0.5;
			category = 1;
			angularMomentum = 2;
			ops = PairType(offset1,offset1);
		}

		if (term==TERM_PAIR) {
			fermionOrBoson = ProgramGlobals::BOSON;
			SizeType offset1 = 6;
			assert(dofs = 0);
			angularFactor = 1;
			category = 2;
			angularMomentum = 2;
			ops = PairType(offset1,offset1);
			return;
		}

		if (term==TERM_NINJ) {
			fermionOrBoson = ProgramGlobals::BOSON;
			SizeType offset1 = 7;
			angularFactor = 1;
			angularMomentum = 0;
			category = 0;
			ops = PairType(offset1,offset1);
			return;
		}
	}

	template<typename SomeStructType>
	static void valueModifier(SparseElementType& value,
	                          SizeType term,
	                          SizeType,
	                          bool isSu2,
	                          const SomeStructType&)
	{
		if (term == TERM_HOPPING || term == TERM_LAMBDA || term == TERM_NINJ)
			return;

		if (term == TERM_PAIR) {
			value *= -1.0;
			return;
		}

		assert(term==TERM_SPLUS || term == TERM_SZ);

		if (isSu2) value = -value;
		value *= 0.5;
	}

	static SizeType terms() { return 6; }
}; // class LinkProductHubbardAncillaExtended
} // namespace Dmrg
/*@}*/
#endif //DMRG_LINKPROD_HUBBARD_ANCILLA_EXTENDED_H

