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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_CONCURRENTMATRIX_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_CONCURRENTMATRIX_HPP
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <set>
#include <list>

namespace lightstreamer::util {

    template<typename R, typename C>
    class ConcurrentMatrix {
    private:
        std::unordered_map<R, std::unordered_map<C, std::string>> matrix;
        std::string NULL_VALUE = "NULL";
        std::mutex mtx; // Mutex for thread-safe access

    public:
        void insert(const std::string& value, const R& row, const C& column) {
            std::lock_guard<std::mutex> lock(mtx);
            auto& matrixRow = matrix[row];
            matrixRow[column] = value.empty() ? NULL_VALUE : value;
        }

        std::string get(const R& row, const C& column) {
            std::lock_guard<std::mutex> lock(mtx);
            auto itRow = matrix.find(row);
            if (itRow != matrix.end()) {
                auto itCol = itRow->second.find(column);
                if (itCol != itRow->second.end()) {
                    return itCol->second == NULL_VALUE ? "" : itCol->second;
                }
            }
            return "";
        }

        void del(const R& row, const C& column) {
            std::lock_guard<std::mutex> lock(mtx);
            auto itRow = matrix.find(row);
            if (itRow != matrix.end()) {
                itRow->second.erase(column);
                if (itRow->second.empty()) {
                    matrix.erase(row);
                }
            }
        }

        // ElementCallback and RowCallback are defined as std::function
        using ElementCallback = std::function<bool(const std::string&, const R&, const C&)>;
        using RowCallback = std::function<bool(const R&, const std::unordered_map<C, std::string>&)>;

        // Inserts a full row in the matrix. If another row is already present, it is overwritten.
        void insertRow(const std::unordered_map<C, std::string>& insRow, const R& row) {
            std::lock_guard<std::mutex> lock(mtx);
            matrix[row] = insRow;
        }

        // Retrieves a row from the matrix. If the row does not exist, returns an empty row.
        std::unordered_map<C, std::string> getRow(const R& row) {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = matrix.find(row);
            if (it != matrix.end()) {
                return it->second;
            }
            return {};
        }

        // Removes the row at the specified position in the matrix.
        void delRow(const R& row) {
            std::lock_guard<std::mutex> lock(mtx);
            matrix.erase(row);
        }

        // Returns the entire matrix. This method is marked as deprecated in the original C# code.
        std::unordered_map<R, std::unordered_map<C, std::string>> EntireMatrix()  {
            std::lock_guard<std::mutex> lock(mtx);
            return matrix;
        }

        // Verifies if the matrix is empty.
        bool Empty()  {
            std::lock_guard<std::mutex> lock(mtx);
            return matrix.empty();
        }

        // Executes a given callback passing each element of the specified row.
        void forEachElementInRow(const R& row, const ElementCallback& callback) {
            std::lock_guard<std::mutex> lock(mtx);
            auto itRow = matrix.find(row);
            if (itRow == matrix.end()) {
                return;
            }

            // Need to use a temporary list of keys if we are potentially modifying the collection during iteration
            std::vector<C> keys;
            for (const auto& pair : itRow->second) {
                keys.push_back(pair.first);
            }

            for (const auto& key : keys) {
                auto& value = itRow->second[key];
                // Assuming callback returns false to keep the item, true to remove it
                if (callback(value, row, key)) {
                    itRow->second.erase(key);
                }
            }

            // Remove the row if it becomes empty after modifications
            if (itRow->second.empty()) {
                matrix.erase(itRow);
            }
        }

        // Executes a given callback passing each element of the Matrix.
        void forEachElement(const ElementCallback& callback) {
            std::lock_guard<std::mutex> lock(mtx);
            for (const auto& rowPair : matrix) {
                for (const auto& colPair : rowPair.second) {
                    bool shouldRemove = callback(colPair.second, rowPair.first, colPair.first);
                    if (shouldRemove) {
                        // As modifying collection during iteration is risky, defer actual removal
                        // Consider a mechanism to track and remove after iteration if needed
                    }
                }
            }
            // Note: Implement removal logic if elements are flagged for removal during callback
        }

        // Executes a given callback passing the key of each row containing at least one element.
        void forEachRow(const RowCallback& callback) {
            std::lock_guard<std::mutex> lock(mtx);
            std::vector<R> rowsToRemove;

            for (const auto& rowPair : matrix) {
                bool shouldRemove = callback(rowPair.first, rowPair.second);
                if (shouldRemove) {
                    rowsToRemove.push_back(rowPair.first);
                }
            }

            // Remove flagged rows after iteration to avoid modifying the collection during iteration
            for (const auto& row : rowsToRemove) {
                matrix.erase(row);
            }
        }



    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_CONCURRENTMATRIX_HPP
