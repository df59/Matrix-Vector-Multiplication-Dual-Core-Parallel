#include <array>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <mpi.h>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

struct Coordinate {
  std::size_t x;
  std::size_t y;
};

class Matrices {
public:
  std::size_t m_mn_matrix_size;
  std::vector<long> m_mn_matrix;
  std::vector<long> m_n1_matrix;
  size_t m_m, m_n;
  std::stringstream m_file_contents;

  explicit Matrices(std::size_t m, std::size_t n, std::size_t mn_size,
                    std::vector<long> mn_matrix, std::vector<long> n1_matrix)
      : m_m(m), m_n(n), m_mn_matrix_size(mn_size), m_mn_matrix(mn_matrix),
        m_n1_matrix(n1_matrix) {}

  [[nodiscard]] long &
  operator[](Coordinate &coordinate) { // returns 1D coordinate in the m x n
                                       // matrix from 2d coordinates
    return m_mn_matrix[(m_n * coordinate.y) + coordinate.x];
  }

  [[nodiscard]] long &at(Coordinate &coordinate) {
    return m_mn_matrix[(m_n * coordinate.y) + coordinate.x];
  }

  void multiply_matrices(int rank) {
    std::cout << "call to multiply matrices from rank " << rank << '\n';

    std::cout << "m_n is " << m_n << '\n' << "m_m is " << m_m << '\n';
    long next;

    if (rank == 0) {
      std::vector<long> output_matrix;
      MPI_Recv(&next, 1, MPI_LONG, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      output_matrix.push_back(next);
      std::cout << "initial from output matrix pushing back received "
                << output_matrix.back() << " from rank " << rank << '\n';
      for (std::size_t y = 1; y < m_m; y += 2) {

        long cur_product = 0;
        for (std::size_t x = 0; x < m_n; x++) {
          Coordinate cur_coordinate = {x, y};

          cur_product = cur_product + (at(cur_coordinate) * m_n1_matrix[x]);
        }
        output_matrix.push_back(cur_product);
        std::cout << "from output matrix pushing back " << output_matrix.back()
                  << " from rank " << rank << '\n';
        if (output_matrix.size() < m_m) {
          MPI_Recv(&next, 1, MPI_LONG, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          output_matrix.push_back(next);
          std::cout << "from output matrix pushing back received "
                    << output_matrix.back() << " from rank " << rank << '\n';
        }
      }
    }
    if (rank == 1) {
      //   std::vector<long> local_matrix;
      //   local_matrix.reserve(m_mn_matrix_size / 2);
      for (std::size_t y = 0; y < m_m; y += 2) {
        long cur_product = 0;
        for (std::size_t x = 0; x < m_n; x++) {
          Coordinate cur_coordinate = {x, y};

          cur_product = cur_product + (at(cur_coordinate) * m_n1_matrix[x]);
        }
        // local_matrix.push_back(cur_product);
        MPI_Send(&cur_product, 1, MPI_LONG, 0, 1, MPI_COMM_WORLD);
        std::cout << "from local matrix sending " << cur_product
                  << " from rank " << rank << '\n';
      }
    }
  }
};

[[nodiscard]] long getNumber(std::stringstream &content_of_file) {
  std::string current_number;
  bool negative = false;

  while (content_of_file.peek() == ' ' || content_of_file.peek() == '\n' ||
         content_of_file.peek() == '\r') {
    content_of_file.get();
  }
  if (content_of_file.peek() == '-') {
    negative = true;
  }
  while (content_of_file.peek() != ' ' && content_of_file.peek() != '\n' &&
         content_of_file.peek() != '\r' && content_of_file.peek() != -1) {
    char c_temp = content_of_file.get();
    std::size_t i_temp = c_temp - '0';
    if (i_temp <= 9 && i_temp >= 0) {
      current_number += c_temp;
    }
  }

  long final_u = std::stoi(current_number);
  if (negative) {
    return -final_u;
  } else {
    return final_u;
  }
}

[[nodiscard]] std::stringstream
getFile(std::string const input_file) { // takes in file as input
  auto i_filestream = std::ifstream{input_file.c_str()};

  if (!i_filestream.is_open()) {
    throw std::runtime_error(
        "File did not open. Make sure it is in the project directory \n");
  }
  auto ss = std::stringstream();
  i_filestream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  ss << i_filestream.rdbuf();

  return ss;
}

int main() {

  int my_rank;
  int size;
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  std::cout << "my rank is " << my_rank << " and my size is " << size << '\n';

  auto content_of_file = getFile("mv-data.txt");

  std::size_t m = getNumber(content_of_file);
  //   std::cout << "m: " << m << '\n';
  std::size_t n = getNumber(content_of_file);
  //   std::cout << "n: " << n << '\n';

  std::size_t mn_size = m * n;
  //   std::cout << "mn_size: " << mn_size << '\n';

  std::vector<long> mn_matrix;
  std::vector<long> n1_matrix;

  for (auto i = 0; i < mn_size; i++) {
    mn_matrix.push_back(getNumber(content_of_file));
  }

  for (auto i = 0; i < n; i++) {
    n1_matrix.push_back(getNumber(content_of_file));
  }

  Matrices matrices(m, n, mn_size, mn_matrix, n1_matrix);
  matrices.multiply_matrices(my_rank);

  MPI_Finalize();

  // Coordinate ex_coord = {2, 0};
  // std::cout << '\n' << matrices[ex_coord];

  // for(auto i = 0; i < mn_size; i++) {
  //     std::cout << mn_matrix[i];
  // }
  // std::cout << '\n';

  // for(auto i = 0; i < n; i++) {
  //     std::cout << n1_matrix[i] << '\n';
  // }

  // std::size_t m = getNumber(content_of_file);
  // std::cout << "m: " << m << '\n';

  // while (!content_of_file.eof()) {
  //     char c = content_of_file.get();
  //     // if (c == '\n' || c == '\r' || c == ' ') {
  //     // continue;
  //     // }
  //     int n = c - '0';
  //     // if (n == '\n' || n == '\r' || n == ' ') {

  //     // }
  //     std::cout << "c is " << c << '\n';
  //     std::cout << "n is " << n << '\n';
  //     if(n <= 9 && n >=0) {
  //     i_vec.push_back(n);
  //     }
  //     std::cout << "back of vector: " << i_vec.back() << '\n';
  // }
  // for (auto i = 0; i < i_vec.size(); i++) {
  //     std::cout << i_vec[i] << '\n';
  // }
}