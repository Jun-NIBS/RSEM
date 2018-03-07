/* Copyright (c) 2016
   Bo Li (University of California, Berkeley)
   bli25@berkeley.edu

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.   

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA
*/

#ifndef RNASEQMODEL_H_
#define RNASEQMODEL_H_

#include "SEQstring.hpp"
#include "QUALstring.hpp"

#include "MateLenDist.hpp"
#include "NoiseProfile.hpp"
#include "QualDist.hpp"
#include "NoiseQProfile.hpp"

#include "BamAlignment.hpp"
#include "AlignmentGroup.hpp"

class RNASeqModel {
public:
	RNASeqModel(int model_type);
	~RNASeqModel();

	void update(const AlignmentGroup& ag, bool notAligned) {
		bool paired = ag.isPaired();
		assert(!paired || model_type >= 2); // paired-end model allows single-end reads
		
		mld1->update(ag.getSeqLength());
		if (paired) mld2->update(ag.getSeqLength(2));

		if (notAligned) { 
			if (model_type & 1) ag.getQUAL(qual);
			ag.getSEQ(readseq);
			if (model_type & 1) { qd->update(qual); nqpro->update(qual, readseq); }
			else npro->update(readseq);

			if (paired) {
				if (model_type & 1) ag.getQUAL(qual, 2);
				ag.getSEQ(readseq, 2);
				if (model_type & 1) { qd->update(qual); nqpro->update(qual, readseq); }
				else npro->update(readseq);
			}
		}
		else if (paired) {
			int s = ag.size(), frag_len;
			const BamAlignment *ba;
			for (int i = 0; i < s; ++i) {
				ba = ag.getAlignment(i);
				if (ba->isAligned() == 1) {
					frag_len = ba->getInsertSize();
					if (frag_min > frag_len) frag_min = frag_len;
					if (frag_max < frag_len) frag_max = frag_len;
				}
			}
		}
	}

	// ssF, sufficient statistics file; paramF, parameter file
	void write(const char* ssF, const char* paramF);
	
private:
	int model_type;
	int frag_min, frag_max; // minimum and maximum fragment length seen for paired-end reads
	
	MateLenDist *mld1, *mld2; // mate length distribution 1 & 2
	QualDist *qd;
	NoiseProfile *npro;
	NoiseQProfile *nqpro;

	SEQstring readseq;
	QUALstring qual;
};

#endif
