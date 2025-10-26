# Control-flow-flattening-pass
Control flow flattening is a transformation on a function that routes the natural control flow of a function via a central dispatcher made up of a while loop and switch statement.
This project implements a simple control flow flattening in an LLVM pass.

## Before and after example

### Before
<img width="763" height="573" alt="Screenshot 2025-10-26 231126" src="https://github.com/user-attachments/assets/3435fdf5-862c-48c6-ae45-4ecb0e6552f0" />

### After
<img width="743" height="439" alt="Screenshot 2025-10-26 231029" src="https://github.com/user-attachments/assets/46e6d6ad-0624-4b80-ad86-9be52aba9fea" />






