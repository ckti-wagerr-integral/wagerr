// Copyright (c) 2020 The Wagerr developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef WAGERR_DICE_H
#define WAGERR_DICE_H

#include "uint256.h"

namespace quickgames {

typedef enum QuickGamesDiceBetType {
    qgDiceEqual = 0x00,
    qgDiceNotEqual = 0x01,
    qgDiceTotalOver = 0x02,
    qgDiceTotalUnder = 0x03,
    qgDiceEven = 0x04,
    qgDiceOdd = 0x05
} QuickGamesDiceBetType;

uint32_t DiceHandler(std::vector<unsigned char>& betInfo, uint256 seed);

} // namespace quickgames


#endif //WAGERR_DICE_H
