## Cheat Sheet for Data Lab

---

### Logical vs Bitwise

| Operator | Type | What it does |
|----------|------|-------------|
| `!` | Logical | "Is this zero?" → returns only 0 or 1 |
| `~` | Bitwise | Flips every single bit independently |

**`!` (logical NOT):**
- Treats the entire number as one yes/no question: "is it zero?"
- `!0` = 1 (yes it's zero)
- `!5` = 0 (no it's not zero)
- `!(-3)` = 0 (no it's not zero)
- Output is ALWAYS 0 or 1, nothing else

**`~` (bitwise NOT):**
- Goes through all 32 bits and flips each one individually
- `~0x00000000` = `0xFFFFFFFF` (all 0s become all 1s)
- `~0xFFFFFFFF` = `0x00000000` (all 1s become all 0s)
- `~0x0000000F` = `0xFFFFFFF0` (every bit flipped)

---

### Two's Complement Identities

**How negative numbers work:**
- Positive numbers look normal in binary: 5 = `0x00000005`
- Negative numbers have the top bit (bit 31) set: -1 = `0xFFFFFFFF`
- The top bit is called the "sign bit"

**Key values:**
- **-1** = `0xFFFFFFFF` = all 1s = `~0`
- **0** = `0x00000000` = all 0s
- **Min int (Tmin)** = `0x80000000` = `1 << 31` = -2147483648 (only the sign bit is set)
- **Max int (Tmax)** = `0x7FFFFFFF` = 2147483647 (every bit set EXCEPT sign bit)

**Negate (flip the sign):**
- `-x` = `~x + 1`
- Example: `-5` = `~5 + 1` = `0xFFFFFFFA + 1` = `0xFFFFFFFB`
- Example: `-1` = `~1 + 1` = `0xFFFFFFFE + 1` = `0xFFFFFFFF`
- Special: `-0` = `~0 + 1` = `0xFFFFFFFF + 1` = `0x00000000` (zero is its own negative)

**Overflow/wraparound:**
- `0xFFFFFFFF + 1` = `0x00000000` (wraps back to zero)
- Numbers are on a "circle" — going past max wraps to min

---

### Bitwise Operators

**`&` (AND) — both bits must be 1:**
- `1 & 1` = 1
- `1 & 0` = 0
- `0xFFFFFFFF & x` = `x` (all 1s keeps everything)
- `0x00000000 & x` = `0` (all 0s kills everything)
- Use case: masking — extracting specific bits

**`|` (OR) — at least one bit must be 1:**
- `1 | 0` = 1
- `0 | 0` = 0
- `x | 0` = `x` (ORing with 0 changes nothing)
- Use case: combining two masked results together

**`^` (XOR) — bits must be different:**
- `1 ^ 0` = 1
- `1 ^ 1` = 0
- `x ^ x` = 0 (anything XOR itself is zero)
- `x ^ 0` = `x` (XOR with zero changes nothing)
- Use case: checking if two values are equal (`!(x ^ y)` returns 1 if equal)

---

### Shift Operators

**`<<` (left shift) — moves bits to the left, fills with 0s:**
- `1 << 3` = 8 (binary: 0001 becomes 1000)
- `x << n` is like multiplying by 2^n
- Bits that fall off the left are lost

**`>>` (right shift) — moves bits to the right:**
- This lab uses ARITHMETIC right shift
- If the number is negative (sign bit = 1), it fills with 1s from the left
- If the number is positive (sign bit = 0), it fills with 0s from the left
- `x >> 31` spreads the sign bit across all 32 bits:
  - Positive/zero: gives `0x00000000`
  - Negative: gives `0xFFFFFFFF`

---

### Useful Tricks & Patterns

**`!!x` — squish to 0 or 1:**
- Apply `!` twice
- `!!0` = 0, `!!5` = 1, `!!(-3)` = 1
- Tells you "is x nonzero?" as a clean 0 or 1

**`~(!!x) + 1` — turn 0 or 1 into a full mask:**
- If `!!x` = 0: `~0 + 1` = `0xFFFFFFFF + 1` = `0x00000000`
- If `!!x` = 1: `~1 + 1` = `0xFFFFFFFE + 1` = `0xFFFFFFFF`
- Use case: the conditional/selection pattern

**Selection pattern (how `conditional` works):**
- `(mask & y) | (~mask & z)`
- When mask = all 1s: keeps y, kills z
- When mask = all 0s: kills y, keeps z

**`!(x ^ y)` — equality check:**
- XOR gives 0 only when x and y are identical
- `!` then turns that 0 into 1
- Returns 1 if equal, 0 if not

**`x >> 31` — extract the sign:**
- Arithmetic shift copies the sign bit into all 32 positions
- Negative → `0xFFFFFFFF`, non-negative → `0x00000000`
- Faster than building a mask from `!!`

---

### Mask Building

When you need a constant bigger than `0xFF` (255), build it with shifts and adds:

- Need `0xAAAAAAAA`? Build it byte by byte:
  `(((((0xAA << 8) + 0xAA) << 8) + 0xAA) << 8) + 0xAA`
- Need `0xFFFF0000`? Try: `0xFF << 24` + `0xFF << 16`
- You can only use constants 0x00 through 0xFF directly

---

### Sign / Comparison Tips

**Checking if a result is negative:**
- Compute `x + (~y + 1)` (this is x - y using allowed ops)
- Then check the sign bit: `>> 31`
- If negative → x was less than y

**Watch out for overflow:**
- Subtracting two numbers with DIFFERENT signs can overflow
- Example: huge positive minus huge negative overflows
- Safe approach: check if signs differ FIRST, handle that case separately

---

### Float (IEEE 754 Single Precision)

**Layout (32 bits total):**
```
| sign (1 bit) | exponent (8 bits) | fraction (23 bits) |
|   bit 31     |   bits 30-23      |   bits 22-0        |
```

**Extracting fields:**
- Sign: `(uf >> 31) & 1`
- Exponent: `(uf >> 23) & 0xFF`
- Fraction: `uf & 0x7FFFFF`

**Three categories based on exponent field:**

| Exponent | Name | Value |
|----------|------|-------|
| 0 | Denormalized | very small numbers, no implicit leading 1 |
| 1-254 | Normalized | normal numbers, implicit leading 1 |
| 255 | Special | infinity (frac=0) or NaN (frac!=0) |

**How the value is calculated:**
- **Normalized:** (-1)^sign x 1.fraction x 2^(exponent - 127)
- **Denormalized:** (-1)^sign x 0.fraction x 2^(1 - 127)
- Bias = 127 (subtract this from exponent field to get actual power)

**Useful float facts:**
- Multiplying by 2 = add 1 to exponent (for normalized)
- Multiplying by 2 for denorm = left shift the fraction by 1
- If denorm fraction shifts into bit 23, it becomes normalized
