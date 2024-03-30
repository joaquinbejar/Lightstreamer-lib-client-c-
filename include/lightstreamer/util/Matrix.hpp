/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 30/3/24
 ******************************************************************************/

/*******************************************************************************
 Copyright (c) 2024.

 This program is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation, either version 3 of the License, or (at your
 option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program. If not, see <https://www.gnu.org/licenses/>..
 ******************************************************************************/

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_MATRIX_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_MATRIX_HPP
#include <unordered_map>
#include <vector>
#include <set>
#include <functional>

namespace lightstreamer::util {

    /**
     * @class Matrix
     *
     * @brief Class representing a matrix with unordered map implementation.
     *
     * The Matrix class stores elements in an unordered map, where each row is represented by a key of type R
     * and each column is represented by a key of type C. The value at each element is of type V.
     *
     * The class provides functionalities to insert, get, delete, and manipulate rows and elements in the matrix.
     */
    template<typename R, typename C, typename V>
    class Matrix {
    private:
        std::unordered_map<R, std::unordered_map<C, V>> matrix;

    public:
        void insert(const V& value, const R& row, const C& column) {
            matrix[row][column] = value;
        }

        V get(const R& row, const C& column) {
            if (matrix.find(row) != matrix.end() && matrix[row].find(column) != matrix[row].end()) {
                return matrix[row][column];
            }
            return V(); // Returns default if not found
        }

        void del(const R& row, const C& column) {
            if (matrix.find(row) != matrix.end()) {
                matrix[row].erase(column);
                if (matrix[row].empty()) {
                    matrix.erase(row);
                }
            }
        }

        void insertRow(const std::unordered_map<C, V>& insRow, const R& row) {
            matrix[row] = insRow;
        }

        std::unordered_map<C, V> getRow(const R& row) {
            if (matrix.find(row) != matrix.end()) {
                return matrix[row];
            }
            return {};
        }

        void delRow(const R& row) {
            matrix.erase(row);
        }

        std::unordered_map<R, std::unordered_map<C, V>> EntireMatrix() const {
            return matrix;
        }

        bool Empty() const {
            return matrix.empty();
        }

        size_t Count(const R& row) const {
            if (matrix.find(row) != matrix.end()) {
                return matrix.at(row).size();
            }
            return 0;
        }

        // ElementCallback and RowCallback are defined as std::function accepting specific parameters and returning bool
        using ElementCallback = std::function<bool(const V&, const R&, const C&)>;
        using RowCallback = std::function<bool(const R&, const std::unordered_map<C, V>&)>;

        void forEachElement(const ElementCallback& callback) {
            for (auto& rowPair : matrix) {
                for (auto& colPair : rowPair.second) {
                    if (!callback(colPair.second, rowPair.first, colPair.first)) {
                        // Assuming callback returns false to signal deletion
                        del(rowPair.first, colPair.first);
                    }
                }
            }
        }

        void forEachRow(const RowCallback& callback) {
            for (auto it = matrix.begin(); it != matrix.end(); ) {
                if (!callback(it->first, it->second)) {
                    it = matrix.erase(it);
                } else {
                    ++it;
                }
            }
        }

        std::vector<V> sortAndCleanMatrix() {
            std::vector<V> sorted;
            std::set<R> rows;

            for (auto& rowPair : matrix) {
                rows.insert(rowPair.first);
            }

            for (auto& row : rows) {
                std::set<C> cols;
                for (auto& colPair : matrix[row]) {
                    cols.insert(colPair.first);
                }

                for (auto& col : cols) {
                    sorted.push_back(matrix[row][col]);
                }
            }

            matrix.clear();
            return sorted;
        }

        void clear() {
            matrix.clear();
        }
    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_MATRIX_HPP
