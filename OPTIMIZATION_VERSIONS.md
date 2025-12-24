# Performance Optimization Versions - Testing Guide

This document explains the different versions created to test each optimization independently.

## Version Overview

### V10 - Baseline (Original)
The original version before any optimizations were applied.
- No incremental Zobrist hashing
- Generates all moves then filters in quiescence
- Copies move lists during filtering

### V11 - Incremental Zobrist Hashing
**Added:** Incremental Zobrist hash updates in `make_move_temp()`
**Files modified:** `Engine/movegen.c`, `Engine/search.c`

**What it does:**
- Stores `zobrist_key` in Board struct
- Updates hash incrementally using XOR operations instead of recalculating
- Transposition table uses cached `board->zobrist_key`

**Expected impact:** 3-5x speedup in transposition table operations

**Testing command:**
```bash
make v11
# Compare v10 vs v11 to measure incremental Zobrist impact
```

### V12 - Incremental Zobrist + Capture-Only Generation
**Added:** `generate_capture_moves_only()` for quiescence search
**Files modified:** `Engine/movegen.c`, `Engine/quiescence.c`

**What it does:**
- Includes all V11 optimizations
- Generates only captures directly in quiescence search
- Avoids generating ~60 moves when only ~5-10 captures are needed

**Expected impact:** Additional 2-3x speedup in quiescence search

**Testing command:**
```bash
make v12
# Compare v10 vs v12 to measure combined Zobrist + capture-only impact
# Compare v11 vs v12 to measure capture-only impact alone
```

### V13 - All Optimizations
**Added:** In-place move filtering and move picking
**Files modified:** `Engine/movegen.c`, `Engine/move_ordering.c`

**What it does:**
- Includes all V11 and V12 optimizations
- Filters moves in-place without allocating temporary MoveList
- Uses `pick_next_move()` for lazy move selection

**Expected impact:** Additional 1.5x speedup by reducing memory operations

**Testing command:**
```bash
make v13
# Compare v10 vs v13 to measure total impact
# Compare v12 vs v13 to measure in-place filtering impact alone
```

## How to Build and Test

### Build all versions:
```bash
make v10 v11 v12 v13
```

### Build all versions at once:
```bash
make all_versions
```

### Verify correctness with perft:
```bash
# Test each version
echo -e "perft 4\nquit" | versions/v10_build/chess_engine_v10 2>&1 | grep Total
echo -e "perft 4\nquit" | versions/v11_build/chess_engine_v11 2>&1 | grep Total
echo -e "perft 4\nquit" | versions/v12_build/chess_engine_v12 2>&1 | grep Total
echo -e "perft 4\nquit" | versions/v13_build/chess_engine_v13 2>&1 | grep Total
```

All should output: `Total: 197281`

## ELO Testing Strategy

To identify which optimization is causing performance issues:

1. **Test V10 vs V11**: Isolates incremental Zobrist impact
   - If V11 is worse, the incremental Zobrist has a bug or negative impact
   
2. **Test V10 vs V12**: Measures Zobrist + capture-only together
   - If V12 is worse but V11 was good, the capture-only generation has issues
   
3. **Test V10 vs V13**: Measures all optimizations
   - If V13 is worse but V12 was good, the in-place filtering has issues

4. **Test intermediate comparisons**:
   - V11 vs V12: Isolates capture-only generation impact
   - V12 vs V13: Isolates in-place filtering impact

## Expected Results

Based on the original analysis from Paul Sonkoly, we expect:
- **V11**: ~3-5x faster (fewer zobrist_hash() calls)
- **V12**: ~6-15x faster (V11 + quiescence optimization)
- **V13**: ~10-20x faster (all optimizations combined)

If any version shows ELO regression, that indicates a bug in that specific optimization that needs to be fixed.

## Code Organization

All optimizations are wrapped with VERSION macros:
- `#if VERSION >= 11` - Incremental Zobrist
- `#if VERSION >= 12` - Capture-only generation
- `#if VERSION >= 13` - In-place filtering

This allows clean separation and independent testing of each optimization.
