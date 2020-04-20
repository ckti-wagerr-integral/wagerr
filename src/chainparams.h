// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2018 The PIVX developers
// Copyright (c) 2018 The Wagerr developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CHAINPARAMS_H
#define BITCOIN_CHAINPARAMS_H

#include "chainparamsbase.h"
#include "checkpoints.h"
#include "primitives/block.h"
#include "protocol.h"
#include "uint256.h"

#include "libzerocoin/Params.h"
#include <vector>

typedef unsigned char MessageStartChars[MESSAGE_START_SIZE];

struct CDNSSeedData {
    std::string name, host;
    CDNSSeedData(const std::string& strName, const std::string& strHost) : name(strName), host(strHost) {}
};

typedef enum QuickGamesType {
    qgDice = 0x00,
} QuickGamesType;

typedef enum QuickGamesDiceBetType {
    qgDiceEqual = 0x00,
    qgDiceNotEqual = 0x01,
    qgDiceTotalOver = 0x02,
    qgDiceTotalUnder = 0x03,
    qgDiceEven = 0x04,
    qgDiceOdd = 0x05
} QuickGamesDiceBetType;

/* The quick game handler prototype, handles
 * a quick game bet with an incoming seed (pos hash)
 * and returns the odds factor which indicate win (more than odds divisor), lose (0) or refund (odds divisor).
 */
typedef uint32_t (*const BetHandler)(std::vector<unsigned char>& betInfo, uint256 seed);

/* The quick games framework model */
class CQuickGamesView
{
public:
    const std::string name;
    const QuickGamesType type;
    const BetHandler handler;
    const std::string specialAddress;
    const uint32_t nFeePermille = 10; // 1%
    const uint32_t nDevRewardPermille;
    const uint32_t nOMNORewardPermille;

    explicit CQuickGamesView() = delete;
    explicit CQuickGamesView(const std::string name, const QuickGamesType type, BetHandler handler, const std::string specialAddress, const uint32_t nOMNORewardPermille, const uint32_t nDevRewardPermille) :
        name(name), type(type), handler(handler), specialAddress(specialAddress), nOMNORewardPermille(nOMNORewardPermille), nDevRewardPermille(nDevRewardPermille) { }
    // move constructor
    explicit CQuickGamesView(const CQuickGamesView&& view) :
        name(view.name), type(view.type), handler(view.handler), specialAddress(view.specialAddress), nFeePermille(view.nFeePermille), nOMNORewardPermille(view.nOMNORewardPermille),  nDevRewardPermille(view.nDevRewardPermille) { }
    // copy constructor
    explicit CQuickGamesView(const CQuickGamesView& view) = delete;
};

/**
 * CChainParams defines various tweakable parameters of a given instance of the
 * Wagerr system. There are three: the main network on which people trade goods
 * and services, the public test network which gets reset from time to time and
 * a regression test mode which is intended for private networks only. It has
 * minimal difficulty to ensure that blocks can be found instantly.
 */
class CChainParams
{
public:
    enum Base58Type {
        PUBKEY_ADDRESS,
        SCRIPT_ADDRESS,
        SECRET_KEY,     // BIP16
        EXT_PUBLIC_KEY, // BIP32
        EXT_SECRET_KEY, // BIP32
        EXT_COIN_TYPE,  // BIP44

        MAX_BASE58_TYPES
    };

    const uint256& HashGenesisBlock() const { return hashGenesisBlock; }
    const MessageStartChars& MessageStart() const { return pchMessageStart; }
    const std::vector<unsigned char>& AlertKey() const { return vAlertPubKey; }
    int GetDefaultPort() const { return nDefaultPort; }
    const uint256& ProofOfWorkLimit() const { return bnProofOfWorkLimit; }
    int SubsidyHalvingInterval() const { return nSubsidyHalvingInterval; }
    /** Used to check majorities for block version upgrade */
    int EnforceBlockUpgradeMajority() const { return nEnforceBlockUpgradeMajority; }
    int RejectBlockOutdatedMajority() const { return nRejectBlockOutdatedMajority; }
    int ToCheckBlockUpgradeMajority() const { return nToCheckBlockUpgradeMajority; }
    int MaxReorganizationDepth() const { return nMaxReorganizationDepth; }

    /** Used if GenerateBitcoins is called with a negative number of threads */
    int DefaultMinerThreads() const { return nMinerThreads; }
    const CBlock& GenesisBlock() const { return genesis; }
    /** Make miner wait to have peers to avoid wasting work */
    bool MiningRequiresPeers() const { return fMiningRequiresPeers; }
    /** Headers first syncing is disabled */
    bool HeadersFirstSyncingActive() const { return fHeadersFirstSyncingActive; };
    /** Default value for -checkmempool and -checkblockindex argument */
    bool DefaultConsistencyChecks() const { return fDefaultConsistencyChecks; }
    /** Allow mining of a min-difficulty block */
    bool AllowMinDifficultyBlocks() const { return fAllowMinDifficultyBlocks; }
    /** Skip proof-of-work check: allow mining of any difficulty block */
    bool SkipProofOfWorkCheck() const { return fSkipProofOfWorkCheck; }
    /** Make standard checks */
    bool RequireStandard() const { return fRequireStandard; }
    int64_t TargetSpacing() const { return nTargetSpacing; }

    /** returns the coinbase maturity **/
    int COINBASE_MATURITY() const { return nMaturity; }

    /** returns the coinstake maturity (min depth required) **/
    int COINSTAKE_MIN_DEPTH() const { return nStakeMinDepth; }
    bool HasStakeMinAgeOrDepth(const int contextHeight, const uint32_t contextTime, const int utxoFromBlockHeight, const uint32_t utxoFromBlockTime) const;

    /** returns the max future time (and drift in seconds) allowed for a block in the future **/
    int FutureBlockTimeDrift(const bool isPoS) const { return isPoS ? nFutureTimeDriftPoS : nFutureTimeDriftPoW; }
    uint32_t MaxFutureBlockTime(uint32_t time, const bool isPoS) const { return time + FutureBlockTimeDrift(isPoS); }

    CAmount MaxMoneyOut() const { return nMaxMoneyOut; }
    /** The masternode count that we will allow the see-saw reward payments to be off by */
    int MasternodeCountDrift() const { return nMasternodeCountDrift; }
    /** Make miner stop after a block is found. In RPC, don't return until nGenProcLimit blocks are generated */
    bool MineBlocksOnDemand() const { return fMineBlocksOnDemand; }
    /** In the future use NetworkIDString() for RPC fields */
    bool TestnetToBeDeprecatedFieldRPC() const { return fTestnetToBeDeprecatedFieldRPC; }
    /** Return the BIP70 network string (main, test or regtest) */
    std::string NetworkIDString() const { return strNetworkID; }
    const std::vector<CDNSSeedData>& DNSSeeds() const { return vSeeds; }
    const std::vector<unsigned char>& Base58Prefix(Base58Type type) const { return base58Prefixes[type]; }
    const std::vector<CAddress>& FixedSeeds() const { return vFixedSeeds; }
    virtual const Checkpoints::CCheckpointData& Checkpoints() const = 0;
    int PoolMaxTransactions() const { return nPoolMaxTransactions; }
    /** Return the number of blocks in a budget cycle */
    int GetBudgetCycleBlocks() const { return nBudgetCycleBlocks; }
    int64_t GetProposalEstablishmentTime() const { return nProposalEstablishmentTime; }

    /** Spork key and Masternode Handling **/
    std::string SporkKey() const { return strSporkKey; }
    std::string SporkKeyOld() const { return strSporkKeyOld; }
    int64_t NewSporkStart() const { return nEnforceNewSporkKey; }
    int64_t RejectOldSporkKey() const { return nRejectOldSporkKey; }
    std::string ObfuscationPoolDummyAddress() const { return strObfuscationPoolDummyAddress; }
    int64_t StartMasternodePayments() const { return nStartMasternodePayments; }
    int64_t Budget_Fee_Confirmations() const { return nBudget_Fee_Confirmations; }

    CBaseChainParams::Network NetworkID() const { return networkID; }

    /** Zerocoin **/
    std::string Zerocoin_Modulus() const { return zerocoinModulus; }
    libzerocoin::ZerocoinParams* Zerocoin_Params(bool useModulusV1) const;
    int Zerocoin_MaxSpendsPerTransaction() const { return nMaxZerocoinSpendsPerTransaction; }
    int Zerocoin_MaxPublicSpendsPerTransaction() const { return nMaxZerocoinPublicSpendsPerTransaction; }
    CAmount Zerocoin_MintFee() const { return nMinZerocoinMintFee; }
    int Zerocoin_MintRequiredConfirmations() const { return nMintRequiredConfirmations; }
    int Zerocoin_RequiredAccumulation() const { return nRequiredAccumulation; }
    int Zerocoin_DefaultSpendSecurity() const { return nDefaultSecurityLevel; }
    int Zerocoin_HeaderVersion() const { return nZerocoinHeaderVersion; }
    int Zerocoin_RequiredStakeDepth() const { return nZerocoinRequiredStakeDepth; }

    /** Height or Time Based Activations **/
    int ModifierUpgradeBlock() const { return nModifierUpdateBlock; }
    int LAST_POW_BLOCK() const { return nLastPOWBlock; }
    int Zerocoin_StartHeight() const { return nZerocoinStartHeight; }
    int Zerocoin_Block_EnforceSerialRange() const { return nBlockEnforceSerialRange; }
    int Zerocoin_Block_RecalculateAccumulators() const { return nBlockRecalculateAccumulators; }
    int Zerocoin_Block_FirstFraudulent() const { return nBlockFirstFraudulent; }
    int Zerocoin_Block_LastGoodCheckpoint() const { return nBlockLastGoodCheckpoint; }
    int Zerocoin_StartTime() const { return nZerocoinStartTime; }
    int Block_Enforce_Invalid() const { return nBlockEnforceInvalidUTXO; }
    int Zerocoin_Block_V2_Start() const { return nBlockZerocoinV2; }
    int BIP65Height() const { return nBIP65Height; }
    bool IsStakeModifierV2(const int nHeight) const { return nHeight >= nBlockStakeModifierlV2; }

    // fake serial attack
    int Zerocoin_Block_EndFakeSerial() const { return nFakeSerialBlockheightEnd; }
    CAmount GetSupplyBeforeFakeSerial() const { return nSupplyBeforeFakeSerial; }

    int Zerocoin_Block_Double_Accumulated() const { return nBlockDoubleAccumulated; }
    CAmount InvalidAmountFiltered() const { return nInvalidAmountFiltered; };

    int Zerocoin_Block_Public_Spend_Enabled() const { return nPublicZCSpends; }

    int Zerocoin_AccumulationStartHeight() const { return nZerocoinAccumulationStartHeight; }

    /** Betting on blockchain **/
    std::vector<std::string> OracleWalletAddrs() const { return vOracleWalletAddrs; }
    int BetBlocksIndexTimespan() const { return nBetBlocksIndexTimespan; }
    int BetStartHeight() const { return nBetStartHeight; }
    std::string DevPayoutAddr() const { return strDevPayoutAddr; }
    std::string OMNOPayoutAddr() const { return strOMNOPayoutAddr; }
    uint64_t OMNORewardPermille() const { return nOMNORewardPermille; }
    uint64_t DevRewardPermille() const { return nDevRewardPermille; }
    uint64_t OddsDivisor() const { return nOddsDivisor; }
    uint64_t BetXPermille() const { return nBetXPermille; }
    int BetBlockPayoutAmount() const { return nBetBlockPayoutAmount; } // Currently not used
    int64_t MaxBetPayoutRange() const { return nMaxBetPayoutRange; }
    int64_t MinBetPayoutRange() const { return nMinBetPayoutRange; }
    int64_t MaxParlayBetPayoutRange() const { return nMaxBetPayoutRange; }
    int BetPlaceTimeoutBlocks() const { return nBetPlaceTimeoutBlocks; }
    int MaxParlayLegs() const { return nMaxParlayLegs; }
    int ParlayBetStartHeight() const { return nParlayBetStartHeight; }
    const std::vector<CQuickGamesView>& QuickGamesArr() const { return quickGamesArr; }

    /** temp worarounds **/
    int ZerocoinCheckTX() const { return nZerocoinCheckTX; }
    int ZerocoinCheckTXexclude() const { return nZerocoinCheckTXexclude; }

protected:
    CChainParams() {}

    uint256 hashGenesisBlock;
    MessageStartChars pchMessageStart;
    //! Raw pub key bytes for the broadcast alert signing key.
    std::vector<unsigned char> vAlertPubKey;
    int nDefaultPort;
    uint256 bnProofOfWorkLimit;
    int nMaxReorganizationDepth;
    int nSubsidyHalvingInterval;
    int nEnforceBlockUpgradeMajority;
    int nRejectBlockOutdatedMajority;
    int nToCheckBlockUpgradeMajority;
    int64_t nTargetSpacing;
    int nLastPOWBlock;
    int nMasternodeCountDrift;
    int nMaturity;
    int nStakeMinDepth;
    int nFutureTimeDriftPoW;
    int nFutureTimeDriftPoS;

    int nModifierUpdateBlock;
    CAmount nMaxMoneyOut;
    int nMinerThreads;
    std::vector<CDNSSeedData> vSeeds;
    std::vector<unsigned char> base58Prefixes[MAX_BASE58_TYPES];
    CBaseChainParams::Network networkID;
    std::string strNetworkID;
    CBlock genesis;
    std::vector<CAddress> vFixedSeeds;
    bool fMiningRequiresPeers;
    bool fAllowMinDifficultyBlocks;
    bool fDefaultConsistencyChecks;
    bool fRequireStandard;
    bool fMineBlocksOnDemand;
    bool fSkipProofOfWorkCheck;
    bool fTestnetToBeDeprecatedFieldRPC;
    bool fHeadersFirstSyncingActive;
    int nPoolMaxTransactions;
    int nBudgetCycleBlocks;
    std::string strSporkKey;
    std::string strSporkKeyOld;
    int64_t nEnforceNewSporkKey;
    int64_t nRejectOldSporkKey;
    std::string strObfuscationPoolDummyAddress;
    int64_t nStartMasternodePayments;
    std::string zerocoinModulus;
    int nMaxZerocoinSpendsPerTransaction;
    int nMaxZerocoinPublicSpendsPerTransaction;
    CAmount nMinZerocoinMintFee;
    CAmount nInvalidAmountFiltered;
    int nMintRequiredConfirmations;
    int nRequiredAccumulation;
    int nDefaultSecurityLevel;
    int nZerocoinHeaderVersion;
    int64_t nBudget_Fee_Confirmations;
    int nZerocoinStartHeight;
    int nZerocoinStartTime;
    int nZerocoinRequiredStakeDepth;
    int64_t nProposalEstablishmentTime;

    int nBlockEnforceSerialRange;
    int nBlockRecalculateAccumulators;
    int nBlockFirstFraudulent;
    int nBlockLastGoodCheckpoint;
    int nBlockEnforceInvalidUTXO;
    int nBlockZerocoinV2;
    int nBlockDoubleAccumulated;
    int nPublicZCSpends;
    int nBIP65Height;
    int nBlockStakeModifierlV2;

    // fake serial attack
    int nFakeSerialBlockheightEnd = 0;
    CAmount nSupplyBeforeFakeSerial = 0;

    int nZerocoinAccumulationStartHeight;

    std::vector<std::string> vOracleWalletAddrs;
    int nBetBlocksIndexTimespan;
    int nBetStartHeight;
    std::string strDevPayoutAddr;
    std::string strOMNOPayoutAddr;
    uint64_t nOMNORewardPermille;
    uint64_t nDevRewardPermille;
    uint64_t nOddsDivisor;
    uint64_t nBetXPermille;
    uint64_t nBetBlockPayoutAmount;
    int64_t nMinBetPayoutRange;
    int64_t nMaxBetPayoutRange;
    int64_t nMaxParlayBetPayoutRange;
    int nBetPlaceTimeoutBlocks;
    int nMaxParlayLegs;
    int nParlayBetStartHeight;

    std::vector<CQuickGamesView> quickGamesArr;

    // workarounds
    int nZerocoinCheckTX;
    int nZerocoinCheckTXexclude;
};

/**
 * Modifiable parameters interface is used by test cases to adapt the parameters in order
 * to test specific features more easily. Test cases should always restore the previous
 * values after finalization.
 */

class CModifiableParams
{
public:
    //! Published setters to allow changing values in unit test cases
    virtual void setSubsidyHalvingInterval(int anSubsidyHalvingInterval) = 0;
    virtual void setEnforceBlockUpgradeMajority(int anEnforceBlockUpgradeMajority) = 0;
    virtual void setRejectBlockOutdatedMajority(int anRejectBlockOutdatedMajority) = 0;
    virtual void setToCheckBlockUpgradeMajority(int anToCheckBlockUpgradeMajority) = 0;
    virtual void setDefaultConsistencyChecks(bool aDefaultConsistencyChecks) = 0;
    virtual void setAllowMinDifficultyBlocks(bool aAllowMinDifficultyBlocks) = 0;
    virtual void setSkipProofOfWorkCheck(bool aSkipProofOfWorkCheck) = 0;
};


/**
 * Return the currently selected parameters. This won't change after app startup
 * outside of the unit tests.
 */
const CChainParams& Params();

/** Return parameters for the given network. */
CChainParams& Params(CBaseChainParams::Network network);

/** Get modifiable network parameters (UNITTEST only) */
CModifiableParams* ModifiableParams();

/** Sets the params returned by Params() to those for the given network. */
void SelectParams(CBaseChainParams::Network network);

/**
 * Looks for -regtest or -testnet and then calls SelectParams as appropriate.
 * Returns false if an invalid combination is given.
 */
bool SelectParamsFromCommandLine();

#endif // BITCOIN_CHAINPARAMS_H
