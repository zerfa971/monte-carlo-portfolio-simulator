\# Monte Carlo Portfolio Simulator (C++)



A C++ Monte Carlo simulation engine for analyzing the risk and return characteristics of a multi-asset investment portfolio.  

The simulator models correlated stochastic returns, performs daily compounding, and computes key performance and risk metrics such as Expected Return, Volatility, Sharpe Ratio, Value at Risk (VaR), and Conditional Value at Risk (CVaR).



---



\## Overview



This project simulates thousands of possible future portfolio outcomes using Monte Carlo methods.  

It is designed to replicate how quantitative analysts and risk engineers evaluate portfolio uncertainty under stochastic market behavior.



Key objectives:

\- Estimate expected portfolio performance

\- Quantify downside risk using VaR and CVaR

\- Analyze the distribution of final portfolio values

\- Practice production-style C++ for quantitative finance applications



---



\## Features



\- Multi-asset portfolio with configurable weights

\- Daily compounding over a user-defined time horizon

\- Correlated asset returns using Gaussian random variables

\- Monte Carlo simulation across tens of thousands of scenarios

\- Risk and performance metrics:

&nbsp; - Expected return

&nbsp; - Volatility

&nbsp; - Sharpe ratio (0% risk-free rate)

&nbsp; - Value at Risk (VaR)

&nbsp; - Conditional Value at Risk (CVaR)

\- Percentile analysis of final portfolio value distribution



---



\## Methodology



\- Asset returns are modeled as stochastic daily returns derived from annualized expected return and volatility.

\- Correlation between assets is approximated via a shared stochastic factor.

\- Portfolio value evolves through daily compounding.

\- Risk metrics are computed from the empirical distribution of simulated outcomes.



---



\## Tech Stack



\- \*\*Language:\*\* C++17

\- \*\*Compiler:\*\* GCC (MinGW-w64)

\- \*\*Libraries:\*\* C++ Standard Library  

&nbsp; (`<vector>`, `<random>`, `<algorithm>`, `<numeric>`, `<cmath>`, `<iomanip>`)



---



\## How to Build and Run



\### Compile

```bash

g++ main.cpp -O2 -std=c++17 -o montecarlo



