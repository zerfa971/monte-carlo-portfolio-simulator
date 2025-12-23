#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iomanip>
#include <string>

constexpr double PI = 3.14159265358979323846;

struct Asset {
    std::string name;
    double weight;
    double expectedReturn;  // Annual
    double volatility;      // Annual
};

struct SimulationResults {
    double expectedReturn;
    double volatility;
    double var;
    double cvar;
    std::vector<double> finalValues;
    std::vector<double> percentiles; // 5th, 25th, 50th, 75th, 95th
};

class MonteCarloSimulator {
private:
    std::vector<Asset> assets;
    double initialInvestment;
    int timeHorizon;  // in days
    int numScenarios;
    double confidenceLevel;
    std::mt19937 rng;
    
    // Box-Muller transform for normal distribution
    double generateNormal() {
        std::uniform_real_distribution<double> uniform(0.0, 1.0);
        double u1 = uniform(rng);
        double u2 = uniform(rng);
        return std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * PI * u2);
    }
    
    // Generate correlated returns for all assets for one time step
    std::vector<double> generateCorrelatedReturns(double correlation = 0.3) {
        std::vector<double> z(assets.size());
        std::vector<double> returns(assets.size());
        
        // Generate independent normal random variables
        for (size_t i = 0; i < assets.size(); ++i) {
            z[i] = generateNormal();
        }
        
        // Apply correlation structure
        for (size_t i = 0; i < assets.size(); ++i) {
            double correlatedZ = z[i];
            if (i > 0) {
                // Simple correlation with first asset
                correlatedZ = correlation * z[0] + 
                             std::sqrt(1 - correlation * correlation) * z[i];
            }
            
            // Convert to asset return
            double drift = assets[i].expectedReturn / 252.0;
            double vol = assets[i].volatility / std::sqrt(252.0);
            returns[i] = drift + vol * correlatedZ;
        }
        
        return returns;
    }
    
    // Run a single scenario
    double runSingleScenario() {
        double portfolioValue = initialInvestment;
        
        for (int t = 0; t < timeHorizon; ++t) {
            std::vector<double> assetReturns = generateCorrelatedReturns();
            
            // Calculate portfolio return
            double portfolioReturn = 0.0;
            for (size_t a = 0; a < assets.size(); ++a) {
                portfolioReturn += assets[a].weight * assetReturns[a];
            }
            
            portfolioValue *= (1.0 + portfolioReturn);
        }
        
        return portfolioValue;
    }
    
public:
    MonteCarloSimulator(const std::vector<Asset>& assets_, 
                        double initialInv, 
                        int horizon, 
                        int scenarios,
                        double confLevel = 95.0)
        : assets(assets_), 
          initialInvestment(initialInv),
          timeHorizon(horizon),
          numScenarios(scenarios),
          confidenceLevel(confLevel),
          rng(std::random_device{}()) {}
    
    SimulationResults simulate() {
        std::cout << "Running " << numScenarios << " scenarios..." << std::endl;
        
        std::vector<double> finalValues(numScenarios);
        
        // Run all scenarios
        for (int s = 0; s < numScenarios; ++s) {
            finalValues[s] = runSingleScenario();
            
            // Progress indicator
            if ((s + 1) % 10000 == 0) {
                std::cout << "Completed " << (s + 1) << " scenarios" << std::endl;
            }
        }
        
        // Sort results for percentile calculations
        std::vector<double> sortedValues = finalValues;
        std::sort(sortedValues.begin(), sortedValues.end());
        
        // Calculate returns
        std::vector<double> returns(numScenarios);
        for (int i = 0; i < numScenarios; ++i) {
            returns[i] = (finalValues[i] - initialInvestment) / initialInvestment;
        }
        
        // Expected return
        double expectedReturn = std::accumulate(returns.begin(), returns.end(), 0.0) / numScenarios;
        
        // Volatility (standard deviation of returns)
        double variance = 0.0;
        for (double r : returns) {
            variance += std::pow(r - expectedReturn, 2);
        }
        variance /= numScenarios;
        double volatility = std::sqrt(variance);
        
        // VaR calculation
        int varIndex = static_cast<int>(numScenarios * (1.0 - confidenceLevel / 100.0));
        double var = initialInvestment - sortedValues[varIndex];
        
        // CVaR calculation (average of losses worse than VaR)
        double cvarSum = 0.0;
        for (int i = 0; i < varIndex; ++i) {
            cvarSum += sortedValues[i];
        }
        double cvar = initialInvestment - (cvarSum / varIndex);
        
        // Percentiles
        std::vector<double> percentiles = {
            sortedValues[static_cast<int>(numScenarios * 0.05)],
            sortedValues[static_cast<int>(numScenarios * 0.25)],
            sortedValues[static_cast<int>(numScenarios * 0.50)],
            sortedValues[static_cast<int>(numScenarios * 0.75)],
            sortedValues[static_cast<int>(numScenarios * 0.95)]
        };
        
        return {expectedReturn, volatility, var, cvar, sortedValues, percentiles};
    }
    
    void printResults(const SimulationResults& results) {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "MONTE CARLO SIMULATION RESULTS" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
        std::cout << std::fixed << std::setprecision(2);
        
        std::cout << "\nPortfolio Composition:" << std::endl;
        for (const auto& asset : assets) {
            std::cout << "  " << asset.name << ": " 
                     << (asset.weight * 100) << "% "
                     << "(Return: " << (asset.expectedReturn * 100) << "%, "
                     << "Vol: " << (asset.volatility * 100) << "%)" << std::endl;
        }
        
        std::cout << "\nSimulation Parameters:" << std::endl;
        std::cout << "  Initial Investment: $" << initialInvestment << std::endl;
        std::cout << "  Time Horizon: " << timeHorizon << " days" << std::endl;
        std::cout << "  Number of Scenarios: " << numScenarios << std::endl;
        std::cout << "  Confidence Level: " << confidenceLevel << "%" << std::endl;
        
        std::cout << "\nKey Metrics:" << std::endl;
        std::cout << "  Expected Return: " << (results.expectedReturn * 100) << "%" << std::endl;
        std::cout << "  Volatility: " << (results.volatility * 100) << "%" << std::endl;
        std::cout << "  Sharpe Ratio: " << (results.expectedReturn / results.volatility) << std::endl;
        
        std::cout << "\nRisk Metrics:" << std::endl;
        std::cout << "  VaR (" << confidenceLevel << "%): $" << results.var 
                 << " (" << (results.var / initialInvestment * 100) << "% of portfolio)" << std::endl;
        std::cout << "  CVaR (" << confidenceLevel << "%): $" << results.cvar 
                 << " (" << (results.cvar / initialInvestment * 100) << "% of portfolio)" << std::endl;
        
        std::cout << "\nPercentiles of Final Portfolio Value:" << std::endl;
        std::cout << "  5th:  $" << results.percentiles[0] << std::endl;
        std::cout << "  25th: $" << results.percentiles[1] << std::endl;
        std::cout << "  50th: $" << results.percentiles[2] << " (Median)" << std::endl;
        std::cout << "  75th: $" << results.percentiles[3] << std::endl;
        std::cout << "  95th: $" << results.percentiles[4] << std::endl;
        
        std::cout << "\n" << std::string(60, '=') << std::endl;
    }
};

int main() {
    // Define portfolio assets
    std::vector<Asset> portfolio = {
        {"US Stocks", 0.40, 0.10, 0.18},
        {"International Stocks", 0.30, 0.08, 0.20},
        {"Bonds", 0.20, 0.04, 0.06},
        {"REITs", 0.10, 0.09, 0.15}
    };
    
    // Simulation parameters
    double initialInvestment = 100000.0;
    int timeHorizon = 252;  // 1 year of trading days
    int numScenarios = 50000;
    double confidenceLevel = 95.0;
    
    // Create simulator and run
    MonteCarloSimulator simulator(portfolio, initialInvestment, 
                                  timeHorizon, numScenarios, confidenceLevel);
    
    auto results = simulator.simulate();
    simulator.printResults(results);
    
    return 0;
}