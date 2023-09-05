#ifndef _TYPES_H_
#define _TYPES_H_

#ifdef _WIN32
#pragma warning(disable : 4710) // function not inlined
#pragma warning(disable : 4711) // function selected for inline expansion
// #pragma warning(disable : 4514) // unreferenced inline function has been removed

#undef VERBOSE
#define VERBOSE (CODE_DEBUG || 0)
#if !CODE_DEBUG
#undef _HAS_EXCEPTIONS
#define _HAS_EXCEPTIONS (0)
#endif
#endif

// Enable performance testing
#define PERF_TEST (0)
#if PERF_TEST
constexpr size_t _perf_num_trials = 100;       // 100 Trials
constexpr size_t _perf_sample_per_trial = 106; // 106 samples per trial (input.csv)
#endif

// Custom types
typedef int Uid;   //!< Order UID
typedef int Qty;   //!< Order Qu_perf_sample_per_trialantity
typedef int Price; //!< Order Pricing
typedef int Epoch; //!< Order Epoch
typedef char Side; //!< Side 'B' or 'S'
typedef char Type; //!< Order Type (MARKET or LIMIT)

// Order side mapping to (0,1) which simplies indexing of buy/sell book
constexpr Side BUY = 0;
constexpr Side SELL = 1;

// Order types
constexpr Type MARKET = 'M';
constexpr Type LIMIT = 'L';

#endif