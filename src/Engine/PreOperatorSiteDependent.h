/*
Copyright (c) 2009-2012, UT-Battelle, LLC
All rights reserved

[DMRG++, Version 2.0.0]
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

/*! \file PreOperatorSiteDependent.h
 *
 * TBW
 *
 */

#ifndef PRE_OP_SITE_DEPENDENT_H
#define PRE_OP_SITE_DEPENDENT_H
#include <iostream>
#include "Matrix.h"
#include "PreOperatorBase.h"

namespace Dmrg {

template<typename ModelType>
class PreOperatorSiteDependent  : public PreOperatorBase<ModelType> {

	typedef typename ModelType::ModelHelperType ModelHelperType;
	typedef typename ModelHelperType::SparseMatrixType SparseMatrixType;
	typedef typename SparseMatrixType::value_type ValueType;
	typedef PsimagLite::Matrix<ValueType> MatrixType;
	typedef typename ModelType::OperatorType OperatorType;
	typedef typename OperatorType::Su2RelatedType Su2RelatedType;

public:

	PreOperatorSiteDependent(size_t dof,size_t dof2,const ModelType& model,const std::string& str,size_t threadId)
		: dof_(dof),dof2_(dof2),model_(model),str_(str),threadId_(threadId)
	{}

	virtual OperatorType operator()(size_t site) const
	{
		MatrixType opCup = model_.naturalOperator("c",site,dof_);
		MatrixType opCdown = model_.naturalOperator("c",site,dof2_);
		MatrixType opCupTranspose;
		transposeConjugate(opCupTranspose,opCup);
		MatrixType Afull = opCupTranspose * opCdown;
		SparseMatrixType A(Afull);
		Su2RelatedType su2Related1;
		OperatorType opA(A,1,std::pair<size_t,size_t>(0,0),1,su2Related1);

		return opA;
	}

	virtual bool isValid(size_t i0) const
	{
		if (!siteDependent()) return true;
		size_t hilbert = model_.hilbertSize(i0);
		size_t nx=0;
		while(hilbert) {
			nx++;
			hilbert >>= 1;
		}
		if (nx>0) nx--;
		return (dof_<nx && dof2_<nx);
	}

	virtual std::string label() const { return str_; }

	virtual size_t threadId() const { return threadId_; }

	virtual bool siteDependent() const
	{
		if (model_.params().model=="Immm") return true;
		return false;
	}

private:

	size_t dof_;
	size_t dof2_;
	const ModelType& model_;
	std::string str_;
	size_t threadId_;
};     //class PreOperatorSiteDependent
} // namespace Dmrg
/*@}*/
#endif