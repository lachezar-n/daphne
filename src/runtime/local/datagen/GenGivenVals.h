/*
 * Copyright 2021 The DAPHNE Consortium
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SRC_RUNTIME_LOCAL_DATAGEN_GENGIVENVALS_H
#define SRC_RUNTIME_LOCAL_DATAGEN_GENGIVENVALS_H

#include <runtime/local/datastructures/CSRMatrix.h>
#include <runtime/local/datastructures/DataObjectFactory.h>
#include <runtime/local/datastructures/DenseMatrix.h>

#include <vector>

#include <cassert>
#include <cstddef>
#include <cstring>

// ****************************************************************************
// Struct for partial template specialization
// ****************************************************************************

template<template<typename> class DT, typename VT>
struct GenGivenVals {
    static DT<VT> * generate(size_t numRows, const std::vector<VT> & elements) = delete;
};

// ****************************************************************************
// Convenience function
// ****************************************************************************

/**
 * @brief A very simple data generator which populates a matrix with the
 * elements of the given `std::vector`.
 * 
 * Meant only for small matrices, mainly as a utility for testing and
 * debugging. Note that it can easily be used with an initializer list as
 * follows:
 * 
 * ```c++
 * // Generates the matrix  3 1 4
 * //                       1 5 9
 * auto m = genGivenVals<DenseMatrix, double>(2, {3, 1, 4, 1, 5, 9});
 * ```
 * 
 * @param numRows The number of rows.
 * @param elements The data elements to populate the matrix with. Their number
 * must be divisible by `numRows`.
 * @return A matrix of the specified data type `DT` and value type `VT`
 * containing the provided data elements.
 */
template<template<typename> class DT, typename VT>
DT<VT> * genGivenVals(size_t numRows, const std::vector<VT> & elements) {
    return GenGivenVals<DT, VT>::generate(numRows, elements);
}

// ****************************************************************************
// (Partial) template specializations for different data/value types
// ****************************************************************************

// This data generator is not meant to be efficient. Nevertheless, note that we
// do not use the generic `set`/`append` interface to matrices here since this
// generator is meant to be used for writing tests for, besides others, those
// generic interfaces.

// ----------------------------------------------------------------------------
// DenseMatrix
// ----------------------------------------------------------------------------

template<typename VT>
struct GenGivenVals<DenseMatrix, VT> {
    static DenseMatrix<VT> * generate(size_t numRows, const std::vector<VT> & elements) {
        const size_t numCells = elements.size();
        assert((numCells % numRows == 0) && "number of given data elements must be divisible by given number of rows");
        const size_t numCols = numCells / numRows;
        auto res = DataObjectFactory::create<DenseMatrix<VT>>(numRows, numCols, false);
        memcpy(res->getValues(), elements.data(), numCells * sizeof(VT));
        return res;
    }
};

// ----------------------------------------------------------------------------
// CSRMatrix
// ----------------------------------------------------------------------------

template<typename VT>
struct GenGivenVals<CSRMatrix, VT> {
    static CSRMatrix<VT> * generate(size_t numRows, const std::vector<VT> & elements) {
        const size_t numCells = elements.size();
        assert((numCells % numRows == 0) && "number of given data elements must be divisible by given number of rows");
        const size_t numCols = numCells / numRows;
        size_t numNonZeros = 0;
        for(VT v : elements)
            if(v != VT(0))
                numNonZeros++;
        auto res = DataObjectFactory::create<CSRMatrix<VT>>(numRows, numCols, numNonZeros, false);
        VT * values = res->getValues();
        size_t * colIdxs = res->getColIdxs();
        size_t * rowOffsets = res->getRowOffsets();
        size_t pos = 0;
        size_t colIdx = 0;
        size_t rowIdx = 0;
        rowOffsets[0] = 0;
        for(VT v : elements) {
            if(v != VT(0)) {
                values[pos] = v;
                colIdxs[pos] = colIdx;
                pos++;
            }
            colIdx++;
            if(colIdx == numCols) {
                colIdx = 0;
                rowOffsets[rowIdx++ + 1] = pos;
            }
        }
        return res;
    }
};

#endif //SRC_RUNTIME_LOCAL_DATAGEN_GENGIVENVALS_H