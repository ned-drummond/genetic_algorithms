#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <random>
#include <vector>

namespace {
  constexpr size_t GENERATION_SIZE = 1000;
  double MUTATION_RATE = 0.0;

  std::random_device rd;
  std::mt19937 gen(rd());

  // Set distribution to standard, printable char range
  std::uniform_int_distribution<int> chars(32, 126);

  // Generates true or false 50/50
  std::bernoulli_distribution half(0.5);

  // Generates true or false based on MUTATION_RATE
  // std::bernoulli_distribution mutation(MUTATION_RATE);
}

bool target_found(const std::vector<std::string>& generation, const std::string& target) {
  // for (size_t i = 0; i < generation.size(); ++i) {
    // if (generation[i] == target) return true;
  // }

  // return false;
  return generation[0] == target;
}

void mutate(std::string& individual) {
  static std::bernoulli_distribution mutation(MUTATION_RATE);
  for (auto& c : individual) {
    if (mutation(gen)) {
      c = static_cast<char>(chars(gen));
    }
  }
}

std::string reproduce(const std::string& parent_a, const std::string& parent_b) {
  assert(parent_a.length() == parent_b.length());

  std::string offspring;
  offspring.reserve(parent_a.size());
  for (size_t i = 0; i < parent_a.size(); ++i) {
    const auto c = half(gen) ? parent_a[i] : parent_b[i];
    offspring.push_back(c);
  }

  mutate(offspring);
  return offspring;
}


size_t calculate_fitness(const std::string& individual, const std::string& target) {
  assert(individual.length() == target.length());

  size_t fitness = 0;
  for (size_t i = 0; i < target.length(); ++i) {
    if (target[i] == individual[i]) ++fitness;
  }

  return fitness;
}

void order_by_fitness(std::vector<std::string>& generation, const std::string& target) {
  std::sort(generation.begin(), generation.end(),
    [&target](const std::string& a, const std::string& b) {
      return calculate_fitness(a, target) > calculate_fitness(b, target);
  });
}

std::vector<std::string> generate_initial_generation(size_t str_len) {
  std::vector<std::string> generation;
  generation.reserve(GENERATION_SIZE);

  for (size_t i = 0; i < GENERATION_SIZE; ++i) {
    std::string offspring(str_len, '\0');
    for (auto& c : offspring) c = static_cast<char>(chars(gen));

    generation.push_back(std::move(offspring));
  }

  return generation;
}

void print_generation(const std::vector<std::string>& generation, const std::string& target) {
  for (size_t i = 0; i < generation.size(); ++i) {
    std::cout << generation[i] << " fitness: " << calculate_fitness(generation[i], target) << std::endl;
  }
}

void print_generation_fittest(const std::vector<std::string>& generation, const std::string& target) {
  std::cout << generation[0] << " fitness: " << calculate_fitness(generation[0], target) << std::endl;
}

void run_sim(const std::string& target) {
  std::cout << "Genetic Algo v1" << std::endl;
  std::cout << "Target: " << target << std::endl << std::endl;

  MUTATION_RATE = 1.0 / target.length();
  size_t generation_count = 1;
  auto current_generation = generate_initial_generation(target.length());

  while (true) {
    order_by_fitness(current_generation, target);

    std::cout << "Generation " << generation_count << ": ";
    print_generation_fittest(current_generation, target);

    if (target_found(current_generation, target)) {
      std::string output = "Target found after " + std::to_string(generation_count) + " generation";
      if (generation_count > 1) {
        output += "s";
      }
      output += "!";

      std::cout << output << std::endl;
      break;
    }

    std::vector<std::string> next_generation;
    next_generation.reserve(GENERATION_SIZE);

    for (size_t i = 0; i < GENERATION_SIZE; ++i) {
      next_generation.push_back(reproduce(current_generation[0], current_generation[1]));
    }

    current_generation = std::move(next_generation);
    ++generation_count;
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << R"(Takes 1 arg: ./a.out "Hello, World!")" << std::endl;
    return 1;
  }

  run_sim(argv[1]);

  return 0;
}
