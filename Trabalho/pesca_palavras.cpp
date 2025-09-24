#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <algorithm>
using namespace std;

// Estrutura para armazenar o resultado da busca de uma palavra
struct ResultadoBusca {
    bool encontrada;
    int linha;
    int coluna;
    string direcao_str;
    string palavra;
};

// Variáveis globais compartilhadas entre as threads
vector<string> diagrama;
vector<string> palavras_busca;
vector<ResultadoBusca> resultados;
mutex mtx; // Mutex para sincronização

// Direções de busca (Horizontal, Vertical, Diagonal e seus reversos)
int delta_linha[] = {-1, -1, -1, 0, 0, 1, 1, 1};
int delta_coluna[] = {-1, 0, 1, -1, 1, -1, 0, 1};
string nomes_direcao[] = {"Cima/Esquerda", "Cima", "Cima/Direita", "Esquerda", "Direita", "Baixo/Esquerda", "Baixo", "Baixo/Direita"};

/**
 * @brief Verifica se uma palavra está no diagrama a partir de uma posição e em uma direção.
 * @param linha Linha inicial.
 * @param coluna Coluna inicial.
 * @param palavra A palavra a ser procurada.
 * @param d_linha Incremento da linha para a direção.
 * @param d_coluna Incremento da coluna para a direção.
 * @return true se a palavra for encontrada, false caso contrário.
 */
bool busca_na_direcao(int linha, int coluna, const string& palavra, int d_linha, int d_coluna) {
    for (int i = 0; i < palavra.length(); ++i) {
        int l = linha + i * d_linha;
        int c = coluna + i * d_coluna;

        if (l < 0 || l >= diagrama.size() || c < 0 || c >= diagrama[0].size() ||
            tolower(diagrama[l][c]) != tolower(palavra[i])) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Função executada por cada thread para procurar um subconjunto de palavras.
 * @param id_thread O ID da thread.
 * @param num_threads O número total de threads.
 */
void busca_palavras_thread(int id_thread, int num_threads) {
    for (int i = id_thread; i < palavras_busca.size(); i += num_threads) {
        const string& palavra = palavras_busca[i];
        bool encontrada = false;
        ResultadoBusca resultado_local = {false, -1, -1, "", palavra};

        for (int l = 0; l < diagrama.size() && !encontrada; ++l) {
            for (int c = 0; c < diagrama[0].size() && !encontrada; ++c) {
                if (tolower(diagrama[l][c]) == tolower(palavra[0])) {
                    for (int dir = 0; dir < 8; ++dir) {
                        if (busca_na_direcao(l, c, palavra, delta_linha[dir], delta_coluna[dir])) {
                            // Bloqueia o mutex para escrita segura nos dados compartilhados
                            lock_guard<mutex> lock(mtx);

                            // Atualiza o diagrama com a palavra em maiúsculas
                            for (int k = 0; k < palavra.length(); ++k) {
                                diagrama[l + k * delta_linha[dir]][c + k * delta_coluna[dir]] =
                                    toupper(diagrama[l + k * delta_linha[dir]][c + k * delta_coluna[dir]]);
                            }

                            // Armazena o resultado
                            resultado_local = {true, l + 1, c + 1, nomes_direcao[dir], palavra};
                            encontrada = true;
                            break;
                        }
                    }
                }
            }
        }

        // Armazena o resultado final (dentro ou fora do mutex, mas sem race condition)
        {
            lock_guard<mutex> lock(mtx);
            resultados[i] = resultado_local;
        }
    }
}

/**
 * @brief Lê o arquivo de entrada e preenche as estruturas de dados.
 * @param nome_arquivo O nome do arquivo de entrada.
 * @return true se a leitura for bem-sucedida, false caso contrário.
 */
bool ler_entrada(const string& nome_arquivo) {
    ifstream arquivo(nome_arquivo);
    if (!arquivo.is_open()) {
        cerr << "Erro: Nao foi possivel abrir o arquivo de entrada: " << nome_arquivo << endl;
        return false;
    }

    int linhas, colunas;
    arquivo >> linhas >> colunas;

    diagrama.resize(linhas);
    for (int i = 0; i < linhas; ++i) {
        arquivo >> diagrama[i];
    }

    // Agora lê as palavras para buscar
    string palavra;
    while (arquivo >> palavra) {
        if (palavra.length() < 20) { // palavras de busca são curtas
            palavras_busca.push_back(palavra);
        }
    }

    arquivo.close();
    return true;
}

/**
 * @brief Escreve o resultado no arquivo de saída.
 * @param nome_arquivo O nome do arquivo de saída.
 */
void escrever_saida(const string& nome_arquivo) {
    ofstream arquivo(nome_arquivo);
    if (!arquivo.is_open()) {
        cerr << "Erro: Nao foi possivel abrir o arquivo de saida: " << nome_arquivo << endl;
        return;
    }

    // Escreve o diagrama modificado
    for (const auto& linha_str : diagrama) {
        arquivo << linha_str << endl;
    }

    // Escreve os resultados da busca
    for (const auto& resultado : resultados) {
        if (resultado.encontrada) {
            arquivo << resultado.palavra << " (" << resultado.linha << "," << resultado.coluna << "): " << resultado.direcao_str << endl;
        } else {
            arquivo << resultado.palavra << ": nao encontrada" << endl;
        }
    }

    arquivo.close();
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Uso: " << argv[0] << " <arquivo_entrada> <arquivo_saida>" << endl;
        return 1;
    }

    string nome_entrada = argv[1];
    string nome_saida = argv[2];

    if (!ler_entrada(nome_entrada)) {
        return 1;
    }

    resultados.resize(palavras_busca.size());
    // Define o número de threads (pode ser ajustado)
    const int num_threads = thread::hardware_concurrency();
    vector<thread> threads;

    // Inicia as threads
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(busca_palavras_thread, i, num_threads);
    }

    // Aguarda todas as threads terminarem
    for (auto& t : threads) {
        t.join();
    }

    escrever_saida(nome_saida);

    cout << "Busca concluida. Resultados salvos em " << nome_saida << endl;

    return 0;
}

//Link do relatório: https://docs.google.com/document/d/1uHwFMP2sNeRzfMYBq8FTCtyB05cJNZGj/edit?usp=sharing&ouid=105217494896110442543&rtpof=true&sd=true