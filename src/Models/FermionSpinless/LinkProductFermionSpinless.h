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

/*! \file LinkProductFermionSpinless.h
 *
 *  FIXME
 *
 */
#ifndef DMRG_LINK_PROD_FERMION_SPINLESS_H
#define DMRG_LINK_PROD_FERMION_SPINLESS_H

namespace Dmrg {

template<typename ModelHelperType>
class LinkProductFermionSpinless {

	typedef typename ModelHelperType::SparseMatrixType SparseMatrixType;
	typedef std::pair<SizeType,SizeType> PairType;

public:

	enum {TERM_HOPPING=0,TERM_NINJ=1};

	typedef typename ModelHelperType::RealType RealType;
	typedef typename SparseMatrixType::value_type SparseElementType;

	template<typename SomeStructType>
	static void setLinkData(SizeType term,
	                        SizeType dofs,
	                        bool,
	                        SizeType& fermionOrBoson,
	                        PairType& ops,
	                        std::pair<char,char>&,
	                        SizeType& angularMomentum,
	                        RealType& angularFactor,
	                        SizeType& category,const SomeStructType&)
	{
		assert(dofs == 0);
		if (term==TERM_NINJ) fermionOrBoson = ProgramGlobals::BOSON;
		else fermionOrBoson = ProgramGlobals::FERMION;
		if (term==TERM_NINJ) ops = PairType(1,1);
		else ops = PairType(dofs,dofs);
		angularFactor = 1;
		if (dofs==1) angularFactor = -1;
		angularMomentum = 1;
		if (term==TERM_NINJ) angularMomentum = 0;
		category = dofs;
	}

	template<typename SomeStructType>
	static void valueModifier(SparseElementType&,
	                          SizeType,
	                          SizeType,
	                          bool,
	                          const SomeStructType&)
	{}

	// up up is the only connections possible for this model
	template<typename SomeStructType>
	static SizeType dofs(SizeType,const SomeStructType&) { return 1; }

	template<typename SomeStructType>
	static std::pair<SizeType,SizeType> connectorDofs(SizeType,
	                                                  SizeType,
	                                                  const SomeStructType&)
	{
		return PairType(0,0); // no orbital and no dependence on spin
	}

	static SizeType terms() { return 2; }
}; // class LinkProductFermionSpinless

} // namespace Dmrg
/*@}*/
#endif

