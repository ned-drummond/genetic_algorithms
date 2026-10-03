#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <random>
#include <vector>

namespace {
  constexpr size_t GENERATION_SIZE = 1000;
  constexpr size_t ELITE_COUNT = 1;
  constexpr double TOURNAMENT_FRACTION = 0.1;

  double mutation_rate = 0.0;

  std::random_device rd;
  std::mt19937 gen(rd());

  // Set distribution to standard, printable char range
  std::uniform_int_distribution<int> chars(32, 126);

  // Generates true or false 50/50
  std::bernoulli_distribution half(0.5);

  // Generates true or false based on mutation_rate
  std::bernoulli_distribution mutation;
}

struct Individual {
  std::string genome;
  size_t fitness = 0;
};

bool target_found(const std::vector<Individual>& generation, const std::string& target) {
  return generation[0].fitness == target.length();
}

void mutate(std::string& individual) {
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


void calculate_fitness(Individual& individual, const std::string& target) {
  assert(individual.genome.length() == target.length());

  individual.fitness = 0;
  for (size_t i = 0; i < target.length(); ++i) {
    if (target[i] == individual.genome[i]) ++individual.fitness;
  }
}

void order_by_fitness(std::vector<Individual>& generation) {
  std::sort(generation.begin(), generation.end(),
    [](const Individual& a, const Individual& b) {
      return a.fitness > b.fitness;
  });
}

std::vector<Individual> generate_initial_generation(const std::string& target) {
  std::vector<Individual> generation;
  generation.reserve(GENERATION_SIZE);

  for (size_t i = 0; i < GENERATION_SIZE; ++i) {
    Individual individual;
    individual.genome.assign(target.length(), '\0');
    for (auto& c : individual.genome) c = static_cast<char>(chars(gen));
    calculate_fitness(individual, target);

    generation.push_back(std::move(individual));
  }

  return generation;
}

std::vector<Individual> run_tournament(const std::vector<Individual>& generation, const std::string& target) {
  std::vector<Individual> results;
  results.reserve(GENERATION_SIZE - ELITE_COUNT);
  const size_t count = generation.size() * TOURNAMENT_FRACTION;

  std::uniform_int_distribution<int> parent_indices(0, static_cast<int>(count) - 1);
  for (size_t i = 0; i < (GENERATION_SIZE - ELITE_COUNT); ++i) {
    const size_t parent_a_index = parent_indices(gen);
    const size_t parent_b_index = parent_indices(gen);

    Individual individual;
    individual.genome = reproduce(generation[parent_a_index].genome, generation[parent_b_index].genome);
    calculate_fitness(individual, target);

    results.push_back(std::move(individual));
  }

  return results;
}

void print_generation(const std::vector<Individual>& generation) {
  for (size_t i = 0; i < generation.size(); ++i) {
    std::cout << generation[i].genome << " fitness: " << generation[i].fitness << std::endl;
  }
}

void print_generation_fittest(const std::vector<Individual>& generation) {
  std::cout << generation[0].genome << " fitness: " << generation[0].fitness << std::endl;
}

void run_sim(const std::string& target) {
  std::cout << "Genetic Algo v2" << std::endl;
  std::cout << "Target: " << target << std::endl << std::endl;

  mutation_rate = 1.0 / static_cast<double>(target.length());
  mutation = std::bernoulli_distribution(mutation_rate);
  size_t generation_count = 1;
  auto current_generation = generate_initial_generation(target);

  while (true) {
    order_by_fitness(current_generation);

    std::cout << "Generation " << generation_count << ": ";
    print_generation_fittest(current_generation);

    if (target_found(current_generation, target)) {
      std::string output = "Target found after " + std::to_string(generation_count) + " generation";
      if (generation_count > 1) {
        output += "s";
      }
      output += "!";

      std::cout << output << std::endl;
      break;
    }

    std::vector<Individual> next_generation;
    next_generation.reserve(GENERATION_SIZE);
    for (size_t i = 0; i < ELITE_COUNT; ++i)
      next_generation.push_back(current_generation[i]);

    const auto results = run_tournament(current_generation, target);
    next_generation.insert(next_generation.end(), results.begin(), results.end());

    current_generation = std::move(next_generation);
    ++generation_count;
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: ./a.out \"<target string>\"\n";
    return 1;
  }

  run_sim(argv[1]);

  return 0;
}
