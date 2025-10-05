# Overcurrent (OC) Protection and System Evaluation

## OC Threshold

Noise on the current measurement makes accurate threshold determination impossible.  
However, the threshold is **certainly above 12.5 A**.

---

## OC Propagation Delay (OC PC Delay)

### Required Maximum Delay @ 50 VDC

- \( I_{trip} = 13 A \)
- \( I_{max} = 260 A @ 300 µs \)

\[
\frac{dI}{dt} = \frac{V_{DC}}{L} = 22 A/µs
\]

\[
t_d = \frac{I_{max} - I_{trip}}{dI/dt} = \frac{240 A}{22 A/µs} = 10 µs
\]

> **Note:**  
> This calculation assumes constant inductance (**L**).  
> However, **L decreases with saturation (~7 A)**, making the OCP potentially **useless or too slow**.

> **Reference:**  
> Short Circuit Withstand Times for various EPC devices were evaluated in the referenced paper.  
> All measured withstand times were **> 5 µs**.

---

## Theoretical Delay Breakdown

| Component | Bandwidth / Spec | Delay (td) |
|------------|------------------|------------|
| Current Sensor | 1 MHz | 0.8 µs (5 τ) |
| Differential Amplifier | GBW = 1 MHz | 0.8 µs |
| Non-Inverting Amplifier | BW = 10 MHz | 0.08 µs |
| Comparator | With deglitch capacitor | 1 µs |
| D-FF | CLK → Q | 7 ns |
| 2× OR-Gate | — | 12 ns |
| Programmable Gate | — | 6 ns |
| Gate Driver | — | 45 ns |

**→ Total theoretical delay:**

\[
t_{d,tot} ≈ 1.75 µs
\]

---

## Measurement Procedure

### Without Exact Threshold Measurement
Measure **propagation delay** from **after the comparator** to **gate-low**,  
then **add theoretical delay**.

### With Exact Threshold Measurement
Measure from **current step** to **gate-low**.

---

## Measurement Results

- **Measured propagation delay (after comparator → gate-off):** 70 ns  
- **Expected total delay:** ≈ 2 µs

---

## E-Stop and Disable Delay

*(No measurement data provided.)*

---

## Precharge Tracking

### Setup

- Input: **30 V** DC power supply  
- Output: **6 F capacitor bank** with **15 Ω resistor** in parallel  
- Load: nearly constant (essentially a short circuit)  
- Reference step: **3 A**  
- Controller gains:  
  - \( K_P = 0.01 \)  
  - \( K_I = 1 \)

### Observations

- Controller **tracks the reference** even with weak gains.  
- Increasing gains could **damage the device**, since OCP functionality is **not yet verified**.  
- **Sawtooth error** may result from **aliasing of ripple current**, which can be reduced by using a **larger inductor**.

### Notes

- An **offset of 0.35 A** was applied empirically.  
- Likely due to **zero-current reference voltage error**, causing the **difference amplifier to clamp to zero**.

---

## Continuous Precharge

### Setup

Same setup as used for **Precharge Tracking**.

### Results

- After a few seconds, the **synchronous rectifier** of the buck converter **failed permanently**.  
- Failure results in a **short circuit between the switching node and GND**.

### Possible Cause

- **Inductance decrease** due to:  
  - **Thermal effects**, and/or  
  - **Current saturation**

### Solutions

- Use a **larger inductor** to mitigate both thermal and saturation issues.

---

## Thermal, Accuracy, and Ripple

*(Section not yet detailed.)*

---

## Continuous Bypass Thermal

*(Section not yet detailed.)*

---

## Possibly Destructive Tests

- Startup sequence (**PWR before CTRL**)  
- Isolation  
- Maximum input voltage
