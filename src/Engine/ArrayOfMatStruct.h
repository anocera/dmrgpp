/*
Copyright (c) 2012, UT-Battelle, LLC
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
// END LICENSE BLOCK
/** \ingroup DMRG */
/*@{*/

/*! \file ArrayOfMatStruct.h
 *
 *
 */

#ifndef ARRAY_OF_MAT_STRUCT_H
#define ARRAY_OF_MAT_STRUCT_H
#include "CrsMatrix.h"

namespace Dmrg {

template<typename SparseMatrixType,typename GenGroupType>
class ArrayOfMatStruct {

public:

	typedef typename SparseMatrixType::value_type ComplexOrRealType;

	ArrayOfMatStruct() {}

	ArrayOfMatStruct(const SparseMatrixType& sparse,GenGroupType& istart)
	    : data_(istart.size()-1,istart.size()-1)
	{
		SizeType ngroup = istart.size()-1;

		for (SizeType j=0;j<ngroup;j++) {
			SizeType j1 = istart(j);
			SizeType j2 = istart(j+1);

			for (SizeType i=0;i<ngroup;++i) {
				SizeType i1 = istart(i);
				SizeType i2 = istart(i+1);

				data_(i,j) = new SparseMatrixType(i2-i1,j2-j1);
				SparseMatrixType& tmp = *data_(i,j);
				SizeType counter = 0;

				for (SizeType ii=i1;ii<i2;++ii) {
					SizeType row = ii - i1;
					tmp.setRow(row,counter);

					SizeType start = sparse.getRowPtr(ii);
					SizeType end = sparse.getRowPtr(ii+1);

					for (SizeType k = start;k<end;++k) {
						int col = sparse.getCol(k)-j1;
						if (col<0) continue;
						if (SizeType(col)>=j2-j1) continue; // ARE COLUMNS SORTED?
						tmp.pushValue(sparse.getValue(k));
						tmp.pushCol(col);
						counter++;
					}
				}

				tmp.setRow(i2-i1,counter);
				tmp.checkValidity();
			}
		}
	}

	const SparseMatrixType& operator()(SizeType i,SizeType j) const
	{
		assert(i<data_.n_row() && j<data_.n_col());
		return *data_(i,j);
	}

	~ArrayOfMatStruct()
	{
		for (SizeType i = 0; i < data_.n_row(); ++i)
			for (SizeType j = 0; j < data_.n_col(); ++j)
				if (data_(i,j)) delete data_(i,j);
	}

private:

	PsimagLite::Matrix<SparseMatrixType*> data_;

}; //class ArrayOfMatStruct
} // namespace Dmrg

/*@}*/

#endif // ARRAY_OF_MAT_STRUCT_H

