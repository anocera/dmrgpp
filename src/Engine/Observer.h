/*
Copyright (c) 2008-2015, UT-Battelle, LLC
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

/*! \file Observer.h
 *
 *  A class to perform post-processing calculation of observables
 *
 */
#ifndef DMRG_OBSERVE_H
#define DMRG_OBSERVE_H
#include "ObserverHelper.h"
#include "OnePointCorrelations.h"
#include "CorrelationsSkeleton.h"
#include "TwoPointCorrelations.h"
#include "FourPointCorrelations.h"
#include "VectorWithOffsets.h" // for operator*
#include "VectorWithOffset.h" // for operator*
#include "Profiling.h"
#include "Parallel4PointDs.h"
#include "MultiPointCorrelations.h"
#include "Concurrency.h"
#include "Parallelizer.h"
#include "Utils.h"
#include "Braket.h"

namespace Dmrg {

template<typename VectorWithOffsetType_,typename ModelType_,typename IoInputType>
class Observer {

	typedef typename VectorWithOffsetType_::value_type FieldType;
	typedef PsimagLite::SparseVector<FieldType> VectorType;
	typedef typename ModelType_::RealType RealType;
	typedef PsimagLite::Matrix<FieldType> MatrixType;
	typedef typename PsimagLite::Vector<MatrixType>::Type VectorMatrixType;
	typedef typename ModelType_::BasisWithOperatorsType BasisWithOperatorsType;
	typedef typename BasisWithOperatorsType::SparseMatrixType SparseMatrixType;
	typedef typename ModelType_::ModelHelperType::LeftRightSuperType LeftRightSuperType;
	typedef ObserverHelper<IoInputType,
	MatrixType,
	VectorType,
	VectorWithOffsetType_,
	LeftRightSuperType> ObserverHelperType;
	typedef CorrelationsSkeleton<ObserverHelperType,ModelType_> CorrelationsSkeletonType;
	typedef OnePointCorrelations<ObserverHelperType> OnePointCorrelationsType;
	typedef TwoPointCorrelations<CorrelationsSkeletonType> TwoPointCorrelationsType;
	typedef FourPointCorrelations<CorrelationsSkeletonType> FourPointCorrelationsType;
	typedef MultiPointCorrelations<CorrelationsSkeletonType> MultiPointCorrelationsType;
	typedef PsimagLite::Profiling ProfilingType;

	static SizeType const GROW_RIGHT = CorrelationsSkeletonType::GROW_RIGHT;
	static SizeType const GROW_LEFT = CorrelationsSkeletonType::GROW_LEFT;
	static SizeType const DIAGONAL = CorrelationsSkeletonType::DIAGONAL;
	static SizeType const NON_DIAGONAL = CorrelationsSkeletonType::NON_DIAGONAL;

	enum {GS_VECTOR=ObserverHelperType::GS_VECTOR,
		  TIME_VECTOR=ObserverHelperType::TIME_VECTOR};
	enum {LEFT_BRAKET=ObserverHelperType::LEFT_BRAKET,
		  RIGHT_BRAKET=ObserverHelperType::RIGHT_BRAKET};

public:

	typedef ModelType_ ModelType;
	typedef VectorWithOffsetType_ VectorWithOffsetType;
	typedef Braket<ModelType> BraketType;

	Observer(IoInputType& io,
	         SizeType nf,
	         SizeType trail,
	         bool hasTimeEvolution,
	         const ModelType& model,
	         bool verbose=false)
	    : helper_(io,nf,trail,model.params().nthreads,hasTimeEvolution,verbose),
	      verbose_(verbose),
	      onepoint_(helper_),
	      skeleton_(helper_,model,verbose),
	      twopoint_(helper_,skeleton_),
	      fourpoint_(helper_,skeleton_)
	{}

	SizeType size() const { return helper_.size(); }

	RealType time(SizeType threadId) const { return helper_.time(threadId); }

	SizeType site(SizeType threadId) const { return helper_.site(threadId); }

	SizeType marker(SizeType threadId) const { return helper_.marker(threadId); }

	void setPointer(SizeType threadId,SizeType x) { helper_.setPointer(threadId,x); }

	bool endOfData() const { return helper_.endOfData(); }

	// return true if
	// we're at site 1 or n-2
	bool isAtCorner(SizeType numberOfSites,SizeType threadId) const
	{
		bool es = (helper_.direction(threadId) == ProgramGlobals::EXPAND_SYSTEM);
		if (es && helper_.site(threadId) ==  numberOfSites-2) return true;
		if (!es && helper_.site(threadId) == 1) return true;
		return false;
	}

	void setBrakets(const PsimagLite::String& left,const PsimagLite::String& right)
	{
		helper_.setBrakets(braketStringToNumber(left),
		                   braketStringToNumber(right));
	}

	void twoPoint(MatrixType& storage,
	              const BraketType& braket)
	{
		assert(braket.points() == 2);

		SizeType flag = 0;

		try {
			braket.site(0);
			flag |= 1;
		} catch (std::exception&) {}

		try {
			braket.site(1);
			flag |= 2;
		} catch (std::exception&) {}

		SparseMatrixType m0 = braket.op(0).data;
		SparseMatrixType m1 = braket.op(1).data;
		int fermionSign = braket.op(0).fermionSign;

		SizeType threadId = 0;
		SizeType site1 = 0;
		SizeType sites= storage.n_col();
		PsimagLite::String str("twopoint: Give no site, first site, or all sites\n");

		switch (flag) {
		case 0: // no sites given
			return twopoint_(storage,m0,m1,fermionSign);
		case 1: //first site given
			for (site1 = 0; site1 < sites; ++site1)
				storage(braket.site(0),site1) = twopoint_.calcCorrelation(braket.site(0),
				                                                          site1,
				                                                          braket.op(0).data,
				                                                          braket.op(1).data,
				                                                          fermionSign,
				                                                          threadId);
			return;

		case 3:
			storage(braket.site(0),braket.site(1)) = twopoint_.calcCorrelation(braket.site(0),
			                                                                   braket.site(1),
			                                                                   braket.op(0).data,
			                                                                   braket.op(1).data,
			                                                                   fermionSign,
			                                                                   threadId);
			return;
		default:
			throw PsimagLite::RuntimeError(str);
		}
	}

	void twoPoint(MatrixType& m,
	                   const SparseMatrixType& O1,
	                   const SparseMatrixType& O2,
	                   int fermionicSign)
	{
		twopoint_(m, O1, O2, fermionicSign);
	}

	void threePoint(const BraketType& braket,
	                SizeType rows,
	                SizeType cols)
	{
		assert(braket.points() == 3);

		SizeType flag = 0;
		try {
			braket.site(0);
			flag |= 1;
		} catch (std::exception&) {}

		try {
			braket.site(1);
			flag |= 2;
		} catch (std::exception&) {}

		try {
			braket.site(2);
			flag |= 4;
		} catch (std::exception&) {}

		if (flag != 0 && flag != 7 && flag != 1) {
			PsimagLite::String str("threePoint: ");
			str += "Give no site, first site, or all sites\n";
			throw PsimagLite::RuntimeError(str);
		}

		SizeType threadId = 0;
		SparseMatrixType m0 = braket.op(0).data;
		SparseMatrixType m1 = braket.op(1).data;
		SparseMatrixType m2 = braket.op(2).data;

		int fermionSign = braket.op(0).fermionSign;

		if (flag == 7) {
			std::cout<<"#Fixed all sites\n";
			typename MatrixType::value_type tmp = fourpoint_.threePoint('N',braket.site(0),m0,
			                                                            'N',braket.site(1),m1,
			                                                            'N',braket.site(2),m2,
			                                                            fermionSign,
			                                                            threadId);
			std::cout<<braket.site(0)<<" ";
			std::cout<<braket.site(1)<<" "<<braket.site(2)<<"  "<<tmp<<"\n";
			return;
		}

		if (flag == 1) {
			SizeType site0 = braket.site(0);
			std::cout<<"#Fixed site0= "<<site0<<"\n";
			for (SizeType site1 = site0+1; site1 < rows; ++site1) {
				for (SizeType site2 = site1+1; site2 < cols; ++site2) {
					typename MatrixType::value_type tmp = fourpoint_.threePoint('N',site0,m0,
					                                                            'N',site1,m1,
					                                                            'N',site2,m2,
					                                                            fermionSign,
					                                                            threadId);
					std::cout<<site1<<" "<<site2<<"  "<<tmp<<"\n";

				}
			}

			return;
		}

		assert(flag == 0);
		for (SizeType site0 = 0; site0 < rows; ++site0) {
			for (SizeType site1 = site0+1; site1 < rows; ++site1) {
				for (SizeType site2 = site1+1; site2 < cols; ++site2) {

					typename MatrixType::value_type tmp = fourpoint_.threePoint('N',site0,m0,
					                                                            'N',site1,m1,
					                                                            'N',site2,m2,
					                                                            fermionSign,
					                                                            threadId);
					std::cout<<site0<<" "<<site1<<" "<<site2<<"  "<<tmp<<"\n";
				}
			}
		}
	}

	const FourPointCorrelationsType& fourpoint() const {return fourpoint_; }

	void fourPoint(const BraketType& braket,
	               SizeType rows,
	               SizeType cols)
	{
		assert(braket.points() == 4);

		SizeType flag = 0;
		try {
			braket.site(0);
			flag |= 1;
		} catch (std::exception&) {}

		try {
			braket.site(1);
			flag |= 2;
		} catch (std::exception&) {}

		try {
			braket.site(2);
			flag |= 4;
		} catch (std::exception&) {}

		try {
			braket.site(3);
			flag |= 8;
		} catch (std::exception&) {}

		if (flag != 0 && flag != 3 && flag != 15) {
			PsimagLite::String str("fourPoint: ");
			str += "Give no site, first two sites, or all sites\n";
			throw PsimagLite::RuntimeError(str);
		}

		SizeType threadId = 0;
		SparseMatrixType m0 = braket.op(0).data;
		SparseMatrixType m1 = braket.op(1).data;
		SparseMatrixType m2 = braket.op(2).data;
		SparseMatrixType m3 = braket.op(3).data;

		RealType fermionSign = braket.op(0).fermionSign;

		if (flag == 15) {
			std::cout<<"#Fixed all sites\n";
			SizeType site0 = braket.site(0);
			SizeType site1 = braket.site(1);
			typename FourPointCorrelationsType::SparseMatrixType O2gt;
			fourpoint_.firstStage(O2gt,'N',site0,m0,'N',site1,m1,fermionSign,threadId);
			typename MatrixType::value_type tmp = fourpoint_.
			        secondStage(O2gt,site1,
			                    'N',braket.site(2),m2,
			                    'N',braket.site(3),m3,
			                    fermionSign,
			                    threadId);
			std::cout<<site0<<" "<<site1<<" ";
			std::cout<<braket.site(2)<<" "<<braket.site(3)<<"  "<<tmp<<"\n";
			return;
		}

		if (flag == 3) {
			SizeType site0 = braket.site(0);
			SizeType site1 = braket.site(1);
			std::cout<<"#Fixed site0= "<<site0<<"\n";
			std::cout<<"#Fixed site1= "<<site1<<"\n";
			typename FourPointCorrelationsType::SparseMatrixType O2gt;
			fourpoint_.firstStage(O2gt,'N',site0,m0,'N',site1,m1,fermionSign,threadId);

			for (SizeType site2 = site1+1; site2 < rows; ++site2) {
				for (SizeType site3 = site2+1; site3 < cols; ++site3) {
					typename MatrixType::value_type tmp = fourpoint_.
					        secondStage(O2gt,site1,'N',site2,m2,'N',site3,m3,fermionSign,
					                    threadId);
					std::cout<<site2<<" "<<site3<<" "<<tmp<<"\n";
				}
			}

			return;
		}

		assert(flag == 0);
		for (SizeType site0 = 0; site0 < rows; ++site0) {
			for (SizeType site1 = site0+1; site1 < cols; ++site1) {
				for (SizeType site2 = site1+1; site2 < rows; ++site2) {
					for (SizeType site3 = site2+1; site3 < cols; ++site3) {
						typename FourPointCorrelationsType::SparseMatrixType O2gt;
						fourpoint_.firstStage(O2gt,'N',site0,m0,'N',site1,m1,fermionSign,
						                      threadId);
						typename MatrixType::value_type tmp = fourpoint_.
						        secondStage(O2gt,site1,'N',site2,m2,'N',site3,m3,fermionSign,
						                    threadId);
						std::cout<<site0<<" "<<site1<<" ";
						std::cout<<site2<<" "<<site3<<" "<<tmp<<"\n";
					}
				}
			}
		}
	}

	VectorMatrixType ladder(const BraketType& braket,
	                        SizeType rows,
	                        SizeType,
	                        SizeType threadId)
	{
		SizeType rowsOver2 = utils::exactDivision(rows,2);

		MatrixType m0,m1,m2,m3;
		crsMatrixToFullMatrix(m0,braket.op(0).data);
		crsMatrixToFullMatrix(m1,braket.op(1).data);
		crsMatrixToFullMatrix(m2,braket.op(2).data);
		crsMatrixToFullMatrix(m3,braket.op(3).data);
		int f = braket.op(0).fermionSign;
		VectorMatrixType v(4);
		for (SizeType i = 0; i < v.size(); ++i) v[i].resize(rowsOver2,rowsOver2);

		for (SizeType i = 0; i < rowsOver2; i += 2) {
			for (SizeType j = rowsOver2; j < rows; j += 2) {
				SizeType jj = j - rowsOver2;
				v[0](i,jj) = ladder_(m0,i,m1,i+1,m2,j,m3,j+1,f,threadId);
				v[1](i,jj) = f*ladder_(m0,i,m1,i+1,m3,j,m2,j+1,f,threadId);
				v[2](i,jj) = f*ladder_(m1,i,m0,i+1,m2,j,m3,j+1,f,threadId);
				v[3](i,jj) = ladder_(m1,i,m0,i+1,m3,j,m2,j+1,f,threadId);
			}
		}

		return v;
	}

	void fourPointDeltas(MatrixType& fpd,
	                     const typename PsimagLite::Vector<SizeType>::Type& gammas,
	                     const ModelType& model)
	{
		if (gammas.size()!=4) {
			std::cerr<<"Observer: fourPointDeltas(...):  wrong number of gammas ";
			std::cerr<<" expected "<<4<<" got "<<gammas.size()<<"\n";
			throw PsimagLite::RuntimeError("Observer::fourPointDeltas(...)\n");
		}

		SizeType nsites = 2*fpd.n_row();
		assert(fpd.n_row()==fpd.n_col());

		SizeType hs = model.hilbertSize(0);
		SizeType nx = 0;
		while (hs) {
			hs>>=1;
			nx++;
		}

		nx /= 2;

		assert(fpd.n_row()>1);
		typedef std::pair<SizeType,SizeType> PairType;

		typename PsimagLite::Vector<PairType>::Type pairs;
		for (SizeType i=0;i<fpd.n_row();i++) {
			if (2*i+1>=nsites) continue;
			for (SizeType j=i+1;j<fpd.n_col();j++) {
				if (2*j+1>=nsites) continue;
				pairs.push_back(PairType(i,j));
			}
		}

		typedef Parallel4PointDs<ModelType,FourPointCorrelationsType>
		        Parallel4PointDsType;
		typedef PsimagLite::Parallelizer<Parallel4PointDsType> ParallelizerType;
		ParallelizerType threaded4PointDs(PsimagLite::Concurrency::npthreads,
		                                  PsimagLite::MPI::COMM_WORLD);

		Parallel4PointDsType helper4PointDs(fpd,fourpoint_,model,gammas,pairs);

		threaded4PointDs.loopCreate(pairs.size(),helper4PointDs);
	}

	template<typename ApplyOperatorType>
	FieldType onePoint(SizeType site,
	                   const typename ApplyOperatorType::OperatorType& A,
	                   typename ApplyOperatorType::BorderEnum corner)
	{
		return onepoint_.template operator()<ApplyOperatorType>(site,A,corner);
	}

	template<typename ApplyOperatorType>
	FieldType onePointHookForZero(SizeType site,
	                              const typename ApplyOperatorType::OperatorType& A,
	                              bool corner = false)
	{
		return onepoint_.template hookForZero<ApplyOperatorType>(site,A,corner);
	}

	template<typename VectorLikeType>
	typename PsimagLite::EnableIf
	<PsimagLite::IsVectorLike<VectorLikeType>::True,void>::Type
	multiCorrelations(VectorLikeType& result,
	                  const SparseMatrixType& O,
	                  SizeType rows,
	                  SizeType cols)
	{
		SizeType nthreads = 1;
		MultiPointCorrelationsType multi(nthreads,helper_,skeleton_);
		multi(result,O,rows,cols);
	}

private:

	SizeType braketStringToNumber(const PsimagLite::String& str) const
	{
		if (str == "gs") return GS_VECTOR;
		if (str == "time") return TIME_VECTOR;
		PsimagLite::String msg("Observer::braketStringToNumber:");
		throw PsimagLite::RuntimeError(msg + " must be gs or time\n");
	}

	FieldType ladder_(const MatrixType& O1,
	                  SizeType i1,
	                  const MatrixType& O2,
	                  SizeType i2,
	                  const MatrixType& O3,
	                  SizeType j1,
	                  const MatrixType& O4,
	                  SizeType j2,
	                  int fermionicSign,
	                  SizeType threadId)
	{
		char mod = 'N';
		return fourpoint_(mod,
		                  i1,
		                  O1,
		                  mod,
		                  i2,
		                  O2,
		                  mod,
		                  j1,
		                  O3,
		                  mod,
		                  j2,
		                  O4,
		                  fermionicSign,
		                  threadId);
	}

	ObserverHelperType helper_;
	bool verbose_;
	OnePointCorrelationsType onepoint_;
	CorrelationsSkeletonType skeleton_;
	TwoPointCorrelationsType twopoint_;
	FourPointCorrelationsType fourpoint_;
};  //class Observer
} // namespace Dmrg

/*@}*/
#endif

