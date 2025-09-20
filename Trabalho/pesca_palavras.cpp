#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <algorithm>

// Estrutura para armazenar o resultado da busca de uma palavra
struct SearchResult {
    bool found;
    int row;
    int col;
    std::string direction_str;
    std::string word;
};

// Variáveis globais compartilhadas entre as threads
std::vector<std::string> grid;
std::vector<std::string> words_to_find;
std::vector<SearchResult> results;
std::mutex mtx; // Mutex para sincronização

// Direções de busca (Horizontal, Vertical, Diagonal e seus reversos)
int dr[] = {-1, -1, -1, 0, 0, 1, 1, 1};
int dc[] = {-1, 0, 1, -1, 1, -1, 0, 1};
std::string direction_names[] = {"Cima/Esquerda", "Cima", "Cima/Direita", "Esquerda", "Direita", "Baixo/Esquerda", "Baixo", "Baixo/Direita"};

/**
 * @brief Verifica se uma palavra está no diagrama a partir de uma posição e em uma direção.
 * @param row Linha inicial.
 * @param col Coluna inicial.
 * @param word A palavra a ser procurada.
 * @param delta_row Incremento da linha para a direção.
 * @param delta_col Incremento da coluna para a direção.
 * @return true se a palavra for encontrada, false caso contrário.
 */
bool search_in_direction(int row, int col, const std::string& word, int delta_row, int delta_col) {
    for (int i = 0; i < word.length(); ++i) {
        int r = row + i * delta_row;
        int c = col + i * delta_col;

        if (r < 0 || r >= grid.size() || c < 0 || c >= grid[0].size() || 
            std::tolower(grid[r][c]) != std::tolower(word[i])) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Função executada por cada thread para procurar um subconjunto de palavras.
 * @param thread_id O ID da thread.
 * @param num_threads O número total de threads.
 */
void find_words_thread(int thread_id, int num_threads) {
    for (int i = thread_id; i < words_to_find.size(); i += num_threads) {
        const std::string& word = words_to_find[i];
        bool found = false;
        SearchResult local_result = {false, -1, -1, "", word};

        for (int r = 0; r < grid.size() && !found; ++r) {
            for (int c = 0; c < grid[0].size() && !found; ++c) {
                if (std::tolower(grid[r][c]) == std::tolower(word[0])) {
                    for (int dir = 0; dir < 8; ++dir) {
                        if (search_in_direction(r, c, word, dr[dir], dc[dir])) {
                            // Bloqueia o mutex para escrita segura nos dados compartilhados
                            std::lock_guard<std::mutex> lock(mtx);

                            // Atualiza a grid com a palavra em maiúsculas
                            for (int k = 0; k < word.length(); ++k) {
                                grid[r + k * dr[dir]][c + k * dc[dir]] = std::toupper(grid[r + k * dr[dir]][c + k * dc[dir]]);
                            }

                            // Armazena o resultado
                            local_result = {true, r + 1, c + 1, direction_names[dir], word};
                            found = true;
                            break;
                        }
                    }
                }
            }
        }
        
        // Armazena o resultado final (dentro ou fora do mutex, mas sem race condition)
        {
            std::lock_guard<std::mutex> lock(mtx);
            results[i] = local_result;
        }
    }
}

/**
 * @brief Lê o arquivo de entrada e preenche as estruturas de dados.
 * @param filename O nome do arquivo de entrada.
 * @return true se a leitura for bem-sucedida, false caso contrário.
 */
bool read_input(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erro: Nao foi possivel abrir o arquivo de entrada: " << filename << std::endl;
        return false;
    }

    int rows, cols;
    file >> rows >> cols;

    grid.resize(rows);
    for (int i = 0; i < rows; ++i) {
        file >> grid[i];
    }

    // Agora lê as palavras para buscar
    // (ignorando qualquer linha extra que possa existir no arquivo)
    std::string word;
    while (file >> word) {
        // Filtra apenas palavras que são claramente para buscar
        // (não são linhas longas como as do grid)
        if (word.length() < 20) { // palavras de busca são curtas
            words_to_find.push_back(word);
        }
    }

    file.close();
    return true;
}

/**
 * @brief Escreve o resultado no arquivo de saída.
 * @param filename O nome do arquivo de saída.
 */
void write_output(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erro: Nao foi possivel abrir o arquivo de saida: " << filename << std::endl;
        return;
    }

    // Escreve a grid modificada
    for (const auto& row_str : grid) {
        file << row_str << std::endl;
    }

    // Escreve os resultados da busca
    for (const auto& result : results) {
        if (result.found) {
            file << result.word << " (" << result.row << "," << result.col << "): " << result.direction_str << std::endl;
        } else {
            file << result.word << ": nao encontrada" << std::endl;
        }
    }

    file.close();
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <arquivo_entrada> <arquivo_saida>" << std::endl;
        return 1;
    }

    std::string input_filename = argv[1];
    std::string output_filename = argv[2];

    if (!read_input(input_filename)) {
        return 1;
    }

    results.resize(words_to_find.size());
    // Define o número de threads (pode ser ajustado)
    const int num_threads = std::thread::hardware_concurrency(); // Usa o número de núcleos do processador
    std::vector<std::thread> threads;

    // Inicia as threads
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(find_words_thread, i, num_threads);
    }

    // Aguarda todas as threads terminarem
    for (auto& t : threads) {
        t.join();
    }

    write_output(output_filename);

    std::cout << "Busca concluida. Resultados salvos em " << output_filename << std::endl;

    return 0;
}