/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 28/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_GLOBALRANDOM_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_GLOBALRANDOM_HPP
#include <random>

namespace lightstreamer::util {

    /**
     * A utility class for generating random numbers.
     */
    class GlobalRandom {
    private:
        static std::mt19937 randomEngine;
        static std::uniform_real_distribution<double> distribution;
        static bool initialized;

        /**
         * Initializes the random number generator with a seed
         * obtained from a random device.
         */
        static void Initialize() {
            if (!initialized) {
                std::random_device rd;
                randomEngine = std::mt19937(rd());
                distribution = std::uniform_real_distribution<double>(0.0, 1.0);
                initialized = true;
            }
        }

    public:
        /**
         * Generates a random double between 0.0 and 1.0.
         * @return A random double.
         */
        static double NextDouble() {
            Initialize();
            return distribution(randomEngine);
        }
    };

    // Initialize static members
    std::mt19937 GlobalRandom::randomEngine;
    std::uniform_real_distribution<double> GlobalRandom::distribution;
    bool GlobalRandom::initialized = false;

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_GLOBALRANDOM_HPP
