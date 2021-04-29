/**
 * @author Nicholas Buckeridge
 * 
 * Coins have a value and a weight. The aim is to work out the maximim value
 * of coins I can have given a weight constraint.
 * 
 * Improvements:
 *    - There is a better way to deep copy of coin vectors apart from `copy_combination(Coins& coins)`.
 *    - I have assumed where deep copies should be. More thought needs to be put into where deep copies are.
 *    - The coin set is hard coded to be the australian specific coins in main and hash functions. This should be
 *          more vaiable dependent.
 *    - Hashing uses prime numbers, there is a better way to hash.
 *    - Multiple areas of this code require try/catch.
 *    - More caching can be added to speed up the process.
 * 
 * Overall comments:
 *    Cool problem. The caching is not very well implemented in this solution. Need more time for a better caching solution.
 * 
 * Coin weights are based on Australian coins.
 *   - ref: https://en.wikipedia.org/wiki/Coins_of_the_Australian_dollar
 */


#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <optional>
#include <utility>
#include <vector>


/* Prototypes. */
class Coin;
std::ostream& operator<< (std::ostream &out, Coin const& coin);
std::ostream& operator<< (std::ostream &out, std::vector<Coin> const& coins);
typedef std::vector<Coin> Coins;
Coins copy_combination(Coins& coins);
Coins best_value(std::vector<Coins> const& combinations);
std::optional<Coins> find_best_combination(Coin const& coin, int remaining_weight, Coins const& options);
unsigned long hash(Coin const& coin);
unsigned long hash(Coins const& coins);


class Coin {
public:
    int value;
    int weight;
    Coin(int value, int weight) : value(value), weight(weight) {}

    /* Static cast the value as an int. */
    operator int() const { return value; }
};


int main() {

    /*
     * Most important variable are below. Maximim weight I can carry and the
     * available coins I have to choose from.
     */
    int maximum_weight = 300; // 50 g
    std::vector<Coin> coin_options {
        Coin(1, 26), // 1c, 2.6g
        Coin(2, 52),
        Coin(5, 28),
        Coin(10, 56),
        Coin(20, 113),
        Coin(50, 155),
        Coin(100, 90),
        Coin(200, 66),
    };

    /*
     * Sort coins by value density. Value density is the value per weight unit.
     * Best bang for your buck (value for [smallest] weight) first.
     */
    std::sort(coin_options.begin(), coin_options.end(),
        [] (Coin const& c1, Coin const& c2) {
            float c1_value_density = float(c1.value) / float(c1.weight);
            float c2_value_density = float(c2.value) / float(c2.weight);
            return c1_value_density >= c2_value_density;
        }
    );

    std::cout << "Coins ordered by most valuable first:" << std::endl;
    std::cout << coin_options << std::endl;

    // Create a null coin.
    Coin root(0,0);

    // Perform DFS style recursion.
    std::optional<Coins> result;
    result = find_best_combination(root, maximum_weight, coin_options);
    if (result.has_value()) {
        std::cout << "Maximum possible result found!" << std::endl;
        std::cout << *result << std::endl;
    } else {
        std::cout << "No result found :(" << std::endl;
        return 1;
    }

    // Remove root coin.
    result->pop_back();

    // print the stats
    int total_value = 0;
    int total_weight = 0;
    for (auto const& coin : *result) {
        total_value += coin.value;
        total_weight += coin.weight;
    }

    std::cout << "Total value: " << total_value << std::endl;
    std::cout << "Total weight: " << total_weight << std::endl;


    return 0;
}

/**
 * @brief Recursive DFS to find be best value combination.
 * @param coin representing the current node.
 */
std::optional<Coins> find_best_combination(Coin const& coin, int remaining_weight, Coins const& options) {

    /* Construct a cache. */
    static std::map<unsigned long, std::optional<Coins>> cache;

    /* Capture the result for caching. */
    std::optional<Coins> result;

    /* Search the cache for a result */
    unsigned long h = hash(coin) + std::pow(remaining_weight, 23);
    auto cache_value = cache.find(h);
    if (cache_value != cache.end()) {
        result = cache_value->second;
        return result;
    }


    /* Base case 1: Coin is over the remaining weight. */
    if (coin.weight > remaining_weight) {
        /* Not an elegible coin. */
        result = std::nullopt;
        return result;
    }

    /* Base case 2: Coin matches the remaining weight perfectly. */
    if (coin.weight == remaining_weight) {
        /* Found a maximum combination of coins. */
        result = std::optional<Coins> {{coin}};
        // std::cout << "found" << std::endl;
        return result;
    }

    /*
     * Inductive case: There might be extra weight to add another coin.
     */
    std::vector<Coins> combinations;
    for (Coin const& next_coin : options) {
        auto combination = find_best_combination(next_coin, remaining_weight - coin.weight, options);
        if (!combination.has_value()) {
            continue;
        } else {
            combinations.push_back(copy_combination(*combination));
        }
    }

    // In the case no coins were found, return the node.
    if (combinations.size() == 0) {
        result = std::optional<Coins> {{coin}};
        return result;
    }

    Coins coins = best_value(combinations);
    coins.push_back(coin);
    result = std::optional<Coins> {coins};
    return result;
}

/**
 * @brief Return the combination with the best total value.
 */
Coins best_value(std::vector<Coins> const& combinations) {
    /* Calculate the total value of each combination. */
    std::vector<std::pair<int, Coins>> total_value_and_combination;
    for (auto const& combination : combinations) {
        int total_value = 0;
        for (auto const& coin: combination) {
            total_value += static_cast<int>(coin); // ;)
        }
        std::pair<int, Coins> v_c(total_value, combination);
        total_value_and_combination.push_back(v_c);
    }

    /* Return the combination with the highest total value. */
    std::sort(total_value_and_combination.begin(), total_value_and_combination.end(),
        [] (auto const& val_comb1, auto const& val_comb2) {
            auto& value1 = val_comb1.first;
            auto& value2 = val_comb2.first;
            return value1 >= value2;
        }
    );

    return copy_combination(total_value_and_combination.front().second);
}


/**
 * @brief Pretty print the coin object.
 */
std::ostream& operator<< (std::ostream &out, Coin const& coin) {
    out << "Coin(v=" << coin.value << ", w=" << coin.weight << ")";
    return out;
}


/**
 * @brief Pretty print a vector of coin objects.
 */
std::ostream& operator<< (std::ostream &out, std::vector<Coin> const& coins) {
    out << "[";
    for (auto const& coin : coins) {
        out << coin << ", ";
    }
    out << "]";
    return out;
}

Coins copy_combination(Coins& coins) {
    Coins copy;
    for (auto const& coin : coins) {
        copy.push_back(Coin(coin.value, coin.weight));
    }
    return copy;
}

unsigned long hash(Coin const& coin) {
    static const std::map<int, int> value_to_prime {
        {0, 0},
        {1, 2}, // 1c
        {2, 3},
        {5, 5},
        {10, 7},
        {20, 11},
        {50, 13},
        {100, 17},
        {200, 19},
    };
    return value_to_prime.at(coin.value);
}

unsigned long hash(Coins const& coins) {
    unsigned long h = 0;
    for (Coin const& coin : coins) {
        h += hash(coin);
    }
    return h;
}