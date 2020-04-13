#include "betting/quickgames/dice.h"
#include "chainparams.h"
#include "clientversion.h"
#include "streams.h"

uint64_t Dice_NumberOfWinCases(uint64_t sum) {              //    sum: 2, 3, 4, 5, 6, 7, 8, 9, 10  11, 12
    return (sum <= 7) * (sum - 1) + (sum > 7) * (13 - sum); // result: 1, 2, 3, 4, 5, 6, 5, 4,  3,  2,  1
}

namespace quickgames {

uint32_t DiceHandler(std::vector<unsigned char>& betInfo, uint256 seed)
{
    static const uint32_t NUMBER_OF_OUTCOMES = 36;
    CDataStream ss{betInfo, SER_NETWORK, CLIENT_VERSION};
    uint8_t betType;
    ss >> betType;

    uint64_t firstDice = seed.Get64(0) % 6 + 1;
    uint64_t secondDice = seed.Get64(1) % 6 + 1;
    uint64_t sum = firstDice + secondDice;

    if (betType == qgDiceOdd && sum % 2 == 1) {
        return Params().OddsDivisor() * 2;
    }
    else if (betType == qgDiceEven && sum % 2 == 0) {
        return Params().OddsDivisor() * 2;
    }

    uint32_t betNumber;
    ss >> betNumber;

    if (betType == qgDiceEqual && sum == betNumber) {
        return Params().OddsDivisor() * NUMBER_OF_OUTCOMES / Dice_NumberOfWinCases(betNumber);
    }
    else if (betType == qgDiceNotEqual && sum != betNumber) {
        return Params().OddsDivisor() * NUMBER_OF_OUTCOMES / (NUMBER_OF_OUTCOMES - Dice_NumberOfWinCases(betNumber));
    }
    else if (betType == qgDiceTotalUnder && sum <= betNumber) {
        uint64_t numberOfWinCases = 0;
        for (uint32_t i = 2; i <= betNumber; ++i)
            numberOfWinCases += Dice_NumberOfWinCases(i);
        return Params().OddsDivisor() * NUMBER_OF_OUTCOMES / numberOfWinCases;
    }
    else if (betType == qgDiceTotalOver && sum >= betNumber) {
        uint64_t numberOfWinCases = 0;
        for (uint32_t i = 2; i <= betNumber; ++i)
            numberOfWinCases += Dice_NumberOfWinCases(i);
        return Params().OddsDivisor() * NUMBER_OF_OUTCOMES / (NUMBER_OF_OUTCOMES - numberOfWinCases);
    }

    return 0;
}

} // namespace quickgames